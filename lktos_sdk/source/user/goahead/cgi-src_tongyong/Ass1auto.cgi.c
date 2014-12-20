int main (int argc, char *argv[])
{
	printf("\
	Server: %s\n\
	Pragma: no-cache\n\
	Content-type: text/html\n\n",
	"Ass1AutoDownload");
		//print document header
	printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n\
		<html xmlns=\"http://www.w3.org/1999/xhtml\">\n\
		<head>\n\
		<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n\
		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no\" >\n\
		<title>第一特装机工具</title>\n\
		</head>\n");

	// print body header
	printf("<body>\n");

	//print style
	printf("<style>\n\
/* CSS Document */\n\
*{ margin:0; padding:0;}\n\
body{ font-family:\"微软雅黑\"; max-width:600px; margin:0 auto; }\n\
.addWrap{width:100%%;background:#fff;margin:auto;position:relative;}\n\
.addWrap .swipe{overflow: hidden;visibility: hidden;position:relative;}\n\
.addWrap .swipe-wrap{overflow:hidden;position:relative;}\n\
.addWrap .swipe-wrap > div {float: left;width: 100%%;position:relative;}\n\
.addWrap .swipe-wrap img { width:16.6666666666%%; float:left;}\n\
#position{padding:0;text-align:center; position: absolute; bottom: 5px; width: 100%%;}\n\
#position li{width:7px;height:7px;margin:0 3px;display:inline-block;-webkit-border-radius:5px;border-radius:5px;background-color:#AFAFAF;}\n\
#position li.cur{background-color:#fff;}\n\
.logo-1{ width:100%%; margin-top:8%%; text-align:center;}\n\
.logo-1 img{ width:26%%;}\n\
.logo-2{ color:#39aeb1; font-size:22px; text-align:center; margin:3%% 0 1%% 0;}\n\
.logo-3{ color:#acb4b5; font-size:15px; text-align:center;}\n\
.logo-4{ color:#ffffff; font-size:20px; text-align:center; width:90px; border-radius:5px; margin:6%% auto 2%%; padding:13px 50px; background:#4abe3f;}\n\
.logo-4 a:link{ color:#ffffff; text-decoration: blink;}\n\
.logo-4 a:visited{ color:#ffffff; text-decoration: blink;}\n\
.logo-4.on{ background:#3baa2f; }\n\
.logo-5{ color:#acb4b5; font-size:12px; text-align:center; margin-bottom:10%%; }\n\
.logo-6{ color:#39aeb1; font-size:15px; text-align:center;  }\n\
.logo-6.on{ color:#0b9696; text-decoration:underline;}\n\
.logo-6 img{ width:20px; position:relative; top:4px;}\n\
.logo-7{ color:#545454; font-size:15px; text-align:center; margin:5%% 0 2%%;  }\n\
.text{ border:1px solid #dddddd; border-radius:2px; width:80%%; margin-left:10%%; font-size:28px; color:#4abe3f; padding:10px 0; text-align:center;}\n\
.logo-8{  text-align:center; margin:5%% 0; }\n\
.logo-8 img{ width:85px;}\n\
.logo-9{ color:#545454; font-size:15px; text-align:center;  }\n\
</style>");


	//print body content
	printf("<div class=\"addWrap\">\n\
	<div class=\"swipe\" id=\"mySwipe\">\n\
        <div class=\"swipe-wrap\">\n\
            <div>\n\
            	<img src=\"../images/pic1_1.png\" />\n\
                <img src=\"../images/pic1_2.png\" />\n\
                <img src=\"../images/pic1_3.png\" />\n\
                <img src=\"../images/pic1_4.png\" />\n\
                <img src=\"../images/pic1_5.png\" />\n\
                <img src=\"../images/pic1_6.png\" />\n\
            </div>\n\
            <div>\n\
            	<img src=\"../images/pic2_1.png\" />\n\
            	<img src=\"../images/pic2_2.png\" />\n\
            	<img src=\"../images/pic2_3.png\" />\n\
            	<img src=\"../images/pic2_4.png\" />\n\
            	<img src=\"../images/pic2_5.png\" />\n\
            	<img src=\"../images/pic2_6.png\" />\n\
            </div>\n\
        </div>\n\
        <ul id=\"position\">\n\
          <li class=\"cur\"></li>\n\
          <li class=\"\"></li>\n\
        </ul>\n\
    </div>\n\
</div>\n\
<div class=\"logo-1\"><img src=\"../images/logo.png\" /></div>\n\
<div class=\"logo-2\">第一特装机工具</div>\n\
<div class=\"logo-3\">简洁·绿色·安全·快捷</div>\n\
<div class=\"logo-4\" id=\"logo-4\" onmousedown=\"s_bj()\"><a href='http://192.168.169.1/media/sda1/apk/firstteassitant.apk'>点击下载</a></div>\n\
<div class=\"logo-5\">下载第一特装机工具可提供免费上网</div>\n\
<div class=\"logo-6\" id=\"logo-6\" onmousedown=\"s_link()\"><img src=\"../images/about.png\" /> 关于我们</div>\n");
	
	//print javascript
	printf("\n\
<script>\n\
function Swipe(container,options){var noop=function(){};var offloadFn=function(fn){setTimeout(fn||noop,0)};var browser={addEventListener:!!window.addEventListener,touch:(\"ontouchstart\" in window)||window.DocumentTouch&&document instanceof DocumentTouch,transitions:(function(temp){var props=[\"transformProperty\",\"WebkitTransform\",\"MozTransform\",\"OTransform\",\"msTransform\"];for(var i in props){if(temp.style[props[i]]!==undefined){return true}}return false})(document.createElement(\"swipe\"))};if(!container){return}var element=container.children[0];var slides,slidePos,width;options=options||{};var index=parseInt(options.startSlide,10)||0;var speed=options.speed||300;options.continuous=options.continuous?options.continuous:true;function setup(){slides=element.children;slidePos=new Array(slides.length);width=container.getBoundingClientRect().width||container.offsetWidth;element.style.width=(slides.length*width)+\"px\";var pos=slides.length;while(pos--){var slide=slides[pos];slide.style.width=width+\"px\";slide.setAttribute(\"data-index\",pos);if(browser.transitions){slide.style.left=(pos*-width)+\"px\";move(pos,index>pos?-width:(index<pos?width:0),0)}}if(!browser.transitions){element.style.left=(index*-width)+\"px\"}container.style.visibility=\"visible\"}function prev(){if(index){slide(index-1)}else{if(options.continuous){slide(slides.length-1)}}}function next(){if(index<slides.length-1){slide(index+1)}else{if(options.continuous){slide(0)}}}function slide(to,slideSpeed){if(index==to){return}if(browser.transitions){var diff=Math.abs(index-to)-1;var direction=Math.abs(index-to)/(index-to);while(diff--){move((to>index?to:index)-diff-1,width*direction,0)}move(index,width*direction,slideSpeed||speed);move(to,0,slideSpeed||speed)}else{animate(index*-width,to*-width,slideSpeed||speed)}index=to;offloadFn(options.callback&&options.callback(index,slides[index]))}function move(index,dist,speed){translate(index,dist,speed);slidePos[index]=dist}function translate(index,dist,speed){var slide=slides[index];var style=slide&&slide.style;if(!style){return}style.webkitTransitionDuration=style.MozTransitionDuration=style.msTransitionDuration=style.OTransitionDuration=style.transitionDuration=speed+\"ms\";style.webkitTransform=\"translate(\"+dist+\"px,0)\"+\"translateZ(0)\";style.msTransform=style.MozTransform=style.OTransform=\"translateX(\"+dist+\"px)\"}function animate(from,to,speed){if(!speed){element.style.left=to+\"px\";return}var start=+new Date;var timer=setInterval(function(){var timeElap=+new Date-start;if(timeElap>speed){element.style.left=to+\"px\";if(delay){begin()}options.transitionEnd&&options.transitionEnd.call(event,index,slides[index]);clearInterval(timer);return}element.style.left=(((to-from)*(Math.floor((timeElap/speed)*100)/100))+from)+\"px\"},4)}var delay=options.auto||0;var interval;function begin(){interval=setTimeout(next,delay)}function stop(){delay=0;clearTimeout(interval)}var start={};var delta={};var isScrolling;var events={handleEvent:function(event){switch(event.type){case\"touchstart\":this.start(event);break;case\"touchmove\":this.move(event);break;case\"touchend\":offloadFn(this.end(event));break;case\"webkitTransitionEnd\":case\"msTransitionEnd\":case\"oTransitionEnd\":case\"otransitionend\":case\"transitionend\":offloadFn(this.transitionEnd(event));break;case\"resize\":offloadFn(setup.call());break}if(options.stopPropagation){event.stopPropagation()}},start:function(event){var touches=event.touches[0];start={x:touches.pageX,y:touches.pageY,time:+new Date};isScrolling=undefined;delta={};element.addEventListener(\"touchmove\",this,false);element.addEventListener(\"touchend\",this,false)},move:function(event){if(event.touches.length>1||event.scale&&event.scale!==1){return}if(options.disableScroll){event.preventDefault()}var touches=event.touches[0];delta={x:touches.pageX-start.x,y:touches.pageY-start.y};if(typeof isScrolling==\"undefined\"){isScrolling=!!(isScrolling||Math.abs(delta.x)<Math.abs(delta.y))}if(!isScrolling){event.preventDefault();stop();delta.x=delta.x/((!index&&delta.x>0||index==slides.length-1&&delta.x<0)?(Math.abs(delta.x)/width+1):1);translate(index-1,delta.x+slidePos[index-1],0);translate(index,delta.x+slidePos[index],0);translate(index+1,delta.x+slidePos[index+1],0)}},end:function(event){var duration=+new Date-start.time;var isValidSlide=Number(duration)<250&&Math.abs(delta.x)>20||Math.abs(delta.x)>width/2;var isPastBounds=!index&&delta.x>0||index==slides.length-1&&delta.x<0;var direction=delta.x<0;if(!isScrolling){if(isValidSlide&&!isPastBounds){if(direction){move(index-1,-width,0);move(index,slidePos[index]-width,speed);move(index+1,slidePos[index+1]-width,speed);index+=1}else{move(index+1,width,0);move(index,slidePos[index]+width,speed);move(index-1,slidePos[index-1]+width,speed);index+=-1}options.callback&&options.callback(index,slides[index])}else{move(index-1,-width,speed);move(index,0,speed);move(index+1,width,speed)}}element.removeEventListener(\"touchmove\",events,false);element.removeEventListener(\"touchend\",events,false)},transitionEnd:function(event){if(parseInt(event.target.getAttribute(\"data-index\"),10)==index){if(delay){begin()\
}options.transitionEnd&&options.transitionEnd.call(event,index,slides[index])}}};setup();if(delay){begin()}if(browser.addEventListener){if(browser.touch){element.addEventListener(\"touchstart\",events,false)}if(browser.transitions){element.addEventListener(\"webkitTransitionEnd\",events,false);element.addEventListener(\"msTransitionEnd\",events,false);element.addEventListener(\"oTransitionEnd\",events,false);element.addEventListener(\"otransitionend\",events,false);element.addEventListener(\"transitionend\",events,false)}window.addEventListener(\"resize\",events,false)}else{window.onresize=function(){setup()}}return{setup:function(){setup()},slide:function(to,speed){slide(to,speed)},prev:function(){stop();prev()},next:function(){stop();next()},getPos:function(){return index},kill:function(){stop();element.style.width=\"auto\";element.style.left=0;var pos=slides.length;while(pos--){var slide=slides[pos];slide.style.width=\"100%%\";slide.style.left=0;if(browser.transitions){translate(pos,0,0)}}if(browser.addEventListener){element.removeEventListener(\"touchstart\",events,false);element.removeEventListener(\"webkitTransitionEnd\",events,false);element.removeEventListener(\"msTransitionEnd\",events,false);element.removeEventListener(\"oTransitionEnd\",events,false);element.removeEventListener(\"otransitionend\",events,false);element.removeEventListener(\"transitionend\",events,false);window.removeEventListener(\"resize\",events,false)}else{window.onresize=null}}}}if(window.jQuery||window.Zepto){(function($){$.fn.Swipe=function(params){return this.each(function(){$(this).data(\"Swipe\",new Swipe($(this)[0],params))})}})(window.jQuery||window.Zepto)};\n\
</script>\n\
<script type=\"text/javascript\">\n\
var bullets = document.getElementById('position').getElementsByTagName('li');\n\
var banner = Swipe(document.getElementById('mySwipe'), {\n\
	auto: 2000,\n\
	continuous: true,\n\
	disableScroll:false,\n\
	callback: function(pos) {\n\
		var i = bullets.length;\n\
		while (i--) {\n\
		  bullets[i].className = ' ';\n\
		}\n\
		bullets[pos].className = 'cur';\n\
	}\n\
});\n\
\n\
var first = 0;\n\
function wanDetectSubmit()\n\
	{\n\
		if(0 == first)\n\
		{\n\
			http_request = false;\n\
			if (window.XMLHttpRequest) {\n\
			http_request = new XMLHttpRequest();\n\
			if (http_request.overrideMimeType) {\n\
			http_request.overrideMimeType('text/xml');\n\
		}\n\
		} else if (window.ActiveXObject) {\n\
			try {\n\
				http_request = new ActiveXObject(\"Msxml2.XMLHTTP\");\n\
			} catch (e) {\n\
			try {\n\
				http_request = new ActiveXObject(\"Microsoft.XMLHTTP\");\n\
			} catch (e) {}\n\
			}\n\
			}\n\
			if (!http_request) {\n\
				return false;\n\
			}\n\
			http_request.open('POST', '/goform/setAccess', true);\n\
			http_request.send('n\\a');\n\
			first = first +1;\n\
		}\n\
	}\n\
function s_bj(){\n\
	document.getElementById('logo-4').className = 'on logo-4';\n\
}\n\
\n\
function s_link(){\n\
	document.getElementById('logo-6').className = 'on logo-6';\n\
	location=\"/about.html\";\n\
}\n\
\n\
</script>\n");

printf("<script type=\"text/javascript\">\nsetInterval(\"wanDetectSubmit()\",5000);\n</script>\n");
	//print end
	printf("</body>\n\
</html>\n");
}

