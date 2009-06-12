
function succsFunc(msg) {
	//console.log("success: " + msg);
	setMsg("success", msg);
}
function errFunc(msg) {
	setMsg("error", msg);
}
function beforeFunc() {
	$("#loading").show();
}
function completeFunc() {
	$("#loading").hide();
}

function ajaxParams(overrideParams) {
	var defaults = defaultJQueryParams();

	defaults['error'] = function(data) {
		errFunc("Internal error, or no connection.");
	};
	defaults['success'] = function(data) {
		if (data['error']) {
			overrideParams['error'] 
				? overrideParams['error'](data) 
				: errFunc(data['error']);
			return;
		}
		overrideParams['success'] 
			? overrideParams['success'](data)
			: succsFunc(data['ok']);
	};
	defaults['beforeSend'] = beforeFunc;
	defaults['complete'] = completeFunc;
	defaults['url'] = "clienttpl.cgi";


	$.each(overrideParams, function(key, val) {
		if (key != "success")
			defaults[key] = val;
	});
	return defaults;
}
