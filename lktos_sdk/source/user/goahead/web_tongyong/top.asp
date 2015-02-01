<html>
<head>
<title>title</title>
<meta http-equiv="content-type" content="text/html;charset=iso-8859-1" />
<style type="text/css">
body {
	//background:url(top_bg.jpg) no-repeat;
	margin-left: 0px;
	margin-top: 0px;
	margin-right: 0px;
	margin-bottom: 0px;
}
</style>
<script language="JavaScript" type="text/javascript">
function initValue()
{
	var lang = "<% getCfgGeneral(1, "LanguageType"); %>";
	if (lang == "en") {
		document.getElementById("bk_img").src ="top_bg_en.jpg";
	}
	else
		document.getElementById("bk_img").src ="top_bg.jpg";
}
</script>
</head>

<body onLoad="initValue()">
<table cellpadding="0" cellspacing="0" border="0" width="1024" height="164">
<tr>
<td align="center"><img id="bk_img" src="top_bg.jpg" style="margin:auto;height:auto;width:auto"></td>
</tr>
</table>
</body>
</html>
