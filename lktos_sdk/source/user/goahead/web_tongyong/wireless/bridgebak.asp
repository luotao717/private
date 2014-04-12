<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="../images/style.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
var apcli_enable = '<% getCfgGeneral(1, "apClient"); %>';
var mode = '<% getCfgGeneral(1, "ApCliAuthMode"); %>';
var enc = '<% getCfgGeneral(1, "ApCliEncrypType"); %>';

var keyid = '<% getCfgGeneral(1, "ApCliDefaultKeyID"); %>';
var key1type = '<% getCfgGeneral(1, "ApCliKey1Type"); %>';
//var key2type = '<% getCfgGeneral(1, "ApCliKey2Type"); %>';
//var key3type = '<% getCfgGeneral(1, "ApCliKey3Type"); %>';
//var key4type = '<% getCfgGeneral(1, "ApCliKey4Type"); %>';

function SecurityModeSwitch()
{
	document.getElementById("div_apcli_key1").style.display = "none";
	document.getElementById("div_aplci_enc").style.display = "none";
	document.getElementById("div_apcli_wpapsk").style.display = "none";
	
	if (document.apcli_form.apcli_mode.options.selectedIndex == 1)//WEP
	{
		document.getElementById("div_apcli_key1").style.display = "";
		document.apcli_form.apcli_default_key.options.selectedIndex = 0;
		document.apcli_form.apcli_enc_open.options.selectedIndex = 1;
	}	
	else if (document.apcli_form.apcli_mode.options.selectedIndex >= 2)//WPAPSK, WPA2PSK
	{
		document.getElementById("div_aplci_enc").style.display = "";
		document.getElementById("div_apcli_wpapsk").style.display = "";
	}
	else 
	{
		document.apcli_form.apcli_default_key.options.selectedIndex = 0;
		document.apcli_form.apcli_enc_open.options.selectedIndex = 0;
	}
}

function Load_Setting()
{
	var bridge_mode = 1*<% getCfgZero(1, "BridgeMode"); %>;
	if (bridge_mode == 0)
		document.apcli_form.bridgeMode[0].checked = true;
	else if(bridge_mode == 1)
		document.apcli_form.bridgeMode[1].checked = true;
	else
		alert("script error");
	//document.apcli_form.bridgeMode.options.selectedIndex = bridge_mode;	
	document.apcli_form.apcliEnbl.options.selectedIndex = apcli_enable;
	
	updateApClient();
	
	if (mode == "OPEN" && enc == "NONE")
		document.apcli_form.apcli_mode.options.selectedIndex = 0;//Disable
	else if (mode == "OPEN" && enc == "WEP")
		document.apcli_form.apcli_mode.options.selectedIndex = 1;//WEP
	else if (mode == "WPAPSK")
		document.apcli_form.apcli_mode.options.selectedIndex = 2;//WPAPSK
	else if (mode == "WPA2PSK")
		document.apcli_form.apcli_mode.options.selectedIndex = 3;//WPA2PSK
	else
		document.apcli_form.apcli_mode.options.selectedIndex = 0;//Disable

	SecurityModeSwitch();
	
	if (enc == "NONE")
		document.apcli_form.apcli_enc_open.options.selectedIndex = 0;//NONE
	else if (enc == "WEP")
	{
		document.apcli_form.apcli_enc_open.options.selectedIndex = 1;//WEP	
		document.apcli_form.apcli_key1type.options.selectedIndex = 0;
	}
	
	if (enc == "TKIP")
		document.apcli_form.apcli_enc.options.selectedIndex = 0;//TKIP
	else if (enc == "AES")
		document.apcli_form.apcli_enc.options.selectedIndex = 1;//AES
}

function check_wep()
{
	if (document.apcli_form.apcli_enc_open.value == "WEP") 
	{
		if (document.apcli_form.apcli_key1.value.length == 0) 
		{
			alert(JS_msg52+'!');
			document.apcli_form.apcli_key1.focus();
			return false;
		}
	}

	if (document.apcli_form.apcli_key1type.options.selectedIndex == 0) 
	{
		if (document.apcli_form.apcli_key1.value.length != 0 && 
			document.apcli_form.apcli_key1.value.length != 5 && 
			document.apcli_form.apcli_key1.value.length != 13) 
		{
			alert(JS_msg53+"!");
			document.apcli_form.apcli_key1.focus();
			return false;
			
			if (!stringCheck(document.apcli_form.apcli_key1, MM_wep_key))
				return false;
		}
	}
	
	return true;
}

function check_wpa()
{
	var keyvalue = document.apcli_form.apcli_wpapsk.value;
	if (keyvalue.length == 0)
	{
		alert(JS_msg16);
		document.apcli_form.apcli_wpapsk.focus();
		return false;
	}
	
	if (keyvalue.length < 8)
	{
		alert(JS_msg17);
		document.apcli_form.apcli_wpapsk.focus();
		return false;
	}
	
	if (!stringCheck(document.apcli_form.apcli_wpapsk, MM_wpa_key))
		return false;
	
	return true;
}

function formCheck()
{
	//if (document.apcli_form.apcliEnbl.options.selectedIndex == 1)
	/*
	if (document.apcli_form.bridgeMode[1].checked == true)
	{
		if (!ssidCheck(document.apcli_form.apcli_ssid, MM_ssid)) 
			return false;
	
		if (document.apcli_form.apcli_bssid.value != '') 
		{
			if (!macCheck(document.apcli_form.apcli_bssid, "(BSSID)"+MM_macaddr))  
				return false;
		}
	
		if (document.apcli_form.apcli_mode.options.selectedIndex == 1)
			return check_wep();
		else if (document.apcli_form.apcli_mode.options.selectedIndex > 1)
			return check_wpa();
	}
	*/
		
	return true;
}

function resetForm()
{
	location=location; 
}

function open_search_ap() 
{ 
	window.open("site_survey.asp", "newwindow", "height=500, width=700, toolbar=no, menubar=no, scrollbars=yes, resizable=yes, location=no, status=no"); 
} 

function updateApClient()
{
	if (document.apcli_form.apcliEnbl.options.selectedIndex == 1)
	{
		document.getElementById("div_apclient").style.display = "";
	/*
		document.apcli_form.apcli_ssid.disabled = false;
		document.apcli_form.apcli_bssid.disabled = false;
		document.apcli_form.apcli_mode.disabled = false;
		document.apcli_form.apcli_enc_open.disabled = false;
		document.apcli_form.search_AP.disabled = false;
		document.apcli_form.apcli_default_key.disabled = false;
		document.apcli_form.apcli_key1type.disabled = false;
		document.apcli_form.apcli_key1.disabled = false;
		document.apcli_form.apcli_key2type.disabled = false;
		document.apcli_form.apcli_key2.disabled = false;
		document.apcli_form.apcli_key3type.disabled = false;
		document.apcli_form.apcli_key3.disabled = false;
		document.apcli_form.apcli_key4type.disabled = false;
		document.apcli_form.apcli_key4.disabled = false;		
		document.apcli_form.apcli_enc.disabled = false;
		document.apcli_form.apcli_wpapsk.disabled = false;
		*/
	}
	else
	{
		document.getElementById("div_apclient").style.display = "none";
	/*
		document.apcli_form.apcli_ssid.disabled = true;
		document.apcli_form.apcli_bssid.disabled = true;
		document.apcli_form.apcli_mode.disabled = true;
		document.apcli_form.apcli_enc_open.disabled = true;
		document.apcli_form.search_AP.disabled = true;
		document.apcli_form.apcli_default_key.disabled = true;
		document.apcli_form.apcli_key1type.disabled = true;
		document.apcli_form.apcli_key1.disabled = true;
		document.apcli_form.apcli_key2type.disabled = true;
		document.apcli_form.apcli_key2.disabled = true;
		document.apcli_form.apcli_key3type.disabled = true;
		document.apcli_form.apcli_key3.disabled = true;
		document.apcli_form.apcli_key4type.disabled = true;
		document.apcli_form.apcli_key4.disabled = true;		
		document.apcli_form.apcli_enc.disabled = true;
		document.apcli_form.apcli_wpapsk.disabled = true;
		*/
	}
}

function opera()
{
	var i;
	var a = window.parent.document.getElementsByTagName('iframe');  
	for (var i=0; i<a.length; i++){  
		if (a[i].name == self.name) {  
			a[i].height = document.body.scrollHeight+"50px"; 
			return;  
		}  
	}  
}

function switch_master_slave(id)
{
	if (document.apcli_form.bridgeMode[0].checked == true)
	{
		document.getElementById("div_apclient").style.display = "none";
	/*
		document.apcli_form.apcli_ssid.disabled = true;
		document.apcli_form.apcli_bssid.disabled = true;
		document.apcli_form.apcli_mode.disabled = true;
		document.apcli_form.apcli_enc_open.disabled = true;
		document.apcli_form.search_AP.disabled = true;
		document.apcli_form.apcli_default_key.disabled = true;
		document.apcli_form.apcli_key1type.disabled = true;
		document.apcli_form.apcli_key1.disabled = true;
		document.apcli_form.apcli_key2type.disabled = true;
		document.apcli_form.apcli_key2.disabled = true;
		document.apcli_form.apcli_key3type.disabled = true;
		document.apcli_form.apcli_key3.disabled = true;
		document.apcli_form.apcli_key4type.disabled = true;
		document.apcli_form.apcli_key4.disabled = true;		
		document.apcli_form.apcli_enc.disabled = true;
		document.apcli_form.apcli_wpapsk.disabled = true;
		*/
	}
	else
	{
		document.getElementById("div_apclient").style.display = "";
	/*
		document.apcli_form.apcli_ssid.disabled = false;
		document.apcli_form.apcli_bssid.disabled = false;
		document.apcli_form.apcli_mode.disabled = false;
		document.apcli_form.apcli_enc_open.disabled = false;
		document.apcli_form.search_AP.disabled = false;
		document.apcli_form.apcli_default_key.disabled = false;
		document.apcli_form.apcli_key1type.disabled = false;
		document.apcli_form.apcli_key1.disabled = false;
		document.apcli_form.apcli_key2type.disabled = false;
		document.apcli_form.apcli_key2.disabled = false;
		document.apcli_form.apcli_key3type.disabled = false;
		document.apcli_form.apcli_key3.disabled = false;
		document.apcli_form.apcli_key4type.disabled = false;
		document.apcli_form.apcli_key4.disabled = false;		
		document.apcli_form.apcli_enc.disabled = false;
		document.apcli_form.apcli_wpapsk.disabled = false;
		*/
	}
}
</script>
</head>
<body onLoad="opera();Load_Setting()">

<table width="800" border="0" cellpadding="0" cellspacing="0" bgcolor="#cae9fa">
<tr>
	<td colspan="3" valign="top" height="11"></td>
</tr>
<tr>
		<td width="20"></td>
		<td valign="top">
		<table width="760" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td class="pgTitle" height="34"><script>dw(MM_opmode)</script></td>
				<td class="pgButton" align="right"></td>
			</tr>
			<tr>
				<td colspan="2" class="pgHelp"><script>dw(JS_msg_wlopmode)</script></td>
			</tr>
		</table>

		</td>
		<td width="20"></td>
</tr>
<tr>
	<td colspan="3" height="10"></td>
</tr>
<form method=post name=apcli_form action="/goform/wirelessBridge">
		<input type="hidden" name="submit-url" value="/wireless/bridge.asp">
		<input type="hidden" name="apcli_channel" value="<% getCfgGeneral(1, "Channel"); %>">
<tr>
		<td width="20"></td>
		<td valign="top">
		<table width="760" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td class="pgTitle" height="34"><script>dw(MM_opmode_crip)</script></td>
				<td class="pgButton" align="right"> <script>dw('<input type=submit class=button value="'+BT_apply+'" onClick="return formCheck()">')</script></td>
			</tr>
		</table>
			
		<table width="760" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td colspan="2" class="pgLable"><input type="radio" name="bridgeMode" value="0"  onClick="switch_master_slave(0)"/> <script>dw(MM_HP_master)</script></td>
			</tr>
			<tr>
				<td colspan="2" class="pgMemo"><script>dw(MM_HP_master_msg)</script></td>
			</tr>
			<tr>
				<td colspan="2" class="pgLable"><input type="radio" name="bridgeMode" value="1"  onClick="switch_master_slave(1)"/> <script>dw(MM_HP_slave)</script></td>
			</tr>
			<tr>
				<td colspan="2" class="pgMemo"><script>dw(MM_HP_slave_msg)</script></td>
			</tr>
		</table>
		<table width="760" border=0 style="display:none"  cellpadding="0" cellspacing="0"> 
  		<tr>
    		<td class="thead"><script>dw(MM_apclient_mode)</script>:</td>
    		<td class="pgRight"><select name="apcliEnbl" onChange="updateApClient()">
				<option value="0"><script>dw(MM_disable)</script></option>
				<option value="1"><script>dw(MM_enable)</script></option>
				</select>
			</td>
  		</tr>
		</table>
		<table id="div_apclient" style="display:none" width="760" border=0 cellpadding="0" cellspacing="0">   
		  <tr> 
			<td class="pgleft"><script>dw(MM_ssid)</script>:</td>
			<td class="pgRight"><input type=text name="apcli_ssid" size=32 maxlength=32 value="<% getCfgToHTML(1, "ApCliSsid"); %>"> <script>dw('<input type=button class=button name="search_AP" value='+BT_search_ap+' onClick="open_search_ap()">')</script></td>
		  </tr>
		  <tr> 
			<td class="pgleft">BSSID (<script>dw(MM_macaddr)</script>):</td>
			<td class="pgRight"><input type=text name="apcli_bssid" size=17 maxlength=17 value="<% getCfgGeneral(1, "ApCliBssid"); %>"> (<script>dw(MM_optional)</script>)</td>
		  </tr>
		  <tr> 
			<td class="pgleft"><script>dw(MM_security_mode)</script>:</td>
			<td class="pgRight"><select name="apcli_mode" onChange="SecurityModeSwitch();">
				<option value="OPEN"><script>dw(MM_disable)</script></option>
				<option value="OPEN">WEP</option>
				<option value="WPAPSK">WPAPSK</option>
				<option value="WPA2PSK">WPA2PSK</option>
				</select></td>
		  </tr>
		  <tr style="display:none">
			<td class="pgleft"><script>dw(MM_encryp_type)</script>:</td>
			<td class="pgRight"><select name="apcli_enc_open">
				<option value="NONE"><script>dw(MM_none)</script></option>
				<option value="WEP">WEP</option></select></td>
		  </tr>
  
		  <tr style="display:none">
			<td class="pgleft">WEP <script>dw(MM_default_key)</script>:</td>
			<td class="pgRight"><select name="apcli_default_key">
				<option value="1"><script>dw(MM_key)</script> 1</option>
				<option value="2"><script>dw(MM_key)</script> 2</option>
				<option value="3"><script>dw(MM_key)</script> 3</option>
				<option value="4"><script>dw(MM_key)</script> 4</option>
				</select></td>
		  </tr>  
		  <!--key 1-->
		  <tr style="display:none">
			<td class="pgleft"><script>dw(MM_encryp_type)</script>:</td>
			<td class="pgRight"><select name="apcli_key1type"> 
				<option value="1"><script>dw(MM_ascii)</script></option>
				<option value="0"><script>dw(MM_hex)</script></option>
				</select></td>
		  </tr>
		  <tr id="div_apcli_key1">
			<td class="pgleft"><script>dw(MM_wep_key)</script>:</td>
			<td class="pgRight"><input name="apcli_key1" maxlength="26" value="<% getCfgGeneral(1, "ApCliKey1Str"); %>"></td>
		  </tr>
  
		  <!--key 2-->
		  <tr style="display:none">
			<td class="pgleft"><script>dw(MM_encryp_type)</script> 2:</td>
			<td class="pgRight"><select name="apcli_key2type"> 
				<option value="1"><script>dw(MM_ascii)</script></option>
				<option value="0"><script>dw(MM_hex)</script></option>
			  </select> <input name="apcli_key2" maxlength="26" value="<% getCfgGeneral(1, "ApCliKey2Str"); %>"></td>
		  </tr>
		  
		  <!--key 3-->
		  <tr style="display:none">
			<td class="pgleft"><script>dw(MM_encryp_type)</script> 3:</td>
			<td class="pgRight"><select name="apcli_key3type"> 
				<option value="1"><script>dw(MM_ascii)</script></option>
				<option value="0"><script>dw(MM_hex)</script></option>
			  </select> <input name="apcli_key3" maxlength="26" value="<% getCfgGeneral(1, "ApCliKey3Str"); %>"></td>
		  </tr>
		  
		  <!--key 4-->
		  <tr style="display:none">
			<td class="pgleft"><script>dw(MM_encryp_type)</script> 4:</td>
			<td class="pgRight"><select name="apcli_key4type"> 
				<option value="1"><script>dw(MM_ascii)</script></option>
				<option value="0"><script>dw(MM_hex)</script></option>
			  </select> <input name="apcli_key4" maxlength="26" value="<% getCfgGeneral(1, "ApCliKey4Str"); %>"></td>
		  </tr>
  
		  <tr id="div_aplci_enc">
			<td class="pgleft"><script>dw(MM_encryp_type)</script>:</td>
			<td class="pgRight"><select name="apcli_enc">
				<option value="TKIP">TKIP</option>
				<option value="AES">AES</option></select></td>
		  </tr>
		  <tr id="div_apcli_wpapsk"> 
			<td class="pgleft"><script>dw(MM_passphrase)</script>:</td>
			<td class="pgRight"><input type=text name="apcli_wpapsk" size=32 maxlength=64 value="<% getCfgGeneral(1, "ApCliWPAPSK"); %>"></td>
		  </tr>
		</table>
		<table width="760" height="300" border="0" cellpadding="0" cellspacing="0" bgcolor="#cae9fa">
<tr>
<td>
</td>
<td>
</td>
</tr>
</table>
		</td>
		<td width="20"></td>
	</tr>	
</form>  
</table>

</body></html>
