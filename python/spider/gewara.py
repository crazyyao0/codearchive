#! python
# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

import os
import time
import shutil
import urllib
import socket
from lxml import etree
import json

socket.setdefaulttimeout(10)
urlopener = urllib.URLopener()
parser = etree.HTMLParser(encoding='utf-8')

content_map = {'image/png':'.png', 
               'text/html':'.html', 
               'application/javascript':'.js', 
               'text/css':'.css', 
               'image/jpeg':'.jpg', 
               'image/gif':'.gif'}

def download_file(url, filename):
    a,b = urlopener.retrieve(url, "tmpfile")
    content_type = b.dict["content-type"].split(";")[0].lower()
    if os.path.splitext(filename)[1] != content_map[content_type]:
        filename = os.path.splitext(filename)[0] + content_map[content_type]
    dirpath = os.path.dirname(filename)
    if dirpath!="" and not os.path.exists(dirpath):
        os.makedirs(dirpath)
    shutil.move("tmpfile", filename)
    return filename

def innertext(tag):
  return (tag.text or '') + ''.join(innertext(e) for e in tag) + (tag.tail or '')

def dumpmoviepage(page):
    url = "http://www.gewara.com/movie/searchMovieStore.xhtml?pageNo=%d&order=releasedate&movietime=all"%(page)
    filename = download_file(url, "temp.html")
    tree = etree.parse(filename, parser)
    for node in tree.xpath("//div[@class='ui_text']"):
        movie = {}
        anode =  node.xpath(".//a")[0]
        pnodes = node.xpath(".//p")
    
        movie["id"] = anode.attrib["href"][7:]
        movie["name"] = anode.attrib["title"]
        movie["date"] = pnodes[0].text[5:].strip()
        movie["type"] = pnodes[1].text[5:].strip()
        movie["nation"] = pnodes[2].text[6:].strip()
        movie["language"] = pnodes[3].text[3:].strip()
        movie["length"] = pnodes[4].text[3:].strip()
        movie["director"] = pnodes[5].text[3:].strip()
        movie["actor"] = pnodes[6].text[3:].strip()
        movie["hot"] = node.xpath(".//span[@data-keynum='%s_clickedtimes']"%(movie["id"]))[0].text.strip()
        movie["grade"] = node.xpath(".//sub[@data-keynum='%s_mark1']"%(movie["id"]))[0].text.strip() + node.xpath(".//sup[@data-keynum='%s_mark2']"%(movie["id"]))[0].text.strip()
        print json.dumps(movie, ensure_ascii=False, sort_keys=True, encoding="utf-8")

#for i in range(671):
#    dumpmoviepage(i)
#    time.sleep(0.1)
#dumpmoviepage(0)

def dumpcinemapage(page):
    url = "http://www.gewara.com/shanghai/cinemalist?pageNo=%d"%(page)
    filename = download_file(url, "temp.html")
    tree = etree.parse(filename, parser)
    for node in tree.xpath("//div[@class='ui_text']"):
        cinema = {}
        anodes =  node.xpath(".//a")
        cinema["id"] = anodes[0].attrib["href"][8:]
        cinema["name"] = anodes[0].attrib["title"]
        cinema["region"] = anodes[1].text.strip("[]")
        cinema["address"] = anodes[1].tail.strip()
        print json.dumps(cinema, ensure_ascii=False, sort_keys=True, encoding="utf-8")

#for i in range(19):
#    dumpcinemapage(i)
#    time.sleep(0.1)

def dumpprice(date, mid, cid):
    tickets = []
    try:
        url = "http://www.gewara.com/movie/v5/getCommonOpiItem.xhtml?fyrq=%s&movieid=%s&cid=%s"%(date, mid, cid)
        filename = download_file(url, "temp.html")
        tree = etree.parse(filename, parser)
        for node in tree.xpath("//ul[@class='clear']/li"):
            ticket = {}
            ticket["cid"] = cid
            ticket["time"] = node.xpath("./span[@class='td opiTime']/b")[0].text.strip()
            ticket["edition"] = node.xpath("./span[@class='td opiEdition']/b")[0].text.strip()
            ticket["room"] = innertext(node.xpath("./span[@class='td opiRoom']/b")[0]).strip()
            ticket["price"] = node.xpath("./span[@class='td opiPrice']/b")[0].text.strip()
            tickets.append(ticket)
    except:
        pass
    return tickets

def getcinemalist(date, mid):
    url = "http://www.gewara.com/movie/v5/getCurAllCinemaList.xhtml?playDate=%s&movieid=%s"%(date, mid)
    filename = download_file(url, "temp.html")
    tree = etree.parse(filename, parser)
    cinemalist=[]
    for node in tree.xpath("//ul[@id='wrapped_splayList']/li"):
        cinemalist.append(node.attrib["cid"])
    return cinemalist
    

def loadcinemafromjson():
    cinemas = {}
    with open('cinema.json') as data_file:    
        data = json.load(data_file)
    for cinema in data:
        cinemas[cinema["id"]] = cinema
    return cinemas


cinemas = loadcinemafromjson()
cinemasids = getcinemalist("2015-05-19", "207295462")

minprice = 1000
mintickets = []

for cid in cinemasids:
    tickets = dumpprice("2015-05-19", "207295462", cid)
    cinema = cinemas[cid]
    for ticket in tickets:
        print "%s\t%s\t%s\t%s\t%s\t%s"%(cinema["region"], cinema["name"], cinema["address"], ticket["price"], ticket["room"], ticket["time"])
        '''
        price = int(ticket["price"])
        if price < minprice:
            mintickets = [ticket]
            minprice = price
        elif price == minprice:
            mintickets.append(ticket)
        '''
#for minticket in mintickets:
#    print json.dumps(cinemas[minticket["cid"]], ensure_ascii=False, sort_keys=True, encoding="utf-8")
#    print json.dumps(minticket, ensure_ascii=False, sort_keys=True, encoding="utf-8")