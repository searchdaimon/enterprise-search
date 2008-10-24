
API_IS_SAVING = 1;
API_IS_SAVED = 2;
API_ERR_SAVING = 3;

MSG_MAPPED = 1;
MSG_UNMAPPED = 2;

function debug(s) { try { console.log(s); } catch(e) { } }

/* init functions */
$(document).ready(function () {
	buildMappingList();
	buttonEvents();
	userEvents();
	
	$("#loadingMsg").hide();

});

function buttonEvents() {
	$("#automapBtn").click(function() {
		automap();
		buildMappingList();
	});
	$("#saveBtn").click(function() {
		apiUpdMapping();
		saveBtnEnabled(false);
	});
}

function userEvents() {
	/*$.each($('[@id^="field_prim"]'), function(i, radiobtn) {
		$(radiobtn).click(function() {
			var prim_usr = fieldUsr(PRIM_SYS, radiobtn);
			var sec_usr;

			if (sec_usr = userMapping[prim_usr]) {
				var sec_id = usrToId(SEC_SYS, sec_usr);
				var field = document.getElementById(sec_id); // jquery magic failed on space
				$(field).attr('checked', 'checked');
			}
		});
	});*/

	$.each($('[@id^="field_sec"]'), function(i, radiobtn) {
		$(radiobtn).click(function() {
			var secUsr = fieldUsr(SEC_SYS, (radiobtn));
			var primField = $('[@id^="field_prim"]:checked');
			var primUsr = fieldUsr(PRIM_SYS, primField);
			if (primUsr) {
				if (mapUsrs(primUsr, secUsr)) {
					delField(primField, true);
					delField(radiobtn, true);
					addMapRow(primUsr, secUsr, true);
					setMsg(MSG_MAPPED, primUsr, secUsr);

				}
			}
			else {
				setMsg(null, "You need to select primary user first.");
			}
		});
	});
}

/* helper functions */
function usrToId(system, usr) {
	if (!usr)
		return null;

	if (system == SEC_SYS)
		return SEC_RADIO_PREFIX + escapeHTML(usr);

	else if (system == PRIM_SYS)
		return PRIM_RADIO_PREFIX + escapeHTML(usr);

	alert("Unknown sys constant " + system);
	return null;
}

function fieldUsr(system, obj) {
	var id = $(obj).attr('id');
	return id ? idToUsr(system, id) : null;
}

function idToUsr(system, id) {
	if (!id) 
		return null;

	if (system == PRIM_SYS)
		return unescapeHTML(id.substring(PRIM_RADIO_PREFIX.length));

	else if (system == SEC_SYS)
		return unescapeHTML(id.substring(SEC_RADIO_PREFIX.length));

	alert("Unknown sys constant " + system);
	return null;
}

/* data manipulation */
function mapUsrs(prim, sec) {
	if (userMapping[prim] && userMapping[prim] == sec) {
		/* Ignore if unchanged */
		return false;
	}

	userMapping[prim] = sec; 
	saveBtnEnabled(true);

	return true;
}


function unmapUsr(prim) {
	if (!userMapping[prim]) {
		/* Ignore if unchanged */
		return false;
	}
	eval("delete userMapping." + prim);
	saveBtnEnabled(true);

	return true;
}


function automap() {
	var modified = false;

	$.each($('[@id^="field_prim"]'), function(i, radiobtn) {
		var usr = fieldUsr(PRIM_SYS, radiobtn);
		var secId = usrToId(SEC_SYS, usr);

		var secField = document.getElementById(secId); // jquery escape fail.
		//$("#" + secId);
		if (fieldUsr(SEC_SYS, secField)) {
			// found on second system.
			if (mapUsrs(usr, usr)) {
				//console.log("mapped user " + usr + " to secUsr");
				modified = true;
				delField(radiobtn, true);
				delField(secField, true);
			}
		}
	});
	return modified;
}

/* API functions */

function apiUpdMapping() {
	
	updApiMsg(API_IS_SAVING);
	var data = {
		api : "upd_mapping",
		id : SEC_SYSTEM_ID
	};

	$.each(userMapping, function(prim, sec) {
		data["mapping." + prim] = sec;
		
	});

	$.ajax({
		type : "POST",
		url  : "usersys.cgi",
		dataType : "json",
		data : data,
		success : function() { 
			updApiMsg(API_IS_SAVED); 
		},
		error : function() { 
			updApiMsg(API_ERR_SAVING); 
			saveBtnEnabled(true);
		}
	});

}

/* UI functions */
function addField(system, user) {
	var id = usrToId(system, user);
	var elem = document.getElementById(id);
	
	
	$(elem).parent().fadeIn(100);
}

function delField(fieldObj, fadeOut) {
	$(fieldObj).removeAttr('checked');
	fadeOut
		? $(fieldObj).parent().fadeOut(100)
		: $(fieldObj).parent().hide();
}

function saveBtnEnabled(setEnabled) {
	var btn = $("#saveBtn");
	setEnabled 
		? btn.removeAttr('disabled') 
		: btn.attr('disabled', 'disabled');
}
function colorMapList() { colorTable('mapList'); }
function setMsg(state, val1, val2) {
	if (state == null) {
		$("#msg").html(escapeHTML(val1));
		return;
	}
	var msg;

	switch (state) {
	case MSG_MAPPED:
		msg = "User '" + val1 + "' mapped to user '" + val2 + "'.";
		break;
	case MSG_UNMAPPED:
		msg = "User '" + val1 + "' has no longer a mapping.";
		break;
	default:
		msg = "Unknown state " + state;
	}
	$("#msg").html(escapeHTML(msg));
}



function buildMappingList() {

	// Remove all rows
	var first = true;
	$.each($("#mapList tr"), function(i, rowObj) {
		//console.log(rowObj);
		if (first) // skip header row 
			first = false;
		else
			delRow(rowObj);
	});

	// (re)add rows
	$.each(userMapping, function(prim, sec) {
		addMapRow(prim, sec);
	});

	colorMapList();
}


function delRow(rowObj, fadeOut, recolor) {
	if (fadeOut) {
		$(rowObj).fadeOut(100, function() {
			$(this).remove();
			if (recolor)
				colorMapList();
		});
	}
	else {
		$(rowObj).remove();
		if (recolor)
			colorMapList();
	}

}

function addMapRow(prim, sec, fadeIn) {
	$("#lastAddedRow").removeAttr('id');

	var primEsc = escapeHTML(prim);
	var secEsc = escapeHTML(sec);
	$("#mapList").append(
		'<tr id="lastAddedRow" style="display : none">'
		+ "<td>" + primEsc + "</td>"
		+ "<td>" + secEsc + "</td>"
		+ '<td style="text-align : right"><button onclick="'
			  + "unmapUsr('"  + primEsc + "');"
			  + "delRow($(this).parent().parent(), 1, 1);"
			  + "addField(PRIM_SYS, '" + primEsc + "');"
			  + "addField(SEC_SYS, '" + secEsc + "');"
			  + "setMsg(MSG_UNMAPPED,'" + primEsc + "');"
			  + "\">"
			+ "Remove</button>"
		);

	if (fadeIn) {
		$("#lastAddedRow").fadeIn(100, function() { colorMapList(); })
	}
	else {
		$("#lastAddedRow").show();
	}
}


var last_api_save = null;

function updApiMsg(state) {
	var msg = null;
	var show_anim = false;

	switch (state) {
	case API_IS_SAVING:
		show_anim = true;
		msg = "Saving changes...";
		break;
	case API_IS_SAVED:
		last_api_save = new Date();
		msg = "Saved at " + last_api_save.toLocaleTimeString() + ".";
		break;
	case API_ERR_SAVING:
		msg = "<strong>Error saving.</strong>.<br /> ";
		if (last_api_save) {
		 	msg += "Last save at "
			    + last_api_save.toLocaleString() + ".";
		}
		else  {
			msg += " Changes are unsaved.";
		}
		break;
	default:
		msg = "Unknown state " + state;
	}

	var anim = $("#loading");
	show_anim ? $(anim).show() : $(anim).hide();
	
	$("#apiMsg span").html(msg);
}


