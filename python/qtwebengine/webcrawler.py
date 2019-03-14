#!/usr/bin/python
# -*- coding: utf-8 -*-
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QUrl
from PyQt5.QtWebEngineWidgets import QWebEngineView as QWebView
from PyQt5.QtWebEngineWidgets import QWebEnginePage as QWebPage

class WebPage(QWebPage):
    def javaScriptConsoleMessage(self, message, lineNumber, sourceID):
        sys.stderr.write('Javascritp error at line number %d\n' % (lineNumber))
        sys.stderr.write('%s\n' % (message, ))
        sys.stderr.write('Source ID: %s\n' % (sourceID, ))

class Crawler(QApplication):
    
    def __init__(self, url):
        super(Crawler, self).__init__(sys.argv)
        self.url = url
        self.web_view = QWebView()
        self.web_page = WebPage()
        self.web_view.loadFinished.connect(self.loadFinished)
        self.web_view.setPage(self.web_page)
        self.web_page.load(QUrl(self.url))
        
    def loadFinished(self):
        print ("on_webview_loadFinished")
        self.web_page.runJavaScript('"Hello world";', lambda v: print(v))


if __name__ == '__main__':
    # url = 'http://product.dangdang.com/product.aspx?product_id=22848151'
    url = 'http://www.baidu.com/'
    crawler = Crawler(url)
    sys.exit(crawler.exec_())