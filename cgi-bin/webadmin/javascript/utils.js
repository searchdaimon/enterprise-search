
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

