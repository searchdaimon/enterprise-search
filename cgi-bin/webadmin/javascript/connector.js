

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

function hideMsgs() {
    $.each($(".message_box"), function(index, obj) {
        $(obj).hide("fast");
    });
}


$(document).ready(function() {
        /**
         * Draw tabs
         */
        $('#connContainer > ul').tabs({ 
            selected : window.selected_tab ? selected_tab : 0,
            show     : function() { hideMsgs() }
        });
});


function setMsg(visible, boxelem, msgelem, msg) {
        if (msg == null) msg = "";
        if (visible) {
            msgelem.html(msg);
            boxelem.show("fast");
        }
        else {
            boxelem.hide();
        }
    }

function setError(msg) { 
        if ($('#succsBox').is(":visible")) 
            setSuccs(null);

        setMsg(msg != null, $('#errorBox'), $('#errorBox .mb_text'), msg); 
    }
function setSuccs(msg) {
        if ($('#errorBox').is(":visible"))
            setError(null);


        setMsg(msg != null, $('#succsBox'), $('#succsBox .mb_text'), msg); 
    }


