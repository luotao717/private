<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../style/normal_ws.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
</head>
<body>
<table width=600><tr><td>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
<tr><td class="title"> <script>dw(IVT_CLIENTLIST_TITLE)</script></td></tr>
<tr><td><script>dw(IVT_CLIENTLIST_INFO)</script></td></tr>
<tr><td><hr></td></tr>
</table>
<br>
<br>
<table width=100% border=1 cellpadding=3 style="border-collapse: collapse" bordercolor="#C8C8C8">
  <tr bgcolor=#f2f2f2>
    <td align=center><b><script>dw(IVT_CLIENTLIST_WEIXIN)</script></b></td>
    <td align=center><b><script>dw(IVT_CLIENTLIST_CLIENTMAC)</script></b></td>
  </tr>
  <% getWeixinCliList(); %>
</table>

<br>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td>
      <script>dw('<input type=button class=button value="'+BT_refresh+'" onClick="window.location.reload()"> &nbsp; &nbsp;\
      <input type=button class=button value="'+BT_close+'" onClick="window.close();">')</script>
    </td>
  </tr>
</table>

</td></tr></table>
</body></html>
