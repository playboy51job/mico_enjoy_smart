var device_online_listen = false;
var add_subdevice_timer;
var recv_data_count = 0;
var is_mqtt_client_connect = false;




//返回设备列表页面 在内部调用stopListenDevice 可能会导致Android app 无响应
function back_device_list_html() {
	//	mico2.stopListenDevice(function(ret, err) {
	//		if(ret) {
	//			//alert("停止监听成功：" + JSON.stringify(ret));
	//			send_event('refesh_device_list', 'peripherals');
	//			closeWin();
	//		} else {
	//			alert("停止监听失败：" + JSON.stringify(err));
	//			send_event('refesh_device_list', 'peripherals');
	//			closeWin();
	//		}
	//	});

	send_event('refesh_device_list', 'peripherals');
	closeWin();
}

//设置UI显示的 设备在线还是离线
function set_device_UI_offline(status) {
	if(status == true) {
		document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-green">在线</span>';
		//mui.toast("设备在线");
	} else if(status == false) {
		document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-danger">离线</span>';
		alert("设备离线");
	}
}

function goto_rgb_html() {
	/*
	var json_param = {
		"mac": "abcdefghij",
		"did": 100
	};
	*/
	openWin('reb_led', 'widget://html/rgb_led.html', true, null);
}

function fnSwitchChanged() {
	//获取透传模式开关的状态
	var isActive = document.getElementById("per_switch_motor_1").classList.contains("mui-active");
	if(isActive == true) {
		var app_command = {
			"command_id": 2, //此处1表示RGB 2表示电机,平常客户可以自定义一个协议
			"motor_switch": true
		};
		send_message_to_device(JSON.stringify(app_command));
	    console.log("Command send to device:" + JSON.stringify(app_command));
	} else {
		var app_command = {
			"command_id": 2, //此处1表示RGB 2表示电机,平常客户可以自定义一个协议
			"motor_switch": false
		};
		send_message_to_device(JSON.stringify(app_command));
	    console.log("Command send to device:" + JSON.stringify(app_command));
	}
}


//处理resume事件
function deal_resume_event() {
	get_device_info(false);
}

//获取设备信息
function get_device_info(is_listen_device) {
	var app_token = check_app_token();

	golobal_template_json_obj = null; //模板清零
	//alert("golobal_template_json_obj:" + JSON.stringify(golobal_template_json_obj));

	if(monitor_net() == "none") {
		back_device_list_html();
	}

	var device_id = localStorage.getItem(_DEVICE_ID);
	if(device_id == null) {
		back_device_list_html();
	}

	var device_info_param = {
		deviceid: device_id,
		token: app_token
	};

	//console.log(JSON.stringify(device_info_param));

	mico2.getDeviceInfo(device_info_param, function(ret, err) {
		if(ret && ret.meta.code == 0) {
			//alert(JSON.stringify(ret));

			//再次获取设备详情 update操作
			var device_pw = ret.data.devicepw;
			var device_online = ret.data.online;
			var device_alias = ret.data.alias;
			var is_sub = ret.data.is_sub;
			var parent_id = ret.data.parentid;
			var device_type = ret.data.gatewaytype;

			localStorage.setItem(_DEVICE_NAME, device_alias);
			localStorage.setItem(_DEVICE_PW, device_pw);
			localStorage.setItem(_DEVICE_ID, device_id);
			localStorage.setItem(_IS_SUB_DEVICE, is_sub); //bool类型
			localStorage.setItem(_PARENT_ID, parent_id);

			if(device_type == 0) { //普通设备
				localStorage.setItem(_DEVICE_TYPE, "C");
			} else if(device_type == 1) { //网关设备
				localStorage.setItem(_DEVICE_TYPE, "G");
			} else if(device_type == 2) { //子设备
				localStorage.setItem(_DEVICE_TYPE, "S");
			} else { //错误情况
				localStorage.setItem(_DEVICE_TYPE, "NULL");
			}

			document.getElementById("per_device_name").innerHTML = device_alias;

			set_device_UI_offline(device_online);

			if(is_listen_device == true) {
				listen_device(); //开启监听设备信息 
			}

		} else {
			console.log("获取设备详情错误:" + JSON.stringify(ret) + JSON.stringify(err));
			get_device_info(is_listen_device);
		}
	});
}



//监听设备 先停止监听 再开启监听   ps:如果不先停止监听,可能会导致Android APP死掉
function listen_device() {
	mico2.stopListenDevice(function(ret, err) {
		if(ret) {
			//alert("停止监听成功：" + JSON.stringify(ret));
		} else {
			//alert("停止监听失败：" + JSON.stringify(err));
		}

	});

	start_listen_device();
}

//开启设备监听
function start_listen_device() {
	var client_id = null;
	var device_id = null;
	var passwd = null;
	var is_sub = null;
	var parent_id = null;

	device_online_listen = false; //处理该标志位

	device_id = localStorage.getItem(_DEVICE_ID);
	client_id = localStorage.getItem(_CLINET_ID);
	passwd = localStorage.getItem(_PASSWORD); //登录密码
	is_sub = localStorage.getItem(_IS_SUB_DEVICE); //是否为子设备 得到字符串true和false
	parent_id = localStorage.getItem(_PARENT_ID); //父设备id

	if(device_id == null || client_id == null || passwd == null) {
		back_device_list_html();
	}

	if(monitor_net() == "none") {
		back_device_list_html();
	}

	var param = {
		host: _FOG_MQTT_HOST_NAME,
		port: _FOG_MQTT_HOST_PORT_SSL,
		username: client_id,
		password: passwd,
		deviceid: device_id,
		clientid: client_id,
		isencrypt: _FOG_MQTT_SSL_CHOOSE
	};

	//	alert(JSON.stringify(param));
	//	console.log(JSON.stringify(param));

	if(is_mqtt_client_connect == true) {
		console.log("mqtt is already connect! is_mqtt_client_connect = " + is_mqtt_client_connect);
		return;
	} else {
		console.log("start mqtt client");
	}

	try {
		mico2.startListenDevice(param, function(ret, err) {
			if(ret) {
				//zfw add
				//alert(JSON.stringify(ret));
				console.log("ret" + JSON.stringify(ret));
				if(ret.code == 2001) {
					is_mqtt_client_connect = true;
					add_device_online_listener(device_id);
				}
				else if(ret.code == 2008) {
					is_mqtt_client_connect = false;
					send_event('mqtt_client_restart', ''); //mqtt客户端重连
				} 
				else {
					//最主要的处理是在这里，获取控制命令并显示在页面上
					if(ret.topic != null) {
						process_mqtt_data(device_id, ret); //MQTT消息
					}
				}
			} else {
				mui.toast("监听失败:" + JSON.stringify(err));
				console.log("监听失败:" + JSON.stringify(err));

				stop_listen_device();
			}
		});
	} catch(e) {
		alert("收到异常数据,请检查数据格式" + e.message);
	}
}

//处理mqtt数据
function process_mqtt_data(device_id, data) {
	if(data.payload.code == 2002)
	{
		document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-danger">离线</span>';
		alert("设备离线");
		return;
	}
	else if(data.payload.code == 2001)
	{
		document.getElementById('per_device_online').innerHTML = '<span class="mui-badge mui-badge-green">在线</span>';
		return;
	}
	//json解析数据
	document.getElementById("per_temperature_1").innerHTML = data.payload.device_info.temperature;
	document.getElementById("per_humidity_1").innerHTML = data.payload.device_info.humidity;
	document.getElementById("per_light_1").innerHTML = data.payload.device_info.light_sensor;
	document.getElementById("per_infrared_1").innerHTML = data.payload.device_info.infrared_reflective;
	return;
}

//停止设备监听
function stop_listen_device() {
	mico2.stopListenDevice(function(ret, err) {
		if(ret) {
			mui.toast("取消监听成功：" + JSON.stringify(ret));
		} else {
			mui.toast("取消监听失败：" + JSON.stringify(err));
		}

		is_mqtt_client_connect = false;
	});
}

//监听设备是否在线
function add_device_online_listener(device_id) {
	var add_listen_param = {
		topic: "d2c/" + device_id + "/cmd",
		qos: 0
	};

	//alert("增加监听");
	mico2.addDeviceListener(add_listen_param, function(ret, err) {
		if(ret) {
			console.log("增加设备监听成功:" + JSON.stringify(ret));
		} else {
			alert("增加设备监听失败:" + JSON.stringify(err));
		}
		send_event('get_device_template', 'mqtt_connect_success'); //触发指定的监听事件
	});
}



//在外设界面监听Android返回按键
function peripherals_listen_keyback() {
	api.addEventListener({
		name: 'keyback'
	}, function(ret, err) {
		back_device_list_html();
	});
}