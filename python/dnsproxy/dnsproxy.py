import socket
import struct
from datetime import datetime
import sys
import logging
logging.basicConfig(stream=sys.stdout, level=logging.INFO, format="%(asctime)s [%(name)s] %(levelname)s: %(message)s")

def hexdump(data):
    pos = 0    
    while pos<len(data):
        linebuf = "%08X "%(pos)
        for i in range(16):
            if i+pos < len(data):
                linebuf += " %02X"%(ord(data[pos+i]))
            else:
                linebuf += "   "
        
        linebuf += " "
        for i in range(16):
            if i+pos < len(data): 
                if ord(data[pos+i])>=32 and ord(data[pos+i]) < 127: 
                    linebuf += data[pos+i]
                else:
                    linebuf += "."
        print linebuf
        pos += 16

_dns_type1 = [
"UNKNOWN","A","NS","MD","MF","CNAME","SOA","MB","MG","MR",
"NULL","WKS","PTR","HINFO","MINFO","MX","TXT","RP","AFSDB","X25",
"ISDN","RT","NSAP","NSAPPTR","SIG","KEY","PX","GPOS","AAAA","LOC",
"NXT","EID","NBSTAT","SRV","ATMA","NAPTR","KX","CERT","A6","DNAME",
"SINK","OPT","APL","DS","SSHFP","IPSECKEY","RRSIG","NSEC","DNSKEY","DHCID",
"NSEC3","NSEC3PARAM","TLSA","SMIMEA","UNKNOWN","HIP","NINFO","RKEY","TALINK","CDS",
"CDNSKEY","OPENPGPKEY","CSYNC"] #0-62
_dns_type2 = ["SPF","UINFO","UID","GID","UNSPEC","NID","L32","L64","LP","EUI48","EUI64"] #99-109
_dns_type3 = ["TKEY","TSIG","IXFR","AXFR","MAILB","MAILA","ANY","URI","CAA","AVC"] #249-258
_dns_type = _dns_type1 + ["UNKNOWN"] * 36 + _dns_type2 + ["UNKNOWN"] * 139 + _dns_type3
_dns_opcode = ["Query ","IQuery","Status","Notify","Update"] #0-5
_dns_class = ["UNKNOWN","IN","CS","CH","HS"] #0-5
_dns_rcode = ["NoError","FormErr","ServFail","NXDomain","NotImp","Refused","YXDomain","YXRRSet","NXRRSet","NotAuth",
"NotZone","Unknown","Unknown","Unknown","Unknown","Unknown","BADVERS","BADSIG","BADKEY","BADTIME",
"BADMODE","BADNAME","BADALG","BADTRUNC","BADCOOKIE"] #0-24

def _dns_type_name(i):
    return _dns_type[i] if i < len(_dns_type) else "unknown%d"%i
def _dns_type_id(name):
    return _dns_type.index(name)
def _dns_opcode_name(i):
    return _dns_opcode[i] if i < len(_dns_opcode) else "unknown%d"%i
def _dns_opcode_id(name):
    return _dns_opcode.index(name)
def _dns_class_name(i):
    return _dns_class[i] if i < len(_dns_class) else "unknown%d"%i
def _dns_class_id(name):
    return _dns_class.index(name)
def _dns_rcode_name(i):
    return _dns_rcode[i] if i < len(_dns_rcode) else "unknown%d"%i
def _dns_rcode_id(name):
    return _dns_rcode.index(name)

def dns_dump_rr(rr):
    classname = _dns_class_name(rr["CLASS"])
    typename = _dns_type_name(rr["TYPE"])
    msg = " %s %s %5s %s"%(rr["RRTYPE"], classname, typename, rr["NAME"])
    if rr["RRTYPE"] != "QD":
        msg += " TTL=%d LEN=%d"%(rr["TTL"], len(rr["DATA"]))
        if "COMMENT" in rr:
            msg += " [%s]"%(rr["COMMENT"])
    print msg

def dns_dump(dns):
    if dns["QR"]:
        msg = "Response: ID=%04X OP=%s RCODE=%s"%(dns["ID"], _dns_opcode_name(dns["OPCODE"]), _dns_rcode_name(dns["RCODE"]))
    else:
        msg = "Request:  ID=%04X OP=%s"%(dns["ID"], _dns_opcode_name(dns["OPCODE"]))
    if dns["AA"]:
        msg += " AA"
    if dns["TC"]:
        msg += " TC"
    if dns["RD"]:
        msg += " RD"
    if dns["RA"]:
        msg += " RA"
    print msg
    for rr in dns["QD"]:
        dns_dump_rr(rr)
    for rr in dns["AN"]:
        dns_dump_rr(rr)
    for rr in dns["NS"]:
        dns_dump_rr(rr)
    for rr in dns["AR"]:
        dns_dump_rr(rr)
    print 

def dns_dump2(dns):
    for rr in dns["AN"]:
        print "%s\t%s"%(rr["COMMENT"], rr["NAME"])

def parse_dnsname(data, cur):
    name = ""
    while True:
        a = ord(data[cur])
        if a >= 0xC0:
            b = ord(data[cur + 1])
            c, d = parse_dnsname(data, (a - 0xc0) * 256 + b)
            return name + c, cur + 2
        elif a>0:
            name += data[cur+1:cur+1+a] + "."
            cur += a + 1
        else:
            return name[0:len(name)-1], cur + 1

def dns_rrparser(data, cur, rrtype):
    name, cur = parse_dnsname(data, cur)
    rr = {"NAME":name, "RRTYPE":rrtype, "TTL":0, "DATA":""}
    if rrtype == "QD":
        rr["TYPE"], rr["CLASS"] = struct.unpack_from(">HH", data, cur)
        cur += 4
        return rr, cur
    else:
        rr["TYPE"], rr["CLASS"], rr["TTL"], rdlen = struct.unpack_from(">HHIH", data, cur)
        cur += 10
        rr["DATA"] = data[cur:cur+rdlen]
        
        typename = _dns_type_name(rr["TYPE"])
        if typename == "A":
            rr["COMMENT"] = ".".join(["%d"%v for v in struct.unpack_from("BBBB", rr["DATA"], 0)])
        elif typename == "AAAA":
            rr["COMMENT"] = ":".join(["%x"%v for v in struct.unpack_from(">HHHHHHHH", rr["DATA"], 0)])
        elif typename == "CNAME" or typename == "PTR":
            rr["COMMENT"], t = parse_dnsname(data, cur)
        elif typename == "SOA":
            mname, t = parse_dnsname(data, cur)
            rname, t = parse_dnsname(data, t)
            serial, refresh, retry, expire, minimum = struct.unpack_from(">IIIII", data, t)
            rr["COMMENT"] = "%s %s %d %d %d %d %d"%(mname, rname, serial, refresh, retry, expire, minimum)
        return rr, cur + rdlen

def dns_parser(data):
    dns = {"raw":data}
    dns["ID"], flag, QDC, ANC, NSC, ARC = struct.unpack_from(">HHHHHH", data, 0)
    dns["RCODE"] = flag & 0xF
    dns["OPCODE"] = (flag >> 11) & 0xF
    dns["QR"] = (flag & 0x8000) != 0
    dns["AA"] = (flag & 0x400) != 0
    dns["TC"] = (flag & 0x200) != 0
    dns["RD"] = (flag & 0x100) != 0
    dns["RA"] = (flag & 0x80) != 0
    dns["QD"] = []
    dns["AN"] = []
    dns["NS"] = []
    dns["AR"] = []
    cur = 12
    for i in range(0, QDC):
        rr, cur = dns_rrparser(data, cur, "QD")
        dns["QD"].append(rr)
    for i in range(0, ANC): 
        rr, cur = dns_rrparser(data, cur, "AN")
        dns["AN"].append(rr)
    for i in range(0, NSC): 
        rr, cur = dns_rrparser(data, cur, "NS")
        dns["NS"].append(rr)
    for i in range(0, ARC): 
        rr, cur = dns_rrparser(data, cur, "AR")
        dns["AR"].append(rr)
    return dns

def dns_build_rr(rr, off, namedict):
    outstr = ""
    namesep = rr["NAME"].split(".")
    while len(namesep):
        fullname=".".join(namesep)
        if fullname in namedict:
            outstr += struct.pack(">H", namedict[fullname] + 0xC000)
            break
        else:
            namedict[fullname] = off + len(outstr)          
            outstr += struct.pack("B", len(namesep[0])) + namesep[0]
            del namesep[0]
        if len(namesep) == 0:
            outstr += "\0"
    outstr += struct.pack(">HH", rr["TYPE"], rr["CLASS"])
    if rr["RRTYPE"] != "QD":
        outstr += struct.pack(">IH", rr["TTL"], len(rr["DATA"])) + rr["DATA"]
    return outstr
    
def dns_build(dns):
    flag = (dns["OPCODE"] << 11) + dns["RCODE"]
    if dns["QR"]:
        flag = flag | 0x8000
    if dns["AA"]:
        flag = flag | 0x400
    if dns["TC"]:
        flag = flag | 0x200
    if dns["RD"]:
        flag = flag | 0x100
    if dns["RA"]:
        flag = flag | 0x80
    dnsbuf = struct.pack(">HHHHHH", dns["ID"], flag, len(dns["QD"]), len(dns["AN"]), len(dns["NS"]), len(dns["AR"]))
    namedict = {}
    for rr in dns["QD"]:
        dnsbuf += dns_build_rr(rr, len(dnsbuf), namedict)
    for rr in dns["AN"]:
        dnsbuf += dns_build_rr(rr, len(dnsbuf), namedict)
    for rr in dns["NS"]:
        dnsbuf += dns_build_rr(rr, len(dnsbuf), namedict)
    for rr in dns["AR"]:
        dnsbuf += dns_build_rr(rr, len(dnsbuf), namedict)
    return dnsbuf
    
cache = [None] * 1024
curid = 0
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 53))

def handle_request(dns, addr):
    if len(dns["QD"]) == 0:
        return False
    
    rr = dns["QD"][0]
    if _dns_type_name(rr["TYPE"])=="A" and _dns_class_name(rr["CLASS"]) == "IN":
        domain = rr["NAME"]
        if domain in hosts:
            ip = [int(v) for v in hosts[domain].split(".")]
            anrr = {"NAME": rr["NAME"], "RRTYPE":"AN", "TYPE":rr["TYPE"], "CLASS":rr["CLASS"], "TTL":120, 
                    "DATA":struct.pack("BBBB", ip[0], ip[1], ip[2], ip[3]), "COMMENT":hosts[domain]}
            dns["AN"].append(anrr)
            dns["QR"] = True
            dns["RA"] = True
            dns["RCODE"] = 0
            data = dns_build(dns)
            logging.debug("send %d bytes to %s"%(len(data), str(addr)))
            dns_dump2(dns)
            sock.sendto(data, addr)
            return True

def start_dns_server():
    global curid
    while True:
        try:
            data, _addr = sock.recvfrom(2048)
        except socket.error as e:
            continue
        if len(data)<12:
            continue
        dns = dns_parser(data)    
        if dns["QR"]:
            cache_obj = cache[dns["ID"]]
            if cache_obj == None:
                continue
            data = struct.pack(">H", cache_obj["original_id"]) + data[2:]
            logging.debug("send %d bytes to %s"%(len(data), str(cache_obj["addr"])))
            dns_dump2(dns)
            sock.sendto(data, cache_obj["addr"])
        else:
            logging.debug("recv %d bytes from %s"%(len(data), str(_addr)))
            #dns_dump(dns)
            if handle_request(dns, _addr):
                continue
            cache[curid] = {"id":curid, "original_id":dns["ID"], "addr":_addr}
            data = struct.pack(">H", curid) + data[2:]
            sock.sendto(data, ("10.190.202.200", 53))
            curid = curid + 1 if curid < 1023 else 0

hosts = {
    "login.weixin.qq.com":"10.103.16.88",
    "h5.shqmxx.com":"10.103.16.88"
}
start_dns_server()
