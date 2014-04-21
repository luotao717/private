<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../images/style.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
document.write('<div id="loading" style="display: none;"><br>'+JS_msg90+'<br></div>');

var _singleton = 0;
function uploadFirmwareCheck()
{
	if (_singleton)
		return false;
		
	if (document.UploadFirmware.filename.value == ""){
		alert(JS_msg10);
		return false;
	}

	//document.UploadFirmware.UploadFirmwareSubmit.disabled = true;
	//document.UploadFirmware.filename.disabled = true;
    document.getElementById("loading").style.display="block";
	_singleton = 1;
	return true;
}

function Load_Setting()
{
    document.UploadFirmware.UploadFirmwareSubmit.disabled = false;
//	document.UploadFirmware.filename.disabled = false;
	document.getElementById("loading").style.display="none";
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
<td>
<table width="760" border=0 cellpadding=0 cellspacing=0 height="60"> 
<tr><td class="pgTitle" height="34"><script>dw(MM_management)</script></td></tr>
<tr><td class="pgHelp"><script>dw(MM_management_msginfo)</script></td></tr>
</table>
</td>
</td>
		<td width="20"></td>
	</tr>
	<tr>
	<td colspan="3" height="10"></td>
	</tr>
	<tr>
		<td width="20"></td>
		<td valign="top">
<form method="post" name="UploadFirmware" action="/cgi-bin/upload.cgi" enctype="multipart/form-data">
<table width="760" border=0 cellpadding=0 cellspacing=0>
<tr>
<td class="pgTitle"  height="34"> <script>dw(MM_firmware)</script></td>
<td class="pgButton" align="right"><script>dw('<input type="submit" value="'+BT_upgrade+'" class=button name="UploadFirmwareSubmit" onClick="return uploadFirmwareCheck();"> &nbsp; &nbsp;\
      <input type=button class=button value="'+BT_reset+'" onClick="resetForm();">')</script></td>
</tr>
<tr><td class="pgHelp" colspan="2"><script>dw(JS_msg_firmware)</script></td></tr> 
<tr>
  	<td class="pgleft"><script>dw(MM_cur_firmware)</script>:</td>
  	<td class="pgRight"><% getSdkVersion(); %></td>
</tr>
<tr>
  	<td class="pgleft"><script>dw(MM_firmware_date)</script>:</td>
  	<td class="pgRight"><script>dw(checkDate("<% getSysBuildTime(); %>"));</script></td>
</tr>
<tr>
    <td class="pgleft"><script>dw(MM_select_firmware_file)</script>:</td>
	<td class="pgRight"><input type="file" name="filename" size="20" maxlength="256"></td>
</tr>

</table>
</form>
</td></tr>
<tr>
	<td colspan="3" height="10"></td>
	</tr>
	<tr>
		<td width="20"></td>
		<td valign="top">
<table width="760" border=0 cellpadding=0 cellspacing=0>
<tr>
<td class="pgTitle"  height="34"> <script>dw(MM_sysconfig)</script></td>
<td class="pgButton" align="right"></td>
</tr>
<tr><td class="pgHelp" colspan="2"><script>dw(JS_msg_settings)</script></td></tr> 
<form method="post" name="ExportSettings" action="/cgi-bin/ExportSettings.sh">
<tr>
  	<td class="pgleft"><script>dw(MM_save_config_file)</script>:</td>
  	<td class="pgRight"><script>dw('<input type="submit" class=button3 value="'+BT_save+'" name="Export">')</script></td>
</tr>
</form>
<form method="post" name="ImportSettings" action="/cgi-bin/upload_settings.cgi" enctype="multipart/form-data">
<tr>
  	<td class="pgleft"><script>dw(MM_update_config_file)</script>:</td>
  	<td class="pgRight"><input type="File" name="filename" size="20" maxlength="256"> <script>dw('<input type=submit class=button3 value="'+BT_update+'" onClick="return reloadfileCheck()">')</script></td>
</tr>
</form>
<form method="post" name="LoadDefaultSettings" action="/goform/LoadDefaultSettings">
<tr>
    <td class="pgleft"><script>dw(MM_load_factory_default)</script>:</td>
	<td class="pgRight"><script>dw('<input type="submit" class=button3 value="'+BT_restore_default+'" name="LoadDefault" onClick="return resetClick()">')</script></td>
</tr>
</form>

<form method="post" name="RebootSystem" action="/goform/RebootSystem">
<tr>
    <td class="pgleft"><script>dw(MM_reboot_device_system)</script>:</td>
	<td class="pgRight"><script>dw('<input type="submit" class=button3 value="'+BT_reboot+'" name="Reboot" onClick="return rebootClick()">')</script></td>
</tr>
</form>
</table>
</td></tr>
<tr>
	<td colspan="3" height="10"></td>
	</tr>
	<tr>
		<td width="20"></td>
		<td valign="top">
<form method="post" name="Adm" action="/goform/setSysAdm">
<input type="hidden" name="submit-url" value="/adm/password.asp">
<table width="760" border=0 cellpadding=0 cellspacing=0>
<tr>
<td class="pgTitle"  height="34"> <script>dw(MM_admin_settings)</script></td>
<td class="pgButton" align="right"><script>dw('<input type=submit class=button value="'+BT_apply+'" onClick="return formCheck()"> &nbsp; &nbsp;\
      <input type=button class=button value="'+BT_reset+'" onClick="resetForm();">')</script></td>
</tr>
<tr><td class="pgHelp" colspan="2"><script>dw(JS_msg_password)</script></td></tr> 
<tr>
  	<td class="pgleft"><script>dw(MM_username)</script>:</td>
  	<td class="pgRight"><input type="text" name="admuser" maxlength="16" value="<% getCfgGeneral(1, "Login"); %>"></td>
</tr>
<tr>
  	<td class="pgleft"><script>dw(MM_password)</script>:</td>
  	<td class="pgRight"><input type="password" name="admpass" maxlength="32" value="<% getCfgGeneral(1, "Password"); %>"></td>
</tr>
<tr id="div_watchdog" style="display:none">
    <td class="pgleft">WatchDog:</td>
	<td class="pgRight"><input type="radio" name="admwatchdog" value="1"><script>dw(MM_enable)</script>
    <input type="radio" name="admwatchdog" value="0"><script>dw(MM_disable)</script></td>
</tr>

</table>
</form>
</td></tr>
</table>
<table width="800" height="300" border="0" cellpadding="0" cellspacing="0" bgcolor="#cae9fa">
<tr>
<td>
</td>
</tr>
</table>
</body></html>
