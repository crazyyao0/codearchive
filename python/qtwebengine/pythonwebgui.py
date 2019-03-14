import sys
import threading
import time
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import QUrl, QBuffer, QByteArray, QIODevice
from PyQt5.QtWebEngineWidgets import QWebEngineView, QWebEnginePage, QWebEngineProfile
from PyQt5.QtWebEngineCore import QWebEngineUrlRequestInterceptor, QWebEngineUrlSchemeHandler, QWebEngineUrlRequestJob


def threadfunc():
    global browser
    for i in range(10):
        browser.page().runJavaScript('"Hello world!"', lambda v: print(v))
    
def on_webview_loadFinished():
    print ("on_webview_loadFinished")
    #threading.Thread(target=threadfunc, name='LoopThread').start()

class MyWebEnginePage(QWebEnginePage):
    def __init__(self, parent=None):
        super().__init__(parent)
        
    def javaScriptConsoleMessage(self, level, message, lineno, sourceid):
        print(level, message, lineno)
        
class WebEngineUrlRequestInterceptor(QWebEngineUrlRequestInterceptor):
    def __init__(self, parent=None):
        super().__init__(parent)

    def interceptRequest(self, info):
        print("interceptRequest: " + str(info.requestUrl()))
        
class WebEngineUrlSchemeHandler(QWebEngineUrlSchemeHandler):
    def __init__(self, parent=None):
        super().__init__(parent)

    def requestStarted(self, request):
        print("requestStarted: " + str(request.requestUrl()))
        request.fail(QWebEngineUrlRequestJob.RequestAborted)


app = QApplication(sys.argv)
page = MyWebEnginePage()
page.profile().setRequestInterceptor(WebEngineUrlRequestInterceptor(page))
page.profile().installUrlSchemeHandler(b"http", WebEngineUrlSchemeHandler(page))
page.setHtml('''<html><head>
<meta http-equiv="Access-Control-Allow-Origin" content="*">
<script type="text/javascript">
window.onload = function(){
oReq = new XMLHttpRequest();
oReq.open("GET", "http://localhost/event/onload", true);
oReq.send(null);
};
</script>
</head>
<body>Hello World!</body></html>''')

browser = QWebEngineView()
browser.loadFinished.connect(on_webview_loadFinished)
browser.setPage(page)
browser.resize(1024,768)
browser.show()

sys.exit(app.exec_()) 