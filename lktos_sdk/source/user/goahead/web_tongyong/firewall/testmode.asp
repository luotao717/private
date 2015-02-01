<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../style/normal_ws.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
function Load_Setting()
{
	var rm = 1*<% getCfgGeneral(1, "testMode"); %>;
	
	if (rm == 1)
		document.remote.remoteManagementEnabled.options.selectedIndex = 1;
	else
		document.remote.remoteManagementEnabled.options.selectedIndex = 0;
}

function resetForm()
{
	location=location; 
}
</script>
</head>
<body onLoad="Load_Setting()" class="mainbody">
<blockquote>
<table width=700><tr><td>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
<tr><td class="title"><script>dw(MM_testmode_title)</script></td></tr>
<tr><td><script>dw(MM_testmode_info)</script></td></tr>
<tr><td><hr></td></tr>
</table>

<form method=post name="remote" action=/goform/websTestMode>
<input type="hidden" name="submit-url" value="/firewall/remote.asp">
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
	<td class="thead"><script>dw(MM_testmode_title)</script>:</td>
	<td><select name="remoteManagementEnabled">
	<option value=0><script>dw(MM_disable)</script></option>
	<option value=1><script>dw(MM_enable)</script></option>
	</select></td>
  </tr>
  <tr>
	<td class="thead">IPCAM1 IP:</td>
	<td><input name="testIpcam1" maxlength=16 value="<% getCfgGeneral(1, "testIpcam1"); %>"></td>
  </tr>
  <tr>
	<td class="thead">IPCAM2 IP:</script>:</td>
	<td><input name="testIpcam2" maxlength=16 value="<% getCfgGeneral(1, "testIpcam2"); %>"></td>
  </tr>
  <tr>
	<td class="thead">IPCAM3 IP::</td>
	<td><input name="testIpcam3" maxlength=16 value="<% getCfgGeneral(1, "testIpcam3"); %>"></td>
  </tr>
   <tr>
	<td class="thead">IPCAM4 IP::</td>
	<td><input name="testIpcam4" maxlength=16 value="<% getCfgGeneral(1, "testIpcam4"); %>"></td>
  </tr>
</table>

<br>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td>
      <script>dw('<input type=submit class=button value="'+BT_apply+'"> &nbsp; &nbsp;\
      <input type=button class=button value="'+BT_reset+'" onClick="resetForm();">')</script>
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</blockquote>
</body></html>
