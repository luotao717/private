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
<tr><td class="pgTitle"  height="34" colspan="2"> <script>dw(MM_firmware)</script></td></tr>
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
<tr>
  	<td class="pgleft"></td>
    <td class="pgRight">
      <script>dw('<input type="submit" value="'+BT_upgrade+'" class=button name="UploadFirmwareSubmit" onClick="return uploadFirmwareCheck();"> &nbsp; &nbsp;\
      <input type=button class=button value="'+BT_reset+'" onClick="resetForm();">')</script>
    </td>
  </tr>
</table>
</form>
</td></tr></table>
<table width="800" height="600" border="0" cellpadding="0" cellspacing="0" bgcolor="#cae9fa">
<tr>
<td>
</td>
</tr>
</table>
</body></html>
