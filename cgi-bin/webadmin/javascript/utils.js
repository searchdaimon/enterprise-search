
/**
 * params for jquery ajax request
 */
function defaultJQueryParams() {
	return {
		type : "GET",
		url  : "connector.cgi",
		dataType: "json",
		async : true,
		cache : false,
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
