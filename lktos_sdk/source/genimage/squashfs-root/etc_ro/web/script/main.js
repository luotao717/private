
var firstOnload = false;
var first_getConnect = false;
var connect_status=0;

var m_obj_statusData;
var m_editingNewSSID=false;

var m_UartConfigString="19200,8,N,1";


window.onload = function(){
	window.islogin = false;
	window.init18n = false;
	window.langindex = "en_us";
	initI18N(window.langindex);
	onUartLoad();
	onStatusLoad1();
	onSystemLoad();
	onNetworkLoad1();
//	onVideoLoad();
//	onRemoveLoad();
//	setTimeout(function(){
//	}, 1000);
};
var userUri = "/cgi-bin/user.cgi";

var orignalvalue_save = {
	"network_Security":"none",
	"network_password":"none",
	"network_apbridge":"none"
};
var wifiLoaded = false;
var sliderloader =false;
var versioncheck = false;

var FirmVersionInfoPostFilter = new Filter(function (src, command, request, context){
	var lastMsg = context.getAttribute(CommandSupport.commandResultKey);
	var msg = lastMsg.replace("SystemCommandRespone:getfwinfo:", "");
	//msg = msg.split("\n")[0];
	//alert(msg);
	document.getElementById("firmVersionValue").innerHTML = msg;
	context.setAttribute(CommandSupport.commandResultKey, msg);
});

var Toast = function(config){  
    this.context = config.context==null?$('body'):config.context;//上下文  
    this.message = config.message;//显示内容  
    this.time = config.time==null?3000:config.time;//持续时间  
    this.left = config.left;//距容器左边的距离  
    this.top = config.top;//距容器上方的距离  
    this.init();  
}  
var msgEntity;  
Toast.prototype = {  
    //初始化显示的位置内容等  
    init : function(){  
        $("#toastMessage").remove();  
        //设置消息体  
        var msgDIV = new Array();  
        msgDIV.push('<div id="toastMessage">');  
        msgDIV.push('<span>'+this.message+'</span>');  
        msgDIV.push('</div>');  
        msgEntity = $(msgDIV.join('')).appendTo(this.context);  
        //设置消息样式  
        var left = this.left == null ? this.context.width()/2-msgEntity.find('span').width()/2-10 : this.left;  
//        var top = this.top == null ? '20px' : this.top;
        var top = this.top == null ? this.context.height()/2-msgEntity.find('span').height()/2-10 : this.top;  
        msgEntity.css({position:'absolute','border-radius':'15px',top:top,'z-index':'99',left:left,'background-color':'black',color:'white','font-family':'Helvetica,Arial,sans-serif','font-size':'26px','padding':'10px','margin':'10px'});  
        left = this.left == null ? this.context.width()/2-msgEntity.find('span').width()/2 : this.left;  
        top = this.top == null ? this.context.height()/2-msgEntity.find('span').height()/2 : this.top;  
        msgEntity.css({position:'absolute','border-radius':'15px',top:top,'z-index':'99',left:left,'background-color':'black',color:'white','font-family':'Helvetica,Arial,sans-serif','font-size':'26px','padding':'10px','margin':'10px'});  
        msgEntity.hide();  
    },  
    //显示动画  
    show :function(){  
        msgEntity.fadeIn(this.time/2);  
        msgEntity.fadeOut(this.time/2);  
    }  
          
}  

function  trim(str){
    for(var  i  =  0  ;  i<str.length  &&  str.charAt(i)=="  "  ;  i++  )  ;
    for(var  j  =str.length;  j>0  &&  str.charAt(j-1)=="  "  ;  j--)  ;
    if(i>j)  return  "";
    return  str.substring(i,j);  
} 

function onSetUartConfig()
{
	var str;
	str = $("#baudrateSelect").val();
	str += ",";
	str += $("#databitsSelect").val();
	str += ",";
	str += $("#paritySelect").val();
	str += ",";
	str += $("#stopbitsSelect").val();
	
	m_UartConfigString = str;
	
	sendCommand("setUartCmd:"+str, null);
	
	new Toast({context:$('body'),message:i18n_alert[window.langindex]["uart.please_wait"]}).show(); 	
			
}


function onGetUartConfig(req)
{
	if(req == "")
	{
	}
	else
	{
		m_UartConfigString = req.responseText;
		arr = m_UartConfigString.split(",");
 		// $("#baudrateSelect").empty();
		// $("#baudrateSelect").append("<option value='300'>300</option>"); 	
		// $("#baudrateSelect").append("<option value='600'>600</option>"); 	
		// $("#baudrateSelect").append("<option value='1200'>1200</option>"); 	
		// $("#baudrateSelect").append("<option value='1800'>1800</option>"); 	
		// $("#baudrateSelect").append("<option value='2400'>2400</option>"); 	
		// $("#baudrateSelect").append("<option value='4800'>4800</option>"); 	
		// $("#baudrateSelect").append("<option value='9600'>9600</option>"); 	
		// $("#baudrateSelect").append("<option value='19200'>19200</option>"); 	
		// $("#baudrateSelect").append("<option value='38400'>38400</option>"); 	
		$("#baudrateSelect").val(trim(arr[0]));
		$( "#baudrateSelect" ).selectmenu( "refresh" );
		$("#databitsSelect").val(trim(arr[1]));
		$( "#databitsSelect" ).selectmenu( "refresh" );
		$("#paritySelect").val(trim(arr[2]));
		$( "#paritySelect" ).selectmenu( "refresh" );
		$("#stopbitsSelect").val(trim(arr[3]));
		$( "#stopbitsSelect" ).selectmenu( "refresh" );
	}
	
}

function onGetUartExist(req)
{
	if(req.responseText=="1")
	{
		sendCommand("getUartCmd", onGetUartConfig);	
	}
	else
	{
		$(".uartPage").remove();
		$(".liuartPageLink").remove();
	}
}

function onUartLoad()
{
	sendCommand("getUartExist",onGetUartExist);
//		$(".liuartPageLink").hide();
//		$(".systemPage").page( "removeContainerBackground" );
}

function OnWifiLoad(){

	var func = function(){
	//document.getElementById("networkState_ssid").innerHTML=i18n_alert[window.langindex]["network.dlg.connect.getting"];
	//	getWifiConnectAP();
	  $.mobile.showPageLoadingMsg();
		sendCommand("wlanGetApList", showWiFiList);
	window.location.href = "#wifipage";
	};
	func();
	
	wifiLoaded = true;
}

function OnWifiLoad1(){

	var func = function(){
	//document.getElementById("networkState_ssid").innerHTML=i18n_alert[window.langindex]["network.dlg.connect.getting"];
	//	getWifiConnectAP();
	 // $.mobile.showPageLoadingMsg();
		sendCommand("wlanGetApList", showWiFiList);
	};
	func();
	
	wifiLoaded = true;
}



function OnRemotePageLoad(){

	var func = function(){
	setTimeout(function(){
			OnGetPlayState();
	}, 1000);
	window.location.href = "index.html#RemotePage";
	};
	func();
	
}

function onSetNewSSID()
{
	var ssid=$("#textNewSSID").val();
	
	setTimeout(onShowChangedSSID, 500);
	sendCommand("setSSID:"+ssid, null);
}

function onShowChangedSSID()
{
	var elem=document.getElementById("alert_network");
    elem.innerHTML=i18n_alert[window.langindex]["wifi_ssid.notify"];
	var OpenPopup=function(){
		$('#popupDialog_network').popup('open');
	}
	OpenPopup();
}

function onChangeLanguage()
{
	var language=$("#languageSelect").val();
	if(language=="en_us")
	{
		window.langindex = "en_us";
		//initI18N(window.langindex);
	}
	else if(language=="zh_cn")
	{
		window.langindex = "zh_cn";
		//initI18N(window.langindex);		
	}
}

function onStatusLoad1 () {
	var func = function(){
		
		$.getJSON("httpapi.asp?command=getStatus", function(json){
			//isTimeout = 0;
			//isRefreshing=0;
			//alert(json);
			//document.getElementById("languageProperty").innerHTML = extra[document.getElementById("languageProperty").getAttribute("index")]
			//	.prop;
			m_obj_statusData = json;
			
			document.getElementById("languageValue").innerHTML = json.language;
			document.getElementById("deviceNameValue").innerHTML = json.hardware;
			document.getElementById("firmVersionValue").innerHTML = json.firmware;
			document.getElementById("UUIDValue").innerHTML = json.uuid.toUpperCase();
			document.getElementById("Apcli0Value").innerHTML = json.apcli0;
			document.getElementById("Eth2Value").innerHTML = json.eth2;	
			
			//if(window.location.href.indexOf("statusPage")>=0)
			//{		
				$("#textNewSSID").val(json.ssid);
			//}


			if(json.language == "zh_cn"){
				window.langindex = "zh_cn";
			}
			else if(json.language == "en_us"){
				window.langindex = "en_us";
			}
			else{
				window.langindex = "en_us";
			}
			$("#languageSelect").val(window.langindex);
			if(window.init18n == false){
				initI18N(window.langindex);
				window.init18n = true;
			}
			
			if(!firstOnload)
			{
				firstOnload = true;
				getWifiConnectAP();
				var tmp = function(){
					if(first_getConnect == false){
						setTimeout(tmp, 250);
					}else{
						if(!connect_status)
						{
							OnWifiLoad()
						}else
						{
							//OnRemotePageLoad();
							//OnStatusLoad1();
							onSystemLoad();
							window.location.href = "index.html#systemPage";
							onSystemPageActive();
    				       setTimeout(function(){
								OnWifiLoad1();
							}, 1000);
						}
					}
				};
				tmp();
			}
		});
							
	};
	
	$(".statusLink").bind("click", function(){
		func();
	});
	document.getElementById("Refreshwifi").onclick = function(){
	  $.mobile.showPageLoadingMsg();
	  sendCommand("wlanGetApList", showWiFiList);
	};		
//	if(!statusLoaded){
		func();	
//	}

	setTimeout(function(){
			onStatusLoad2();
	}, 5000);

	//setTimeout(OnWifiLoad1,500);
  
}

function onStatusLoad2 () {
	var func = function(){
		
		$.getJSON("httpapi.asp?command=getStatus", function(json){
			//isTimeout = 0;
			//isRefreshing=0;
			//alert(json);
			//document.getElementById("languageProperty").innerHTML = extra[document.getElementById("languageProperty").getAttribute("index")]
			//	.prop;
			m_obj_statusData = json;
			
			document.getElementById("languageValue").innerHTML = json.language;
			document.getElementById("deviceNameValue").innerHTML = json.hardware;
			document.getElementById("firmVersionValue").innerHTML = json.firmware;
			document.getElementById("UUIDValue").innerHTML = json.uuid.toUpperCase();
			document.getElementById("Apcli0Value").innerHTML = json.apcli0;
			document.getElementById("Eth2Value").innerHTML = json.eth2;			
			if(window.location.href.indexOf("statusPage")>=0)
			{		
				$("#textNewSSID").val(json.ssid);
			}


			if(json.language == "zh_cn"){
				window.langindex = "zh_cn";
			}
			else if(json.language == "en_us"){
				window.langindex = "en_us";
			}
			else{
				window.langindex = "en_us";
			}
			$("#languageSelect").val(window.langindex);
			if(window.init18n == false){
				initI18N(window.langindex);
				window.init18n = true;
			}
			
			if(!firstOnload)
			{
				firstOnload = true;
				getWifiConnectAP();
				var tmp = function(){
					if(first_getConnect == false){
						setTimeout(tmp, 250);
					}else{
						if(!connect_status)
						{
							OnWifiLoad()
						}else
						{
							//OnRemotePageLoad();
							//OnStatusLoad1();
							onSystemLoad();
							window.location.href = "index.html#systemPage";
    				       setTimeout(function(){
								OnWifiLoad1();
							}, 1000);
						}
					}
				};
				tmp();
			}
		});
							
	};
	
	$(".statusLink").bind("click", function(){
		func();
	});
		func();	

	setTimeout(function(){
			onStatusLoad2();
	}, 5000);  
}


function onStatusLoad(){
	var eventSrc = "status";
	EventMgr.addListener(EventObjectSupport.SystemInfoFetched, new EventListener(eventSrc, systemStatusFetchedListener));
	EventMgr.addListener(EventObjectSupport.NetworkInfoFetched, new EventListener(eventSrc, networkStatusFetchedListener));
	EventMgr.addListener(EventObjectSupport.VideoInfoFetched, new EventListener(eventSrc, videoStatusFetchedListener));
	//EventMgr.addListener(EventObjectSupport.AudioInfoFetched, new EventListener(eventSrc, audioStatusFetchedListener));
	EventMgr.addListener(EventObjectSupport.VirtualAPFetched, new EventListener(eventSrc, virtualAPStatusFetchedListener));
	EventMgr.addListener(EventObjectSupport.WlanConnInfoFetched, new EventListener(eventSrc, wlanConnInfoStatusFetchedListener));
	EventMgr.addListener(EventObjectSupport.FirmVersionInfoFetched, new EventListener(eventSrc,firmVersionInfoFetchedListener));
	var func = function(){
		
		var events = [ EventObjectSupport.SystemInfoFetched,
						EventObjectSupport.NetworkInfoFetched,
						EventObjectSupport.VideoInfoFetched,
						//EventObjectSupport.AudioInfoFetched,
						EventObjectSupport.VirtualAPFetched,
						EventObjectSupport.WlanConnInfoFetched,
						EventObjectSupport.FirmVersionInfoFetched ];
		var sel = new EventSelector();
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetSystemControl);
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetNetWorkControl);
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetVideoControl);
		//CommandSupport.sendCommand(eventSrc, CommandStrings.GetAudioControl);
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetVirtualAPControl);
	//	CommandSupport.sendCommand(eventSrc, CommandStrings.GetWlanConnectInfo, null, [WlanConnInfoPostFilter]);
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetFirmVersionInfo, null, [FirmVersionInfoPostFilter]);
		
		sel.waitAll(eventSrc, events, function() {
			if(!firstOnload)
			{
				firstOnload = true;
				getWifiConnectAP();
				var tmp = function(){
					if(first_getConnect == false){
						setTimeout(tmp, 250);
					}else{
						if(!connect_status)
						{
							OnWifiLoad()
						}else
						{
							OnRemotePageLoad();
						}
					}
				};
				tmp();
			}
			
		});
	};

	
	$(".statusLink").bind("click", function(){
		func();
	});
	document.getElementById("Refreshwifi").onclick = function(){
	  $.mobile.showPageLoadingMsg();
	  sendCommand("wlanGetApList", showWiFiList);
	};		
//	if(!statusLoaded){
		func();	
//	}
}
function systemStatusFetchedListener(src, eventObj, extra){

	alert(src);
	alert(eventObj);
	alert(extra);

	document.getElementById("languageProperty").innerHTML = extra[document.getElementById("languageProperty").getAttribute("index")]
		.prop;
	document.getElementById("languageValue").innerHTML = extra[document.getElementById("languageValue").getAttribute("index")]
		.value[document.getElementById("languageValue").getAttribute("index")].text;
	var langVal = extra[document.getElementById("languageValue").getAttribute("index")]
	.value[document.getElementById("languageValue").getAttribute("index")].value;
	
	if(langVal == "[OSD_SCHINESE]"){
		window.langindex = "zh_cn";
	}
	else if(langVal == "[OSD_ENGLISH]"){
		window.langindex = "en_us";
	}
	else{
		window.langindex = "en_us";
	}
	if(window.init18n == false){
		initI18N(window.langindex);
		window.init18n = true;
	}
}

function networkStatusFetchedListener(src, eventObj, extra){
	
	document.getElementById("deviceNameProperty").innerHTML = extra[document.getElementById("deviceNameProperty").getAttribute("index")]
		.prop;
	document.getElementById("deviceNameValue").innerHTML = extra[document.getElementById("deviceNameValue").getAttribute("index")]
		.value;
	
	window.document.title = extra[document.getElementById("deviceNameValue").getAttribute("index")].value;
	
	
	document.getElementById("virtualApBridgeProperty").innerHTML = extra[document.getElementById("virtualApBridgeProperty").getAttribute("index")]
		.prop;
	document.getElementById("virtualApBridgeValue").innerHTML = extra[document.getElementById("virtualApBridgeValue").getAttribute("index")]
		.value[0].text;
}

function videoStatusFetchedListener(src, eventObj, extra){
	document.getElementById("aspectRatioProperty").innerHTML = extra[document
			.getElementById("aspectRatioProperty").getAttribute("index")].prop;
	document.getElementById("aspectRatioValue").innerHTML = extra[document
			.getElementById("aspectRatioValue").getAttribute("index")].value[0].text;
	
	document.getElementById("tvSystemProperty").innerHTML = extra[document
			.getElementById("tvSystemProperty").getAttribute("index")].prop;
	document.getElementById("tvSystemValue").innerHTML = "HDMI";//extra[document
	//		.getElementById("tvSystemValue").getAttribute("index")].value[0].text;
	
	document.getElementById("scaleVideoProperty").innerHTML = extra[document
			.getElementById("scaleVideoProperty").getAttribute("index")].prop;
	document.getElementById("scaleVideoValue").innerHTML = extra[document
			.getElementById("scaleVideoValue").getAttribute("index")].value[0].text;
	
	document.getElementById("videoFullProperty").innerHTML = extra[document
			.getElementById("videoFullProperty").getAttribute("index")].prop;
	document.getElementById("videoFullValue").innerHTML = extra[document
			.getElementById("videoFullValue").getAttribute("index")].value[0].text;
	
	document.getElementById("vgaResProperty").innerHTML = extra[document
			.getElementById("vgaResProperty").getAttribute("index")].prop;
	document.getElementById("vgaResValue").innerHTML = extra[document
			.getElementById("vgaResValue").getAttribute("index")].value[0].text;
}

function virtualAPStatusFetchedListener(src, eventObj, extra){
	document.getElementById("wirelessSecurityProperty").innerHTML = extra[document
			.getElementById("wirelessSecurityProperty").getAttribute("index")].prop;
	document.getElementById("wirelessSecurityValue").innerHTML = extra[document
			.getElementById("wirelessSecurityValue").getAttribute("index")].value[0].text;
}

function wlanConnInfoStatusFetchedListener(src, eventObj, extra){
	
	document.getElementById("connectedLanProperty").innerHTML = extra[document
			.getElementById("connectedLanProperty").getAttribute("index")].prop;
	document.getElementById("connectedLanValue").innerHTML = extra[document
			.getElementById("connectedLanValue").getAttribute("index")].value;
}

function firmVersionInfoFetchedListener(src, eventObj, extra){
	document.getElementById("firmVersionValue").innerHTML = extra[document
			.getElementById("firmVersionValue").getAttribute("index")].value;
}

function onSystemLoad(){
	var eventSrc = "system";
	EventMgr.addListener(EventObjectSupport.NetworkInfoFetched, new EventListener(eventSrc, deviceFriendlyNameFetchedListener));
	EventMgr.addListener(EventObjectSupport.SystemInfoFetched, new EventListener(eventSrc, systemInfoFetchedListener));
	
	$(".systemLink").bind("click", function(){
		var events = [ EventObjectSupport.SystemInfoFetched,
		   			EventObjectSupport.NetworkInfoFetched];
		var sel = new EventSelector();
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetNetWorkControl);
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetSystemControl);
		sel.waitAll(eventSrc, events, function() {
			document.getElementById("onlineup_upgrade").onclick = function(){
				if(document.getElementById("firmFile").value == ""){
					//alert(i18n_alert[window.langindex]["online_update_alert.file"]);
					//	window.location.href = "#popupDialog_test";
					// $("#onlineup_upgrade").href("popupDialog_test");
					 var OpenPopup=function(){
					    $('#popupDialog_nofile').popup('open');
					}
					OpenPopup();
					return;
				}
				else{
					initUploadFrame();
					document.getElementById("firmware_update_process").style.visibility = "visible";
					ShowFirmwareUploadPercent(0);
					document.getElementById("onlineup_upform").submit();
				}
			};
		});
	});

	//document.getElementById("system.passSel").onchange = function(){
	//	if(this.value == "0"){
	//		document.getElementById("system.usernameTxt").disabled = "disabled";
	//		document.getElementById("system.passwordTxt").disabled = "disabled";
	//	}
	//	else{
	//		document.getElementById("system.usernameTxt").disabled = "";
	//		document.getElementById("system.passwordTxt").disabled = "";
	//	}
	//};
	document.getElementById("status_restore").onclick = function(){
		//if(confirm(i18n_alert[window.langindex]["system_restore.factory"])){
		//	CommandSupport.sendCommand(eventSrc, CommandStrings.RestoreFactory);
		//}
		sendCommand("restoreToDefault", funRestoreToDefaultReturn);

		
	};
	
	document.getElementById("check_online_update").onclick = function(){
		funStartCheckRemoteUpdate();		
	};	

	document.getElementById("online_update").onclick = function(){
		//CommandSupport.sendCommand(eventSrc, CommandStrings.OnlineupdateCheck);
		//var onlinestatus="CHECKING";
		//versioncheck = false;
		//GetOnlineUpdateStatus(onlinestatus);

		funStartRemoteUpdate();
	};	
	
	$("#div_online_update").hide();
	$("#div_check_online_update").show();
	//initI18N(window.langindex);
		
	document.getElementById("system_save").onclick = function(){
		//CommandSupport.sendCommand(eventSrc, CommandStrings.ChangeLanguage + document.getElementById("languageSelect").value);
		sendCommand("setLanguage:"+document.getElementById("languageSelect").value, null);

		//if(device_save_last["deivce_name"]!=document.getElementById("deviceNameInput").value)
		//{
		//	device_save_last["deivce_name"]=document.getElementById("deviceNameInput").value;
		//	CommandSupport.sendCommand(eventSrc, CommandStrings.SetDeviceFriendlyName + document.getElementById("deviceNameInput").value);
		//}
		setTimeout(function(){
			window.location.href = "#wifipage";
			window.location.reload();
		}, 1000);
	};
	
//	document.getElementById("system_reset").onclick = function(){
		//CommandSupport.sendCommand(eventSrc, CommandStrings.GetNetWorkControl);
		//CommandSupport.sendCommand(eventSrc, CommandStrings.GetSystemControl);
//		setTimeout(function(){
//				onSystemLoad();
//			}, 100);
//	};
}

var count1=0;
function onSystemPageActive()
{
	count1++;
	funStartCheckRemoteUpdate();
}

function deviceFriendlyNameFetchedListener(src, eventObj, extra){
	var input = document.getElementById("deviceNameInput"); 
	input.value = extra[parseInt(input.getAttribute("index"))].value;
	//device_save_last["deivce_name"]=input.value;
	var span = document.getElementById("deviceNameSpan"); 
	span.innerHTML = extra[parseInt(input.getAttribute("index"))].prop;
}

function systemInfoFetchedListener(src, eventObj, extra){
	evalSpanAndSelect("languageSpan", "languageSelect", extra, 2);
}

function onChangeSecuritySelect()
{
	if($("#wirelessSecuritySelect").val() == "open")
	{
		//$("#divwirelessPasswordInput").hide();
		//document.getElementById("wirelessPasswordInputInput").disabled = "disabled";
		$( "#wirelessPasswordInputInput" ).textinput( "disable" );
	}	
	else
	{
		//$("#divwirelessPasswordInput").show();		
	//		document.getElementById("wirelessPasswordInputInput").disabled = "";
		$( "#wirelessPasswordInputInput" ).textinput( "enable" );
	}
}


function onNetworkLoad1()
{
//	var func = function(){
		//$("#divwirelessPasswordInput").hide();			
		document.getElementById("wirelessPasswordInputInput").disabled = true;
		
		$.getJSON("httpapi.asp?command=getNetwork", function(json){


			if(json.securemode == "0")
			{
				document.getElementById("wirelessSecuritySelect").value = "open";	
				//$("#divwirelessPasswordInput").hide();			
				document.getElementById("wirelessPasswordInputInput").disabled = true;
			}
			else
			{
				document.getElementById("wirelessSecuritySelect").value = "wpapsk";				
				//$("#divwirelessPasswordInput").show();			
				document.getElementById("wirelessPasswordInputInput").disabled = false;
			}
			
			document.getElementById("wirelessPasswordInputInput").value = json.psk;
		});
//	};	
	
	document.getElementById("network_save").onclick = function(){
		if(document.getElementById("wirelessSecuritySelect").value == "open"){
//			if(orignalvalue_save["network_Security"] != "open")
//			{
//				CommandSupport.sendCommand(eventSrc, CommandStrings.SetVirtualAPSetup + document.getElementById("wirelessSecuritySelect").value);
//				orignalvalue_save["network_Security"] = "[NETWL_SEC_OPEN]";
//				var elem=document.getElementById("alert_network");
//			    elem.innerHTML=i18n_alert[window.langindex]["wifi_encrypt.notify"];
//				//alert(i18n_alert[window.langindex]["wifi_encrypt.notify"]);
//				var OpenPopup=function(){
//					$('#popupDialog_network').popup('open');
//				}
//				OpenPopup();
//			}
			var cmd = "setNetwork:0: ";
			sendCommand(cmd, null);
			var elem=document.getElementById("alert_network");
		    elem.innerHTML=i18n_alert[window.langindex]["wifi_encrypt.notify"];
			//alert(i18n_alert[window.langindex]["wifi_encrypt.notify"]);
			var OpenPopup=function(){
				$('#popupDialog_network').popup('open');
			}
			OpenPopup();
		}
		else{
			
			var password = document.getElementById("wirelessPasswordInputInput").value;

//			if(orignalvalue_save["network_password"] == "true")
//			{
//				orignalvalue_save["network_password"] = "false";
//				var password = document.getElementById("wirelessPasswordInputInput").value;
//				if(password == ""){
//				//	alert(i18n_alert[window.langindex]["wifi_encrypt.password_no"]);
//					var elem=document.getElementById("alert_network");
//				    elem.innerHTML=i18n_alert[window.langindex]["wifi_encrypt.password_no"];
//					var OpenPopup=function(){
//						$('#popupDialog_network').popup('open');
//					}
//					OpenPopup();
//					return;
//				}
//				else 
				if(password.length != 8){
				//	alert(i18n_alert[window.langindex]["wifi_encrypt.password_length"]);
					var elem=document.getElementById("alert_network");
				    elem.innerHTML=i18n_alert[window.langindex]["wifi_encrypt.password_length"];
					var OpenPopup=function(){
						$('#popupDialog_network').popup('open');
					}
					OpenPopup();
					return;
				}
				else
				{
					var cmd = "setNetwork:1:"+password;
					sendCommand(cmd, null);
//					CommandSupport.sendCommand(eventSrc, CommandStrings.SetVirtualAPSetup + document.getElementById("wirelessSecuritySelect").value + "+" + password);
					//alert(i18n_alert[window.langindex]["wifi_encrypt.notify"]);
					var elem=document.getElementById("alert_network");
				    elem.innerHTML=i18n_alert[window.langindex]["wifi_encrypt.notify"];
					var OpenPopup=function(){
						$('#popupDialog_network').popup('open');
					}
					OpenPopup();
				}
				
				
			}
//		}

		
		
	};

//	document.getElementById("network_reset").onclick = function(){
//		setTimeout(function(){
//				onNetworkLoad1();
//			}, 100);
//	};
	
}

function onNetworkLoad(){
	var eventSrc = "network";
	EventMgr.addListener(EventObjectSupport.VirtualAPFetched, new EventListener(eventSrc, virtualAPFetchedListener));
	EventMgr.addListener(EventObjectSupport.NetworkInfoFetched, new EventListener(eventSrc, networkInfoFetchedListener));
	
	$(".networkLink").bind("click", function(){
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetVirtualAPControl);
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetNetWorkControl);
	});
	
	document.getElementById("network_save").onclick = function(){
		if(document.getElementById("wirelessSecuritySelect").value == "[NETWL_SEC_OPEN]"){
			if(orignalvalue_save["network_Security"] != "[NETWL_SEC_OPEN]")
			{
				CommandSupport.sendCommand(eventSrc, CommandStrings.SetVirtualAPSetup + document.getElementById("wirelessSecuritySelect").value);
				orignalvalue_save["network_Security"] = "[NETWL_SEC_OPEN]";
				var elem=document.getElementById("alert_network");
			    elem.innerHTML=i18n_alert[window.langindex]["wifi_encrypt.notify"];
				//alert(i18n_alert[window.langindex]["wifi_encrypt.notify"]);
				var OpenPopup=function(){
					$('#popupDialog_network').popup('open');
				}
				OpenPopup();
			}
		}
		else{

			if(orignalvalue_save["network_password"] == "true")
			{
				orignalvalue_save["network_password"] = "false";
				var password = document.getElementById("wirelessPasswordInput").value;
				if(password == ""){
				//	alert(i18n_alert[window.langindex]["wifi_encrypt.password_no"]);
					var elem=document.getElementById("alert_network");
				    elem.innerHTML=i18n_alert[window.langindex]["wifi_encrypt.password_no"];
					var OpenPopup=function(){
						$('#popupDialog_network').popup('open');
					}
					OpenPopup();
					return;
				}
				else if(password.length != 8){
				//	alert(i18n_alert[window.langindex]["wifi_encrypt.password_length"]);
					var elem=document.getElementById("alert_network");
				    elem.innerHTML=i18n_alert[window.langindex]["wifi_encrypt.password_length"];
					var OpenPopup=function(){
						$('#popupDialog_network').popup('open');
					}
					OpenPopup();
					return;
				}else
				{
					CommandSupport.sendCommand(eventSrc, CommandStrings.SetVirtualAPSetup + document.getElementById("wirelessSecuritySelect").value + "+" + password);
					//alert(i18n_alert[window.langindex]["wifi_encrypt.notify"]);
					var elem=document.getElementById("alert_network");
				    elem.innerHTML=i18n_alert[window.langindex]["wifi_encrypt.notify"];
					var OpenPopup=function(){
						$('#popupDialog_network').popup('open');
					}
					OpenPopup();
				}
			}
		}
		
		if(orignalvalue_save["network_apbridge"] != document.getElementById("virtualApBridgeSelect").value)
		{
			CommandSupport.sendCommand(eventSrc, CommandStrings.SetVirtualAPBridge + document.getElementById("virtualApBridgeSelect").value);
			orignalvalue_save["network_apbridge"] = document.getElementById("virtualApBridgeSelect").value;
			var elem=document.getElementById("alert_network");
			 elem.innerHTML=i18n_alert[window.langindex]["wifi_apbridge.notify"];
			var OpenPopup=function(){
				$('#popupDialog_network').popup('open');
			}
					OpenPopup();
			//alert(i18n_alert[window.langindex]["wifi_apbridge.notify"]);
		}
	};
	
	document.getElementById("network_reset").onclick = function(){
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetVirtualAPControl);
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetNetWorkControl);
		
	};
	
}
function SeekPlayer()
{
	var elem=document.getElementById("player-slider");
 	sendCommand('SetPlayInfo'+elem.value, function(req){
 	});
}
function OnGetPlayState()
{
	if(!sliderloader)
	{
		sliderloader = true;
   		$( "#player-slider" ).on( 'slidestop', function( event, ui ) {
	   		SeekPlayer();
	   });
	}

	 sendCommand('getPlayInfo', function(req){
	 	var resp = req.responseText;
		var strArr = resp.split("&");
			 var playstate = strArr[0]
			 var play_context = strArr[1]
			 var TotalSeconds = strArr[2]
			 var CurrentSeconds = strArr[3];

		var elem=document.getElementById("player-slider");
		elem.value=CurrentSeconds;
		elem.max = TotalSeconds;
		elem.min = 0;
	
  		$("#player-slider").slider('refresh');
		var elem1=document.getElementById("total_seconds");
		elem1.innerHTML=strArr[4];
		var elem1=document.getElementById("current_seconds");
		elem1.innerHTML=strArr[5];
      });
	 
	setTimeout(function(){
			OnGetPlayState();
		}, 2000);
}

function onRemoveLoad()
{
	var eventSrc = "system";
	document.getElementById("resetbutton").onclick = function(){
		var OpenPopup=function(){
				$('#popupDialog_reset').popup('open');
		}
		OpenPopup();
	};

	document.getElementById("reset_device").onclick = function(){
		//if(confirm(i18n_alert[window.langindex]["device_reset.confirm"])){
			CommandSupport.sendCommand(eventSrc, CommandStrings.ReBootDevice);
		//}
		
	};
		
	$(".RemoteLink").bind("click", function(){
	
   		$( "#player-slider" ).on( 'slidestop', function( event, ui ) {
	   		SeekPlayer();
	   });
		setTimeout(function(){
			OnGetPlayState();
		}, 1000);
	});

		
   	$( "#player-slider" ).on( 'slidestop', function( event, ui ) {
	   		SeekPlayer();
	 });
	//$( "#player-slider" ).slider();

	//('.selector').slider({ start: function(event, ui) { alert(i18n_alert[window.langindex]["wifi_apbridge.notify"]);} });    
	//('.selector').bind('slidestart', function(event, ui) {});    

}
function virtualAPFetchedListener(src, eventObj, extra){
	evalSpanAndSelect("wirelessSecuritySpan", "wirelessSecuritySelect", extra);
	document.getElementById("wirelessPasswordSpan").innerHTML = extra[1]["prop"];
	
	orignalvalue_save["network_Security"]=document.getElementById("wirelessSecuritySelect").value;
		
	if(document.getElementById("wirelessSecuritySelect").value == "[NETWL_SEC_OPEN]"){
		document.getElementById("wirelessPasswordInput").disabled = true;
	}
	
	document.getElementById("wirelessSecuritySelect").onchange = function(){
		if(this.value == "[NETWL_SEC_OPEN]"){
			document.getElementById("wirelessPasswordInput").disabled = true;
		}
		else{
			document.getElementById("wirelessPasswordInput").disabled = false;
		}
	};
	document.getElementById("wirelessPasswordInput").onchange = function(){
		orignalvalue_save["network_password"]="true";
	}
}

function networkInfoFetchedListener(src, eventObj, extra){
	evalSpanAndSelect("virtualApBridgeSpan", "virtualApBridgeSelect", extra);
	orignalvalue_save["network_apbridge"]=document.getElementById("virtualApBridgeSelect").value;
}
function onVideoLoad(){
	var eventSrc = "video";
	EventMgr.addListener(EventObjectSupport.VideoInfoFetched, new EventListener(eventSrc, videoInfoFetchedListener));
	$(".videoLink").bind("click", function(){
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetVideoControl);
	});
	
	document.getElementById("video_save").onclick = function(){
		//CommandSupport.sendCommand(eventSrc, CommandStrings.SetVideoRatio + document.getElementById("aspactRatioSelect").value);
		CommandSupport.sendCommand(eventSrc, CommandStrings.SetTVSystem + document.getElementById("tvSystemSelect").value);
		CommandSupport.sendCommand(eventSrc, CommandStrings.SetScaleVideoOutput + document.getElementById("scaleVideoSelect").value);
		CommandSupport.sendCommand(eventSrc, CommandStrings.SetVideoFullScreent + document.getElementById("videoFullSelect").value);
		//CommandSupport.sendCommand(eventSrc, CommandStrings.Set1080P24HZ + document.getElementById("1080P24HzSelect").value);
		//CommandSupport.sendCommand(eventSrc, CommandStrings.SetVGAOutput + document.getElementById("vgaResSelect").value);
	};
	document.getElementById("video_reset").onclick = function(){
		CommandSupport.sendCommand(eventSrc, CommandStrings.GetVideoControl);
	};
	
}

function videoInfoFetchedListener(src, eventObj, extra){
	evalSpanAndSelect("aspactRatioSpan", "aspactRatioSelect", extra);
	evalSpanAndSelect("tvSystemSpan", "tvSystemSelect", extra, null, function(index, item){
		if(item.value == "[1080P_60HZ]" || item.value == "[HDMI_AUTO_DETECT]" || item.value == "[720P_60HZ]"){
			return false;
		}
		else{
			return true;
		}
	});
	evalSpanAndSelect("scaleVideoSpan", "scaleVideoSelect", extra);
	evalSpanAndSelect("videoFullSpan", "videoFullSelect", extra);
	evalSpanAndSelect("1080P24HzSpan", "1080P24HzSelect", extra);
	evalSpanAndSelect("vgaResSpan", "vgaResSelect", extra);
}


function evalSpanAndSelect(spanId, selId, obj, limit, filter){
	var select = document.getElementById(selId);
	select.innerHTML = "";
	if(select.getAttribute("index") == null)
		return;
	var index = parseInt(select.getAttribute("index"));
	select.name = obj[index]["prop"];
	if(document.getElementById(spanId) != null)
		document.getElementById(spanId).innerHTML = select.name;
	
	var items = obj[index]["value"];
	var len = items.length;
	if(limit != null){
		len = limit;
	}
	for(var i=0; i<len; i++){
		if(filter != null && filter(i, items[i])){
			continue;
		}
		else{
			var option = document.createElement("option");
			option.text = items[i].text;
			option.value = items[i].value;
			select.options[select.options.length] = option;
		}
	}
	
	var myselect = $("#" + selId);
	myselect[0].selectedIndex = 0;
	myselect.selectmenu("refresh");

	
}

var m_SavedWifiConnCounter1=0;

function getWifiConnectStatus(a,wifistate)
{
	if(wifistate == "FAIL"){
		
	  m_SavedWifiConnCounter1++;
	  
	  if(m_SavedWifiConnCounter1>=10)
	  {
		  sendCommand("wlanStopConnect", function(req){
			 // alert("connecting wlan failed");
			 a.style.backgroundImage="";
			 document.getElementById("networkState_change").innerHTML=i18n_alert[window.langindex]["wifi_connect.password_empty"];
			 getWifiConnectAP();
		  });  	  	
	  }	
	  else
	  {
       setTimeout(function(){
			getWifiConnectStatus(a,wifistate);
		}, 1000);	  	
	  }
		
	  return;
	}
	else if(wifistate == "PAIRFAIL")
	{
		a.style.backgroundImage="";
		document.getElementById("wirelessPasswordInputInput").value = "";
		document.getElementById("networkState_change").innerHTML=i18n_alert[window.langindex]["network.dlg.disconnect"];
		var elem=document.getElementById("alert_wifi");
		elem.innerHTML=i18n_alert[window.langindex]["wifi_connect.password_wrong"];
		var OpenPopup=function(){
			$('#popupDialog_wifi_alert').popup('open');
		}
		OpenPopup();
		return;
	}
	
	m_SavedWifiConnCounter1=0;
	
	if(wifistate != "OK") {
		if(wifistate == "START")
		{
			document.getElementById("networkState_change").innerHTML=i18n_alert[window.langindex]["network.dlg.start"];
		}else if(wifistate == "PROCESS")
		{
			document.getElementById("networkState_change").innerHTML=i18n_alert[window.langindex]["network.dlg.process"];
		}
		
       	  sendCommand('wlanGetConnectState', function(req){
    	   wifistate = req.responseText;//req.responseText.split(":")[2].substring(0, req.responseText.split(":")[2].length-2);
       });
       setTimeout(function(){
			getWifiConnectStatus(a,wifistate);
		}, 1000);
       return;
	}

	
	if(wifistate == "OK"){
		  //  sendCommand('wlanGetConnectInfo', function(req){
		//	alert("wlan connected");
		//	document.getElementById("networkState_change").innerHTML="wlan connected";
		//});
		a.style.backgroundImage="";
			getWifiConnectAP();
		return;
	}
}
function onAPConnected(req){
	//alert("start connecting ...");
	document.getElementById("networkState_ssid").innerHTML="";
	document.getElementById("networkState_change").innerHTML=i18n_alert[window.langindex]["network.dlg.disconnect"];
	var elem=document.getElementById("alert_wifi");
	elem.innerHTML=i18n_alert[window.langindex]["wifi_connect.notify"];
	//alert(i18n_alert[window.langindex]["wifi_encrypt.notify"]);
	var OpenPopup=function(){
		$('#popupDialog_wifi_alert').popup('open');
	}
	OpenPopup();
}
function connectToAP(a){
	
	 var myElement = document.getElementById("wifilist_passwordSpan"); 
	if(myElement)
	  	myElement.parentNode.removeChild(myElement); 
	  	
	if(a.wifi.bssid.substring(0,8)=="00:22:6c")
	{
		var elem=document.getElementById("alert_wifi");
		elem.innerHTML=i18n_alert[window.langindex]["wifi_connect.no_cascade"];
		var OpenPopup=function(){
			$('#popupDialog_wifi_alert').popup('open');
		}
		OpenPopup();		
	}
	else
	{
		if(a.wifi.level != "OPEN"){
		/*
			var span = document.createElement("span");
			var Str="";
			//Str+= "<span id=wifilist_passwordSpan style=display:none>";
			Str+= "<input id=wiFiPwd type=password size=width:50px style=width:30%;height:24px;font-size:16px>";
			Str+= "<input id=wiFiBtn type=button style=height:24px;font-size:16px value=";
			Str+= i18n[window.langindex]["network.connect"]
			Str+= ">";
			//Str+= "</span>";
			span.innerHTML=Str;
			span.style.display = "inline";
			span.setAttribute("id","wifilist_passwordSpan");
			a.parentNode.appendChild(span);
		*/
			var OpenPopup=function(){
				$('#popupDialog_wifi').popup('open');
			}
			OpenPopup();
			document.getElementById("wiFiBtn").onclick = function(){
				var pwd = document.getElementById("wiFiPwd").value;
				if(pwd == null || pwd == ""){
					alert(i18n_alert[window.langindex]["wifi_connect.password_empty"]);
				}
				else{
					a.style.backgroundImage="url(style/image/jindu.gif)";
					a.style.backgroundRepeat="no-repeat";
					a.style.backgroundPosition="center";
					var cmd = "wlanConnectAp:" + a.wifi.ssid + ":" + a.wifi.channel + ":" + a.wifi.auth + ":" + a.wifi.encry+":"+pwd+":"+a.wifi.extch;
					sendCommand(cmd, onAPConnected);	
					var wifistate = "NONE";
					getWifiConnectStatus(a,wifistate);
					span.style.display = "none";
					document.getElementById("wiFiPwd").value = "";
					document.getElementById("wiFiBtn").onclick = null;
				}
			};
			
		}
		else{
			a.style.backgroundImage="url(style/image/jindu.gif)";
			a.style.backgroundRepeat="no-repeat";
			a.style.backgroundPosition="center";
			var cmd = "wlanConnectAp:" + a.wifi.ssid + ":" + a.wifi.channel + ":" + a.wifi.auth + ":" + a.wifi.encry+":"+"NONE"+":"+a.wifi.extch;
			sendCommand(cmd, onAPConnected);
			var wifistate = "NONE";
			getWifiConnectStatus(a,wifistate);
		}
		
	}
	
}
function showWiFiList(req){
	//document.getElementById("networkContentDiv").innerHTML = "";
	$.mobile.hidePageLoadingMsg();

	var resp = req.responseText;
	if(resp == "")
		return;
	var dataObj=eval("("+resp+")");
	var i;
	var index=1;
	for(i=0;i<dataObj.aplist.length;i++)
	{
				var a = document.getElementById("wifi"+index.toString());
				var wifi = new Object;
				wifi.ssid = dataObj.aplist[i].ssid;
				wifi.level = dataObj.aplist[i].auth;
				wifi.secur = dataObj.aplist[i].encry;
				wifi.value = dataObj.aplist[i].rssi;
				wifi.auth =  dataObj.aplist[i].auth;
				wifi.encry = dataObj.aplist[i].encry;
				wifi.channel = dataObj.aplist[i].channel;
				wifi.bssid = dataObj.aplist[i].bssid;
				wifi.extch = dataObj.aplist[i].extch;
				a.wifi = wifi;
				a.onclick = function(){
					connectToAP(this);
				};
				document.getElementById("wifi_pa"+index.toString()).style.display="block";

				var id = document.getElementById("wifi_ssid"+index.toString());
				id.innerHTML=dataObj.aplist[i].ssid;
				var s = document.getElementById("wifi_sign"+index.toString());

				var pox=0;
				if(wifi.level != "OPEN")
				{
					pox=pox+225;
				}
				var level=parseInt(wifi.value);
			   if(level<30)
			       pox+=45;
			   else if(level<50)
			        pox+=45*2;
			   else if(level<70)
			        pox+=45*3;
			   else if(level<90)
			       pox+=45*4;
			   else 
			   		pox+=45*4;
			   
				s.style.backgroundPosition="-"+pox.toString()+"px 0px";
				
				index++;
	}
	
	
//	var strArr = resp.split("\n");
//	var index=1;
//	if(strArr.length > 2){
//		for(var i=2; i<strArr.length; i++){
//			if(strArr[i] != null && strArr[i] != ""){
//				var wifiInfoArr = strArr[i].split(":");
//				//var li= document.createElement("li");
//				//var d = document.createElement("span");
//				//d.style.width = "90%";
//				//d.style.marginLeft = "auto";
//				//d.style.marginRight = "auto";
//				
//				if(wifiInfoArr[0]=="")
//					continue;
//				var a = document.getElementById("wifi"+index.toString());
//				//a.href = "#";
//				//a.style.display="block"
//				//a.style.width="40%";
//				//a.style.float="left";
//				//a.innerText = wifiInfoArr[0];
//				var wifi = new Object;
//				wifi.ssid = wifiInfoArr[0];
//				wifi.level = wifiInfoArr[1];
//				wifi.secur = wifiInfoArr[2];
//				wifi.value = wifiInfoArr[3];
//				a.wifi = wifi;
//				a.onclick = function(){
//					connectToAP(this);
//				};
//				document.getElementById("wifi_pa"+index.toString()).style.display="block";
//
//				var id = document.getElementById("wifi_ssid"+index.toString());
//				id.innerHTML=wifiInfoArr[0];
//				var s = document.getElementById("wifi_sign"+index.toString());
//
//				var pox=0;
//				if(parseInt(wifi.level)!=0)
//				{
//					pox=pox+225;
//				}
//				var level=parseInt(wifi.value);
//			   if(level<30)
//			       pox+=45;
//			   else if(level<50)
//			        pox+=45*2;
//			   else if(level<70)
//			        pox+=45*3;
//			   else if(level<90)
//			       pox+=45*4;
//			   else 
//			   		pox+=45*4;
//			   
//				s.style.backgroundPosition="-"+pox.toString()+"px 0px";
//				
//				index++;
//				//var d = document.createElement("div");
//				//d.style.clear="both";
//				//<div style="clear: both;"></div>
//				//a.style.display = "inline-block";
//				//a.style.width = "250px";
//				//d.appendChild(a);
//				//li.appendChild(a);
//				//li.appendChild(d);
//				//document.getElementById("networkContentDiv").appendChild(li);
//			}
//		}
//	}
//	hideWaitingDialog();
}
function my_sleep(numberMillis) {    
	var now = new Date();    
	var exitTime = now.getTime() + numberMillis;    
	while (true) {        
		now = new Date();        
	if (now.getTime() > exitTime)            
		return;    
}}
function getWifiConnectAP()
{
	sendCommand("GetCurrentWirelessConnect", function(req){
		first_getConnect = true;
		 var ssid=req.responseText.split(":")[1];
		 if(ssid != "NULL")
		 {
			document.getElementById("networkState_ssid").innerHTML=ssid;
			var connect=req.responseText.split(":")[2];
			if(connect == 1)
			{
				connect_status=1;
				document.getElementById("networkState_change").innerHTML=i18n_alert[window.langindex]["network.dlg.connect"];
			}else
			{
				connect_status=0;
				document.getElementById("networkState_change").innerHTML=i18n_alert[window.langindex]["network.dlg.disconnect"];
			}
			//document.getElementById("networkState_change").innerHTML=connect;
		 }else
		 {
		 	connect_status = 0;
			document.getElementById("networkState_ssid").innerHTML="";
			document.getElementById("networkState_change").innerHTML=i18n_alert[window.langindex]["network.dlg.disconnect"];
		 }
		 
	}); 
	
	return connect_status;
}

function handleCommand(command_xmlhttp, callback)
{
  if(command_xmlhttp.readyState==4)
    {
      if(command_xmlhttp.status == 200){
    	//showDiscription(command_xmlhttp.responseText);
    	//alert(command_xmlhttp.responseText);
    	if(callback == null)
    		return;
		callback(command_xmlhttp);
     }
  }
}

function sendCommand(command, callback)
{
	
	//alert(command);
    var command_xmlhttp = GetXmlHttpObject();
    command_xmlhttp.onreadystatechange = function(){
    	handleCommand(command_xmlhttp, callback);
    };
    //command = command.replace(/\+/g, "%2B");
    //var url="ajax.do?command=" + command;
//    var url="/cgi-bin/web.cgi?command=" + command;
//    var url="httpapi.asp?command=" + command;
    
    var url="httpapi.asp";
	
//    command_xmlhttp.open("GET", url ,true);
    command_xmlhttp.open("POST", url ,true);
    command_xmlhttp.setRequestHeader("If-Modified-Since","0");
    command_xmlhttp.setRequestHeader("Cache-Control","no-cache");
    command_xmlhttp.setRequestHeader("CONTENT-TYPE","application/x-www-form-urlencoded"); 

//    command_xmlhttp.send(null);
    command_xmlhttp.send("command="+command);
}


function sendCommandRemoveControl(command)
{
	
	//alert(command);
    var command_xmlhttp = GetXmlHttpObject();
    command_xmlhttp.onreadystatechange = function(){
    	handleCommand(command_xmlhttp, null);
    };
    //command = command.replace(/\+/g, "%2B");
    //var url="ajax.do?command=" + command;
    var url="/cgi-bin/IpodCGI.cgi?id=0&command=" + command;
    command_xmlhttp.open("GET", url ,true);
    command_xmlhttp.setRequestHeader("If-Modified-Since","0");
    command_xmlhttp.setRequestHeader("Cache-Control","no-cache");
    command_xmlhttp.send(null);
}

var firmware_upload_timer;
function initUploadFrame(){
	var iframe = document.getElementById("uploadFrame");
	if (iframe.attachEvent) {
		iframe.attachEvent("onload", function() {
			onFirmFileUploaded(iframe);
		});
	} else {
		iframe.onload = function() {
			onFirmFileUploaded(iframe);
		};
	}
}

function onFirmFileUploaded(iframe){
	clearTimeout(firmware_upload_timer);
	var elem=document.getElementById("firmware_upload_percent");
	elem.innerHTML = "100%";
	document.getElementById("firm_update_show_bar").style.width="100%";
	
	var OpenPopup=function(){
			$('#popupDialog_finish').popup('open');
	}
	OpenPopup();
	CommandSupport.sendCommand("System", CommandStrings.OnlineUpgrade);
}

function ShowFirmwareUploadPercent(downloadPercent)
{

	if(downloadPercent<100)
	{
		//var bwidth = swidth * downloadPercent/100;
		var elem=document.getElementById("firmware_upload_percent");
		elem.innerHTML = downloadPercent.toString()+"%";
		document.getElementById("firm_update_show_bar").style.width=downloadPercent.toString()+"%";
		sendCommand('GetFWDownloadPrecent', function(req){
   		var per = req.responseText.split(":")[1];
		downloadPercent=parseInt(per);
		firmware_upload_timer=setTimeout(function(){
			ShowFirmwareUploadPercent(downloadPercent);
		}, 1000);
		});
	}else
	{
		var elem=document.getElementById("firmware_upload_percent");
		elem.innerHTML = "100%";
		document.getElementById("firm_update_show_bar").style.width="100%";
	}
}

var m_Timer1;
var m_Timer2;
var m_Timer3;
var m_Timer4;

function funStartCheckRemoteUpdate()
{
	$.ajaxSetup({ cache: false }); 
	sendCommand("getMvRemoteUpdateStartCheck", null);		
  	var elem=document.getElementById("firmware_update_status");
	elem.innerHTML=i18n_alert[window.langindex]["onlineup.please_wait"];
	m_Timer3=setInterval(funCheckRemoteUpdate, "1000");
}

function funCheckRemoteUpdate()
{
	$.ajaxSetup({ cache: false }); 
	sendCommand("getMvRemoteUpdateStatus", funCheckRemoteUpdateCallBack);		
}

function funCheckRemoteUpdateCallBack(req)
{
	var m_RemoteUpdateStatus = parseInt(req.responseText);
	if(m_RemoteUpdateStatus==40)
	{
		clearInterval(m_Timer3);
		//document.getElementById("divmvStartRemoteUpdateTip").innerHTML=_("There is a new firmware");
		//document.getElementById("setmvStartRemoteUpdateButton").style.display = "block";
		//document.mvStartRemoteUpdate.style.display = "block";
		//document.mvStartRemoteUpdate.style.visibility = "visible";
	  	var elem=document.getElementById("firmware_update_status");
		elem.innerHTML=i18n_alert[window.langindex]["onlineup.has_new_firmware"];
		clearInterval(m_Timer3);
		$("#div_check_online_update").hide();
		$("#div_online_update").show();
		//initI18N(window.langindex);
	}
	else if(m_RemoteUpdateStatus==10)
	{
		//document.getElementById("divmvStartRemoteUpdateTip").innerHTML=_("Plesae wait");
	  	var elem=document.getElementById("firmware_update_status");
		elem.innerHTML=i18n_alert[window.langindex]["onlineup.please_wait"];
	}
	else
	{
		//document.getElementById("divmvStartRemoteUpdateTip").innerHTML=_("No update valid");
	  	var elem=document.getElementById("firmware_update_status");
		elem.innerHTML=i18n_alert[window.langindex]["onlineup.no_update"];
		clearInterval(m_Timer3);
	}
}

function funStartRemoteUpdate()
{
	$.ajaxSetup({ cache: false }); 
	sendCommand("getMvRemoteUpdateStart", null);
  	var elem=document.getElementById("firmware_update_status");
	elem.innerHTML=i18n_alert[window.langindex]["onlineup.please_wait"];
	m_Timer1=setInterval(ajaxGetMvRemoteUpdateStatus, "1000");
}

function ajaxGetMvRemoteUpdateStatus()
{
	$.ajaxSetup({ cache: false }); 
	sendCommand("getMvRemoteUpdateStatus", funRemoteUpdateCallBack);
}

function funRemoteUpdateCallBack(req) 
{
	var m_RemoteUpdateStatus = parseInt(req.responseText);
	switch(m_RemoteUpdateStatus)
	{
	case 10:
	  	var elem=document.getElementById("firmware_update_status");
		elem.innerHTML=i18n_alert[window.langindex]["onlineup.please_wait"];
		break;
	case 11:
	  	var elem=document.getElementById("firmware_update_status");
		elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.downloading"];
		break;
//	case 20:
//		$("#divStatus").text(_("MV_REMOTE_UPDATE_MSG_VER_FAIL"));
		//clearInterval(m_Timer1);
//		break;
	case 21:
	  	var elem=document.getElementById("firmware_update_status");
		elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.check.error"];
		//clearInterval(m_Timer1);
		break;
	case 22:
	  	var elem=document.getElementById("firmware_update_status");
		elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.downloaderror"];
		//clearInterval(m_Timer1);
		break;
	case 23:
	  	var elem=document.getElementById("firmware_update_status");
		elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.check.error"];
		//clearInterval(m_Timer1);
		break;
	case 30:
	  	var elem=document.getElementById("firmware_update_status");
		elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.downloadcomplete"];
		clearInterval(m_Timer1);
		m_Timer2=setInterval(startMvWriteImage, "2000");
		break;
	}
}	

function startMvWriteImage()
{
	clearInterval(m_Timer2);
//	showMsgBox1();

//	StopTheClock();

	//document.getElementById("loading").style.display="block";
	//parent.menu.setLockMenu(1);
	sendCommand("getMvRemoteUpdateWriteImage", MvWriteImageReturn);

	$('#popupDialog_burningFirmware').popup('open');
	_singleton = 1;

	
}

function MvWriteImageReturn(req)
{
	if(req.responseText.indexOf("Done...rebooting") >= 0)
	{
		$("#popupDialog_burningFirmware_text").text(i18n_alert[window.langindex]["system.complete_reboot"]);			
	}
	else
	{
		$("#popupDialog_burningFirmware_text").text(req.responseText);			
	}
}

function funRestoreToDefaultReturn(req)
{
	var elem=document.getElementById("popupDialog_restoreFactoryResult_text");
	elem.innerHTML=i18n_alert[window.langindex]["system.complete_reboot"];
	$('#popupDialog_restoreFactoryResult').popup('open');	
}

function GetOnlineUpdateStatus(onlinestatus)
{
	if(onlinestatus == "CHECKING"){
	  	 var elem=document.getElementById("firmware_update_status");
		 elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.checking"];
	}
	
	
	if(onlinestatus == "ERROR") {
	   var elem=document.getElementById("firmware_update_status");
	   elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.check.error"];
       return;
	}

	if(onlinestatus == "NONEWVER") {
	   var elem=document.getElementById("firmware_update_status");
	   elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.check.noupdate"];
       return;
	}

	if(onlinestatus.split("#")[0] == "VERSION")
	{
		if(!versioncheck)
		{
			if(confirm(i18n_alert[window.langindex]["online.upgrade.confirm.a"] + " " + onlinestatus.split("#")[1] + "," + i18n_alert[window.langindex]["online.upgrade.confirm.b"])){
				CommandSupport.sendCommand("System", CommandStrings.OnlineupdateConfirm);
				
			}else
			{
				var elem=document.getElementById("firmware_update_status");
			    elem.innerHTML="";
				return;
			}
			versioncheck = true;
		}
	}
	
	if(onlinestatus == "DOWNLOADING")
	{
		 var elem=document.getElementById("firmware_update_status");
		 elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.downloading"];	
	}
	
	if(onlinestatus == "FWDOWNLOADCOMPLETED")
	{
		 var elem=document.getElementById("firmware_update_status");
		 elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.downloadcomplete"];
	}
	
	if(onlinestatus == "DOWNLOADERROR")
	{
		 var elem=document.getElementById("firmware_update_status");
		 elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.downloaderror"];
		 return;
	}
	
	if(onlinestatus == "READYTOUPDATE")
	{
		 var elem=document.getElementById("firmware_update_status");
		 elem.innerHTML=i18n_alert[window.langindex]["online.upgrade.downloadcomplete"];
		 sendCommand('SystemCommand+firmwareConfirm', function(req){;});
		 return;
	}
	
	 sendCommand('SystemCommand+getOnlineState', function(req){
    	   onlinestatus = req.responseText.split(":")[2].substring(0, req.responseText.split(":")[2].length-2);
       });
	 
	setTimeout(function(){
			GetOnlineUpdateStatus(onlinestatus);
		}, 1000);

}


////////////////////// mono ///////////////////////////////////////////////////////////////////////////////////
var _singleton = 0;
function wiimu_uploadFirmwareCheck()
{
	if(_singleton)
		return false;

	if(document.UploadFirmware.filename.value == ""){
		 var OpenPopup=function(){
		    $('#popupDialog_nofile').popup('open');
		}
		OpenPopup();
		return false;
	}
	else{

	    $('#popupDialog_burningFirmware').popup('open');
		setTimeout(function(){
				killprocess();
			}, 100);

		//parent.menu.setLockMenu(1);
		_singleton = 1;
		return true;
	}	
}

function killprocess()
{
	$.ajaxSetup({ cache: false }); 
	htmlobj=$.ajax({url:"/goform/getMvKillProcess",async:false});
	//alert(htmlobj.responseText);
	//wiimuStartBurnLocalFirmware();

	initUploadFrame();
//	document.getElementById("firmware_update_process").style.visibility = "visible";
//	ShowFirmwareUploadPercent(0);
	document.getElementById("onlineup_upform").submit();
	m_Timer4=setInterval(wiimuchecklocalburnfirmware, "1000");

}

function wiimuchecklocalburnfirmware()
{
	var txt=document.getElementById('uploadFrame').contentWindow.document.body.innerHTML;
	if(txt.indexOf("Done...rebooting") >= 0)
	{
		$("#popupDialog_burningFirmware_text").text(i18n_alert[window.langindex]["system.complete_reboot"]);			
	}
//	else
//	{
//		$("#popupDialog_burningFirmware_text").text(txt);			
//	}	
	
}

