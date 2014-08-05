<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../style/normal_ws.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
var http_request = false;
function autoTestStartSubmit()
{
    http_request = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_request = new XMLHttpRequest();
        if (http_request.overrideMimeType) {
            http_request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_request) {
        alert(JS_msg5);
        return false;
    }
	document.getElementById("layerautoTestStart").style.display = "none";
	document.getElementById("layerautoTestSamba").style.display = "none";
	document.getElementById("layerautoTestTf").style.display = "none";
	document.getElementById("layerautoTestUsbdisk").style.display = "none";
	document.getElementById("layerautoTestWirelessconnect").style.display = "none";
	document.getElementById("layerautoTestWanlink").style.display = "none";
	document.getElementById("layerautoTestStart").style.display = "";
    http_request.onreadystatechange = responseAutoTestStart;
    http_request.open('POST', '/goform/autoTestStart', true);
    http_request.send('n\a');
}

function responseAutoTestStart()
{
    if (http_request.readyState == 4) {
		if (http_request.status == 200) 
		{
			document.getElementById("autoTestStartStr").innerHTML = http_request.responseText;
		} else 
		{
			//alert(JS_msg89);
		}
	}
}


function autoTestSambaSubmit()
{
    http_request = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_request = new XMLHttpRequest();
        if (http_request.overrideMimeType) {
            http_request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_request) {
        alert(JS_msg5);
        return false;
    }
	document.getElementById("layerautoTestStart").style.display = "none";
	document.getElementById("layerautoTestSamba").style.display = "none";
	document.getElementById("layerautoTestTf").style.display = "none";
	document.getElementById("layerautoTestUsbdisk").style.display = "none";
	document.getElementById("layerautoTestWirelessconnect").style.display = "none";
	document.getElementById("layerautoTestWanlink").style.display = "none";
	document.getElementById("layerautoTestStart").style.display = "";
    http_request.onreadystatechange = responseAutoTestStart;
    http_request.open('POST', '/goform/autoTestStart', true);
    http_request.send('n\a');
}

function responseAutoTestStart()
{
    if (http_request.readyState == 4) {
		if (http_request.status == 200) 
		{
			document.getElementById("autoTestStartStr").innerHTML = http_request.responseText;
			autoTestSambaSubmit();
		} else 
		{
			//alert(JS_msg89);
		}
	}
}

function autoTestSambaSubmit()
{
    http_request = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_request = new XMLHttpRequest();
        if (http_request.overrideMimeType) {
            http_request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_request) {
        alert(JS_msg5);
        return false;
    }
	
	document.getElementById("layerautoTestSamba").style.display = "";
    http_request.onreadystatechange = responseAutoTestSamba;
    http_request.open('POST', '/goform/autoTestSamba', true);
    http_request.send('n\a');
}

function responseAutoTestSamba()
{
    if (http_request.readyState == 4) {
		if (http_request.status == 200) 
		{
			document.getElementById("autoTestSambaStr").innerHTML = http_request.responseText;
			autoTestTfSubmit();
		} else 
		{
			//alert(JS_msg89);
		}
	}
}

function autoTestTfSubmit()
{
    http_request = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_request = new XMLHttpRequest();
        if (http_request.overrideMimeType) {
            http_request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_request) {
        alert(JS_msg5);
        return false;
    }
	document.getElementById("layerautoTestTf").style.display = "";
    http_request.onreadystatechange = responseAutoTestTf;
    http_request.open('POST', '/goform/autoTestTf', true);
    http_request.send('n\a');
}

function responseAutoTestTf()
{
    if (http_request.readyState == 4) {
		if (http_request.status == 200) 
		{
			document.getElementById("autoTestTfStr").innerHTML = http_request.responseText;
			autoTestUsbdiskSubmit();
		} else 
		{
			//alert(JS_msg89);
		}
	}
}

function autoTestUsbdiskSubmit()
{
    http_request = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_request = new XMLHttpRequest();
        if (http_request.overrideMimeType) {
            http_request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_request) {
        alert(JS_msg5);
        return false;
    }
	document.getElementById("layerautoTestUsbdisk").style.display = "";
    http_request.onreadystatechange = responseAutoTestUsbdisk;
    http_request.open('POST', '/goform/autoTestUsbdisk', true);
    http_request.send('n\a');
}

function responseAutoTestUsbdisk()
{
    if (http_request.readyState == 4) {
		if (http_request.status == 200) 
		{
			document.getElementById("autoTestUsbdiskStr").innerHTML = http_request.responseText;
			autoTestWirelessconnectSubmit();
		} else 
		{
			//alert(JS_msg89);
		}
	}
}

function autoTestWirelessconnectSubmit()
{
    http_request = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_request = new XMLHttpRequest();
        if (http_request.overrideMimeType) {
            http_request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_request) {
        alert(JS_msg5);
        return false;
    }
	document.getElementById("layerautoTestWirelessconnect").style.display = "";
    http_request.onreadystatechange = responseAutoTestWirelessconnect;
    http_request.open('POST', '/goform/autoTestWirelessconnect', true);
    http_request.send('n\a');
}

function responseAutoTestWirelessconnect()
{
    if (http_request.readyState == 4) {
		if (http_request.status == 200) 
		{
			document.getElementById("autoTestWirelessconnectStr").innerHTML = http_request.responseText;
			autoTestWanlinkSubmit();
		} else 
		{
			//alert(JS_msg89);
		}
	}
}

function autoTestWanlinkSubmit()
{
    http_request = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_request = new XMLHttpRequest();
        if (http_request.overrideMimeType) {
            http_request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_request) {
        alert(JS_msg5);
        return false;
    }
	document.getElementById("layerautoTestWanlink").style.display = "";
    http_request.onreadystatechange = responseAutoTestWanlink;
    http_request.open('POST', '/goform/autoTestWanlink', true);
    http_request.send('n\a');
}

function responseAutoTestWanlink()
{
    if (http_request.readyState == 4) {
		if (http_request.status == 200) 
		{
			document.getElementById("autoTestWanlinkStr").innerHTML = http_request.responseText;
		} else 
		{
			//alert(JS_msg89);
		}
	}
}


function showOpMode()
{
	var opmode = 1* <% getCfgZero(1, "OperationMode"); %>;
	if (opmode == 0)
		document.write(MM_bridge_mode);
	else if (opmode == 1)
		document.write(MM_gateway_mode);
	else if (opmode == 2)
		document.write(MM_wisp_mode);
	else if (opmode == 3)
		document.write(MM_apclient_mode);
}
function checkDate(str)
{
	var month = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
	var week = [MM_sun, MM_mon, MM_tue, MM_wed, MM_thu, MM_fri, MM_sat];
	
	if ((str.substring(4,5)) == " ")
		str = str.replace(" ","");
	else
		str = str;
	
	var t = str.split(" ");	
	for (var j=0; j<12; j++)
	{
		if (t[0] == month[j]) 
			t[0] = j + 1;
	}
	
	return t[2] + "-" + t[0] + "-" + t[1];
}

function Load_Setting()
{
	document.getElementById("layerautoTestStart").style.display = "none";
	document.getElementById("layerautoTestSamba").style.display = "none";
	document.getElementById("layerautoTestTf").style.display = "none";
	document.getElementById("layerautoTestUsbdisk").style.display = "none";
	document.getElementById("layerautoTestWirelessconnect").style.display = "none";
	document.getElementById("layerautoTestWanlink").style.display = "none";
}
</script>
</head>
<body onLoad="Load_Setting()" class="mainbody">
<blockquote>
<table width=700><tr><td>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
<tr><td class="title"><script>dw(MM_AUTOTEST_1)</script></td></tr>
<tr><td><script>dw(MM_AUTOTEST_2)</script></td></tr>
<tr><td><hr></td></tr>
</table>
<br>
<table class="list">
<tr><td class="tcenter"><script>dw(MM_system)</script></td></tr>
<tr><td><table class="list1">

<tr>
  <td class="thead"><script>dw(MM_cur_firmware)</script>:</td>
  <td><% getSdkVersion(); %></td>
</tr>
<tr>
  <td class="thead"><script>dw(MM_firmware_date)</script>:</td>
  <td><script>dw(checkDate("<% getSysBuildTime(); %>"));</script></td>
</tr>
<tr>
  <td class="thead"><script>dw(MM_uptime)</script>:</td>
  <td><% getSysUptime(); %></td>
</tr>
<tr style="display:none">
  <td class="thead"><script>dw(MM_sys_platform)</script>:</td>
  <td><% getPlatform(); %></td>
</tr>
<tr>
  <td class="thead"><script>dw(MM_opmode)</script>:</td>
  <td><script language="javascript">showOpMode();</script></td>
</tr>
<tr>
  <td class="thead"><script>dw(MM_AUTOTEST_WANMAC)</script>:</td>
  <td><% getWanMac(); %></td>
</tr>
<tr>
  <td class="thead"><script>dw(MM_AUTOTEST_LANMAC)</script>:</td>
  <td><% getLanMac(); %></td>
</tr>
<tr>
  <td class="thead"><script>dw(MM_AUTOTEST_WLANMAC)</script>:</td>
  <td><% getWlanCurrentMac(); %></td>
</tr>
<tr>
  <td class="thead"><script>dw(MM_ssid)</script>:</td>
  <td><% getCfgToHTML(1, "SSID1"); %></td>
</tr>
</table></td></tr>


<tr id="div_wan_br"><td class="tcenter"><script>dw('<input type="button" class=button3 value="'+MM_AUTOTEST_9+'" onClick="autoTestStartSubmit();">')</script>
</td></tr>
<tr id="div_wan"><td><table class="list1">
<tr id="layerautoTestStart">
  <td class="thead"><script>dw(MM_AUTOTEST_3)</script>:</td>
  <td><span style="color:#FF0000" id="autoTestStartStr"> </span></td>
</tr>
<tr id="layerautoTestSamba">
  <td class="thead"><script>dw(MM_AUTOTEST_4)</script>:</td>
  <td><span style="color:#FF0000" id="autoTestSambaStr"> </span> </td>
</tr>
<tr id="layerautoTestTf">
  <td class="thead"><script>dw(MM_AUTOTEST_5)</script>:</td>
  <td><span style="color:#FF0000" id="autoTestTfStr"> </span> </td>
</tr>
<tr id="layerautoTestUsbdisk">
  <td class="thead"><script>dw(MM_AUTOTEST_6)</script>:</td>
  <td><span style="color:#FF0000" id="autoTestUsbdiskStr"> </span> </td>
</tr>
<tr id="layerautoTestWirelessconnect">
  <td class="thead"><script>dw(MM_AUTOTEST_7)</script></td>
  <td><span style="color:#FF0000" id="autoTestWirelessconnectStr"> </span> </td>
</tr>
<tr id="layerautoTestWanlink">
  <td class="thead"><script>dw(MM_AUTOTEST_8)</script>:</td>
  <td><span style="color:#FF0000" id="autoTestWanlinkStr"> </span> </td>
</tr>

</table></td></tr>

</table></td></tr>
</table>
<br><br>
</td></tr></table>
</blockquote>
</body></html>
