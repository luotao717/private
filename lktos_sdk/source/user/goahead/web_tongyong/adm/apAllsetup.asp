<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../style/normal_ws.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
function reloadfileCheck()
{
	var str=document.ImportSettings.filename.value;
	if (str == "") { 
		alert(JS_msg7);
		return false;
	} 
	
	if (str.toLowerCase().indexOf(".dat") == -1) {
		alert(JS_msg8);
		return false;
	}
	return true;
}

function resetClick()
{
	if ( !confirm(JS_msg11) )
		return false;
	else
		return true;
}

function rebootClick()
{
	if ( !confirm(JS_msg87) ) 
		return false;
	else
		return true;
}
</script>
</head>
<body class="mainbody">
<blockquote>
<table width=700><tr><td>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
<tr><td class="title"><script>dw(MM_AP_ALLSET1)</script></td></tr>
<tr><td><script>dw(MM_AP_ALLSET2)</script></td></tr>
<tr><td><hr></td></tr>
</table>
<br>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
<form method="post" name="ExportSettings" action="">
  <tr>
    <td class="thead"><script>dw(MM_AP_ALLSET3)</script>:</td>
    <td><input name="hostname" maxlength=16 value=""><script>dw('<input type="submit" class=button3 value="'+BT_apply+'" name="Export">')</script></td>
  </tr>
</form>

<form method="post" name="RebootSystem" action="">
  <tr>
    <td class="thead"><script>dw(MM_AP_ALLSET4)</script>:</td>
    <td><input name="hostname" maxlength=16 value=""><script>dw('<input type="submit" class=button3 value="'+BT_apply+'" name="Reboot" onClick="return rebootClick()">')</script></td>
  </tr>
</form>
</table>

</td></tr></table>
</blockquote>
</body></html>
