bilibili
===========================

000
---------------------------
事情的起因是这样的。我比较喜欢美国的一个科技类节目《流言终结者》。Adam and Jamie, I'm your fans. 因为实在太喜欢了，就动了念头想把整个剧集都下载下来永久收藏。百度了一下发现B站上还比较完整，一共有248集。网上搜索了一下说可以在浏览器缓存里找到视频文件。但是你需要一集一集点过来并且耐心等待他放完。200多集每集40分钟，这要多久呀，有没有什么别的方法呢？

001
---------------------------
打开开发者工具抓了所有的http请求得到了如下的列表。原本有140多个request, 去掉图片，css以及一些已知的js库，再去掉刷新后可以从cache里直接加载的文件，剩下的没有几个了。

![bilibili_files](chrome_capture1.png)

如图所示最后一个就是flv文件的url。带了很多参数。

```  http://cn-jsks5-dx.acgvideo.com/vg8/d/e4/7850017-1.flv?expires=1488864300&platform=pc&ssig=RY59o9L6MfFHS_geAl4c5w&oi=1960979570&nfa=B2jsoD9cEoAmG7KPYo7s2g==&dynamic=1&rnd=0%2E013287325855344534
```

搜索了一下，这个url，发现他可以在下面这个请求所返回的xml文件里找到。

```
https://interface.bilibili.com/playurl?sign=b3affbb1155fe02b8be69028e407e2c9&cid=7850017&player=1&ts=1488850040
```

```xml
<?xml version="1.0" encoding="UTF-8"?>
<video>
	<result>suee</result>
	<timelength>2819262</timelength>
	<format>flv</format>
	<accept_format><![CDATA[mp4,hdmp4,flv]]></accept_format>
	<accept_quality><![CDATA[3,2,1]]></accept_quality>
	<from><![CDATA[local]]></from>
	<seek_param><![CDATA[start]]></seek_param>
	<seek_type><![CDATA[offset]]></seek_type>
	<durl>
		<order>1</order>
		<length>334411</length>
		<size>80927745</size>
		<url><![CDATA[http://cn-jsks5-dx.acgvideo.com/vg8/d/e4/7850017-1.flv?expires=1488864300&platform=pc&ssig=RY59o9L6MfFHS_geAl4c5w&oi=1960979570&nfa=B2jsoD9cEoAmG7KPYo7s2g==&dynamic=1]]></url>
		<backup_url>
		<url><![CDATA[http://cn-zjwz2-dx-v-03.acgvideo.com/vg2/e/5f/7850017-1.flv?expires=1488864300&platform=pc&ssig=JePHX8BUu6sVHLoJXzZhTQ&oi=1960979570&nfa=B2jsoD9cEoAmG7KPYo7s2g==&dynamic=1]]></url>
		<url><![CDATA[http://cn-zjwz2-dx.acgvideo.com/vg17/e/db/7850017-1.flv?expires=1488864300&platform=pc&ssig=OT5gu7rHk22S0J-3wAVZPw&oi=1960979570&nfa=B2jsoD9cEoAmG7KPYo7s2g==&dynamic=1]]></url>
		</backup_url>
	</durl>
	...
<video>
```

这个请求也是从某个flash中发出的，它带的参数少多了。sign可能是一个验证字符串用于判断请求是不是合法。如果修改一下某个参数后就会得到checksum error的错误信息。

![bilibili_files](chrome_capture2.png)

url中的ts应该是当前时间。cid应该是剧集的id。它的值可以在这个页面中找到。

```
http://www.bilibili.com/video/av4836341/
```
```html
<script type='text/javascript'>EmbedPlayer('player', "//static.hdslb.com/play.swf", "cid=7850017&aid=4836341&pre_ad=0");</script>
```

4836341就是aid。也就是流言终结者这部剧的id。这里的javascript代码应该会在网页中嵌入一个flash对象。可以直接从开发者工具-〉elements里找被javascript创建出来的那个对象

![bilibili_files](chrome_capture0.png)

所以现在只要弄明白sign的值是怎么计算的，就可以写脚本来批量下载视频了。

下载那个play.swf (其实还有一个叫StatisticsFromUser.swf也在请求列表中，但是看名字不像就没有去管它)。用JPEXS Free Flash Decompiler打开这个flash文件，可以反编译看见ActionScript的源代码。搜索关键字"sign"，没有直接找到拼接url时类似于"sign="的常量字符串。但找到了两个函数很可疑，getSign和getSign_v2。

```
import com.bilibili.interfaces.getSign;
import com.bilibili.interfaces.getSign_v2;

```

搜索这两个字符串找到所有的调用者。找到一下这些函数。

```
org.lala.utils.PlayerTools
      public function loadCidVideo(param1:String, param2:Object = null) : void
      public function loadBStream(param1:String) : void
      public function loadPlayurl(param1:String) : void
      public function loadPreview(param1:String, param2:Object = null) : void
tv.bilibili.net.ClickService
      private function _send(param1:String, param2:int, param3:Object) : void
```

经过仔细判断鉴别发现最终的那个playurl是由函数loadCidVideo生成的。sign的方法是getSign

```
         var _loc4_:String = this.getCidVideoInfoUrl(param1);
         var _loc5_:Array = _loc4_.split("?");
         var _loc6_:String = getSign(_loc5_[1] + "&player=1");
         var _loc7_:URLRequest = new URLRequest(_loc5_[0] + "?" + _loc6_);
```
getSign接受一个类似于"cid=7850017&ts=1488850040&player=1"字符串， 返回"sign=b3affbb1155fe02b8be69028e407e2c9&cid=7850017&player=1&ts=1488850040"这样的字符串

002
---------------------------


003
--------------------------

The End
