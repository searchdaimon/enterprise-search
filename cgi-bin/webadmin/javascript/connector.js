

succsFunc = function(resp) {
    if (resp['error']) {
        setError(resp['error']);
    }
    else {
        setSuccs(resp['ok']);
    }
};
errorFunc = function() {
    setError("Internal error, or no connection.");
};
beforeFunc = function(x) { $('#loading').show(); }
completeFunc = function(x, y) { $('#loading').hide(); }

function getDefaultParams() {
    return {
        type : "GET",
        url  : "connector.cgi",
        dataType: "json",
        async : true,
        cache : false,
        success: succsFunc,
        error  : errorFunc,
        beforeSend: beforeFunc,
        complete: completeFunc
    };
}



$(document).ready(function() {
        /**
         * Draw tabs
         */
        $('#connContainer > ul').tabs({ 
            selected : window.selected_tab ? selected_tab : 0,
            show     : function() { setMsg(null); }
        });
});


function setError(msg) { 
	setMsg("error", msg);
    }
function setSuccs(msg) {
	setMsg("success", msg);
    }


