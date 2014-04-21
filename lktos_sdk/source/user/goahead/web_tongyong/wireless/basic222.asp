<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=<% getCharset(); %>">
<link rel="stylesheet" href="../images/style.css" type="text/css">
<script language="javascript" src="../js/language_<% getCfgZero(1, "LanguageType"); %>.js"></script>
<script language="javascript" src="../js/common.js"></script>
<script language="javascript">
var PhyMode  = '<% getCfgZero(1, "WirelessMode"); %>';
var HiddenSSID  = '<% getCfgZero(1, "HideSSID"); %>';
var APIsolated = '<% getCfgZero(1, "NoForwarding"); %>';
var mbssidapisolated = '<% getCfgZero(1, "NoForwardingBTNBSSID"); %>';
var ChIdx  = '<% getWlanChannel(); %>';
var fxtxmode = '<% getCfgGeneral(1, "FixedTxMode"); %>';
var CntyCd = '<% getCfgGeneral(1, "CountryCode"); %>';
var ht_mode = '<% getCfgZero(1, "HT_OpMode"); %>';
var ht_bw = '<% getCfgZero(1, "HT_BW"); %>';
var ht_gi = '<% getCfgZero(1, "HT_GI"); %>';
var ht_stbc = '<% getCfgZero(1, "HT_STBC"); %>';
var ht_mcs = '<% getCfgZero(1, "HT_MCS"); %>';
var ht_htc = '<% getCfgZero(1, "HT_HTC"); %>';
var ht_rdg = '<% getCfgZero(1, "HT_RDG"); %>';
var ht_extcha = '<% getCfgZero(1, "HT_EXTCHA"); %>';
var ht_amsdu = '<% getCfgZero(1, "HT_AMSDU"); %>';
var ht_autoba = '<% getCfgZero(1, "HT_AutoBA"); %>';
var ht_badecline = '<% getCfgZero(1, "HT_BADecline"); %>';
var ht_disallow_tkip = '<% getCfgZero(1, "HT_DisallowTKIP"); %>';
var ht_2040_coexit = '<% getCfgZero(1, "HT_BSSCoexistence"); %>';
var ht_f_40mhz = '<% getCfgZero(1, "HT_40MHZ_INTOLERANT"); %>';
var apcli_include = '<% getWlanApcliBuilt(); %>';
var draft3b = '<% getRax11nDraft3Built(); %>';
var mesh_include = '<% getMeshBuilt(); %>';
var tx_stream_idx = '<% getCfgZero(1, "HT_TxStream"); %>';
var rx_stream_idx = '<% getCfgZero(1, "HT_RxStream"); %>';
var txrxStream = '<% getRaxHTStream(); %>';
var max_bssid_num = '<% getMaxBssidNum(); %>';

ChLst_24G = new Array(14);
ChLst_24G[0] = "1--[2412MHz]";
ChLst_24G[1] = "2--[2417MHz]";
ChLst_24G[2] = "3--[2422MHz]";
ChLst_24G[3] = "4--[2427MHz]";
ChLst_24G[4] = "5--[2432MHz]";
ChLst_24G[5] = "6--[2437MHz]";
ChLst_24G[6] = "7--[2442MHz]";
ChLst_24G[7] = "8--[2447MHz]";
ChLst_24G[8] = "9--[2452MHz]";
ChLst_24G[9] = "10--[2457MHz]";
ChLst_24G[10] = "11--[2462MHz]";
ChLst_24G[11] = "12--[2467MHz]";
ChLst_24G[12] = "13--[2472MHz]";
ChLst_24G[13] = "14--[2484MHz]";

ChLst_5G = new Array(33);
ChLst_5G[0] = "36--[5180MHz]";
ChLst_5G[1] = "40--[5200MHz]";
ChLst_5G[2] = "44--[5220MHz]";
ChLst_5G[3] = "48--[5240MHz]";
ChLst_5G[4] = "52--[5260MHz]";
ChLst_5G[5] = "56--[5280MHz]";
ChLst_5G[6] = "60--[5300MHz]";
ChLst_5G[7] = "64--[5320MHz]";
ChLst_5G[16] = "100--[5500MHz]";
ChLst_5G[17] = "104--[5520MHz]";
ChLst_5G[18] = "108--[5540MHz]";
ChLst_5G[19] = "112--[5560MHz]";
ChLst_5G[20] = "116--[5580MHz]";
ChLst_5G[21] = "120--[5600MHz]";
ChLst_5G[22] = "124--[5620MHz]";
ChLst_5G[23] = "128--[5640MHz]";
ChLst_5G[24] = "132--[5660MHz]";
ChLst_5G[25] = "136--[5680MHz]";
ChLst_5G[26] = "140--[5700MHz]";
ChLst_5G[28] = "149--[5745MHz]";
ChLst_5G[29] = "153--[5765MHz]";
ChLst_5G[30] = "157--[5785MHz]";
ChLst_5G[31] = "161--[5805MHz]";
ChLst_5G[32] = "165--[5825MHz]";

HT5GExtCh = new Array(22);
HT5GExtCh[0] = new Array(1, "40--[5200MHz]"); //36's extension channel
HT5GExtCh[1] = new Array(0, "36--[5180MHz]"); //40's extension channel
HT5GExtCh[2] = new Array(1, "48--[5240MHz]"); //44's
HT5GExtCh[3] = new Array(0, "44--[5220MHz]"); //48's
HT5GExtCh[4] = new Array(1, "56--[5280MHz]"); //52's
HT5GExtCh[5] = new Array(0, "52--[5260MHz]"); //56's
HT5GExtCh[6] = new Array(1, "64--[5320MHz]"); //60's
HT5GExtCh[7] = new Array(0, "60--[5300MHz]"); //64's
HT5GExtCh[8] = new Array(1, "104--[5520MHz]"); //100's
HT5GExtCh[9] = new Array(0, "100--[5500MHz]"); //104's
HT5GExtCh[10] = new Array(1, "112--[5560MHz]"); //108's
HT5GExtCh[11] = new Array(0, "108--[5540MHz]"); //112's
HT5GExtCh[12] = new Array(1, "120--[5600MHz]"); //116's
HT5GExtCh[13] = new Array(0, "116--[5580MHz]"); //120's
HT5GExtCh[14] = new Array(1, "128--[5640MHz]"); //124's
HT5GExtCh[15] = new Array(0, "124--[5620MHz]"); //128's
HT5GExtCh[16] = new Array(1, "136--[5680MHz]"); //132's
HT5GExtCh[17] = new Array(0, "132--[5660MHz]"); //136's
HT5GExtCh[18] = new Array(1, "153--[5765MHz]"); //149's
HT5GExtCh[19] = new Array(0, "149--[5745MHz]"); //153's
HT5GExtCh[20] = new Array(1, "161--[5805MHz]"); //157's
HT5GExtCh[21] = new Array(0, "157--[5785MHz]"); //161's

function CreateExtChOpt(vChannel)
{
	var y = document.createElement('option');

	y.text = ChLst_24G[1*vChannel - 1];
//	y.value = 1*vChannel;
	y.value = 1;

	var x = document.getElementById("n_extcha");

	try
	{
		x.add(y,null); // standards compliant
	}
	catch(ex)
	{
		x.add(y); // IE only
	}
}

function InsExtChOpt()
{
	var wmode = document.basic_form.wirelessmode.options.selectedIndex;
	var OptLen; 
	var CurrCh;

	if ((1*wmode == 6) || (1*wmode == 3) || (1*wmode == 4) || (1*wmode == 7))
	{
		var x = document.getElementById("n_extcha");
		var length = document.basic_form.n_extcha.options.length;

		if (length > 1)	
		{
			x.selectedIndex = 1;
			x.remove(x.selectedIndex);
		}

		if ((1*wmode == 6) || (1*wmode == 7))
		{
			CurrCh = document.basic_form.sz11aChannel.value;

			if ((1*CurrCh >= 36) && (1*CurrCh <= 64))
			{
				CurrCh = 1*CurrCh;
				CurrCh /= 4;
				CurrCh -= 9;

				x.options[0].text = HT5GExtCh[CurrCh][1];
				x.options[0].value = HT5GExtCh[CurrCh][0];
			}
			else if ((1*CurrCh >= 100) && (1*CurrCh <= 136))
			{
				CurrCh = 1*CurrCh;
				CurrCh /= 4;
				CurrCh -= 17;

				x.options[0].text = HT5GExtCh[CurrCh][1];
				x.options[0].value = HT5GExtCh[CurrCh][0];
			}
			else if ((1*CurrCh >= 149) && (1*CurrCh <= 161))
			{
				CurrCh = 1*CurrCh;
				CurrCh -= 1;
				CurrCh /= 4;
				CurrCh -= 19;

				x.options[0].text = HT5GExtCh[CurrCh][1];
				x.options[0].value = HT5GExtCh[CurrCh][0];
			}
			else
			{
				x.options[0].text = MM_auto_select;
				x.options[0].value = 0;
			}
		}
		else if ((1*wmode == 3) || (1*wmode == 4))
		{
			CurrCh = document.basic_form.sz11gChannel.value;
			OptLen = document.basic_form.sz11gChannel.options.length;

			if ((CurrCh >=1) && (CurrCh <= 4))
			{
				x.options[0].text = ChLst_24G[1*CurrCh + 4 - 1];
				x.options[0].value = 1;
			}
			else if ((CurrCh >= 5) && (CurrCh <= 7))
			{
				x.options[0].text = ChLst_24G[1*CurrCh - 4 - 1];
				x.options[0].value = 0; //1*CurrCh - 4;
				CurrCh = 1*CurrCh;
				CurrCh += 4;
				CreateExtChOpt(CurrCh);
			}
			else if ((CurrCh >= 8) && (CurrCh <= 9))
			{
				x.options[0].text = ChLst_24G[1*CurrCh - 4 - 1];
				x.options[0].value = 0; //1*CurrCh - 4;

				if (OptLen >=14)
				{
					CurrCh = 1*CurrCh;
					CurrCh += 4;
					CreateExtChOpt(CurrCh);
				}
			}
			else if (CurrCh == 10)
			{
				x.options[0].text = ChLst_24G[1*CurrCh - 4 - 1];
				x.options[0].value = 0; //1*CurrCh - 4;

				if (OptLen > 14)
				{
					CurrCh = 1*CurrCh;
					CurrCh += 4;
					CreateExtChOpt(CurrCh);
				}
			}
			else if (CurrCh >= 11)
			{
				x.options[0].text = ChLst_24G[1*CurrCh - 4 - 1];
				x.options[0].value = 0; //1*CurrCh - 4;
			}
			else
			{
				x.options[0].text = MM_auto_select;
				x.options[0].value = 0;
			}
		}
	}
}

function ChOnChange()
{
	if (document.basic_form.n_bandwidth[2].checked == true)	//20/40M
	{
		var w_mode = document.basic_form.wirelessmode.options.selectedIndex;

		if ((1*w_mode == 6) || (1*w_mode == 7))
		{
			if (document.basic_form.n_bandwidth[2].checked == true)
			{
				document.getElementById("extension_channel").style.display = "";
				document.basic_form.n_extcha.disabled = false;
				document.basic_form.n_2040_coexit[1].checked = true;
			}

			if (document.basic_form.sz11aChannel.options.selectedIndex == 0)
			{
				document.getElementById("extension_channel").style.display = "none";
				document.basic_form.n_extcha.disabled = true;
			}
		}
		else if ((1*w_mode == 3) || (1*w_mode == 4))
		{
			if (document.basic_form.n_bandwidth[2].checked == true)
			{
				document.getElementById("extension_channel").style.display = "";
				document.basic_form.n_extcha.disabled = false;
				document.basic_form.n_2040_coexit[1].checked = true;
			}

			if (document.basic_form.sz11gChannel.options.selectedIndex == 0)
			{
				document.getElementById("extension_channel").style.display = "none";
				document.basic_form.n_extcha.disabled = true;
			}
		}
	}

	InsExtChOpt();
}

function ChBwOnClick()
{
	var w_mode = document.basic_form.wirelessmode.options.selectedIndex;
	if (document.basic_form.n_bandwidth[0].checked == true) //20M
	{
		document.getElementById("extension_channel").style.display = "none";
		document.basic_form.n_extcha.disabled = true;
		document.basic_form.n_2040_coexit[0].checked = true;	//disable
	}
	else if (document.basic_form.n_bandwidth[1].checked == true) //40M
	{
		document.getElementById("extension_channel").style.display = "";
		document.basic_form.n_extcha.disabled = false;
		document.basic_form.n_2040_coexit[0].checked = true;	//disable
	}
	else
	{
		document.getElementById("extension_channel").style.display = "";
		document.basic_form.n_extcha.disabled = false;
		document.basic_form.n_2040_coexit[1].checked = true;	//enable

		if ((1*w_mode == 6) || (1*w_mode == 7))
		{
			if (document.basic_form.sz11aChannel.options.selectedIndex == 0)
			{
				document.getElementById("extension_channel").style.display = "none";
				document.basic_form.n_extcha.disabled = true;
			}
		}
	}
}

function Load_Setting()
{
	var Ch11aIdx;
	var CurrChLen;
	var radio_off = '<% getCfgZero(1, "RadioOff"); %>';
	var wifi_off = '<% getCfgZero(1, "WiFiOff"); %>';
	var mssidb = "<% getMBSSIDBuilt(); %>";
	var BssidNum = 1*'<% getCfgGeneral(1, "BssidNum"); %>';
	var i = 0;

	if (CntyCd == '')
		CntyCd = 'NONE';

	document.getElementById("div_11a_channel").style.display = "none";
	document.basic_form.sz11aChannel.disabled = true;
	document.getElementById("div_11b_channel").style.display = "none";
	document.basic_form.sz11bChannel.disabled = true;
	document.getElementById("div_11g_channel").style.display = "none";
	document.basic_form.sz11gChannel.disabled = true;
	document.getElementById("div_11n").style.display = "none";
	document.basic_form.n_mode.disabled = true;
	document.basic_form.n_bandwidth.disabled = true;
	document.basic_form.n_rdg.disabled = true;
	document.basic_form.n_gi.disabled = true;
	document.basic_form.n_mcs.disabled = true;
	document.getElementById("div_2040_coexit").style.display = "none";
	document.basic_form.n_2040_coexit.disabled = true;

	PhyMode = 1*PhyMode;
	if ((PhyMode >= 8) || (PhyMode == 6))
	{
		document.getElementById("div_11n").style.display = "";
		document.basic_form.n_mode.disabled = false;
		document.basic_form.n_bandwidth.disabled = false;
		document.basic_form.n_rdg.disabled = false;
		document.basic_form.n_gi.disabled = false;
		document.basic_form.n_mcs.disabled = false;
	}

	var Aband = "<% getRaxABand(); %>";
	if (Aband == "1")
	{
		document.basic_form.wirelessmode.options[5] = new Option("11a", "2");
		document.basic_form.wirelessmode.options[6] = new Option("11a/n", "8");
		document.basic_form.wirelessmode.options[7] = new Option("11n(5G)", "11");
	}
	if ((PhyMode == 0) || (PhyMode == 4) || (PhyMode == 9) || (PhyMode == 6))
	{
		if (PhyMode == 0)
			document.basic_form.wirelessmode.options.selectedIndex = 0;
		else if (PhyMode == 4)
			document.basic_form.wirelessmode.options.selectedIndex = 2;
		else if (PhyMode == 9)
			document.basic_form.wirelessmode.options.selectedIndex = 3;
		else if (PhyMode == 6)
			document.basic_form.wirelessmode.options.selectedIndex = 4;

		document.getElementById("div_11g_channel").style.display = "";
		document.basic_form.sz11gChannel.disabled = false;
	}
	else if (PhyMode == 1)
	{
		document.basic_form.wirelessmode.options.selectedIndex = 1;
		document.getElementById("div_11b_channel").style.display = "";
		document.basic_form.sz11bChannel.disabled = false;
	}
	else if ((PhyMode == 2) || (PhyMode == 8) || (PhyMode == 11))
	{
		if (PhyMode == 2)
			document.basic_form.wirelessmode.options.selectedIndex = 5;
		else if (PhyMode == 8)
			document.basic_form.wirelessmode.options.selectedIndex = 6;
		else if (PhyMode == 11)
			document.basic_form.wirelessmode.options.selectedIndex = 7;

		document.getElementById("div_11a_channel").style.display = "";
		document.basic_form.sz11aChannel.disabled = false;
	}

	for (i=1;i<16;i++)
	{
		var ssid = eval("document.basic_form.mssid_"+i+".disabled");
		var div = eval("document.getElementById(\"div_hssid"+i+"\").style");
		div.display = "none";
		ssid = true;
		document.basic_form.hssid[i].disabled = true;
	}
	
	if (mssidb == "1")
	{
		var mssid16b = "<% get16MBSSIDBuilt(); %>";
		var num = 3;//old value=8
		if (mssid16b == "1")
			num = 16;
		
		for (i=1;i<num;i++)
		{
			var ssid = eval("document.basic_form.mssid_"+i+".disabled");
			var div = eval("document.getElementById(\"div_hssid"+i+"\").style");
			div.display = "none";//by chenfei
			ssid = false;
			document.basic_form.hssid[i].disabled = false;
		}
	}

	if (HiddenSSID.indexOf("0") >= 0)
	{
		document.basic_form.broadcastssid[0].checked = true;
		var HiddenSSIDArray = HiddenSSID.split(";");
		for (i=0;i<BssidNum;i++)
		{
			if (HiddenSSIDArray[i] == "1")
				document.basic_form.hssid[i].checked = true;
			else
				document.basic_form.hssid[i].checked = false;
		}
	}
	else
	{
		document.basic_form.broadcastssid[1].checked = true;
		for (i=0;i<BssidNum;i++)
			document.basic_form.hssid[i].checked = true;
	}

	if (APIsolated.indexOf("1") >= 0)
	{
		document.basic_form.apisolated[0].checked = true;
		var APIsolatedArray = APIsolated.split(";");
		for (i=0;i<BssidNum;i++)
		{
			if (APIsolatedArray[i] == "1")
				document.basic_form.isolated_ssid[i].checked = true;
			else
				document.basic_form.isolated_ssid[i].checked = false;
		}
	}
	else
	{
		document.basic_form.apisolated[1].checked = true;
		for (i=0;i<BssidNum;i++)
			document.basic_form.isolated_ssid[i].checked = false;
	}

	if (1*ht_bw == 0 && 1*ht_2040_coexit == 0)
	{
		document.basic_form.n_bandwidth[0].checked = true;	//20M
		document.getElementById("extension_channel").style.display = "none";
		document.basic_form.n_extcha.disabled = true;
	}
	else if (1*ht_bw == 1 && 1*ht_2040_coexit == 0)
	{
		document.basic_form.n_bandwidth[1].checked = true;	//40M
		document.getElementById("extension_channel").style.display = "";
		document.basic_form.n_extcha.disabled = false;
	}
	else
	{
		document.basic_form.n_bandwidth[2].checked = true;	//20/40M
		document.getElementById("extension_channel").style.display = "";
		document.basic_form.n_extcha.disabled = false;
	}

	ChIdx = 1*ChIdx;
	if ((PhyMode == 0) || (PhyMode == 4) || (PhyMode == 6) || (PhyMode == 7) || (PhyMode == 9))
	{
		if ((CntyCd == 'US' || CntyCd == 'TW') && (ChIdx < 1 || ChIdx > 11))
			document.basic_form.sz11gChannel.options.selectedIndex = 0;
		else if ((CntyCd == 'FR' || CntyCd == 'BR' || CntyCd == 'HK' || CntyCd == 'CN') && (ChIdx < 1 || ChIdx > 13))
			document.basic_form.sz11gChannel.options.selectedIndex = 0;
		else if (CntyCd == 'JP' && (ChIdx < 1 || ChIdx > 14))
			document.basic_form.sz11gChannel.options.selectedIndex = 0;
		else
		{
			if (ChIdx < 1 || ChIdx > 14)
				document.basic_form.sz11gChannel.options.selectedIndex = 0;
		}
	}
	else if (PhyMode == 1)
	{
		if ((CntyCd == 'US' || CntyCd == 'TW') && (ChIdx < 1 || ChIdx > 11))
			document.basic_form.sz11bChannel.options.selectedIndex = 0;
		else if ((CntyCd == 'FR' || CntyCd == 'BR' || CntyCd == 'HK' || CntyCd == 'CN') && (ChIdx < 1 || ChIdx > 13))
			document.basic_form.sz11bChannel.options.selectedIndex = 0;
		else if (CntyCd == 'JP' && (ChIdx < 1 || ChIdx > 14))
			document.basic_form.sz11bChannel.options.selectedIndex = 0;
		else
		{
			if (ChIdx < 1 || ChIdx > 14)
				document.basic_form.sz11bChannel.options.selectedIndex = 0;
		}
	}
	else if ((PhyMode == 2) || (PhyMode == 8) || (PhyMode == 11))
	{
		if ((CntyCd == 'HK' || CntyCd == 'CN') && (ChIdx < 36 || (ChIdx > 64 && ChIdx < 149) || ChIdx > 165))
			document.basic_form.sz11aChannel.options.selectedIndex = 0;
		else if (CntyCd == 'BR' && (ChIdx < 36 || (ChIdx > 64 && ChIdx < 100) || ChIdx > 140))
			document.basic_form.sz11aChannel.options.selectedIndex = 0;
		else if (CntyCd == 'FR' && (ChIdx < 36 || ChIdx > 64))
			document.basic_form.sz11aChannel.options.selectedIndex = 0;
		else if (CntyCd == 'JP' && (ChIdx < 36 || ChIdx > 48))
			document.basic_form.sz11aChannel.options.selectedIndex = 0;
		else if (CntyCd == 'US' && (ChIdx < 36 || (ChIdx > 64 && ChIdx < 100) || (ChIdx > 140 && ChIdx < 149) || ChIdx > 165))
			document.basic_form.sz11aChannel.options.selectedIndex = 0;
		else if (CntyCd == 'TW' && (ChIdx < 52 || ChIdx > 64))
			document.basic_form.sz11aChannel.options.selectedIndex = 0;
		else
		{
			if (ChIdx < 36 || (ChIdx > 64 && ChIdx < 100) || (ChIdx > 140 && ChIdx < 149) || ChIdx > 165)
				document.basic_form.sz11aChannel.options.selectedIndex = 0;
		}
	}

	//ABG Rate
	if ((PhyMode == 0) || (PhyMode == 2) || (PhyMode == 4))
	{
		document.basic_form.abg_rate.options.length = 0;
		document.basic_form.abg_rate.options[0] = new Option(MM_auto, "0");
		document.basic_form.abg_rate.options[1] = new Option("1 Mbps", "1");
		document.basic_form.abg_rate.options[2] = new Option("2 Mbps", "2");
		document.basic_form.abg_rate.options[3] = new Option("5.5 Mbps", "5");
		document.basic_form.abg_rate.options[4] = new Option("6 Mbps", "6");
		document.basic_form.abg_rate.options[5] = new Option("9 Mbps", "9");
		document.basic_form.abg_rate.options[6] = new Option("11 Mbps", "11");
		document.basic_form.abg_rate.options[7] = new Option("12 Mbps", "12");
		document.basic_form.abg_rate.options[8] = new Option("18 Mbps", "18");
		document.basic_form.abg_rate.options[9] = new Option("24 Mbps", "24");
		document.basic_form.abg_rate.options[10] = new Option("36 Mbps", "36");
		document.basic_form.abg_rate.options[11] = new Option("48 Mbps", "48");
		document.basic_form.abg_rate.options[12] = new Option("54 Mbps", "54");
		if (fxtxmode == "CCK" || fxtxmode == "cck") 
		{
			if (ht_mcs.indexOf("33") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 0;
			else if (ht_mcs.indexOf("0") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 1;
			else if (ht_mcs.indexOf("1") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 2;
			else if (ht_mcs.indexOf("2") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 3;
			else if (ht_mcs.indexOf("3") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 6;
		}
		else 
		{
			if (ht_mcs.indexOf("33") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 0;
			else if (ht_mcs.indexOf("0") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 4;
			else if (ht_mcs.indexOf("1") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 5;
			else if (ht_mcs.indexOf("2") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 7;
			else if (ht_mcs.indexOf("3") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 8;
			else if (ht_mcs.indexOf("4") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 9;
			else if (ht_mcs.indexOf("5") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 10;
			else if (ht_mcs.indexOf("6") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 11;
			else if (ht_mcs.indexOf("7") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 12;
		}
	}
	else if (PhyMode == 1)
	{
		document.basic_form.abg_rate.options.length = 0;
		document.basic_form.abg_rate.options[0] = new Option(MM_auto, "0");
		document.basic_form.abg_rate.options[1] = new Option("1 Mbps", "1");
		document.basic_form.abg_rate.options[2] = new Option("2 Mbps", "2");
		document.basic_form.abg_rate.options[3] = new Option("5.5 Mbps", "5");
		document.basic_form.abg_rate.options[4] = new Option("11 Mbps", "11");
		if (ht_mcs.indexOf("33") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 0;
		else if (ht_mcs.indexOf("0") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 1;
		else if (ht_mcs.indexOf("1") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 2;
		else if (ht_mcs.indexOf("2") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 3;
		else if (ht_mcs.indexOf("3") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 4;
	}
	else
	{
		document.getElementById("div_abg_rate").style.display = "none";
		document.basic_form.abg_rate.disabled = true;
	}

	document.getElementById("div_mbssidapisolated").style.display = "none";
	document.basic_form.mbssidapisolated.disabled = true;
	if (mssidb == "1")
	{
		document.getElementById("div_mbssidapisolated").style.display = "";
		document.basic_form.mbssidapisolated.disabled = false;
		if (mbssidapisolated == "1")
			document.basic_form.mbssidapisolated[0].checked = true;
		else
			document.basic_form.mbssidapisolated[1].checked = true;
	}

	InsExtChOpt();

	if (1*ht_stbc == 0)
		document.basic_form.n_stbc[0].checked = true;
	else
		document.basic_form.n_stbc[1].checked = true;

	if (1*ht_mode == 0)
		document.basic_form.n_mode[0].checked = true;
	else if (1*ht_mode == 1)
		document.basic_form.n_mode[1].checked = true;
	else if (1*ht_mode == 2)
		document.basic_form.n_mode[2].checked = true;

	if (1*ht_gi == 0)
		document.basic_form.n_gi[0].checked = true;
	else if (1*ht_gi == 1)
		document.basic_form.n_gi[1].checked = true;
	else if (1*ht_gi == 2)
		document.basic_form.n_gi[2].checked = true;

	if (1*txrxStream == 3) 
	{
		for (i = 16; i < 24; i++)
			document.basic_form.n_mcs.options[i] = new Option(i, i);
	}
	
	var mcs_length = document.basic_form.n_mcs.options.length;
	if (txrxStream == "3") 
	{
		document.basic_form.n_mcs.options[mcs_length] = new Option("32", "32");
		mcs_length++;
		document.basic_form.n_mcs.options[mcs_length] = new Option(MM_auto, "33");
		mcs_length++;
	}

	var ht_mcs_nm = 1*ht_mcs.split(";", 1);
	if (ht_mcs_nm <= 23)
		document.basic_form.n_mcs.options.selectedIndex = ht_mcs_nm;
    else if (ht_mcs_nm == 32)
		document.basic_form.n_mcs.options.selectedIndex = mcs_length-2;
    else if (ht_mcs_nm == 33)
		document.basic_form.n_mcs.options.selectedIndex = mcs_length-1;

	if (1*ht_rdg == 0)
		document.basic_form.n_rdg[0].checked = true;
	else
		document.basic_form.n_rdg[1].checked = true;

	var OptLen = document.basic_form.n_extcha.options.length;
	if (1*ht_extcha == 0)
	{
		if (OptLen > 1)
			document.basic_form.n_extcha.options.selectedIndex = 0;
	}
	else if (1*ht_extcha == 1)
	{
		if (OptLen > 1)
			document.basic_form.n_extcha.options.selectedIndex = 1;
	}
	else
	{
		document.basic_form.n_extcha.options.selectedIndex = 0;
	}

	if ((1*PhyMode == 8) || (1*PhyMode == 11))
	{
		if (document.basic_form.sz11aChannel.options.selectedIndex == 0)
		{
			document.getElementById("extension_channel").style.display = "none";
			document.basic_form.n_extcha.disabled = true;
		}
	}
	else if ((1*PhyMode == 9) || (1*PhyMode == 6))
	{
		if (document.basic_form.sz11gChannel.options.selectedIndex == 0)
		{
			document.getElementById("extension_channel").style.display = "none";
			document.basic_form.n_extcha.disabled = true;
		}
	}

	if (1*ht_amsdu == 0)
		document.basic_form.n_amsdu[0].checked = true;
	else
		document.basic_form.n_amsdu[1].checked = true;

	if (1*ht_autoba == 0)
		document.basic_form.n_autoba[0].checked = true;
	else
		document.basic_form.n_autoba[1].checked = true;

	if (1*ht_badecline == 0)
		document.basic_form.n_badecline[0].checked = true;
	else
		document.basic_form.n_badecline[1].checked = true;

	if (1*ht_disallow_tkip == 1)
		document.basic_form.n_disallow_tkip[1].checked = true;
	else
		document.basic_form.n_disallow_tkip[0].checked = true;

	if (1*draft3b == 1)
	{
		document.getElementById("div_2040_coexit").style.display = "none";//by chenfei
		document.basic_form.n_2040_coexit.disabled = false;
		if (1*ht_2040_coexit == 0)
			document.basic_form.n_2040_coexit[0].checked = true;
		else
			document.basic_form.n_2040_coexit[1].checked = true;
	}

	if (1*mesh_include == 1 && 1*apcli_include == 1)
	{
		document.basic_form.mssid_6.disabled = true;
		document.basic_form.mssid_7.disabled = true;
	}
	else if (1*mesh_include == 1 || 1*apcli_include == 1)
		document.basic_form.mssid_7.disabled = true;

	if (txrxStream == "2")
	{
		document.getElementById("div_HtTx2Stream").style.display = "";
		document.getElementById("div_HtRx2Stream").style.display = "";
	}
	else if (txrxStream == "3")
	{
		document.basic_form.rx_stream.options[2] = new Option("3", "3");
		document.basic_form.tx_stream.options[2] = new Option("3", "3");
	}
	else
	{
		document.getElementById("div_HtTx2Stream").style.display = "none";
		tx_stream_idx = 1;
		document.getElementById("div_HtRx2Stream").style.display = "none";
		rx_stream_idx = 1;
	}
	
	document.basic_form.rx_stream.options.selectedIndex = rx_stream_idx - 1;
	document.basic_form.tx_stream.options.selectedIndex = tx_stream_idx - 1;
	
	if (1*radio_off == 1)
		document.basic_form.radioButton.value = "RADIO ON";
	else
		document.basic_form.radioButton.value = "RADIO OFF";
		
	if (1*wifi_off == 1)
		document.basic_form.wlan_disabled.checked = true;
	else
		document.basic_form.wlan_disabled.checked = false;
}

function wirelessModeChange()
{
	document.getElementById("div_11a_channel").style.display = "none";
	document.basic_form.sz11aChannel.disabled = true;
	document.getElementById("div_11b_channel").style.display = "none";
	document.basic_form.sz11bChannel.disabled = true;
	document.getElementById("div_11g_channel").style.display = "none";
	document.basic_form.sz11gChannel.disabled = true;
	document.getElementById("div_abg_rate").style.display = "none";
	document.basic_form.abg_rate.disabled = true;
	document.getElementById("div_11n").style.display = "none";
	document.basic_form.n_mode.disabled = true;
	document.basic_form.n_bandwidth.disabled = true;
	document.basic_form.n_rdg.disabled = true;
	document.basic_form.n_gi.disabled = true;
	document.basic_form.n_mcs.disabled = true;

	var wmode = document.basic_form.wirelessmode.options.selectedIndex;
	if (wmode == 0)
	{
		document.basic_form.wirelessmode.options.selectedIndex = 0;
		document.getElementById("div_11g_channel").style.display = "";
		document.basic_form.sz11gChannel.disabled = false;
	}
	else if (wmode == 1)
	{
		document.basic_form.wirelessmode.options.selectedIndex = 1;
		document.getElementById("div_11b_channel").style.display = "";
		document.basic_form.sz11bChannel.disabled = false;
	}
	else if (wmode == 2)
	{
		document.basic_form.wirelessmode.options.selectedIndex = 2;
		document.getElementById("div_11g_channel").style.display = "";
		document.basic_form.sz11gChannel.disabled = false;
	}
	else if (wmode == 5)
	{
		document.basic_form.wirelessmode.options.selectedIndex = 5;
		document.getElementById("div_11a_channel").style.display = "";
		document.basic_form.sz11aChannel.disabled = false;
	}
	else if ((wmode == 6) || (wmode == 7))
	{
		if (wmode == 7)
			document.basic_form.wirelessmode.options.selectedIndex = 7;
		else
			document.basic_form.wirelessmode.options.selectedIndex = 6;
		
		document.getElementById("div_11a_channel").style.display = "";
		document.basic_form.sz11aChannel.disabled = false;
		document.getElementById("div_11n").style.display = "";
		document.basic_form.n_mode.disabled = false;
		document.basic_form.n_bandwidth.disabled = false;
		document.basic_form.n_rdg.disabled = false;
		document.basic_form.n_gi.disabled = false;
		document.basic_form.n_mcs.disabled = false;

		if (document.basic_form.sz11aChannel.options.selectedIndex == 0)
		{
			document.getElementById("extension_channel").style.display = "none";
			document.basic_form.n_extcha.disabled = true;
		}

		InsExtChOpt();
	}
	else if ((wmode == 3) || (wmode == 4))
	{
		if (wmode == 4)
			document.basic_form.wirelessmode.options.selectedIndex = 4;
		else
			document.basic_form.wirelessmode.options.selectedIndex = 3;

		document.getElementById("div_11g_channel").style.display = "";
		document.basic_form.sz11gChannel.disabled = false;
		document.getElementById("div_11n").style.display = "";
		document.basic_form.n_mode.disabled = false;
		document.basic_form.n_bandwidth.disabled = false;
		document.basic_form.n_rdg.disabled = false;
		document.basic_form.n_gi.disabled = false;
		document.basic_form.n_mcs.disabled = false;

		if (document.basic_form.sz11gChannel.options.selectedIndex == 0)
		{
			document.getElementById("extension_channel").style.display = "none";
			document.basic_form.n_extcha.disabled = true;
		}

		InsExtChOpt();
	}

	//ABG Rate
	if ((wmode == 0) || (wmode == 2) || (wmode == 5))
	{
		document.basic_form.abg_rate.options.length = 0;
		document.basic_form.abg_rate.options[0] = new Option(MM_auto, "0");
		document.basic_form.abg_rate.options[1] = new Option("1 Mbps", "1");
		document.basic_form.abg_rate.options[2] = new Option("2 Mbps", "2");
		document.basic_form.abg_rate.options[3] = new Option("5.5 Mbps", "5");
		document.basic_form.abg_rate.options[4] = new Option("6 Mbps", "6");
		document.basic_form.abg_rate.options[5] = new Option("9 Mbps", "9");
		document.basic_form.abg_rate.options[6] = new Option("11 Mbps", "11");
		document.basic_form.abg_rate.options[7] = new Option("12 Mbps", "12");
		document.basic_form.abg_rate.options[8] = new Option("18 Mbps", "18");
		document.basic_form.abg_rate.options[9] = new Option("24 Mbps", "24");
		document.basic_form.abg_rate.options[10] = new Option("36 Mbps", "36");
		document.basic_form.abg_rate.options[11] = new Option("48 Mbps", "48");
		document.basic_form.abg_rate.options[12] = new Option("54 Mbps", "54");
		if (fxtxmode == "CCK" || fxtxmode == "cck") 
		{
			if (ht_mcs.indexOf("33") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 0;
			else if (ht_mcs.indexOf("0") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 1;
			else if (ht_mcs.indexOf("1") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 2;
			else if (ht_mcs.indexOf("2") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 3;
			else if (ht_mcs.indexOf("3") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 6;
		}
		else 
		{
			if (ht_mcs.indexOf("33") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 0;
			else if (ht_mcs.indexOf("0") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 4;
			else if (ht_mcs.indexOf("1") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 5;
			else if (ht_mcs.indexOf("2") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 7;
			else if (ht_mcs.indexOf("3") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 8;
			else if (ht_mcs.indexOf("4") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 9;
			else if (ht_mcs.indexOf("5") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 10;
			else if (ht_mcs.indexOf("6") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 11;
			else if (ht_mcs.indexOf("7") == 0)
				document.basic_form.abg_rate.options.selectedIndex = 12;
		}

		document.getElementById("div_abg_rate").style.display = "";
		document.basic_form.abg_rate.disabled = false;
	}
	else if (wmode == 1)
	{
		document.basic_form.abg_rate.options.length = 0;
		document.basic_form.abg_rate.options[0] = new Option(MM_auto, "0");
		document.basic_form.abg_rate.options[1] = new Option("1 Mbps", "1");
		document.basic_form.abg_rate.options[2] = new Option("2 Mbps", "2");
		document.basic_form.abg_rate.options[3] = new Option("5.5 Mbps", "5");
		document.basic_form.abg_rate.options[4] = new Option("11 Mbps", "11");
		if (ht_mcs.indexOf("33") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 0;
		else if (ht_mcs.indexOf("0") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 1;
		else if (ht_mcs.indexOf("1") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 2;
		else if (ht_mcs.indexOf("2") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 3;
		else if (ht_mcs.indexOf("3") == 0)
			document.basic_form.abg_rate.options.selectedIndex = 4;

		document.getElementById("div_abg_rate").style.display = "";
		document.basic_form.abg_rate.disabled = false;
	}
}

function switch_hidden_ssid()
{
	var BssidNum = 1*'<% getCfgGeneral(1, "BssidNum"); %>';
	if (document.basic_form.broadcastssid[0].checked == true)
		for (i=0;i<BssidNum;i++)
			document.basic_form.hssid[i].checked = false;
	else
		for (i=0;i<BssidNum;i++)
			document.basic_form.hssid[i].checked = true;
}

function switch_isolated_ssid()
{
	var BssidNum = 1*'<% getCfgGeneral(1, "BssidNum"); %>';
	if (document.basic_form.apisolated[0].checked == true)
		for (i=0;i<BssidNum;i++)
			document.basic_form.isolated_ssid[i].checked = true;
	else
		for (i=0;i<BssidNum;i++)
			document.basic_form.isolated_ssid[i].checked = false;
}

function formCheck()
{
	if (!ssidCheck(document.basic_form.mssid_0, MM_ssid)) 
		return false;

	var wpsenable = "<% getCfgZero(1, "WscModeOption"); %>";
	if (wpsenable != "0" && document.basic_form.hssid[0].checked == true)
		alert("WPS feature is going to turn off because of disabled broadcasting of SSID");

	var submit_ssid_num = 1;

	for (i = 1; i < max_bssid_num; i++)
	{
		if (eval("document.basic_form.mssid_"+i).value != "")
		{
			if (i > 5) 
			{
				if (1*mesh_include == 1 && 1*apcli_include == 1)
					break;
				else if (i == (max_bssid_num - 1))
					if (1*mesh_include == 1 || 1*apcli_include == 1)
						break;
			}
			submit_ssid_num++;
		}
	}

	document.basic_form.bssid_num.value = submit_ssid_num;
	return true;
}

function RadioStatusChange(rs)
{
	if (rs == 1) {
		document.basic_form.radioButton.value = "RADIO OFF";
		document.basic_form.radiohiddenButton.value = 0;
	}
	else {
		document.basic_form.radioButton.value = "RADIO ON";
		document.basic_form.radiohiddenButton.value = 1;
	}
}

function WiFiStatusChange(rs)
{
	if (rs == 1) 
		document.basic_form.wifihiddenButton.value = 0;	//disable wlan
	else 
		document.basic_form.wifihiddenButton.value = 1;	//enable wlan
}

function wireless_onoff_click()
{
	if (document.basic_form.wlan_disabled.checked == true)
		WiFiStatusChange(1); 
	else 
		WiFiStatusChange(0); 
	
	document.basic_form.submit();
}

function setWlanSecurity()
{
	window.location.href = "security.asp#flag=0";
}

function resetForm()
{
	location=location; 
}
</script>
</head>
<body onLoad="Load_Setting()">

<form method=post name=basic_form action="/goform/wirelessBasic">
<input type="hidden" name="submit-url" value="/wireless/basic.asp">
<input type=hidden name=wifihiddenButton value="2">
</form>  
</body></html>
