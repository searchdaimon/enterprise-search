var DEBUG = 0;
var IF_URL = "overview.cgi";
var IF_OVERVIEW_FUNCTION = "fetch_inner&";

/**
  * Create a XMLHttpRequest object instance
  * 
  * Returns:
  *		xmlHttp - New instance of XMLHttpRequest.
  */
function XML_HTTP_instance() {
	var http_req;

	if (window.XMLHttpRequest) { // not ie
		http_req = new XMLHttpRequest();
	}
	else if (window.ActiveXObject) { //ie
		try { http_req = new ActiveXObject("Msxml2.XMLHTTP"); }
		catch (e) {
			try { http_req = new ActiveXObject("Microsoft.XMLHTTP"); }
		    catch (e) { }
        }
        
	}
	return http_req;	
}

/**
* Fetch the new HTML contents. Run update_overview to replace contents.
*/
function run_overview_update() {

	var http_req = XML_HTTP_instance();
	if (!http_req) {
			if (DEBUG) {
				alert("Couldn't create a XMLHTTPRequest instance.");
			}
			return;
	}

	http_req.onreadystatechange = function() {
		if (http_req.readyState==4) {
			if (http_req.responseText.length == 0) {
				if (DEBUG) {
					alert("Response contained nothing.");
				}
				return;
			}

			overview_html = http_req.responseText;
			update_overview(overview_html);
		}
	}

	var url = IF_URL + "?" + IF_OVERVIEW_FUNCTION;
	http_req.open("GET", url, true);
	http_req.send(null);
}

/**
  * Replaces the HTML content of the element with id "overview".
  *
  * Parameters:
  *		html - HTML to replace content of overview with.
  */
function update_overview(html) {
	// Using innerHTML is not a good thing, but since we have to support
	// non-javascript clients, generating DOM object for all the content in js 
	// becomes much duplicate work...

	var overview = document.getElementById('overview');
	var overview_new = document.createElement('div');
	overview_new.setAttribute('id', "overview");
	overview_new.innerHTML = html;

	overview.parentNode.replaceChild(overview_new, overview);
	overview.innerHTML = html;
}


function overview_update_interval(seconds) {
	if (!seconds) {
		// no -- we don't want 0 seconds.
		alert("Argument seconds not provided, not starting interval.");
		return;
	}

	run_overview_update();
	
	var time = seconds * 1000;
	window.setInterval(run_overview_update, time);
}
