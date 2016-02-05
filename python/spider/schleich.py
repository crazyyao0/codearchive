#! python
# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

from lxml import etree
import urllib
import urllib2
import json

parser = etree.HTMLParser()
urlopener = urllib.URLopener()

def translate(s):
    postdata=urllib.urlencode({'f':'en',
                               't':'zh',
                               'w':s})
    req = urllib2.Request(  
        url = 'http://fy.iciba.com/ajax.php?a=fy',
        data = postdata,  
        headers = {
            'Content-Type':'application/x-www-form-urlencoded; charset=UTF-8',
            'Referer':'http://fy.iciba.com/'
        }
    )
    
    res = urllib2.urlopen(req).read()
    obj = json.loads(res)    
    if obj['status'] == 0:
        for msg in obj['content']['word_mean']:
            print msg
    elif obj['status'] == 1:
        print obj['content']['out']

translate("Gorilla")

'''
links = ['http://www.schleich-s.com/en/US/new-products/2015-12/wild_life/']
for link in links:
    print link
    tree = etree.parse(link, parser)
    for node in tree.xpath("//a[@class='thumb-nail']"):
        title = node.attrib['title']
        title = title.split(' ')
        id = title[-1]
        name = ' '.join(title[0:-1])
        picurl = "http://www.schleich-s.com/fileadmin/media/images/item_detail/%s.jpg"%(id)
        urlopener.retrieve(picurl, "schleich/%s, %s.jpg"%(id, name))
        print "%s, %s"%(id, name)
'''