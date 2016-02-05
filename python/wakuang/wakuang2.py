#!python
# -*- coding: utf-8 -*-

from __future__ import print_function
import base64
import zlib
from Crypto.Cipher import AES
import json
import cookielib, urllib2, urllib
import copy
from datetime import datetime
import sys

reload(sys)
sys.setdefaultencoding('utf-8')

allconfigs = ["字体配置","事件表","升级经验表","招募","冒险地点","商城","武具强化","食物","被动技能","英雄技能",
                "怪物组","英雄勇者","奖励事件掉落组","额外成功率价格","TIP配置","矿脉属性","宿命武具","徽记",
                "尘土徽记和宿命武具获得","货币表","道具表","切磋竞技场战斗场景表","升级经验战力结算","宿命锻造",
                "新手_新手值","动作和特效表_角色特效","系统配置表","转生价格","合作技能","游戏功能开启表","魂晶兑换",
                "国王订单","自动挖矿","游戏故事配置","Q点购买表","活动时间表","每日充值活动","累计充值活动",
                "每日消费活动","累计消费活动","目标活动招募","目标活动血钻招募","目标活动击杀巨魔","目标活动击杀BOSS",
                "特殊事件","免费赠送","道具兑换活动","活动道具","商店配置","公告系统","秘术之门","杂项","区域配置",
                "争霸战奖励表","钥匙表","图书馆图鉴收集兑换奖励","图书馆精通点数兑换奖励","书页表","契约加成",
                "国王订单额外奖励","连锁活动总表","连锁活动事件序列表","活动奖励事件掉落组","CDKEY礼包奖励事件掉落组",
                "秘术奖励事件掉落组","签到奖励事件掉落组","招募奖励事件掉落组"]

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
        f.write(s)
    

def printbattleresult(battlestring):
    heros = {}
    for hero in getConfig("英雄勇者")["data"]:
        heros[hero["ID"]]=hero
        
    skills = {}
    for skill in getConfig("英雄技能")["data"]:
        skills[skill["ID"]]=skill
    
    battle = json.loads(battlestring)
    
    report = battle["data"]["report"]
    
    print("左侧队伍属性:")
    MG = report["leftMG"]
    print(u"姓名:%s"%(MG["name"]))
    print("战力:%s"%(MG["maxATK"]))
    print("先攻:%s"%(MG["firstStrike"]))
    print("防御:%s"%(MG["defense"]))
    print("闪避:%s"%(MG["dodge"]))
    print("王者:%s"%(MG["wz"]))
    print("队伍:", end='')
    for mer in MG["merList"]:
        print("  %s"%(heros["%d"%(mer)]["yingxiongming"]), end='')
    print("")
    print("")
    
    print("右侧队伍属性:")
    MG = report["rightMG"]
    print(u"姓名:%s"%(MG["name"]))
    print("战力:%s"%(MG["maxATK"]))
    print("先攻:%s"%(MG["firstStrike"]))
    print("防御:%s"%(MG["defense"]))
    print("闪避:%s"%(MG["dodge"]))
    print("王者:%s"%(MG["wz"]))
    print("队伍:", end='')
    for mer in MG["merList"]:
        print(u" %s"%(heros["%d"%(mer)]["yingxiongming"]), end='')
    print("")
    print("")
    
    lefthp = report["leftMG"]["maxATK"]
    righthp = report["rightMG"]["maxATK"]
    
    if report["leftMG"]["firstStrike"] >= report["rightMG"]["firstStrike"]:
        leftturn = 0
    else:
        leftturn = 1
    
    for i in range(len(report["dodgeList"])):
        print(u"第%d回合"%(i+1), end='')
        skillname = skills["%d"%(report["skillList"][i])]["jinengming"]
        dodge = report["dodgeList"][i]
        if i%2 == leftturn:
            dmg = - report["damageValue"][i][1]
            heal = report["damageValue"][i][0]
            heroname = report["leftMG"]["name"]
            targetname = report["rightMG"]["name"]
            lefthp += heal
            righthp -= dmg
        else:
            dmg = - report["damageValue"][i][0]
            heal = report["damageValue"][i][1]
            heroname = report["rightMG"]["name"]
            targetname = report["leftMG"]["name"]
            righthp += heal
            lefthp -= dmg
            
        print(u", %s使出了%s"%(heroname,skillname), end='')
        if dodge:
            print(u", 可惜没有命中", end='')
        else:
            print(u", %s受到了%d点伤害"%(targetname,dmg), end='')
        if heal > 0:
            print(u", %s恢复了%d点生命"%(heroname, heal), end='')
        print("")
        print(u"%s还剩%d点生命, %s还剩%d点生命"%(report["leftMG"]["name"], lefthp, report["rightMG"]["name"], righthp))
        print("")
        
    if battle["data"]["iswin"]:
        print(u"%s赢了"%(report["leftMG"]["name"]))
    else:
        print(u"%s赢了"%(report["rightMG"]["name"]))

import cgi, cgitb 
form = cgi.FieldStorage()

print("Content-type:text/html\r\n\r\n")
print("<html>")
print("<head>")
print("<title>Hello - Second CGI Program</title>")
print("</head>")
print("<body>")
printbattleresult('''
    {"data": 
      {"report": 
        {"dodgeList": [false, false, false, false, false, false, false, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false], 
         "skillList": [1800005, 2200008, 1900011, 1000001, 1800005, 2200008, 2200005, 1000001, 1900011, 2200008, 1800005, 1000001, 1100015, 1000001, 1800005, 1000001, 1600005, 1000001, 1200029, 2200008, 1200062, 2200008, 1200042, 1100011, 1200064], 
         "damageValue": [[401094, -155046], [-637895, 0], [877393, -149725], [-464867, 0], [431740, -158995], [-607023, 0], [0, -689999], [-401510, 0], [877393, -146033], [-523420, 0], [392881, 0], [-390612, 0], [0, -296525], [-368483, 0], [329013, 0], [-368483, 0], [0, -179362], [-355098, 0], [0, 0], [-475831, 0], [0, -265017], [-449329, 0], [0, 0], [-513040, 0], [0, -235288]], 
         "leftMG": {"dodge": 18, "name": "crazyyao", "firstStrike": 21, "skills": [1100020, 1200033, 1900011, 1900006, 1200064, 1200062, 1100019, 1200061, 1100031, 1200042], "skillsLv": 1, "ATK": 2299851, "defense": 34, "maxATK": 2299851, "merList": [99000001, 19000033, 18000003, 19000018, 19000013, 19000005, 18000037, 18000031, 19000021, 19000029, 18000001, 18000001, 18000026, 18000026, 18000011, 18000029, 18000022, 11000035, 18000041, 18000015], "wz": 0}, "xly": [], 
         "rightMG": {"dodge": 115, "name": "F\u4ee5\u592a\u66b4\u541b", "firstStrike": 5, "skills": [1100011, 2200008], "skillsLv": 1, "ATK": 2178000, "defense": 345, "maxATK": 2178000, "merList": [21000010], "wz": 0}
        }, 
        "eventtype": 1, 
        "rewards": [{"count": 400000, "type": 6, "rewardid": 2700184}, {"type": 36, "id": 200059, "rewardid": 2700185}, {"type": 36, "id": 200060, "rewardid": 2700186}, {"count": 400000, "type": 23, "rewardid": 2700187}], 
        "iswin": true
      }, 
    "ret": 0
    }''')
print("</body>")
print("</html>")
