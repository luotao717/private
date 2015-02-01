<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<script language="javascript" src="js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="JavaScript" type="text/javascript">
function style_display_on()
{
	if (window.ActiveXObject) { // IE
		return "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari,...
		return "table-row";
	}
}

function getNavigator()      
{      
    var navigatorType = navigator.userAgent;
	if (navigatorType.indexOf("MSIE")>0)  return "MSIE";//IE       
	if (navigatorType.indexOf("Firefox")>0)  return "Firefox";//Firefox       
	if (navigatorType.indexOf("Safari")>0)  return "Safari";//Safan       
	if (navigatorType.indexOf("Chrome")>0)  return "Chrome";//Camino       
	if (navigatorType.indexOf("Gecko")>0)  return "Gecko";//Gecko 
	if (navigatorType.indexOf("Opera")>0)  return "Opera";//Opera 
	if (navigatorType.indexOf("AppWebKit")>0)  return "AppWebKit";//AppWebKit  
}

var http_request = false;
function changeLanguage(value)
{
    http_request = false;
	var navigatorType = getNavigator(); 
    if (navigatorType == "MSIE"){   
        http_request = new ActiveXObject("Microsoft.XMLHTTP");  //IE
    }
	else{   
        http_request = new XMLHttpRequest();   
    }   
    http_request.onreadystatechange = doChange;
	http_request.open('POST', '/goform/setLanguage', true);
	http_request.send('langType='+value);
}

function doChange()
{
    if (http_request.readyState == 4){
		if (http_request.status == 200){
			top.location.reload();
		} else {
			//alert(JS_msg6);
		}
	}
}

function updateLanguage(val)
{
	changeLanguage(val);
}

function initValue()
{
	var lang = "<% getCfgGeneral(1, "LanguageType"); %>";
	if (lang == "en") {
		document.getElementById("bk_img").src ="images/ctb_01_en.png";
	}
	else
		document.getElementById("bk_img").src ="images/ctb_01.png";
}

function closeWindow()
{
	//window.opener=null;
	//alert(window.opener);
	//if(window.opener){ 
	//	window.opener.location.reload(true); 
	//	window.close(); 
	//} 
	//document.window.close();
	//alert("11111");
	//window.open('','_parent','');
　　// window.close();
	window.parent.location.href = 'about:blank '; 

}
</script>
<style type="text/css">
<!--
.logod {
	height: 124px;
	width: 1204px;
	background-image: url(images/logo.jpg);
	background-repeat: no-repeat;
	margin: auto;
}
.ctb a {
	color: #333333;
	font-weight: bold;
	text-decoration: none;
}
.foot {
	margin: auto;
	height: 67px;
	width: 835px;
	text-align: center;
}
.foot .d1 {
	margin: 10px;
	font-weight: bold;
	color: #666666;
	font-size: 16px;
	letter-spacing: 3px;
}
.foot .d2 {
	font-size: 12px;
	color: #666666;
}
.ctb .menua {
	color: #666666;
	font-size: 12px;
	font-weight: normal;
}
-->
</style>
</head>
<body bgcolor="#FFFFFF" leftmargin="0" topmargin="0" marginwidth="0" marginheight="0" onLoad="initValue()">
<!-- ImageReady Slices (未标题-1) -->
<div class="logod"></div>
<table width="835" height="555" border="0" align="center" cellpadding="0" cellspacing="0" background="images/ctbbg.jpg" class="ctb">
	<tr>
		<td width="835" height="117" align="center">
			<img id="bk_img" src="images/ctb_01.png" width="591" height="48" alt=""></td>
	</tr>
	<tr>
		<td height="59">
			<table width="835" height="59" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td width="161" height="59">&nbsp;</td>
		<td width="158" align="center">
			<a href="#"><script>dw(CHI_index)</script></a></td>
		<td width="9">
			<img src="images/ctb_02_03.png" width="9" height="59" alt=""></td>
		<td width="248" align="center">
			<span id="english"><a href="javascript:updateLanguage('cn')" id="cn" class="title_link"><script>dw(MM_chinese)</script></a>/<a href="javascript:updateLanguage('en')" id="en" class="title_link"><script>dw(MM_english)</script></a></span></td>
		<td width="10">
			<img src="images/ctb_02_05.png" width="10" height="59" alt=""></td>
		<td width="125" align="center">
			<a href="#" onClick="closeWindow()"><script>dw(CHI_exit)</script></a></td>
		<td width="124">&nbsp;</td>
	</tr>
</table></td>
	</tr>
	<tr>
		<td height="379" align="center">
			<table width="574" height="234" border="0" cellpadding="0" cellspacing="0">
	<tr>
		<td width="67" height="89" align="center" valign="top">
			<a href="#" class="menua" onClick="top.location.href='quicksetupI.asp'"><img src="images/ctb_03_01.jpg" alt="" width="67" height="66" border="0"><script>dw(MM_easywizard)</script></a></td>
		<td width="102">&nbsp;</td>
		<td width="65" align="center" valign="top">
			<a href="#" class="menua" onClick="top.location.href='internetI.asp'"><img src="images/ctb_03_03.jpg" alt="" width="65" height="66" border="0"><script>dw(CHI_internet)</script></a></td>
		<td width="104">&nbsp;</td>
		<td width="66" align="center" valign="top">
			<a href="#" class="menua" onClick="top.location.href='wifiI.asp'"><img src="images/ctb_03_05.jpg" alt="" width="66" height="67" border="0"><script>dw(CHI_wifi)</script></a></td>
		<td width="94">&nbsp;</td>
		<td width="76" align="center" valign="top">
			<a href="#" class="menua" onClick="top.location.href='firewallI.asp'"><img src="images/ctb_03_07.jpg" alt="" width="68" height="66" border="0"><script>dw(MM_firewall)</script></a></td>
	</tr>
	<tr>
		<td height="56">&nbsp;</td>
		<td>&nbsp;</td>
		<td>&nbsp;</td>
		<td>&nbsp;</td>
		<td>&nbsp;</td>
		<td>&nbsp;</td>
		<td>&nbsp;</td>
	</tr>
	<tr align="center">
		<td width="67" height="89" valign="top"><a href="#" class="menua" onClick="top.location.href='storageI.asp'"><img src="images/ctb_03_15.png" alt="" width="67" height="65" border="0" /><script>dw(MM_storage)</script></a></td>
		<td>&nbsp;</td>
		<td valign="top" width="65">
			<a href="#" class="menua" onClick="top.location.href='serviceI.asp'"><img src="images/ctb_03_17.png" alt="" width="65" height="66" border="0"><script>dw(MM_service_settings)</script></a></td>
		<td>&nbsp;</td>
		<td valign="top" width="66">
			<a href="#" class="menua" onClick="top.location.href='systemI.asp'"><img src="images/ctb_03_19.png" alt="" width="66" height="66" border="0"><script>dw(CHI_system)</script></a></td>
		<td>&nbsp;</td>
		<td valign="top" width="68">
			<a href="#" class="menua" onClick="top.location.href='sysstatusI.asp'"><img src="images/ctb_03_21.png" alt="" width="68" height="65" border="0"><script>dw(MM_sysstatus)</script></a></td>
	</tr>
</table></td>
	</tr>
</table>
<div class="foot"><div class="d1"> 深圳大家电气有限公司智臻出品</div>
<div class="d2"> Shenzhen people Electric Co.Ltd.Zhi Zhen product</div></div>
<!-- End ImageReady Slices -->
</body>
</html>