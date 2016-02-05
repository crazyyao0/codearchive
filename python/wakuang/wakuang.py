#!python
# -*- coding: utf-8 -*-


import base64
import zlib
from Crypto.Cipher import AES
import cookielib, urllib2, urllib
import copy
import json
import time
import traceback
from datetime import datetime
from lxml import etree
from random import choice
from sets import Set

'''
def snwllogin():
    parser = etree.HTMLParser(encoding='utf-8')
    htmlstr = urllib2.urlopen("https://10.103.200.2:8086/auth1.html").read()
    tree = etree.fromstring(htmlstr, parser)
    nodes = tree.xpath("//input")
    params = {}
    for node in nodes:
        if "name" in node.attrib and "value" in node.attrib:
            params[node.attrib["name"]] = node.attrib["value"]
    params["userName"] = "wyao"
    params["pwd"] = "Q12345_ww"
    print params
    htmlstr = urllib2.urlopen("https://10.103.200.2:8086/auth.cgi", urllib.urlencode(params)).read()
    print htmlstr
snwllogin()
'''

"""
1001 getaccountinfo
1002 get send gift count
1005 Get mine number
1007 list backpack
1009 use backpack item(id)
1011 {"ret":0,"data":[37,38,39,40,41,44,48,53,54]}
1013 get version

1101 {"ret":0,"data":{"newvalue":0,"sound":0,"value1":"step8_over"}}
1204 {"ret":0,"data":{"isget":true,"isget1":false}}

1401 get temple hero list

1793 enter maze(mazeid)

1801 get order list

2003 Get mine info
2004 digmine (x, y)
2005 getminemap 
2006 getgatherlist
2007 gather (x, y)
2016 Get mine task list
2017 Add mine task, (id = 10001 ~ 10009)
2018 Finish mine task, (id)

2101 {"ret":0,"data":{"isnew":false,"count":0}}

2201 list fight list
2202 fight(id, ranking)

2401 get troop list

3001 get troop list(sortid=1,2,3,4)
3002 dismiss hero(id)
3006 sort troop(sortid=0-8)
3010 list door(doorid=null)
3011 summon door(doorid)

4003 get friend list
4004 send gift(gifttype=3,id)
4005 get gift list
4006 receive gift(id)
4007 attack friend(id)
4008 get new message number
4009 add frient(name:utf-8,urlencoding)
4010 get add friend count
4013 dismiss friend(id)

5001 get maze list
5002 team and general info
5004 update
5005 finish task (type=1,2,3, isxz=0)
5006 get task status(type=1,2,3)
5007 collect reward

6001 get arena list
6002 get weapon list, 点数
6006 get arena reward (type=1, 4, 9)
6007 arena battle (id)
6008 getreward(dslx="勇气点数""力量点数""冠军点数""羁绊点数")
7001 list food
7002 use food (foodid)

"""

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


#urllib2.install_opener(urllib2.build_opener(urllib2.HTTPHandler(debuglevel=1)))
proxy_handler = urllib2.ProxyHandler({})
opener = urllib2.build_opener(proxy_handler)
urllib2.install_opener(opener)

cj = cookielib.MozillaCookieJar()
#cj.load("wakuang.cookie")

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
    
def search(strtofind, prefix, postfix):
    start = strtofind.find(prefix)
    if start == -1:
        return ""
    start = start + len(prefix)
    end = strtofind.find(postfix, start)
    if end == -1:
        return ""
    return strtofind[start:end]

def Login(username, password):
    global userid
    global sid
    global gameserver
    
    req = urllib2.Request("http://youxi.kdslife.com/youxi/play/wakuang/")
    req.add_header("Referer", "http://passport.pchome.net/login.php?action=login&goto=http://youxi.kdslife.com/youxi/play/wakuang/")
    req.add_header("User-agent", "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)")
    print req.get_full_url()
    cj.add_cookie_header(req)
    res = urllib2.urlopen(req)
    data = res.read()
    nexturl = search(data, '<iframe src="', '"')
    
    if nexturl == "":
        req = urllib2.Request("http://passport.pchome.net/login.php?action=login&goto=http://youxi.kdslife.com/youxi/play/wakuang/", 
                          "username="+username+"&password="+password)
        req.add_header("Referer", "http://passport.pchome.net/login.php?action=login&goto=http://youxi.kdslife.com/youxi/play/wakuang/")
        req.add_header("Content-Type", "application/x-www-form-urlencoded")
        req.add_header("User-agent", "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)")        
        print req.get_full_url()
        res = urllib2.urlopen(req)
        res.read()
        
        cj.clear()
        cj.extract_cookies(res, req)
        for cookie in cj:
            c = copy.deepcopy(cookie)
            c.domain = ".kdslife.com"
            cj.set_cookie(c)
        print cj
        cj.save("wakuang.cookie")
        
        req = urllib2.Request("http://youxi.kdslife.com/youxi/play/wakuang/")
        req.add_header("Referer", "http://passport.pchome.net/login.php?action=login&goto=http://youxi.kdslife.com/youxi/play/wakuang/")
        req.add_header("User-agent", "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)")
        cj.add_cookie_header(req)
        print req.get_full_url()
        res = urllib2.urlopen(req)
        data = res.read()
        nexturl = search(data, '<iframe src="', '"')
    

    req = urllib2.Request(nexturl)
    print req.get_full_url()
    res = urllib2.urlopen(req)
    data = res.read()
    
    sid = search(data, "var sid='", "'")
    userid = search(data, "LoadGame(0,", ")")
    gameserver = search(nexturl, "http://", "/")
    gameserver = "http://" + gameserver + "/service/main.ashx"
    

def gamecmd(cmd, params={}):
    global userid
    global sid
    global gameserver
    params["userid"] = userid
    params["sid"] = sid
    params["t"] = cmd
    print str(datetime.now())+" Request: " + str(params)
    res = urllib2.urlopen(gameserver, urllib.urlencode(params))
    data = res.read()
    print str(datetime.now())+" Response: " + str(data)
    return json.loads(data)


def process_friend():
    giftquota = gamecmd("1002")["data"]["lpj"]
    giftlist = gamecmd("4005")
    
    
    friendquota = gamecmd("4010")["data"]["count"]
    friendlist = gamecmd("4003")["data"]["list"]
    friendlist2 = {}

    for friend in friendlist:
        friendlist2[friend["username"]] = friend

    if friendquota > 0:
        enemylist =  gamecmd("6001")["data"]["list"]
        for enemy in enemylist:
            if enemy["userid"]!=0 and not (enemy["username"] in friendlist2):
                gamecmd("4009", {"name":unicode(enemy["username"]).encode("utf-8")})
                
        fightlist =  gamecmd("2201")["data"]["list1"]
        for enemy in fightlist:
            if not (enemy["username"] in friendlist2):
                gamecmd("4009", {"name":unicode(enemy["username"]).encode("utf-8")})    

        
def process_arena():
    arenalist =  gamecmd("6001")
    data = arenalist["data"]
    arenacount = data["arenacount"]
    enemylist = data["list"]
    
    for enemy in enemylist:
        if enemy["iswin"] == 0 and arenacount < 10:
            result = gamecmd("6007", {"id":enemy["id"]})
            enemy["iswin"] = result["data"]["iswin"]
            arenacount = arenacount + 1
    for enemy in enemylist:
        if enemy["iswin"] == 0 and arenacount < 10:
            result = gamecmd("6007", {"id":enemy["id"]})
            enemy["iswin"] = result["data"]["iswin"]
            arenacount = arenacount + 1
    
    arenalist =  gamecmd("6001")
    data = arenalist["data"]
    totalwin = 0
    for enemy in enemylist:
        if enemy["iswin"] == 1:
            totalwin += 1
    
    if data["arenabox3isget"]==0 and totalwin>=9:
        gamecmd("6006", {"type":9})
    if data["arenabox2isget"]==0 and totalwin>=4:
        gamecmd("6006", {"type":4})
    if data["arenabox1isget"]==0 and totalwin>=1:
        gamecmd("6006", {"type":1})



def process_backpack():
    gamecmd("5004")
    result = gamecmd("5002")
    if result["data"]["boxcount"] > 0:
        gamecmd("5007")
        
    backpack = gamecmd("1007")
    itemlist = backpack["data"]["list"]
    for item in itemlist:
        if item["itemid"]//1000000 in (1, 3, 4, 6, 7, 8):
            gamecmd("1009", {"id":item["id"]})
    
def process_food():
    info = gamecmd("5002")
    fullrate = info["data"]["bzd"]
    if fullrate < 18000:
        foodlist = gamecmd("7001")
        for food in foodlist["data"]:
            if food["count"] > 0:
                gamecmd("7002", {"foodid":food["foodid"]})
                break


waittime=[0,3,4,5,999999,999999,5,5,7,10,
      5,7,10,5,7,10,8,10,15,4800,
      7200,14400,1200,2400,1200,1200,2400,2400,0,0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
deepestdig = 0

def processminelist(minelist):
    global mat
    global deepestdig
    for mine in minelist:
        mat[mine['y']][mine['x']+512] = mine['type']
        if mine['y']>deepestdig:
            deepestdig = mine['y']

def isdiggable(x, y, w):
    global mat
    global waittime
    a = mat[y][x + 512]
    if a == 0 or a == -1 or a == 4 or a == 5:
        return False
    if waittime[a]>w:
        return False
    
    if mat[y][x + 512 + 1] == 0:
        return True
    if mat[y][x + 512 - 1] == 0:
        return True
    if mat[y + 1][x + 512] == 0:
        return True
    if mat[y - 1][x + 512] == 0:
        return True
    return False

def getexplcount(x, y):
    global mat
    coords = []
    for i in range (4):
        for j in range(4-i):
            coords.append({"x":i+1, "y":j})
            coords.append({"x":-i, "y":j+1})
            coords.append({"x":-i-1, "y":-j})
            coords.append({"x":i, "y":-j-1})
    
    count = 0
    for coord in coords:
        if mat[y + coord["y"]][x + coord["x"] + 512] == -1:
            count += 1
    return count

def dig(x, y):
    global mat
    print "dig(%d,%d)"%(x,y) 
    result = gamecmd("2004", {"x":x, "y":y})["data"]
    processminelist(result["nodeList"])
    mat[result["y"]][result["x"]+512] = 0
    
    if "exploit" in result:
        timetowait = result["exploit"]["needtime"] - result["exploit"]["time"]
        if timetowait <= 15 and timetowait > 0:
            time.sleep(timetowait)
            while True:
                time.sleep(1)
                gatherlist = gamecmd("2006")
                if len(gatherlist["data"]) == 0:
                    break
                gather = gatherlist["data"][0]
                if(gather["time"] > gather["needtime"]):
                    gamecmd("2007", {"x":gather["x"], "y":gather["y"]})
                    break;
                
def getnearestdig(x, y):
    global mat
    global waittime

    queueditems = ["%d,%d"%(x,y)]
    allitems = Set()

    while len(queueditems)>0:
        item = queueditems.pop(0)
        if item in allitems:
            continue;
        allitems.add(item)
        tx, ty = map(int, item.split(","))
        if ty>=300:
            continue
        for dx, dy in [(1,0), (-1,0), (0,1), (0,-1)]:
            tt = mat[ty + dy][tx + dx + 512]
            if tt == 0:
                return item
            if waittime[tt] <= 15:
                queueditems.append("%d,%d"%(tx+dx, ty+dy))
    return None
        
def trydigto(x,y):
    while True:
        digitem = getnearestdig(x, y)
        if digitem == None:
            return False
        tx, ty = map(int, digitem.split(","))
        dig(tx, ty)
        if tx == x and ty== y:
            break
    return True

def searchresource(restype, random=True):
    global mat
    reslist=[]
    for x in range(1024):
        for y in range(302):
            if mat[y][x] == restype:
                reslist.append({"x":x-512, "y":y, "type":restype})
                
    if len(reslist) == 0:
        return None
    elif random:
        return choice(reslist)
    else:
        return reslist[0]
    
def searchmonsters():
    global mat
    reslist=[]
    for x in range(1024):
        for y in range(302):
            if mat[y][x] >= 28 and mat[y][x] <= 40:
                reslist.append({"x":x-512, "y":y, "type":mat[y][x]})
    return reslist            
  

def trydigany(top=150, bottom=300, left=-50, right=50):
    bestexplcount = 0
    bestdigs = []
    
    for x in range(left, right):
        for y in range(top, bottom):
            if isdiggable(x, y, 15):
                n = getexplcount(x, y)
                if n > bestexplcount:
                    bestdigs = [{"x":x, "y":y}]
                    bestexplcount = n
                elif n == bestexplcount:
                    bestdigs.append({"x":x, "y":y})
                    
    bestdig = choice(bestdigs)
    print "try dig any try dig(%d,%d) and explore %d tile"%(bestdig["x"], bestdig["y"], bestexplcount)
    dig(bestdig["x"], bestdig["y"])

def trydigdeeper(top=0, bottom=300, left=-50, right=50):
    bestdeep = 0
    bestdigs = []
    for x in range(left, right):
        for y in range(top, bottom):
            if isdiggable(x, y, 15): 
                if y > bestdeep:
                    bestdigs=[{"x":x, "y":y}]
                    bestdeep = y
                elif y == bestdeep:
                    bestdigs.append({"x":x, "y":y})
                    
    bestdig = choice(bestdigs)
    print "Try dig deeper try dig(%d,%d)"%(bestdig["x"], bestdig["y"])
    dig(bestdig["x"], bestdig["y"])

def dodigAI(restypes):
    global mat
    global deepestdig
    mat = [[-1 for x in range(1024)] for x in range(350)]
    deepestdig = 0
    minelist = gamecmd("2005")
    processminelist(minelist["data"]["list"])
    print "deepestdig=%d"%(deepestdig)
    while True:
        # 1, kill monster
        monsters = searchmonsters()
        for monster in monsters:
            print "Try kill monster (%d,%d)"%(monster["x"], monster["y"])
            if trydigto(monster["x"], monster["y"]):
                continue
        
        info = gamecmd("2003")
        minecount = info["data"]["szg"]
        if minecount == 0:
            return
        
        # try kill boss
        if "bossid" in info["data"]:
            bossx = info["data"]["bossx"]
            bossy = info["data"]["bossy"]
            if bossx>0:
                bossx += 4
            else:
                bossx -= 4
            digitem = getnearestdig(bossx, bossy)
            if digitem != None:
                tx, ty = map(int, digitem.split(","))
                dig(tx, ty)
                continue
        
        # try dig any resource
        if minecount <= 5:
            for restype in restypes:
                res = searchresource(restype)
                if res != None:
                    print "Try gather resource %d (%d,%d)"%(restype, res["x"], res["y"]) 
                    if trydigto(res["x"], res["y"]):
                        return
        '''
        # try dig any resource
        #for restype in [18,17,16]:
        for restype in [9,8,7]:
            res = searchresource(restype)
            if res != None:
                print "Try gather resource %d (%d,%d)"%(restype, res["x"], res["y"]) 
                trydigto(res["x"], res["y"])
        '''
        
        if deepestdig >= 250:
            trydigany()
        else:
            trydigdeeper()
        
  

def process_mine(minetype, restypes):
    global deepestdig
    info = gamecmd("2003")
    minecount = info["data"]["szg"]
    digAI = True
    
    # gather if finished
    gatherlist = gamecmd("2006")
    for gather in gatherlist["data"]:
        if(gather["time"] >= gather["needtime"]):
            gamecmd("2007", {"x":gather["x"], "y":gather["y"]})
        else:
            digAI = False
    
    # try kill boss
    if digAI and "bossid" in info["data"]:
        dodigAI(restypes)
        return
    
    # processing mine task
    minelist = gamecmd("2016")
    for mine in minelist["data"]:
        if mine["sid"] != 0:
            if "time" in mine and mine["time"] == 0:
                gamecmd("2018", {"id":mine["id"]})
    
    if minetype != "":
        minelist = gamecmd("2016")
        for mine in minelist["data"]:
            if mine["sid"] == 0:
                if minecount >= 20:
                    gamecmd("2017", {"id":minetype})
                else:
                    digAI = False;
    
    if digAI:
        dodigAI(restypes)



def process_difficulttask(taskid):
    result = gamecmd("5006", {"type":taskid})
    if result["data"]["time"] == 0:
        result = gamecmd("5005", {"type":taskid, "isxz":0})
        with open("battle.txt", "a") as myfile:
            myfile.write(json.dumps(result))
            myfile.write("\n")

#Login("crazyyao", "3421170679")

gameserver = "http://ks1mxwk.game.u77.com/service/main.ashx"
sid="ebrlxidaizee2aozjcb3n02u"
userid="1793"

print "gameserver:" + gameserver
print "sid:" + sid
print "userid:" + userid
while True:
    try:
        gamecmd("4008")
        gamecmd("1013")
        
        #gamecmd("2902", {"id":17000002})

        
        sortid = gamecmd("1001")["data"]["sortid"]
        if sortid != 0:
            gamecmd("3001", {"sortid":0})
        process_arena()
        process_friend()
        process_backpack()
        process_food()
        process_mine("", [20, 21, 27])

        if sortid != 0:
            gamecmd("3001", {"sortid":sortid})        
    except Exception as e:
        traceback.print_exc()

    time.sleep(600)



