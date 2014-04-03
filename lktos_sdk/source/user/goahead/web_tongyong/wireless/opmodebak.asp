<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../images/style.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
function Load_Setting()
{
	var bridge_mode = 1*<% getCfgZero(1, "BridgeMode"); %>;
	var mac_pt = <% getMacPtBuilt(); %>;
	
	document.opMode.bridgeMode.options.selectedIndex = bridge_mode;	
	
	document.getElementById("div_wds_setting").style.display = "none";
	document.getElementById("div_bridge_setting").style.display = "none";
	document.getElementById("div_apclient_setting").style.display = "none";
	
	if (bridge_mode == 1 || bridge_mode == 4)//bridge
		document.getElementById("div_bridge_setting").style.display = "";
	else if (bridge_mode == 2)//apclient
		document.getElementById("div_apclient_setting").style.display = "";
	else if (bridge_mode == 3)//wds
		document.getElementById("div_wds_setting").style.display = "";

	//if (mac_pt == 0)
		//document.opMode.bridgeMode.options[4] = null;
	//document.opMode.bridgeMode.options[2] = null;
	//document.opMode.bridgeMode.options[3] = null;
}

function updateBridge()
{
	document.getElementById("div_wds_setting").style.display = "none";
	document.getElementById("div_bridge_setting").style.display = "none";
	document.getElementById("div_apclient_setting").style.display = "none";
}

function resetForm()
{
	location=location; 
}
</script>
</head>

<body onLoad="Load_Setting()">
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
		<td width="20"></td>
		<td valign="top">
		<form method=post name="opMode" action="/goform/wirelessOpMode">
		<input type="hidden" name="submit-url" value="/wireless/opmodebak.asp">
		<table width="760" border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td class="pgTitle" height="34"><script>dw(MM_opmode_crip)</script></td>
				<td class="pgButton" align="right">
				 <script>dw('<input type=submit class=button value="'+BT_apply+'" name=apply>')</script>
				</td>
			</tr>
			<tr>
    <td class="pgleft"><script>dw(MM_mode)</script>:</td>
    <td class="pgRight"><select name="bridgeMode" onChange="updateBridge()">
        <option value="0"><script>dw(MM_gateway_mode)</script></option>
        <option value="1"><script>dw(MM_bridge_mode)</script></option>
		<!--
        <option value="2"><script>dw(MM_apclient_mode)</script></option>
        <option value="3"><script>dw(MM_wds_mode)</script></option>
        <option value="4"><script>dw(MM_bridge_mode)</script>(<script>dw(MM_mac_passthrough)</script>)</option>
		-->
      </select></td>
  </tr>  
		</table>
		</form>
		</td>
		<td width="20"></td>
	</tr>
	<tr>
		<td width="20"></td>
		<td valign="top">
		<span id="div_wds_setting">
	<iframe width="760" height="0" id="WDS_Page" name="WDS_Page" src="wds.asp" frameborder="0" marginheight="0" scrolling="no"> </iframe>
</span>

<span id="div_bridge_setting">
	<iframe width="760" height="0" id="Bridge_Page" name="Bridge_Page" src="bridge.asp" frameborder="0" marginheight="0" scrolling="no"> </iframe>
</span>

<span id="div_apclient_setting">
	<iframe width="760" height="0" id="APClient_Page" name="APClient_Page" src="apcli.asp" frameborder="0" marginheight="0" scrolling="no"> </iframe>
</span>
		</table>
		</td>
		<td width="20"></td>
	</tr>
</table>
<table width="800" height="600" border="0" cellpadding="0" cellspacing="0" bgcolor="#cae9fa">
<tr>
<td></td>
</tr>
</table>
	
</body>
</html>
