hzk16 = open("HZK16", "rb").read()
hzk12 = open("HZK12", "rb").read()

def display16(row, column):
    b = u"\u2588\u2588"
    w = "  "
    offset = ((row-0xa1)*94+(column-0xa1))*32
    txt = hzk16[offset:offset+32]
    txt = ''.join(format(ord(x), '08b') for x in txt)
    for i in range(16):
        print txt[i*16:i*16+16].replace("1", b).replace("0", w)
        
def display12(row, column):
    b = u"\u2588\u2588"
    w = "  "
    offset = ((row-0xa1)*94+(column-0xa1))*24
    txt = hzk12[offset:offset+24]
    txt = ''.join(format(ord(x), '08b') for x in txt)
    for i in range(12):
        print txt[i*16:i*16+12].replace("1", b).replace("0", w)
        
display16(0xb0,0xa1)
display16(0xd2,0xa6)