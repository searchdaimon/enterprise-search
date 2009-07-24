
var API_IS_WORKING = 1;
var API_IS_SAVED   = 2;
var API_ERR_SAVING = 3;
var API_CLEAR      = 4;


/**
 * params for jquery ajax request
 */
function defaultJQueryParams() {
	return {
		type : "GET",
		url  : "?",
		dataType: "json",
		async : true,
		cache : false
    	};
}


function set_checked(prefix, checked) {

    var i = 0;
    var checkBox;
    while (checkBox = document.getElementById(prefix + "" + i)) {
        i++;
        checkBox.checked = checked;
    }
}

/**
 * Adds class 'odd' to odd rows.
 */
function colorTable(tableId) {
	$("#" + tableId + " tbody tr:odd").addClass("odd");
	$("#" + tableId + " tbody tr:even").removeClass("odd");
}

function escapeHTML(str) {
	if (!str.replace)
		throw "escapeHTML can replace on string: '" + str + "'. Is it a string?";
	return str.replace(/&/, "&amp;")
		  .replace(/</, "&lt;")
		  .replace(/>/, "&gt;")
		  .replace(/"/, "&quot;");
}
function unescapeHTML(str) {

	return str.replace(/&amp;/, "&")
		  .replace(/&lt/, "<")
		  .replace(/&gt/, ">")
		  .replace(/&quot/, "\"");
}

function setMsg(type, msg) {
	if (msg == null) {
		$("#msg").hide();
		return;
	}
	if (type == null)
		type = "info";

	$("#msg").fadeOut(250, function() {
		$("#msg").attr('class', 'msgBox ' + type);
		$("#msg .msgBoxText").html(escapeHTML(msg));
		$("#msg").fadeIn(250);	
	});
	
}

var last_api_save = null;
function updApiMsg(state) {
	var msg = null;
	var show_anim = false;
	var highlight = false;

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
		msg = '<span style="font-weight : bolder; color : red;">Error saving.<br /> ';
		if (last_api_save) {
		 	msg += "Last updated at "
			    + last_api_save.toLocaleString() + ".";
		}
		highlight = true;

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
	if (highlight)
		$("#apiMsg").effect("highlight", { }, 4000);
}

