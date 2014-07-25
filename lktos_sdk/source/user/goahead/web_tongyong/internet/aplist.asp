<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../style/normal_ws.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js">


function selectall()
{
	for(var i=0;i<document.all.length;i++)
	{
		if (document.all(i).type=="checkbox")
			document.all(i).checked=true;
	}
   
}
function selectallno()
{
   for(var i=0;i<document.all.length;i++)
	{
		if (document.all(i).type=="checkbox")
			document.all(i).checked=false;
	}
}

function encrypChange()
{
	if (document.sendcmd.encryp.selectedIndex == 1){
		document.getElementById("wpa2text").style.display = "";
//		document.sendcmd.encryp.value = "255.255.255.255";
//		document.addrouting.netmask.readOnly = true;
	}
	else{
		document.getElementById("wpa2text").style.display = "none";
		document.addrouting.netmask.value = "NONE";
		document.addrouting.netmask.readOnly = true;
	}
}
</script>
</head>


<body>
<table width=600><tr><td>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
<form method="post" name="sendcmd" action="/goform/setSendCmd">
<tr><td class="title">AP <script>dw(MM_client_list)</script></td></tr>
<tr><td><script>dw(JS_msg_dhcpinfo)</script></td></tr>
<tr>
	<td class="thead"><script>dw(MM_ssid)</script>:</td>
	<td><input type="text" name="ssid" maxlength="15"></td>
</tr>
<tr>
	<td class="thead"><script>dw(MM_encryp_type)</script>:</td>
	<td><select name="encryp" onChange="encrypChange()">
	<option value="host"><script>dw(MM_disable)</script></option>
	<option value="net">WPA2-PSK(AES)</option></select></td>
	<td id="wpa2text">
	<input type="text" name="encryptext" maxlength="15"></td>
</tr>
<tr>
	<td class="thead"><script>dw(MM_username)</script>:</td>
	<td><input type="text" name="login" maxlength="15"></td>
</tr>
<tr>
	<td class="thead"><script>dw(MM_password)</script>:</td>
	<td><input type="text" name="password" maxlength="15"></td>
</tr>
</tr>
<tr><td><hr></td></tr>
</table>
<br>
<br>
<table width=100% border=1 cellpadding=4 style="border-collapse: collapse" bordercolor="#C8C8C8">
  <tr bgcolor=#f2f2f2>
    <td align=center><b><script>dw(MM_Index)</script></b></td>  
    <td align=center><b><script>dw(MM_hostname)</script></b></td>
    <td align=center><b><script>dw(MM_macaddr)</script></b></td>
    <td align=center><b><script>dw(MM_ipaddr)</script></b></td>
  </tr>
  <% getApScanList(); %>
</table>

<br>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td>
      <script>dw('<input type=button class=button value="'+BT_refresh+'" onClick="window.location.reload()"> &nbsp; &nbsp;\
	  <input type=button class=button value="'+BT_checkall+'" onClick="selectall()"> &nbsp; &nbsp;\
	  <input type=button class=button value="'+BT_checkallno+'" onClick="selectallno()"> &nbsp; &nbsp;\	  
      <input type=button class=button value="'+BT_close+'" onClick="window.close();">')</script>
    </td>
  </tr>
</table>

</td></tr></table>
</body></html>
