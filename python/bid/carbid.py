import hashlib
import string
import base64

def hexmd5(str):
    m = hashlib.md5()
    m.update(str)
    return m.hexdigest()

def getloginrequest(bidnumber, bidpassword, machinecode):
    imagenumber = "237347"
    version = "177"
    cc = str((bidnumber - bidpassword) >> 4)
    cc = hexmd5(cc).lower()
    cc = version + cc[2] + cc[4] + cc[0xa] + cc[0xc] + cc[0x12] + cc[0x14] + cc[0x1a] + cc[0x1c] + imagenumber + machinecode + "AAA"
    cc = hexmd5(cc)
    url = "https://tblogin.alltobid.com/car/gui/login.aspx?BIDNUMBER=%d&BIDPASSWORD=%d&MACHINECODE=%s&CHECKCODE=%s&VERSION=%s&IMAGENUMBER=%s"%(bidnumber, bidpassword, machinecode, cc, version, imagenumber)
    return url

def getcheckrequest(bidnumber, bidpassword, bidprice):
    version = "177"
    cc = str(bidnumber-bidprice) + "#" + version + "@" + str(bidpassword)
    cc = hexmd5(cc)
    url = "https://toubiao.alltobid.com/car/gui/imagecode.aspx?BIDNUMBER=%d&BIDPASSWORD=%d&BIDAMOUNT=%d&VERSION=%s&CHECKCODE=%s"%(bidnumber, bidpassword, bidprice, version, cc)
    return url

def getbidrequest(bidnumber, bidpassword, bidprice, machinecode, imagenumber):
    version = "177"
    cc = str((bidnumber - bidpassword + bidprice) >> 4)
    cc = hexmd5(cc).lower()
    cc = version + cc[2] + cc[4] + cc[0xa] + cc[0xc] + cc[0x12] + cc[0x14] + cc[0x1a] + cc[0x1c] + imagenumber + machinecode + "AAA"
    cc = hexmd5(cc)
    url = "https://tblogin.alltobid.com/car/gui/bid.aspx?BIDNUMBER=%d&BIDPASSWORD=%d&MACHINECODE=%s&CHECKCODE=%s&VERSION=%s&IMAGENUMBER=%s"%(bidnumber, bidpassword, machinecode, cc, version, imagenumber)
    return url

STANDARD_ALPHABET = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
CUSTOM_ALPHABET =   '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/'

ENCODE_TRANS = string.maketrans(STANDARD_ALPHABET, CUSTOM_ALPHABET)
DECODE_TRANS = string.maketrans(CUSTOM_ALPHABET, STANDARD_ALPHABET)
def carbinbase64decode(input):
    print input
    input = input.translate(DECODE_TRANS)
    print input
    return base64.b64decode(input + "==")


input = "/zZ/u00GIaP9HW010G000G01003/sm1302WS7YCU6IWZ8ICjAoWmF6H1F3StF7jONKbaaO2Pbe+0Z8gWjER3eAhQhOgCoF/Bskxr////cy7////w/+Rz//Z/sm130IijBJmrF7P1GNRufOob+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZu+FZ/m00H200X07430I800X410n41/yG07m000GK10G410G400000000000420mG51WS82GeB/yG0jH000W430m840mK510G0005z0G8300GH1H8XCK464r5X1o9n53A1aQ488qAnmHLIqV0aCs9oWWaA5XSO6Heb9YSeAIeqDJOtE3awGqH5HaT8IKfJL5LMLrXPMcDaPMPdQ6bgStHrTdTuUNg3X8M6XuY9YfAJb9MMbvYPcgAZfAMcfwYfghApjBMsjxYvkiB3nCN6nyZ9ojBJrDNMrzZPsk7Yu+JbvkVewUhnylFqzVRt+Fdw/yG07m400m410G410G410G00000000420mG51WS82GeB/yG0jH400W4210G310S510G00G9t00420n441I4n1X91KGTXSHCYCe4854AHeR712ICpKl0LOdBH2XOaDE4byHSO6Hec9oWfAZKsDpWvEaD4HKP7I4bAKrHLLbTOMLfZP6LcPsXfQdDqTNPtU7bwWeE4XOQ7Y8cAafEKbPQNc9cQegEafQQdgAcgihEqjRQtkBcwmiF4nSR7oCdAqjFKrTRNsDdQukFavURdwEdgylFqzVRt+Fdw/ze030C1008H0n40Fm2vQMjlzaXPe8cp6f9A0dEAb+oMozRUB7+uEABFvRA3FJor/3YdlD72srZdZErGM87hWTgP0px9RBrjuiVxWue+oMozRUB7+uEAM6wXbPbZa1sd1KZ1Ny3pYcczjutA6JeS4OF5Aw2zXVibilMtYn/k3YXhMrG5cWXszIIWu/y0hKiLr38nMDzm0pq8nLIUHBYLWpbBItRzvaSEtfz1/NwKmJ7hzX832s1GZ8FsS/uTAGDfto5Oec4cIe4MJnrunqgJJvPtYTxX6JzuTgiC5Lun+5PMc1uzGXMKCfN8mmwS4/rguHvhtx0Qcoo1/m2FKO/wzpn+bFZWitKia4HN91p60LFlnINJtaJWMqIF7ZaiUX/EigMpk9heATiSc38KRxWvnaSdh/dYdoBbkWDH8x5c0I65WSa482ECPvnqv7+UZvBUqXKky48GTIK75O5ghHwaYGoPn9jtg0UEXFVjMXhBaIHmZEq3Tr/pwVhMJPSOtTYols7T63Rng9HbTqG7uVvzHK/sIsNhRnO/t1nMVSmGJ6tFsb8niLTfwgCP7Vy0d+TQm7bW0PsZrESK899BOuwYYYW3gxFvRA3FJor/3YgOipRQcRoQT14MEtSS7aS3yFwLSi/bieCzFBNyEAgsjz3Vol2zkDYZSk/3UtuTQPDx5H4akTN5p0hh261ta40WO1gU8GD6PPONaB4bjYd2UtMgj+ls7KqQr+JS0nNF7NfzEEbQjbShAh1OzYgUWw3DIzMJArqG8RVo9tjGooUMt6Jk5LB6vkR2tT6ifMN9OiGHZZwUrJsd/8IOfzm5iuwO/m0EbQCqU+2IFE5TIkVxkHH7L15aDXUVQe6asR53RSPpZWVurcMTmBZLqcsx0sUCvnykAi9fCqAuZlf4NEJj17zQV9fGMRp89Z2lOAFkyOwvhUaqhtweRqBRGkBf9XCmZKOCVRl/08rdwdEbjU/F6P5U7QL35UC+e+bJ7JxXSVwVBZyUFrgM2nyaiqihIiHj3d+0THmSz+Q7PBUuR5Emr6tIH8OhRoWv09t3ZhtFNj+PfsiGDlZbN91+J7eUrQ4K4SB1rZICaORO1n+ECu/pz96LIfLr3Hdg2CWLYrSgCkLtCZK8q5dRIXLLcKS3hq7ud+bND9PXP3SmAxZZ7yFrgRx7Re51Z1HCa1YI5prwrFy0S/tVvK9QbIckMoEDeeee4555501LYw/X/6YYfUw8Vn8Bh+7yQhqKKHs27mXHHHL5XHHHG0KKKK0555501HHHG1//sG==="
output = carbinbase64decode(input)

url = getloginrequest(52543199, 1608, "S0TZNSAD300213")
print url
url = getcheckrequest(52543199, 1608, 72600)
print url
url = getbidrequest(52543199, 1608, 72600, "S0TZNSAD300213", "608080")
print url

input="f5c3aba6afbac1b6b1b9b0c3d0aba6afbac1c3b6b1b9b0c1becdcfcecb3b15c92b3dcdc6372a3630455c352f4709373448382c552d4a2f2b40324c4a4911493732494e153a3b3d0b441ea1c8cbcfcfa1c8cdc9cfcfa1cecfc5cccfa1cecec5cccfa1cecfc5cccfa1cecec5cfcfa1cecec5cfcfa1cecec5cccfa1cecfc5cccec5cbcba1c8c9cacac9a1c8cdc9cfcfa1cecfc5cccfc5cecec3d0b6b1b9b0c1f5f5f5f5f5f5f6f6f6"
packetbuf = ""
for i in range(0, len(input)//2):
    packetbuf += chr(int(input[i*2:i*2+2], 16))
out=""
for c in packetbuf:
    out += chr(ord(c) ^ 0xFF)
print out

