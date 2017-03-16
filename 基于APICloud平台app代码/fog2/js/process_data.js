

//发送数据到设备端
function send_message_to_device(app_command) {
	var device_id = localStorage.getItem(_DEVICE_ID);
	var device_pw = localStorage.getItem(_DEVICE_PW);
	var is_sub = localStorage.getItem(_IS_SUB_DEVICE);

	var app_token = check_app_token();

	if(monitor_net() == "none") {
		return false;
	}

	if(device_id == null || device_pw == null || is_sub == null || is_sub == "undefined") {
		mui.toast("读取设备信息错误");
		back_device_list_html();
	}

	
	if(is_sub == "false") {
		var param = {
			deviceid: device_id,
			devicepw: device_pw,
			command: app_command,
			token: app_token
		};

		//alert(JSON.stringify(param));

		mico2.sendCommand(param, function(ret, err) {
			try {
				if(ret && ret.meta.code == 0) {
					//mui.toast("发送数据成功");
				} else {
					alert("发送数据失败[err]" + JSON.stringify(err) + JSON.stringify(ret));
				}
			} catch(e) {
				//alert("发送数据失败[ret]:" + JSON.stringify(ret));
			}

		});
	} else if(is_sub == "true") {

		var param = {
			subdeviceid: device_id,
			devicepw: device_pw,
			flag: 3,
			command: app_command,
			format: "json",
			token:app_token
		};
		
		//console.log(JSON.stringify(param));
		
		mico2.sendCommandSub(param, function(ret, err) {
			try {
				if(ret && ret.meta.code == 0) {
					//mui.toast("发送数据成功");
				} else {
					alert("发送数据失败[err]" + JSON.stringify(err) + JSON.stringify(ret));
				}
			} catch(e) {
				alert("发送数据失败[ret]:" + JSON.stringify(ret));
			}
		});

	
	}else{
		alert("is_sub error!" + is_sub);
	}
}