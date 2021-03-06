<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../style/normal_ws.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
var ch1 = "<option value = 1 >1</option>";
var ch2 = "<option value = 2 >2</option>";
var ch3 = "<option value = 3 >3</option>";
var ch4 = "<option value = 4 >4</option>";
var ch5 = "<option value = 5 >5</option>";
var ch6 = "<option value = 6 >6</option>";
var ch7 = "<option value = 7 >7</option>";
var ch8 = "<option value = 8 >8</option>";
var ch9 = "<option value = 9 >9</option>";
var ch10 = "<option value = 10>10</option>";
var ch11 = "<option value = 11>11</option>";
var ch12 = "<option value = 12>12</option>";
var ch13 = "<option value = 13>13</option>";
var ch14 = "<option value = 14>14</option>";

var ch36 = "<option value = 36 >36</option>";
var ch40 = "<option value = 40 >40</option>";
var ch44 = "<option value = 44 >44</option>";
var ch48 = "<option value = 48 >48</option>";
var ch52 = "<option value = 52 >52</option>";
var ch56 = "<option value = 56 >56</option>";
var ch60 = "<option value = 60 >60</option>";
var ch64 = "<option value = 64 >64</option>";
var ch100 = "<option value = 100>100</option>";
var ch104 = "<option value = 104>104</option>";
var ch108 = "<option value = 108>108</option>";
var ch112 = "<option value = 112>112</option>";
var ch116 = "<option value = 116>116</option>";
var ch120 = "<option value = 120>120</option>";
var ch124 = "<option value = 124>124</option>";
var ch128 = "<option value = 128>128</option>";
var ch132 = "<option value = 132>132</option>";
var ch136 = "<option value = 136>136</option>";
var ch140 = "<option value = 140>140</option>";
var ch149 = "<option value = 149>149</option>";
var ch153 = "<option value = 153>153</option>";
var ch157 = "<option value = 157>157</option>";
var ch161 = "<option value = 161>161</option>";
var ch165 = "<option value = 165>165</option>";

var ch34 = "<option value = 34 >34</option>";
var ch38 = "<option value = 38 >38</option>";
var ch42 = "<option value = 42 >42</option>";
var ch46 = "<option value = 46 >46</option>";

function ClientCertUpdate()
{
	makeRequest("/goform/UpdateCert", "client");
}

function KeyCertUpdate()
{
	makeRequest("/goform/UpdateCert", "key");
}

var http_request = false;
function makeRequest(url, content) {
	http_request = false;
	if (window.XMLHttpRequest) { // Mozilla, Safari,...
		http_request = new XMLHttpRequest();
		if (http_request.overrideMimeType) {
			http_request.overrideMimeType('text/xml');
		}
	} else if (window.ActiveXObject) { // IE
		try {
			http_request = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
			http_request = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}
	if (!http_request) {
		alert('Giving up :( Cannot create an XMLHTTP instance');
		return false;
	}

	if (content == "key")
		http_request.onreadystatechange = KeyContents;
	else
		http_request.onreadystatechange = ClientContents;
	http_request.open('POST', url, true);
	http_request.send(content);
}

function ClientContents() 
{
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			ClientCertUpdateHTML(http_request.responseText);
		} else {
			alert('There was a problem with the request.');
		}
	}
}

function KeyContents() 
{
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			KeyCertUpdateHTML(http_request.responseText);
		} else {
			alert('There was a problem with the request.');
		}
	}
}

function ClientCertUpdateHTML(str)
{
	if (str.length > 0) {
		var cert = document.profile_page.cert_client_cert_path;
		cert.options[cert.length] = new Option(str, str);
	}
}

function KeyCertUpdateHTML(str)
{
	if (str.length > 0) {
		var cert = document.profile_page.cert_private_key_path;
		cert.options[cert.length] = new Option(str, str);
	}
}

function Load_Setting()
{
	document.profile_page.network_type.value = <% getStaProfileData(3); %>;
	document.profile_page.power_saving_mode['<%getStaProfileData(4);%>'].checked = true;
	document.profile_page.b_premable_type.value = "<% getStaProfileData(5); %>";
	if (<% getStaProfileData(6); %> == 1)
		document.profile_page.rts_threshold.checked = true;
	document.profile_page.rts_thresholdvalue.value = "<% getStaProfileData(7); %>";
	if (<% getStaProfileData(8); %> == 1)
		document.profile_page.fragment_threshold.checked = true;
	document.profile_page.fragment_thresholdvalue.value = "<% getStaProfileData(9); %>";

	var encryp = <% getStaProfileData(10); %>;
	var auth = <% getStaProfileData(11); %>;
	
	document.profile_page.wep_key_1.value = "<% getStaProfileData(12); %>";
	document.profile_page.wep_key_2.value = "<% getStaProfileData(13); %>";
	document.profile_page.wep_key_3.value = "<% getStaProfileData(14); %>";
	document.profile_page.wep_key_4.value = "<% getStaProfileData(15); %>";
	document.profile_page.wep_key_entry_method.value = <% getStaProfileData(16); %>;
	document.profile_page.wep_key_entry_method.value = <% getStaProfileData(17); %>;
	document.profile_page.wep_key_entry_method.value = <% getStaProfileData(18); %>;
	document.profile_page.wep_key_entry_method.value = <% getStaProfileData(19); %>;
	document.profile_page.wep_key_length.value = <% getStaProfileData(20); %>;
	document.profile_page.wep_key_length.value = <% getStaProfileData(21); %>;
	document.profile_page.wep_key_length.value = <% getStaProfileData(22); %>;
	document.profile_page.wep_key_length.value = <% getStaProfileData(23); %>;

	var keydefaultid = <% getStaProfileData(24); %>;
	if (keydefaultid == 0)
		document.profile_page.wep_default_key.options.selectedIndex = 0;
	else
		document.profile_page.wep_default_key.options.selectedIndex = keydefaultid -1;
	document.profile_page.passphrase.value = "<% getStaProfileData(25); %>";

	document.getElementById("div_power_saving_mode").style.display = "none";
	document.profile_page.power_saving_mode.disabled = true;

	document.getElementById("div_channel").style.display = "none";
	document.profile_page.channel.disabled = true;

	document.getElementById("div_b_premable_type").style.display = "none";
	document.profile_page.b_premable_type.disabled = true;

	// wpa_supplicant
	var keymgmt = <% getStaProfileData(26); %>;
	var eap = <% getStaProfileData(27); %>;

	if (keymgmt == 1) //Rtwpa_supplicantKeyMgmtWPAEAP,
		document.profile_page.cert_auth_type_from_wpa.value = eap;
	else if (keymgmt == 2) //Rtwpa_supplicantKeyMgmtIEEE8021X,
		document.profile_page.cert_auth_type_from_1x.value = eap;

	document.profile_page.cert_client_cert_path.value = "<% getStaProfileData(30); %>";
	document.profile_page.cert_private_key_path.value = "<% getStaProfileData(31); %>";
	document.profile_page.cert_private_key_password.value = "<% getStaProfileData(32); %>";
	document.profile_page.cert_password.value = "<% getStaProfileData(33); %>";
	var tunnel = <% getStaProfileData(34); %>;

	if (eap == 5 ) // Rtwpa_supplicantEAPPEAP,
		document.profile_page.cert_tunnel_auth_peap.value = tunnel;
	else if (eap == 6 ) // Rtwpa_supplicantEAPTTLS,
		document.profile_page.cert_tunnel_auth_ttls.value  = tunnel;

	if (document.profile_page.network_type.value == 1) //infra
	{
		var wpa_supplicantb = "<% getWPASupplicantBuilt(); %>";
		if (wpa_supplicantb == "1" && document.profile_page.security_infra_mode.length <= 4)
		{
			var mode = document.profile_page.security_infra_mode;

	    		mode.options[mode.length] = new Option("WPA-Enterprise", "3");
	    		mode.options[mode.length] = new Option("WPA2-Enterprise", "6");
	    		mode.options[mode.length] = new Option("802.1x", "10");
		}	
	}

	networkTypeChange(auth, encryp);
	RTSThresholdChange();
	FragmentThresholdChange();
}

function getChannel()
{
	var channel = <% getStaAdhocChannel(); %>;
	var wireless_mode = <% getCfgZero(1, "WirelessMode"); %>;

	var bg_channel = channel & 0xFF;
	var a_channel = (channel >> 8) & 0xFF;

	switch (1*wireless_mode)
	{
	case 3: // A/B/G mixed
	case 5: // A/B/G/N mixed
		getBGChannel(bg_channel);
	case 2: // A only
	case 8: // A/N mixed
		getAChannel(a_channel);
		break;
	default:
		getBGChannel(bg_channel);
		break;
	}
	document.profile_page.channel.value = <% getStaProfileData(35); %>;
}

function getBGChannel(channel)
{
	switch (channel)
	{
		case 0:
			document.write(ch1+ch2+ch3+ch4+ch5+ch6+ch7+ch8+ch9+ch10+ch11);
			break;
		case 1:
			document.write(ch1+ch2+ch3+ch4+ch5+ch6+ch7+ch8+ch9+ch10+ch11+ch12+ch13);
			break;
		case 2:
			document.write(ch10+ch11);
			break;
		case 3:
			document.write(ch10+ch11+ch12+ch13);
			break;
		case 4:
			document.write(ch14);
			break;
		case 5:
			document.write(ch1+ch2+ch3+ch4+ch5+ch6+ch7+ch8+ch9+ch10+ch11+ch12+ch13+ch14);
			break;
		case 6:
			document.write(ch3+ch4+ch5+ch6+ch7+ch8+ch9);
			break;
		case 7:
			document.write(ch5+ch6+ch7+ch8+ch9+ch10+ch11+ch12+ch13);
			break;
		default:
			break;
	}
}

function getAChannel(channel)
{
	switch (channel)
	{
		case 0:
			document.write(ch36+ch40+ch44+ch48+ch52+ch56+ch60+ch64+ch149+ch153+ch157+ch161+ch165);
			break;
		case 1:
			document.write(ch36+ch40+ch44+ch48+ch52+ch56+ch60+ch64+ch100+ch104+ch108+ch112+ch116+ch120+ch124+ch128+ch132+ch136+ch140);
			break;
		case 2:
			document.write(ch36+ch40+ch44+ch48+ch52+ch56+ch60+ch64);
			break;
		case 3:
			document.write(ch52+ch56+ch60+ch64+ch149+ch153+ch157+ch161);
			break;
		case 4:
			document.write(ch149+ch153+ch157+ch161+ch165);
			break;
		case 5:
			document.write(ch149+ch153+ch157+ch161);
			break;
		case 6:
			document.write(ch36+ch40+ch44+ch48);
			break;
		case 7:
			document.write(ch36+ch40+ch44+ch48+ch52+ch56+ch60+ch64+ch100+ch104+ch108+ch112+ch116+ch120+ch124+ch128+ch132+ch136+ch140+ch149+ch153+ch157+ch161+ch165);
			break;
		case 8:
			document.write(ch52+ch56+ch60+ch64);
			break;
		case 9:
			document.write(ch34+ch38+ch42+ch46);
			break;
		case 10:
			document.write(ch34+ch36+ch38+ch40+ch42+ch44+ch46+ch48+ch52+ch56+ch60+ch64);
			break;
		default:
			break;
	}
}

function ""
{
	if (window.ActiveXObject) { // IE
		return "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari,...
		return "table-row";
	}
}

function networkTypeChange(auth, encryp)
{
	document.getElementById("div_b_premable_type").style.display = "none";
	document.profile_page.b_premable_type.disabled = true;

	document.getElementById("div_power_saving_mode").style.display = "none";
	document.profile_page.power_saving_mode.disabled = true;

	document.getElementById("div_channel").style.display = "none";
	document.profile_page.channel.disabled = true;

	document.getElementById("div_security_infra_mode").style.display = "none";
	document.profile_page.security_infra_mode.disabled = true;

	document.getElementById("div_security_adhoc_mode").style.display = "none";
	document.profile_page.security_adhoc_mode.disabled = true;
	
	document.getElementById("div_security_encryp_mode").style.display = "none";
	document.profile_page.security_encryp_mode.disabled = true;

	var nmode;
	nmode = 1*document.profile_page.network_type.options.selectedIndex;
	if (nmode==1)
	{
		document.getElementById("div_power_saving_mode").style.display = "";
		document.profile_page.power_saving_mode.disabled = false;

		document.getElementById("div_security_infra_mode").style.display = "";
		document.profile_page.security_infra_mode.disabled = false;

		if (auth == 0) //open
			document.profile_page.security_infra_mode.options.selectedIndex = 0 ;
		else if (auth == 1) //shared
			document.profile_page.security_infra_mode.options.selectedIndex = 1 ;
		else if (auth == 4) //wpa-psk
			document.profile_page.security_infra_mode.options.selectedIndex = 2 ;
		else if (auth == 7) //wpa2-psk
			document.profile_page.security_infra_mode.options.selectedIndex = 3 ;
		else if (auth == 3) //wpa-enterprise
			document.profile_page.security_infra_mode.options.selectedIndex = 4 ;
		else if (auth == 6) //wpa2-enterprise
			document.profile_page.security_infra_mode.options.selectedIndex = 5 ;
		else if (auth == 10) //802.1x
			document.profile_page.security_infra_mode.options.selectedIndex = 6 ;


		if (!auth )
		{
			auth = document.profile_page.security_infra_mode.value;
		}

		document.profile_page.security_infra_mode.value = auth;
		
		if (auth == 0)
		{
			if (encryp == 0)
				document.profile_page.security_encryp_mode.options.selectedIndex = 1;
			else
				document.profile_page.security_encryp_mode.options.selectedIndex = 0;
		} else if (auth>=3) {
			if (encryp == 4) <!--tkip -->
				document.profile_page.cipher[0].checked = true;
			else if (encryp == 6)
				document.profile_page.cipher[1].checked = true;
		}
	}
	else 
	{
		document.getElementById("div_b_premable_type").style.display = "";
		document.profile_page.b_premable_type.disabled = false;

		document.getElementById("div_channel").style.display = "";
		document.profile_page.channel.disabled = false;

		document.getElementById("div_security_adhoc_mode").style.display = "";
		document.profile_page.security_adhoc_mode.disabled = false;

		if (auth == 0) //open
			document.profile_page.security_adhoc_mode.options.selectedIndex = 0 ;
		else if (auth == 5) //wpa-none
			document.profile_page.security_adhoc_mode.options.selectedIndex = 1 ;

		if (!auth )
		{
			auth = document.profile_page.security_adhoc_mode.value;
		}
		document.profile_page.security_adhoc_mode.value = auth;

		if (auth == 0)
		{
			if (encryp == 0)
				document.profile_page.security_encryp_mode.options.selectedIndex = 1;
			else
				document.profile_page.security_encryp_mode.options.selectedIndex = 0;
		} else if (auth>=3) {
			if (encryp == 4) <!--tkip -->
				document.profile_page.cipher[0].checked = true;
			else if (encryp == 6)
				document.profile_page.cipher[1].checked = true;
		}
	}
	securityMode();
}

function RTSThresholdChange()
{
	if (document.profile_page.rts_threshold.checked )
		document.profile_page.rts_thresholdvalue.disabled = false;
	else
		document.profile_page.rts_thresholdvalue.disabled = true;
}

function FragmentThresholdChange()
{
	if (document.profile_page.fragment_threshold.checked )
		document.profile_page.fragment_thresholdvalue.disabled = false;
	else
		document.profile_page.fragment_thresholdvalue.disabled = true;
}

function open_cert_upload(target)
{
	if (target == "client")
		window.open("cert_cacl_upload.asp","cert_ca_client_upload","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=500, height=500");
	else if (target == "key")
		window.open("cert_key_upload.asp","cert_key_upload","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=500, height=500");
}

function checkData()
{
	var securitymode;
	var profilename = document.profile_page.profile_name.value;
	var ssid = document.profile_page.Ssid.value;
	var staProfile = "<% getCfgGeneral(1, "staProfile"); %>";
	var org_profilename = "<% getStaProfileData(1); %>";
	var staProfileArray = staProfile.split("\t");
	var i = 0;

	if (org_profilename != profilename)
	{
		while (i < staProfileArray.length)
		{
			if (staProfileArray[i] == profilename)
			{
				alert('Duplicate Profile Name!');
				return false;
			}
			i++;
		}
	}
	
	if (document.profile_page.network_type.value == 1) //infra
		securitymode = document.profile_page.security_infra_mode.value;
	else
		securitymode = document.profile_page.security_adhoc_mode.value;
	
	if (profilename.length <=0)
	{
		alert('Pleaes input the Profile Name!');
		return false;
	}
	else if (ssid.length <= 0)
	{
		alert('Pleaes input the SSID!');
		return false;
	}
	if (document.profile_page.rts_threshold.checked == true)
	{
		if (isNaN(document.profile_page.rts_thresholdvalue.value) == true)
		{
			alert('Please enter a number into RTS Threshold!');
			return false;
		}
		if (document.profile_page.rts_thresholdvalue.value < 1 ||
		    document.profile_page.rts_thresholdvalue.value > 2347)
		{
			alert('RTS Threshold is between 1 and 2347 !');
			return false;
		}
	}
	if (document.profile_page.fragment_threshold.checked == true)
	{
		if (isNaN(document.profile_page.fragment_thresholdvalue.value) == true)
		{
			alert('Please enter a number into Fragment Threshold!');
			return false;
		}
		if (document.profile_page.fragment_thresholdvalue.value < 256 ||
		    document.profile_page.fragment_thresholdvalue.value > 2346)

		{
			alert('Fragment Threshold is between 256 adn 2346 !');
			return false;
		}
	}
	if (securitymode  == 0 || securitymode  == 1)
	{
		return check_Wep(securitymode);
	}
	else if (securitymode  == 4 ||securitymode == 7 || securitymode == 5)
	{
		var keyvalue = document.profile_page.passphrase.value;

		if (keyvalue.length == 0)
		{
			alert('Please input wpapsk key!');
			return false;
		}

		if (keyvalue.length < 8)
		{
			alert('Please input at least 8 character of wpapsk key !');
			return false;
		}
	}
	//802.1x
	else if (securitymode == 3 || securitymode == 6 || securitymode == 10) //wpa enterprise, 802.1x
	{
		var certid = document.profile_page.cert_id.value;
		if (certid.length == 0)
		{
			alert('Please input the 802.1x identity !');
			return false;
		}

		if (document.profile_page.cert_password.disabled == false)
		{
			var certpassword = document.profile_page.cert_password.value;
			if (certpassword.length == 0)
			{
				alert('Please input the 802.1x password !');
				return false;
			}
		}

		if (document.profile_page.cert_use_client_cert.checked == true)
		{
			var client_cert = document.profile_page.cert_client_cert_path.value;
			var private_key = document.profile_page.cert_private_key_path.value;
			var private_key_password = document.profile_page.cert_private_key_password.value;

			if (client_cert.length == 0)
			{
				alert('Please input the 802.1x Client Certificate Path !');
				return false;
			}

			if (private_key.length == 0)
			{
				alert('Please input the 802.1x Private Key Path !');
				return false;
			}

			if (private_key_password.length == 0)
			{
				alert('Please input the 802.1x Private Key Password !');
				return false;
			}
			
		}
		else
		{
			document.profile_page.cert_client_cert_path.options.selectedIndex = 0;
			document.profile_page.cert_private_key_path.options.selectedIndex = 0;
		}

		if (document.profile_page.cert_auth_type_from_1x.value == 0) //md5
			return check_Wep(securitymode);

	}	
	return true;
}

function profileClose()
{
	opener.location.reload();
}

function wep_switch_key_length()
{
	document.profile_page.wep_key_1.value = "";
	document.profile_page.wep_key_2.value = "";
	document.profile_page.wep_key_3.value = "";
	document.profile_page.wep_key_4.value = "";

	if (document.profile_page.wep_key_length.options.selectedIndex == 0) {
		<!-- KEY length 64 bits -->
		if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
			<!-- HEX -->
			document.profile_page.wep_key_1.maxLength = 10;
			document.profile_page.wep_key_2.maxLength = 10;
			document.profile_page.wep_key_3.maxLength = 10;
			document.profile_page.wep_key_4.maxLength = 10;
		}
		else {
			<!-- ASCII -->
			document.profile_page.wep_key_1.maxLength = 5;
			document.profile_page.wep_key_2.maxLength = 5;
			document.profile_page.wep_key_3.maxLength = 5;
			document.profile_page.wep_key_4.maxLength = 5;
		}
	}
	else {
		<!-- KEY length 128 bits -->
		if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
			<!-- HEX -->
			document.profile_page.wep_key_1.maxLength = 26;
			document.profile_page.wep_key_2.maxLength = 26;
			document.profile_page.wep_key_3.maxLength = 26;
			document.profile_page.wep_key_4.maxLength = 26;
		}
		else {
			<!-- ASCII -->
			document.profile_page.wep_key_1.maxLength = 13;
			document.profile_page.wep_key_2.maxLength = 13;
			document.profile_page.wep_key_3.maxLength = 13;
			document.profile_page.wep_key_4.maxLength = 13;
		}
	}
}

function securityMode()
{
	var security_mode;

	document.getElementById("div_open_none").style.display = "none";

	document.getElementById("div_security_infra_mode").style.display = "none";
	document.profile_page.security_infra_mode.disabled = true;

	document.getElementById("div_security_adhoc_mode").style.display = "none";
	document.profile_page.security_adhoc_mode.disabled = true;

	document.getElementById("div_security_encryp_mode").style.display = "none";
	document.profile_page.security_encryp_mode.disabled = true;

	hideWep();

	document.getElementById("div_wpa").style.display = "none";
	document.getElementById("div_wpa_algorithms").style.display = "none";
	document.getElementById("wpa_passphrase").style.display = "none";
	document.profile_page.cipher[0].disabled = true;
	document.profile_page.cipher[1].disabled = true;
	document.profile_page.passphrase.disabled = true;

	// 802.1x
	document.getElementById("div_8021x").style.display = "none";
	document.getElementById("div_8021x_cert_from_wpa").style.display = "none";
	document.getElementById("div_8021x_cert_from_1x").style.display = "none";
	document.profile_page.cert_auth_type_from_wpa.disabled = true;
	document.profile_page.cert_auth_type_from_1x.disabled = true;

	document.profile_page.cert_tunnel_auth_peap.disabled = true;
	document.profile_page.cert_tunnel_auth_ttls.disabled = true;
	document.profile_page.cert_id.disabled = true;
	document.profile_page.cert_password.disabled = true;
	document.profile_page.cert_client_cert_path.disabled = true;
	document.profile_page.cert_private_key_path.disabled = true;
	document.profile_page.cert_private_key_password.disabled = true;

	if (document.profile_page.network_type.value == 1) //infra
	{
		security_mode = document.profile_page.security_infra_mode.value;
		document.getElementById("div_security_infra_mode").style.display = "";
		document.profile_page.security_infra_mode.disabled = false;
		var wpa_supplicantb = "<% getWPASupplicantBuilt(); %>";
		if (wpa_supplicantb == "1" && document.profile_page.security_infra_mode.length <= 4)
		{
			var mode = document.profile_page.security_infra_mode;

	    		mode.options[mode.length] = new Option("WPA-Enterprise", "3");
	    		mode.options[mode.length] = new Option("WPA2-Enterprise", "6");
	    		mode.options[mode.length] = new Option("802.1x", "10");
		}	
	}
	else
	{
		security_mode = document.profile_page.security_adhoc_mode.value;
		document.getElementById("div_security_adhoc_mode").style.display = "";
		document.profile_page.security_adhoc_mode.disabled = false;
	}

	if (security_mode == 0)
	{
		document.getElementById("div_security_encryp_mode").style.display = "";
		document.profile_page.security_encryp_mode.disabled = false;
		var encrypt_mode = document.profile_page.security_encryp_mode.value;

		if (encrypt_mode == 1)
			showWep();
		else 
			document.getElementById("div_open_none").style.display = "";
	} else if (security_mode == 1) {
		showWep();
	}
	else if (security_mode == 4 || security_mode == 7 || security_mode == 5) {
		<!-- WPA -->
		document.getElementById("div_wpa").style.display = "";
		document.getElementById("div_wpa_algorithms").style.display = "";
		document.profile_page.cipher[0].disabled = false;
		document.profile_page.cipher[1].disabled = false;;
		document.getElementById("wpa_passphrase").style.display = "";
		document.profile_page.passphrase.disabled = false;
	}
	else if (security_mode == 3 || security_mode == 6 || security_mode == 10) //wpa enterprise, 802.1x
	{
		if (security_mode != 10)
		{
			<!-- WPA -->
			document.getElementById("div_wpa").style.display = "";
			document.getElementById("div_wpa_algorithms").style.display = "";
			document.profile_page.cipher[0].disabled = false;
			document.profile_page.cipher[1].disabled = false;;
		}

		
		<!-- 802.1x -->
		document.getElementById("div_8021x").style.display = "";
		
		if (security_mode != 10) //802.1x
		{
			document.getElementById("div_8021x_cert_from_wpa").style.display = "";
			document.profile_page.cert_auth_type_from_wpa.disabled = false;
		}
		else
		{
			document.getElementById("div_8021x_cert_from_1x").style.display = "";
			document.profile_page.cert_auth_type_from_1x.disabled = false;
		}
		document.profile_page.cert_tunnel_auth_peap.disabled = false;
		document.profile_page.cert_tunnel_auth_ttls.disabled = false;
		document.profile_page.cert_id.disabled = false;
		document.profile_page.cert_password.disabled = false;

		certAuthModeChange();
	}
}

function use_client_cert()
{
	if (document.profile_page.cert_use_client_cert.checked)
	{
		document.getElementById("div_client_cert_path").style.display = "";
		document.getElementById("div_private_key_path").style.display = "";
		document.getElementById("div_private_key_password").style.display = "";

		document.profile_page.cert_private_key_path.disabled = false;
		document.profile_page.cert_private_key_password.disabled = false;
		document.profile_page.cert_client_cert_path.disabled = false;

		for (var i=0;i<document.profile_page.cert_client_cert_path.length;i++)
		{
			var tempvalue = document.profile_page.cert_client_cert_path.options[i].value;
			if (tempvalue == document.profile_page.cert_client_cert_path.value)
			{
				document.profile_page.cert_client_cert_path.options.selectedIndex = i;
			}
		}

		for (var i=0;i<document.profile_page.cert_private_key_path.length;i++)
		{
			var tempvalue = document.profile_page.cert_private_key_path.options[i].value;
			if (tempvalue == document.profile_page.cert_private_key_path.value)
			{
				document.profile_page.cert_private_key_path.options.selectedIndex = i;
			}
		}
	}
	else
	{
		document.getElementById("div_client_cert_path").style.display = "none";
		document.getElementById("div_private_key_path").style.display = "none";
		document.getElementById("div_private_key_password").style.display = "none";

		document.profile_page.cert_private_key_path.disabled = true;
		document.profile_page.cert_private_key_password.disabled = true;
		document.profile_page.cert_client_cert_path.disabled = true;
	}
}

function certAuthModeChange()
{
	var auth_mode;
	var security_infra_mode = document.profile_page.security_infra_mode.value;

	if (security_infra_mode == 3 || security_infra_mode == 6) //wpa-enterprise
		auth_mode = document.profile_page.cert_auth_type_from_wpa.value;
	else if (security_infra_mode == 10) // 802.1x
		auth_mode = document.profile_page.cert_auth_type_from_1x.value;

	hideWep();

	document.getElementById("div_tunnel_auth_peap").style.display = "none";
	document.profile_page.cert_tunnel_auth_peap.disabled = true;

	document.getElementById("div_tunnel_auth_ttls").style.display = "none";
	document.profile_page.cert_tunnel_auth_ttls.disabled = true;

	document.getElementById("div_password").style.display = "none";
	document.profile_page.cert_password.disabled = true;
	
	document.profile_page.cert_id.disabled = true;

	document.profile_page.cert_use_client_cert.checked = false;
	document.getElementById("div_use_client_cert").style.display = "none";
	document.profile_page.cert_use_client_cert.disabled = true;

	document.profile_page.cert_private_key_password.disabled = true;
	
	if (auth_mode == 5 || auth_mode == 6) // PEAP & TTLS
	{
		if (auth_mode == 5)
		{
			document.getElementById("div_tunnel_auth_peap").style.display = "";
			document.profile_page.cert_tunnel_auth_peap.disabled = false;
		}
		else 
		{
			document.getElementById("div_tunnel_auth_ttls").style.display = "";
			document.profile_page.cert_tunnel_auth_ttls.disabled = false;
		}
		
		document.profile_page.cert_id.disabled = false;
		
		document.getElementById("div_password").style.display = "";
		document.profile_page.cert_password.disabled = false;

		document.getElementById("div_use_client_cert").style.display = "";
		document.profile_page.cert_use_client_cert.disabled = false;
		
		var certpath = document.profile_page.cert_client_cert_path.value;
		var private_key_password = document.profile_page.cert_private_key_password.value;
		if (certpath.length > 0 && private_key_password.length > 0) {

			document.profile_page.cert_use_client_cert.checked = true;
		}
		else {

			document.profile_page.cert_use_client_cert.checked = false;
		}
	}
	else if (auth_mode == 4) //TLS
	{
		document.getElementById("div_use_client_cert").style.display = "";
		document.profile_page.cert_use_client_cert.disabled = true;

		document.profile_page.cert_use_client_cert.checked = true;

		document.profile_page.cert_id.disabled = false;
	}
	else if (auth_mode == 0) //MD5
	{
		document.profile_page.cert_id.disabled = false;
		document.profile_page.cert_use_client_cert.checked = false;

		document.getElementById("div_password").style.display = "";
		document.profile_page.cert_password.disabled = false;
		
		showWep();
	}

	use_client_cert();
}

function wep_default_key_switch()
{
	var defaultidx = document.profile_page.wep_default_key.value;
	var selected_wep_key = eval("document.profile_page.wep_key_"+defaultidx);

	document.profile_page.wep_key_1.disabled = true;
	document.profile_page.wep_key_2.disabled = true;
	document.profile_page.wep_key_3.disabled = true;
	document.profile_page.wep_key_4.disabled = true;
	selected_wep_key.disabled = false;
}

function hideWep()
{
	document.getElementById("div_wep").style.display = "none";
	//document.profile_page.wep_auth_type.disabled = true;
	document.profile_page.wep_key_length.disabled = true;
	document.profile_page.wep_key_entry_method.disabled = true;
	document.profile_page.wep_key_1.disabled = true;
	document.profile_page.wep_key_2.disabled = true;
	document.profile_page.wep_key_3.disabled = true;
	document.profile_page.wep_key_4.disabled = true;
	document.profile_page.wep_default_key.disabled = true;
}
function showWep()
{
	<!-- WEP -->
	document.getElementById("div_wep").style.display = "";

	//document.profile_page.wep_auth_type.disabled = false;
	document.profile_page.wep_key_length.disabled = false;
	document.profile_page.wep_key_entry_method.disabled = false;
	document.profile_page.wep_default_key.disabled = false;
	wep_default_key_switch();

	if (document.profile_page.wep_key_length.options.selectedIndex == 0) {
		<!-- KEY length 64 bits -->
			if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
				<!-- HEX -->
					document.profile_page.wep_key_1.maxLength = 10;
				document.profile_page.wep_key_2.maxLength = 10;
				document.profile_page.wep_key_3.maxLength = 10;
				document.profile_page.wep_key_4.maxLength = 10;
			}
			else {
				<!-- ASCII -->
					document.profile_page.wep_key_1.maxLength = 5;
				document.profile_page.wep_key_2.maxLength = 5;
				document.profile_page.wep_key_3.maxLength = 5;
				document.profile_page.wep_key_4.maxLength = 5;
			}
	}
	else {
		<!-- KEY length 128 bits -->
			if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
				<!-- HEX -->
					document.profile_page.wep_key_1.maxLength = 26;
				document.profile_page.wep_key_2.maxLength = 26;
				document.profile_page.wep_key_3.maxLength = 26;
				document.profile_page.wep_key_4.maxLength = 26;
			}
			else {
				<!-- ASCII -->
					document.profile_page.wep_key_1.maxLength = 13;
				document.profile_page.wep_key_2.maxLength = 13;
				document.profile_page.wep_key_3.maxLength = 13;
				document.profile_page.wep_key_4.maxLength = 13;
			}
	}
}

function check_Wep(securitymode)
{
	var defaultid = document.profile_page.wep_default_key.value;
	var keylen = 0 ;

	if (defaultid == 1 )
		var keyvalue = document.profile_page.wep_key_1.value;
	else if (defaultid == 2)
		var keyvalue = document.profile_page.wep_key_2.value;
	else if (defaultid == 3)
		var keyvalue = document.profile_page.wep_key_3.value;
	else if (defaultid == 4)
		var keyvalue = document.profile_page.wep_key_4.value;

	if (document.profile_page.wep_key_length.options.selectedIndex == 0) {
		<!-- KEY length 64 bits -->
		if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
			<!-- HEX -->
			keylen = 10;
		}
		else
		{
			<!-- ASCII -->
			keylen = 5;
		}
	}
	else {
		<!-- KEY length 128 bits -->
		if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
			<!-- HEX -->
			keylen = 26;
		}
		else {
			<!-- ASCII -->
			keylen = 13;
		}
	}

	if (keyvalue.length == 0 && ( securitymode == 1 || document.profile_page.cert_auth_type_from_1x.value == 3)) // shared wep  || md5
	{
		alert('Please input wep key'+defaultid+' !');
		return false;
	}

	if (keyvalue.length != 0)
	{
		if (keyvalue.length != keylen)
		{
			alert('Please input '+keylen+' character of wep key !');
			return false;
		}
	}
	return true;
}
function submit_apply()
{
	if (checkData() == true)
	{
		document.profile_page.submit();
		window.close();
	}
}

function resetForm()
{
	location=location; 
}
</script>
</head>
<body onLoad="Load_Setting()" onUnload="profileClose()">
<table><tr><td>
<form method=post name=profile_page action="/goform/editStaProfile">
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td class="title" colspan="2">System Configuration</td>
  </tr>
  <tr>
    <td class="thead">Profile Name</td>
    <td><input type=text name="profile_name" value="<% getStaProfileData(1); %>" maxlength=32></td>
  </tr>
  <tr>
    <td class="thead">SSID</td>
    <td><input type=text name="Ssid" value="<% getStaProfileData(2); %>" maxlength=32></td>
  </tr>
  <tr>
    <td class="thead">Network Type</td>
    <td><select name="network_type" onChange="networkTypeChange()">
	<option value = 0>802.11 Ad Hoc</option>
	<option value = 1 selected>Infrastructure</option>
      </select></td>
  </tr>
  <tr id="div_power_saving_mode">
    <td class="thead">Power Saving Mode</td>
    <td><input type=radio name="power_saving_mode" value="0" checked>CAM (Constantly Awake Mode)<input type=radio name="power_saving_mode" value="1">Power Saving Mode</td>
  </tr>
  <tr id="div_channel">
    <td class="thead" id="editprofChannel"> Channel </td>
    <td><select name="channel">
	<script>getChannel();</script>
      </select></td>
  </tr>
  <tr id="div_b_premable_type">
    <td class="thead">11B Premable Type</td>
    <td><select name="b_premable_type">
	<option value = 0 selected>Auto</option>
	<option value = 1>Long</option>
      </select></td>
  </tr>
  <tr>
    <td class="thead">RTS Threshold</td>
    <td><input type=checkbox name=rts_threshold onClick="RTSThresholdChange()">Used<input type=text name=rts_thresholdvalue value=2347> (range 1 - 2347, default 2347)</td>
  </tr>
  <tr>
    <td class="thead" id="editprofFrag"> Fragement Threshold </td>
    <td><input type=checkbox name=fragment_threshold onClick="FragmentThresholdChange()">Used<input type=text name=fragment_thresholdvalue value=2346> (range 256 - 2346, default 2346)</td>
  </tr>
</table>
<br>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td class="title" colspan="2">Security Policy</td>
  </tr>
  <tr id="div_security_infra_mode"> 
    <td class="thead">Security Mode</td>
    <td><select name="security_infra_mode" id="security_infra_mode" onChange="securityMode()">
	<option value=0 selected>OPEN</option>
	<option value=1>SHARED</option>
	<option value=4>WPA-Personal</option>
	<option value=7>WPA2-Personal</option>
	<!--
	<option value=3>WPA-Enterprise</option>
	<option value=6>WPA2-Enterprise</option>
	<option value=10>802.1x</option>
	-->
      </select></td>
  </tr>
  <tr id="div_security_adhoc_mode"> 
    <td class="thead">Security Mode</td>
    <td><select name="security_adhoc_mode" id="security_adhoc_mode" onChange="securityMode()">
	<option value=0 selected>OPEN</option>
	<option value=5>WPA-NONE</option>
      </select></td>
  </tr>
  <tr id="div_security_encryp_mode"> 
    <td class="thead">Encryption Mode</td>
    <td><select name="security_encryp_mode" id="security_encryp_mode" onChange="securityMode()">
	<option value=0 id="editprofNONE" selected>NONE</option>
	<option value=1 id="editprofWep">WEP</option>
      </select></td>
  </tr>
</table>
<br />
<center><h4><script> 
	document.write("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;".fontsize(10));
	document.write("This is no any security. Are you sure to connect AP?".fontsize(3)); 
      </script></h4></center>
<br />

<table id="div_wep" width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr> 
    <td class="title" colspan="3">Wire Equivalence Protection (WEP)</td>
  </tr>
  <tr> 
    <td class="thead">WEP Key Length</td>
    <td colspan="2"><select name="wep_key_length" onChange="wep_switch_key_length()">
	<option value=0 selected>64 bit (10 hex digits/ 5 ascii keys)</option>
	<option value=1>128 bit (26 hex digits/13 ascii keys)</option>
      </select></td>
  </tr>
  <tr> 
    <td class="thead">WEP Key Entry Method</td>
    <td colspan="2"><select name="wep_key_entry_method" onChange="wep_switch_key_length()">
	<option value=0 selected>Hex</option>
	<option value=1>ASCII</option>
      </select></td>
  </tr>
  <tr> 
    <td class="head1" rowspan="4">WEP Keys</td>
    <td class="head2">WEP Key 1 :</td>
    <td><input type=password name=wep_key_1 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head2">WEP Key 2 </td>
    <td><input type=password name=wep_key_2 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head2">WEP Key 3 </td>
    <td><input type=password name=wep_key_3 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head2">WEP Key 4 </td>
    <td><input type=password name=wep_key_4 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="thead">Default Key</td>
    <td colspan="2"><select name="wep_default_key" onChange="wep_default_key_switch()">
	<option value=1 selected>Key 1</option>
	<option value=2>Key 2</option>
	<option value=3>Key 3</option>
	<option value=4>Key 4</option>
      </select></td>
  </tr>
</table>

<table id="div_wpa" width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td class="title" colspan="2">WPA</td>
  </tr>
  <tr id="div_wpa_algorithms"> 
    <td class="thead">WPA Algorithms</td>
    <td><input type=radio name="cipher" id="cipher" value="0" checked>TKIP<input type=radio name="cipher" id="cipher" value="1">AES</td>
  </tr>
  <tr id="wpa_passphrase">
    <td class="thead">Pass Phrase</td>
    <td><input type=password name=passphrase size=28 maxlength=64 value=""></td>
  </tr>
</table>
<br/>

<!-- 802.1x -->
<table id="div_8021x" width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td class="title" colspan="2">802.1x</td>
  </tr>
  <tr id="div_8021x_cert_from_1x">
    <td class="thead">Authentication Type</td>
    <td><select name="cert_auth_type_from_1x" id="cert_auth_type_from_1x" onChange="certAuthModeChange()">
	<option value=5 selected>PEAP</option>
	<option value=6>TTLS</option>
	<option value=4>TLS</option>
	<option value=0>MD5</option>
      </select></td>
  </tr>
  <tr id="div_8021x_cert_from_wpa">
    <td class="thead">Authentication Type</td>
    <td><select name="cert_auth_type_from_wpa" id="cert_auth_type_from_wpa" onChange="certAuthModeChange()">
	<option value=5 selected>PEAP</option>
	<option value=6>TTLS</option>
	<option value=4>TLS</option>
      </select></td>
  </tr>
  <tr id="div_tunnel_auth_peap">
    <td class="thead">Tunnel Authentication</td>
    <td><select name="cert_tunnel_auth_peap" id="cert_tunnel_auth_peap">
	<option value=1 selected>MSCHAP v2</option>
      </select></td>
  </tr>
  <tr id="div_tunnel_auth_ttls">
    <td class="thead">Tunnel Authentication</td>
    <td><select name="cert_tunnel_auth_ttls" id="cert_tunnel_auth_ttls">
	<option value=0 selected>MSCHAP</option>
	<option value=1>MSCHAP v2</option>
	<option value=2>PAP</option>
      </select></td>
  </tr>
  <tr id="div_identity">
    <td class="thead">Identity</td>
    <td><input type=text name="cert_id" value="<% getStaProfileData(28); %>" maxlength=32></td>
  </tr>
  <tr id="div_password">
    <td class="thead">Password</td>
    <td><input type=password name="cert_password" maxlength=32></td>
  </tr>
  <tr id="div_use_client_cert">
    <td class="thead">Client Certificate</td>
    <td><input type=checkbox name="cert_use_client_cert" onClick="use_client_cert()" >Used</td>
  </tr>
  <tr id="div_client_cert_path">
    <td class="thead">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Client Certificate Path</td>
    <td><select name="cert_client_cert_path">
        <% getCACLCertList(); %>
      </select>
      <input type="button" value="Install" name="ca_cl_cert_upload" id="ca_cl_cert_upload" onClick="open_cert_upload('client')"></td>
  </tr>
  <tr id="div_private_key_path">
    <td class="thead">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Private Key Path</td>
    <td><select name="cert_private_key_path">
        <% getKeyCertList(); %>
      </select>
      <input type="button" value="Install" name="key_cert_upload" id="key_cert_upload" onClick="open_cert_upload('key')"></td>
  </tr>
  <tr id="div_private_key_password">
    <td class="thead">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Private Key Password</td>
    <td><input type=password name="cert_private_key_password" maxlength=32></td>
  </tr>
</table>
<table width=100% border=0 cellpadding=3 cellspacing=1> 
  <tr>
    <td>
      <input type=submit class=button value="Apply" onClick="submit_apply()"> &nbsp; &nbsp;
      <input type=button class=button value="Reset" onClick="resetForm();">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body></html>
