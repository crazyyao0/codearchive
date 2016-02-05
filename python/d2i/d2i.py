class BitStream():
    def __init__(self, bstr):
        self._str = ""
        self._pos = 0
        for c in bstr:
            self._str += '{:08b}'.format(ord(c))[::-1]
    
    def readbit(self):
        ret = self._str[self._pos]
        self._pos += 1
        return ret
    
    def readbits(self, length):
        ret = self._str[self._pos:self._pos + length]
        self._pos += length
        return ret[::-1]
        
    def readuint(self, length):
        s=self.readbits(length)
        return int(s, 2)

    def readasc8(self, length):
        ret = ""
        for _i in range(length):
            ret += chr(self.readuint(8))
        return ret.strip()
    
    def readasc7(self, length):
        ret = ""
        for _i in range(length):
            ret += chr(self.readuint(7))
        return ret.strip()
    
    def read(self, fmt):
        t = fmt[0]
        if t == 'u':
            l = int(fmt[1:])
            return self.readuint(l)
        elif t == 's':
            l = int(fmt[1:])
            return self.readasc8(l)
        elif t == 'S':
            l = int(fmt[1:])
            return self.readasc7(l)
        elif t == 'b':
            return self.readbit() == '1'
        else:
            return "(Error)"
        
    def dump(self):
        a = self._pos // 8 * 8
        b = self._pos % 8
        s = "%04X  "%(self._pos // 8) + self._str[a : a + 8]
        if len(self._str) > a + 8:
            s += " " + self._str[a+8 : a+16]
        if len(self._str) > a + 16:
            s += " " + self._str[a+16 : a+24]
        if len(self._str) > a + 24:
            s += " " + self._str[a+24 : a+32]
        if len(self._str) > a + 32:
            s += " " + self._str[a+32 : a+40]
        if len(self._str) > a + 40:
            s += " " + self._str[a+40 : a+48]
        print s
        print " "*(6 + b) + "^"
        
class DataTable():
    DataFolder = "C:\\Users\\wyao\\Desktop\\diablo2\\config"
    def __init__(self, tablename):
        fd = open(DataTable.DataFolder + "\\" + tablename)
        self._columes = fd.readline().strip('\n').split('\t')
        self._cols = len(self._columes)
        self._data = []
        for line in fd.readlines():
            self._data.append(line.strip('\n').split('\t'))
        self._rows = len(self._data)
    
    def __iter__(self):
        self.current = 0
        return self

    def next(self):
        if self.current >= self._rows:
            raise StopIteration
        o = self.getRow(self.current)
        self.current += 1
        return o
        
    def __len__(self):
        return self._rows
    
    def getRow(self, idx):
        if idx >= self._rows:
            return None
        ret = {}
        for i in range(len(self._columes)):
            ret[self._columes[i]] = self._data[idx][i]
        return ret
    
    def findRow(self, field, value):
        if not field in self._columes:
            return None
        
        ci = self._columes.index(field)
        strvalue = str(value)
        for row in self._data:
            if row[ci] == strvalue:
                ret = {}
                for i in range(len(self._columes)):
                    ret[self._columes[i]] = row[i]
                return ret
        return None

import collections
import json

armor_t = DataTable("Armor.txt")
weapon_t = DataTable("Weapons.txt")
misc_t = DataTable("Misc.txt")
properties_t = DataTable("properties.txt")
fields_t = DataTable("fields.txt")
prefix_t = DataTable("MagicPrefix.txt")
suffix_t = DataTable("MagicSuffix.txt")


def process_properties(bs):
    pl = []
    while True:
        id = bs.readuint(9)        
        if id == 0x1ff:
            break
        prop = properties_t.getRow(id)        
        obj = {}
        obj["code"] = prop["code"]
        obj["name"] = prop["name"]
        obj["description"] = prop["attribute description"]
        for i in range(1, int(prop["numparam"])+1):
            paramtype = prop["paramtype%d"%(i)]
            if paramtype == "":
                break
            
            base = int(prop["base%d"%(i)])
            length = int(prop["length%d"%(i)])
            value = bs.readuint(length) + base
            obj["value%d"%(i)] = "%+d"%(value)
        pl.append(obj)
        
    return pl
        
def process_fields(obj, bs, ft):
    for e in ft:
        type = e["type"]
        if type == "":
            continue
        bits = int(e["bits"])
        name = e["name"]
        
        cond = e["cond1"]
        condvar = e["condvar1"]
        condval = e["condval1"]
        if cond == "0":
            if str(obj[condvar]) != condval:
                continue
        elif cond == "1":
            if str(obj[condvar]) == condval:
                continue

        cond = e["cond2"]
        condvar = e["condvar2"]
        condval = e["condval2"]
        if cond == "0":
            if str(obj[condvar]) != condval:
                continue
        elif cond == "1":
            if str(obj[condvar]) == condval:
                continue
                    
        if type in ("ASC8", "ASCI"):
            obj[name] = bs.readasc8(bits)
        if type == "ASC7":
            obj[name] = bs.readasc7(bits)
        elif type in ("BOOL", "BYTE", "WORD", "DWRD"):
            obj[name] = bs.readuint(bits)
        elif type == "PROP":
            #pass
            obj[name] = process_properties(bs)
        elif type == "FILE":
            ft2 = DataTable(name)
            process_fields(obj, bs, ft2)
                
        if name == "dwType":
            dwType = obj[name]
            cls = armor_t.findRow("code", dwType)
            if cls == None:
                cls = weapon_t.findRow("code", dwType)
            else:
                obj["iType"] = 0
            if cls == None:
                cls = misc_t.findRow("code", dwType)
            else:
                obj["iType"] = 1
            if cls == None:
                raise Exception("code %s not found"%(dwType))
            else:
                obj["iType"] = 2
            
            obj["sType"] = cls["type"]
            obj["bNoDur"] = int(cls["nodurability"])
            obj["bStack"] = int(cls["stackable"])
            obj["iMaxDur"] = 0
            obj["bSet1"] = 0
            obj["bSet2"] = 0
            obj["bSet3"] = 0
            obj["bSet4"] = 0
            obj["bSet5"] = 0
            
            obj["typename"] = cls["name"]
            obj["typelevelreq"] = cls["levelreq"]

def print_propdesc(propdesc, nameprefix):
    txt = nameprefix + "(" + propdesc["Name"] + "):"
    if propdesc["mod1code"] != "":
        txt += " " + propdesc["mod1code"] + "(" + propdesc["mod1param"] + \
            "," + propdesc["mod1min"] + "-" + propdesc["mod1max"] + ")" 
    if propdesc["mod2code"] != "":
        txt += " " + propdesc["mod2code"] + "(" + propdesc["mod2param"] + \
            "," + propdesc["mod2min"] + "-" + propdesc["mod2max"] + ")" 
    if propdesc["mod3code"] != "":
        txt += " " + propdesc["mod2code"] + "(" + propdesc["mod2param"] + \
            "," + propdesc["mod2min"] + "-" + propdesc["mod2max"] + ")" 
    print txt

def print_item(obj):
    txt = ""
    if "wRname1" in obj:
        txt += prefix_t.getRow(obj["wRname1"])["Name"] + " "
    if "wRname2" in obj:
        txt += suffix_t.getRow(obj["wRname2"])["Name"] + " "
    txt += obj["typename"]
    print txt
    
    print "Version: %d"%(obj["unk60"])
    print "GUID: 0x%08X"%(obj["dwGUID"])
    print "Item Level: %d"%(obj["iLevel"])
    
    requirelvl = int(obj["typelevelreq"])
    for name, dt, nameprefix in [("wPref1", prefix_t, "prefix1"), ("wSuff1", suffix_t, "suffix1"), 
                                 ("wPref2", prefix_t, "prefix2"), ("wSuff2", suffix_t, "suffix2"), 
                                 ("wPref3", prefix_t, "prefix3"), ("wSuff3", suffix_t, "suffix3")]:
        if not name in obj:
            continue
        propdesc = dt.getRow(obj[name])
        lvl = int(propdesc["levelreq"])
        if lvl > requirelvl:
            requirelvl = lvl
        print_propdesc(propdesc, "  " + nameprefix)
    print "Required Level: %d"%(requirelvl)
    
        
    

buffer = open("C:\\Users\\wyao\\Desktop\\diablo2\\a.d2i", "rb").read()
bs = BitStream(buffer)
obj = collections.OrderedDict()
process_fields(obj, bs, fields_t)
print json.dumps(obj, indent=2)
print_item(obj)
