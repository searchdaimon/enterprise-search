var API_IS_WORKING = 1;
var API_IS_SAVED   = 2;
var API_ERR_SAVING = 3;
var API_CLEAR      = 4;

var UNMAPPED_LIMIT = 500;
var MAPPED_LIMIT   = 30;

var DIRECTION_NEXT = 1;
var DIRECTION_PREV = 2;

var IMG_LOADING_HTML = '<img src="file.cgi?i=jqueryloader&amp;'
	+ 'ext=gif&amp;size=other" alt="loading" class="loadingImg" />';

/* Classes */

function Navigate() { return; };
Navigate.prototype.updateOffset = function(direction) {
	if (!direction) return;

	if (direction == DIRECTION_NEXT) 
		this.offset += this.limit;
	else if (direction == DIRECTION_PREV) {
		this.offset -= this.limit;
		if (this.offset < 0)
			this.offset = 0;
	}
	else { debug("Error: invalid direction."); return; }
}

function System(unmappedList, sys_type, mapList) {
	this.unmappedList = unmappedList;
	this.offset = 0;
	this.limit = UNMAPPED_LIMIT;
	this.sys_type = sys_type;
	this.mapList = mapList;

	this.statTable = $(unmappedList).parent().parent().find(".stat");
	this.total = -1;
	this.unmapped = -1;
	this.showing = -1;

	this.otherSys = null;
}
System.prototype = new Navigate();
System.prototype.updStatsUI = function() {
	$(this.statTable).find(".total").text(this.total);
	$(this.statTable).find(".unmapped").text(this.unmapped);
	$(this.statTable).find(".showing").text(this.showing);
}

System.prototype.appendUnmapped = function(user) {
	var html = userRowHTML(this.sys_type, user, true);

	if (this.showing > 0) { 
		var last_input = $(this.unmappedList).find("input:last");
		last_input.parent().after(html);
	}
	else { // list is empty
		$(this.unmappedList).append(html);
	}


	var sysObj = this;
	$(this.unmappedList).find("label:last").fadeIn(100, function() {
		$(this).removeAttr('class');
		sysObj.unmapped++;
		sysObj.showing++;
		sysObj.updStatsUI();

		sysObj.addRadioEvent($(this).find("input"));
	});
	return this;
}

System.prototype.getChecked = function() {
	return $(this.unmappedList).find("input:checked");
}

System.prototype.removeChecked = function() {
	var radioBtn = this.getChecked();
	$(radioBtn).removeAttr('checked');
	this.unmapped--;
	this.showing--;
	this.updStatsUI();
	$(radioBtn).parent().fadeOut(100, function() {
		$(this).remove();
	});
}

System.prototype.showUnmapped = function(direction) {
	this.updateOffset(direction);
	this.unmappedList.html(IMG_LOADING_HTML);
	var sysObj = this;
	listUnmapped(this.sys_type, this.offset, this.limit, function(users, total, unmapped) {
			
			var new_html = "";
			var num_users = 0;

			// userlist html
			$.each(users, function(i, u) {
				num_users++;
				new_html += userRowHTML(sysObj.sys_type, u, false); 
			});
			
			// navigation
			if (num_users + sysObj.offset < unmapped) {
				new_html += '<p class="next">Next page</p>';
			}
			if (sysObj.offset > 0) {
				new_html = '<p class="prev">Previous page</p>'
					+ new_html;
			}
			
			sysObj.unmappedList.html(new_html);

			$(sysObj.unmappedList).find(".next").click(function() {
				sysObj.showUnmapped(DIRECTION_NEXT);
			});

			$(sysObj.unmappedList).find(".prev").click(function() {
				sysObj.showUnmapped(DIRECTION_PREV);
			});
			
			// make users clickable
			sysObj.addRadioEvent();

			sysObj.showing = num_users;
			sysObj.total = total;
			sysObj.unmapped = unmapped;
			sysObj.updStatsUI();


		}, function() {
			sysObj.unmappedList.html("Error loading.");
		});
	return this;
};

// Adds event to all buttons, unless provided.
System.prototype.addRadioEvent = function(radioBtn) {
	var sysObj = this;
	var eventFunc;
	var setSelected = function(btn) {
		$(sysObj.unmappedList).find(".selected").removeAttr('class');
		$(btn).parent().attr('class', 'selected');
	}

	if (this.sys_type == "prim") {
		// just highligt
		eventFunc = function() { setSelected(this); }
	}
	else if (this.sys_type == "sec") {
		eventFunc = function() {
			setSelected(this);
			var secUsr = $(this).attr('value');
			var primField = sysObj.otherSys.getChecked();
			var primUsr = $(primField).attr('value');

			if (!primUsr) {
				setMsg('error', "You need to select primary user first.");
				$(this).parent().removeAttr('class');
				return;
			}
			
			$(this).attr('disabled', 'disabled');
			var secField = this;
			mapUsrs(primUsr, secUsr, function() {
				sysObj.removeChecked()
				sysObj.otherSys.removeChecked();
				sysObj.mapList.addMapRow(primUsr, secUsr, true, true);
			});
		};
	}
	else { debug("Invalid sys_type '"+ this.sys_type + "'"); }

	radioBtn 
		? $(radioBtn).click(eventFunc)
		: $(this.unmappedList).find('input').click(eventFunc);
}

function Mapped() {
	this.table = $("#mapList");
	this.loading = $("#mappedLoading");
	this.msg = $("#mapMsg");
	this.nav = $("#mapNav");

	this.offset = 0;
	this.limit = MAPPED_LIMIT;

	this.total = -1;
	this.showing = 0;

	this.addNavEvents();

	this.primSys = null;
	this.secSys  = null;
}
Mapped.prototype = new Navigate();
Mapped.prototype.addMapRow = function(primUser, secUser, updStat, animate) {

	var primEsc = escapeHTML(primUser);
	var secEsc = escapeHTML(secUser);

	if (updStat) {
		this.total++;
		this.showing++;
		this.updStatMsg();
		this.updNavBtns();
	}

	var html = '<tr';
	if (animate) {
		html += ' style="display : none;"'
	}
	html += ">"
		+ "<td>" + primEsc + "</td>"
		+ "<td>" + secEsc + "</td>"
		+ '<td style="text-align : right"><button>Remove</button></td></tr>';
	$(this.table).append(html);
	$(this.table).show();

	var mappedObj = this;
	var lastAdded = $(this.table).find("tr:last");
	$(lastAdded).find("button").click(function () {
		var btn = this;
		$(btn).attr('disabled', 'disabled');
 		unmapUsr(primUser, secUser, function() {
			$(btn).parent().parent().fadeOut(100, function() {
				$(this).remove();
				colorMapList();
			});
			mappedObj.primSys.appendUnmapped(primUser);
			mappedObj.secSys.appendUnmapped(secUser);

			mappedObj.showing--;
			mappedObj.total--;
			mappedObj.updStatMsg();
			mappedObj.updNavBtns();
		});
	});
	if (animate) {
		$(lastAdded).fadeIn(100, function() { colorMapList(); });
	}
};

Mapped.prototype.showMapped = function(direction) {
	this.updateOffset(direction);
	$(this.table).hide();
	$(this.table).find("tr:not(:first)").remove(); // first is header
	$(this.loading).show();

	var mapObj = this;
	
	listMapped(this.offset, this.limit, function (mapping, totalMapped) {
		var rows_added = 0;

		$(mapObj.table).hide();
		$.each(mapping, function(i, users) {
			mapObj.addMapRow(users[0], users[1]);
			rows_added++;
		});
		$(mapObj.loading).hide();

		mapObj.showing = rows_added;
		mapObj.total = totalMapped;
	
		if (rows_added) {
			colorMapList();
			$(mapObj.table).show();
		}
		else {
			$(mapObj.msg).text("No users have been mapped.");
		}
		mapObj.updStatMsg();
		mapObj.updNavBtns();
	});
};

Mapped.prototype.updStatMsg = function() {
	$(this.msg).text("Showing " 
		+ (this.offset+1) + " - " 
		+ (this.offset + this.showing)
		+ " out of " + this.total);
}

Mapped.prototype.addNavEvents = function() {
	var next = $(this.nav).find("#mapNavNext");
	var prev = $(this.nav).find("#mapNavPrev");

	var mapObj = this;
	$(next).click(function() {
		mapObj.showMapped(DIRECTION_NEXT);
		$(this).attr('disabled', 'disabled');
	});
	$(prev).click(function() {
		mapObj.showMapped(DIRECTION_PREV);
		$(this).attr('disabled', 'disabled');
	});

}
Mapped.prototype.updNavBtns = function(numShown) {
	var next = $(this.nav).find("#mapNavNext");
	var prev = $(this.nav).find("#mapNavPrev");
	
	if (this.offset > 0) {
		$(prev).removeAttr('disabled');
	}
	if ((this.offset + this.showing) < this.total) {
		$(next).removeAttr('disabled');
	}
	
}


/* Functions */



function debug(s) { try { console.log(s); } catch(e) { } }


/* API functions */
var defSuccsFunc = function(data) {
	if (data && data['error']) {
		defErrfunc(null, data['error']);
		return;
	}
	if (data && data['ok']) {
		setMsg('success', data['ok']);
	}
	updApiMsg(API_IS_SAVED);

}
var defErrFunc = function(req, errstr, errobj) {
	if (!errstr) errstr = "Unknown error.";
	setMsg("error", errstr);
	updApiMsg(API_ERR_SAVING);
}
function defParams() {
	return {
		id : SEC_SYSTEM_ID,
		type : "POST",
		url  : "usersys.cgi",
		dataType : "json",
		data : { id : SEC_SYSTEM_ID },
		success : defSuccsFunc,
		error : defErrFunc,
		beforeSend: function() {
			updApiMsg(API_IS_WORKING);
		},
        	complete: function() {
			$("#loading").hide();
		}
	};

}

function mapUsrs(prim, sec, succs_func) {
	var param = defParams();
	param['data']['api'] = "add_mapping";
	param['data']['prim_usr'] = prim;
	param['data']['sec_usr']  = sec;
	param['success'] = function(data) {
		defSuccsFunc(data);
		if (succs_func) 
			succs_func();
	};
	$.ajax(param);
}
function unmapUsr(prim, sec, succs_func) {
	var param = defParams();
	param['data']['api'] = "del_mapping";
	param['data']['prim_usr'] = prim;
	param['data']['sec_usr']  = sec;
	param['success'] = function(data) {
		defSuccsFunc(data);
		if (succs_func)
			succs_func();
	};
	$.ajax(param);
}
function listMapped(offset, limit, succs_func) {
	var param = defParams();
	var errfunc = function(req, errstr) {
		setMsg("error", "Unable to fetch mapped users: " + errstr);
	};
	param['data']['offset'] = offset;
	param['data']['limit'] = limit;
	param['data']['api'] = "list_mapping";
	param['success'] = function(data) {
		if (data['error']) {
			errfunc(null, data['error']);
			return;
		}
		succs_func(data['mapping'], data['mapped_total']);
	};
	param['error'] = errfunc;
	param['complete'] = function() { updApiMsg(API_CLEAR); }
	$.ajax(param);
}

function listUnmapped(system, offset, limit, succs_func, err_func) {
	var param = defParams();
	var err = function(req, errstr) {
		setMsg("error", "Unable to fetch unmapped users: " + errstr);
		if (err_func)
			err_func();
	};
	param['data']['offset'] = offset;
	param['data']['limit'] = limit;
	param['data']['api'] = "list_unmapped";
	param['data']['system'] = system;
	param['success'] = function(data) {
		if (data['error']) {
			err(null, data['error']);
			return;
		}
		succs_func(data['users'], data['users_total'], data['unmapped_total']);
	};
	param['error'] = err;
	param['complete'] = function() { updApiMsg(API_CLEAR); }
	$.ajax(param);
}

function simpleApiCall(api_cmd, succs_func) {
	var param = defParams();
	param['data']['api'] = api_cmd;
	param['success'] = function(data) {
		defSuccsFunc(data);
		if (succs_func)
			succs_func();
	};
	$.ajax(param);
}

function clearMapping(succs_func) { 
	simpleApiCall("clear_mapping", succs_func);
}
function automap(succs_func) { 
	simpleApiCall("automap", succs_func);
}


/* UI functions */

function userRowHTML(sys_type, user, attr_for_animation) {

	var user_esc = escapeHTML(user);
	var input_id = "field_" + sys_type + "_" + user_esc;

	var html = '<label for="' + input_id + '"' ;
	if (attr_for_animation) {
		html += ' style="display : none;"';
	}
	html += '><input type="radio" id="' + input_id + '" name="' + sys_type + '"'
		/* + ' class="sys_list_' + sys_type + '"' */
		+ ' value="' + user_esc + '" />' + user_esc
		+ "</label>";
	return html;
}




function colorMapList() { colorTable('mapList'); }


var last_api_save = null;
function updApiMsg(state) {
	var msg = null;
	var show_anim = false;

	switch (state) {
	case API_IS_WORKING:
		show_anim = true;
		msg = "Working...";
		break;
	case API_IS_SAVED:
		last_api_save = new Date();
		msg = "Updated at " + last_api_save.toLocaleTimeString() + ".";
		break;
	case API_ERR_SAVING:
		msg = "<strong>Error</strong>.<br /> ";
		if (last_api_save) {
		 	msg += "Last updated at "
			    + last_api_save.toLocaleString() + ".";
		}
		break;
	case API_CLEAR:
		msg = "";
		break;
	default:
		msg = "Unknown state " + state;
	}

	var anim = $("#loading");
	show_anim ? $(anim).show() : $(anim).hide();
	
	$("#apiMsg span").html(msg);
}


