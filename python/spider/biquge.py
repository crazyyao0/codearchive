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
import HTMLParser



socket.setdefaulttimeout(10)
urlopener = urllib.URLopener()
parser = etree.HTMLParser(encoding='gbk')
htmlparser = HTMLParser.HTMLParser()



def download_file(url, filename):
    a,b = urlopener.retrieve(url, "tmpfile")
    content_type = b.dict["content-type"].split(";")[0].lower()
    dirpath = os.path.dirname(filename)
    if dirpath!="" and not os.path.exists(dirpath):
        os.makedirs(dirpath)
    shutil.move("tmpfile", filename)
    return filename

file = open("biquge/lgqm.txt", "wb")

tree = etree.parse("http://www.biquge.la/book/416/", parser)
for node in tree.xpath("//div[@id='list']//a"):
    url = "http://www.biquge.la/book/416/" + node.attrib["href"]
    retry = 1;
    while True:
        try:
            contenttree = etree.parse(url, parser)
            title = contenttree.xpath("//h1")[0].text
            content = contenttree.xpath("//div[@id='content']")[0]
            text = htmlparser.unescape(content.text + '\r\n') + '\r\n'.join(htmlparser.unescape(e.tail or '') for e in content)
            
            file.write(title + "\r\n\r\n")
            file.write(text + "\r\n\r\n")
            print title
            break;
        except:
            print "Error parsing: %s, retry %d"%(url, retry)
            retry = retry + 1
            
    

