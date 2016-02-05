import os
import hashlib
import base64
from datetime import date
import urllib2
import json
import zlib
import string
import glob


def hexmd5(str):
    m = hashlib.md5()
    m.update(str)
    return m.hexdigest().upper()

def hexmd5str(str):
    s = hexmd5(str)
    return s[0:8] + "-" + s[8:16] + "-" + s[16:24] + "-" + s[24:32]

def getkey():
    #today = date(2014, 12, 30)
    today = date.today()
    key = "[" + hexmd5str("WYAO-0680"+today.strftime("%m%d %y %d%m") + "wyao") + "]" 
    key += "1" + today.strftime("%Y-%m-%d")
    key += "{" + hexmd5str("2009`99991234`0911") + "}"
    return hexmd5(key)

def geturl(yyyymm):
    today = date.today()
    url = "https://bidwiz.duapp.com/bwDemoController.do?login&md5_key={" + hexmd5str("2009`99991234`0911") + "}"
    url += "&bidMonth=" + yyyymm + "&t=1&p=[" + hexmd5str("WYAO-0680"+today.strftime("%m%d %y %d%m") + "wyao") + "]"
    return url

def decrypt(encrypted, key):
    key = hexmd5(key).upper()
    length = (len(encrypted)-2)/2
    seed = int(encrypted[0:2], 16)
    clear =""
    for i in range(0,length):
        a = ord(key[i])
        b = int(encrypted[2+i*2:4+i*2], 16)
        c = a^b
        if c<seed:
            c += 255
        c = c-seed
        seed = b
        clear += chr(c)
    clear = base64.b64decode(clear)
    return clear

def downloadtestcase(yyyymm):
    key = getkey()
    url = geturl(yyyymm)
    print url
    msg = urllib2.urlopen(url).read()
    clear = json.loads(msg)
    bidInfo = clear["bidInfo"]
    for j in range(0, len(bidInfo)):
        for i in range(0, len(bidInfo[j])):
            bidInfo[j][i] = decrypt(bidInfo[j][i], key)

    f = open(yyyymm + ".json", "w")
    f.write(json.dumps(clear, indent=2, ensure_ascii=False))
    f.close()
    
def privatebase64(input):
    STANDARD_ALPHABET = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
    CUSTOM_ALPHABET =   '=DB:T5As2{$6W41pqLQYuV.OH!tFU;lo)g[rIS7JM*]P(aZbxNe/+y_Rdj8@E^ni'
    DECODE_TRANS = string.maketrans(CUSTOM_ALPHABET, STANDARD_ALPHABET)
    input = input.translate(DECODE_TRANS)
    return base64.b64decode(input + "==")

def getimagekey(key1, key2):
    XORTABLE = [0x20, 0x09, 0x91, 0x11, 0x98, 0x19, 0x25, 0x16]
    encrypted = privatebase64(key2)
    key = ""
    for i in range(0, len(encrypted)/2):
        a = int(encrypted[i*2:i*2 + 2], 16) ^ XORTABLE[i % 8]
        key += chr(a)    
    key = decrypt(key, key1)
    return key

def decodeimage(fromname, toname):
    f = open(fromname, "rb")
    imagedata = f.read()
    f.close()
    imagedata = zlib.decompress(imagedata)
    key1 = imagedata[0:4]
    key2 = imagedata[4:138]
    key = getimagekey(key1, key2)
    encryptedbuf = imagedata[138:]
    clearbuf=""
    for i in range(0, len(encryptedbuf)):
        a = ord(encryptedbuf[i]) ^ ord(key[i%16])
        clearbuf += chr(a)
    f = open(toname, "wb")
    f.write(clearbuf)
    f.close()


'''
testcases=["201308", "201309", "201310", "201311", "201312", 
      "201401", "201402", "201403", "201404", "201405", "201406", "201407", "201408", "201409", "201410", "201411", "201412",]
for testcase in testcases:
    downloadtestcase(testcase)
'''

for imagefile in glob.glob("F:\\TDDOWNLOAD\\NetBidClient\\ImageCode\\*.enc"):
    fileName, fileExtension = os.path.splitext(imagefile)
    decodeimage(imagefile, fileName)