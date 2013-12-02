<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<script language="javascript">
function formCheck()
{
	
	return true;
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
<tr><td class="title">自动认证</td></tr>
<tr><td>请填入手机号后点击按钮</td></tr>
<tr><td><hr></td></tr>
</table>

<form method="post" name="DDNS" action="/goform/clientAuth">
<input type="hidden" name="submit-url" value="/home.asp">
<table width=100% border=0 cellpadding=3 cellspacing=1> 

<tr>
  <td class="thead">手机号:</td>
  <td><input type="text" name="Account" value=""> </td>
</tr>
</table>

<br>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td>
      <input type=submit class=button value="提交" onClick="return formCheck()"> &nbsp; &nbsp;
      <input type=button class=button value="重置" onClick="resetForm();">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</blockquote>
</body></html>
