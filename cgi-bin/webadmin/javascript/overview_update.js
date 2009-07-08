OVERVIEW_UPD_URL = "overview.cgi?fetch_inner&";
/**
* Fetch the new HTML contents. Run update_overview to replace contents.
*/
function run_overview_update() {
	if (run_overview_update.running)
		return;

	run_overview_update.running = 1;

	$.ajax({
		url : OVERVIEW_UPD_URL,
		cache : false,
		success : function(html) {
			$("#overview").html(html);
			run_overview_update.running = 0;
		}
	});
}

function overview_update_interval(seconds) {
	if (!seconds) {
		// no -- we don't want 0 seconds.
		throw "invalid argument";
	}

	run_overview_update();
	
	var time = seconds * 1000;
	window.setInterval(run_overview_update, time);
}
