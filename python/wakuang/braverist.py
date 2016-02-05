#!/usr/bin/env python
# -*- coding: utf-8 -*-

import base64
import zlib
from Crypto.Cipher import AES
import urllib2,urllib
import json
import sys
from urllib import urlopen
from PIL import Image
from StringIO import StringIO

reload(sys)
sys.setdefaultencoding('utf-8')

allconfigs = ["字体配置","事件表","升级经验表","招募","冒险地点","商城","武具强化","食物",
                "被动技能","英雄技能","怪物组","英雄勇者","奖励事件掉落组","额外成功率价格",
                "TIP配置","矿脉属性","宿命武具","徽记","尘土徽记和宿命武具获得","货币表",
                "道具表","切磋竞技场战斗场景表","升级经验战力结算","宿命锻造","新手_新手值",
                "动作和特效表_角色特效","系统配置表","转生价格","合作技能","游戏功能开启表",
                "魂晶兑换","国王订单","自动挖矿","游戏故事配置","Q点购买表","活动时间表",
                "每日充值活动","累计充值活动","每日消费活动","累计消费活动","目标活动招募",
                "目标活动血钻招募","目标活动击杀巨魔","目标活动击杀BOSS","特殊事件",
                "道具兑换活动","活动道具","商店配置","公告系统","秘术之门","杂项","区域配置",
                "争霸战奖励表","钥匙表","图书馆图鉴收集兑换奖励","图书馆精通点数兑换奖励",
                "书页表","契约加成","国王订单额外奖励","活动奖励事件掉落组",
                "CDKEY礼包奖励事件掉落组","秘术奖励事件掉落组","签到奖励事件掉落组",
                "招募奖励事件掉落组"]

def decode(encrypted):
    encrypted = base64.b64decode(encrypted)
    cipher = AES.new("qdfgtyjltgfresdf", AES.MODE_CBC, "sftjkiuyhyujkiol" )
    encrypted = cipher.decrypt(encrypted)
    encrypted = base64.b64decode(encrypted)
    clear = zlib.decompress(encrypted, -15)
    return clear
    
def getConfig(name):
    url = "http://ks1mxwk.conf.u77.com/getconfig.ashx?name=%s&v=1.0.32&t=1"%(urllib.quote(name))
    encrypted = urllib2.urlopen(url).read()
    clear = decode(encrypted)
    return json.loads(clear)

def download(config):
    j = getConfig(config)
    with open(u"braverist/%s.json"%(config.decode("utf-8")), "wb") as f:
        s = json.dumps(j, encoding="utf-8", sort_keys=True, ensure_ascii=False, indent=2)
        print s
        f.write(s)
    
#for config in allconfigs:
#    download(config)

imagebg = Image.open("Heroes_UI.png")
imagebg1 = imagebg.crop((0,0,52,52))
imagebg2 = imagebg.crop((0,59,52,111))
imagebg3 = imagebg.crop((0,115,52,167))
imagebg4 = imagebg.crop((0,178,52,230))
imagebg5 = imagebg.crop((0,238,52,290))

def create_image(hero):
    filename = hero["ID"] + ".png"
    quality = int(hero["yxpz"])
    
    image = Image.open(StringIO(urlopen("http://ks1mxwk.res.u77.com/role/" + filename).read()))
    image = image.convert('RGBA')
    image = image.crop((0,0,image.size[0]/3,image.size[1]/4))
    
    imagefg = Image.new("RGBA", (52, 52), "white")
    
    if quality <= 1:
        imagefg.paste(imagebg1)
    elif quality == 2:
        imagefg.paste(imagebg2)
    elif quality == 3:
        imagefg.paste(imagebg3)
    elif quality == 4:
        imagefg.paste(imagebg4)
    elif quality == 5:
        imagefg.paste(imagebg5)
    imagefg.paste(image, ((52-image.size[0])/2,(52-image.size[1])/2), image)
    imagefg.save("image\\" + filename, "PNG")

heros = getConfig("英雄勇者")

data = getConfig("英雄技能")
activeskills={}
for item in data["data"]:
    activeskills[item["jinengming"]] = item

data = getConfig("被动技能")
passiveskills={}
for item in data["data"]:
    passiveskills[item["jinengming"]] = item

data = getConfig("合作技能")
groupskills={}
for item in data["data"]:
    groupskills[item["hzjm"]] = item

def getheroskilldesc(hero):
    if hero["yxjn"] == "":
        return "无"
    desc = ""
    skillname = hero["yxjn"]
    if skillname in passiveskills:
        ps = passiveskills[skillname]
        desc = ps["miaoshu"]
    if skillname in activeskills:
        acs = activeskills[skillname]
        desc = acs["miaoshu"]
    desc = hero["yxjn"] + ": " + desc
    return desc


f=open("heros.html", "w")
f.write('<html lang="zh-cn"><head>')
f.write('<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>')
f.write('<style>')
f.write('table {border-spacing:0px}')
f.write('td {vertical-align:top;padding:0px 2px}')
f.write('</style></head><body>')
f.write("<table>\n")
f.write("<tr><td>图标</td><td width='120'>姓名</td><td>性别</td><td width='120'>种族</td><td width='120'>职业</td><td>初始战力</td><td>战力系数</td><td>觉醒级别</td><td>荣誉碎片</td></tr>")
f.write('<tr><td colspan="10"><hr/></tr>\n\n')

for hero in heros["data"]:
    if hero["tujian"] == "0":
        continue
    
    f.write('<tr id="%s"><td rowspan="8"><img src="image/%s.png"></img></td>'%(hero["yingxiongming"],hero["ID"]))
    f.write('<td><span><b>%s</b></span></td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>'%(
            hero["yingxiongming"],hero["xingbie"],hero["zhongzu"],hero["zhiye"],hero["cszl"],hero["zlxs"],hero["juexingji"],hero["lhsp"]))

    f.write('<tr><td>技能</td><td colspan="9">%s</td></tr>'%(getheroskilldesc(hero)))
    if hero["hezuoji"] != "":
        gs = groupskills[hero["hezuoji"]]
        f.write('<tr><td>合作技</td><td colspan="8">%s: %s<br/>%s</td></tr>'%(gs["hzjm"], gs["fdtj"], gs["jnsm"]))
    else:
        f.write('<tr><td></td></tr>')
        
    if hero["hzj2"] != "":
        gs = groupskills[hero["hzj2"]]
        f.write('<tr><td>合作技</td><td colspan="8">%s: %s<br/>%s</td></tr>'%(gs["hzjm"], gs["fdtj"], gs["jnsm"]))
    else:
        f.write('<tr><td></td></tr>')
    
    if hero["bjmz"] != "":
        f.write('<tr><td>宝具</td><td colspan="8">%s: %s先攻, %s防御, %s闪避, %s王者</td></tr>'%(hero["bjmz"],hero["bjxgjc"],hero["bjfyjc"],hero["bjsbjc"],hero["bjwzjc"]))
    else:
        f.write('<tr><td></td></tr>')
    f.write('<tr><td>精通</td><td colspan="8">增加%s先攻,%s防御,%s闪避,%s王者,需要%s碎片,获得%s精通点数</td></tr>'%(hero["jtxgjc"], hero["jtfyjc"], hero["jtsbjc"], hero["jtwzjc"], hero["jtrysp"], hero["jtds"]))
    f.write('<tr><td>合成</td><td colspan="8">需要%s尘封,%s自然,%s绯红,%s漆黑,%s闪耀,%s金币,%s碎片,召唤需要%s碎片</td></tr>'%(hero["cfzy"], hero["zrzy"], hero["hzy"], hero["qhzy"], hero["syzy"], hero["hcjb"], hero["hcrysp"], hero["tszh"]))

    f.write('<tr><td>人物介绍</td><td colspan="8">%s</td></tr>\n'%(hero["jieshao"]))
    f.write('<tr><td colspan="9"><hr/></tr>\n\n')
    create_image(hero)

f.write("</table></body></html>\n")
