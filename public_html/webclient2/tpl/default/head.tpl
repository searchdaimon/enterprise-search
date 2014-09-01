[% BLOCK header %]<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"    
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <title>[% query | html %] - [% "Enterprise search" | i18n %]</title>


    [% PROCESS 'shoulder.tpl' %]

    [% content %]

      </head>

<body>

<div id="topNav">
    
    <a href="?">
        <img src="img/common/logo_top.jpg" alt="Logo" width="368" height="102"/>
    </a>


    <form method="get" action="?">
        <div>
            <input type="text" name="query" id="queryField" value="[% query | html %]" size="40" />
[% FOREACH p IN query_params %]
        	<input type="hidden" name="[% p.key %]"  value="[% p.value | html %]" />
	[% END %]
	


            <input type="image" id="queryBtn" 
                src="[% "(search_btn_img)" | i18n %]" title="[% "Start search" | i18n %]" alt="[% "Search" | i18n %]" />
        </div>
    </form>

</div>

[% IF query.defined && !hide_infobar_ok %]
<div id="infoBar">
    <span style="float : left;">
        <strong>[% "Results for" | i18n %]: <em>[% query %]</em></strong>
    </span>
    <span style="float : right; display : inline;">
	[% IF res_info.total_res %]

        	[% str = "Showing 1 - 2 out of 3 results. 4 sec."; 
	   	str.i18n(
			res_info.res_from,
			res_info.res_to,
			res_info.total_res, 
			res_info.search_time) 
		%]
    	[% ELSE %]
		[% 
			s = "Showing no results. 1 sec."; 
			s.i18n(res_info.search_time || 0) 
		%]
	[% END %]
    </span>
    <img src="img/default/infobar_right.jpg" alt="" width="18" height="27"/>
</div>
[% END %]
<div style="clear : both; position : relative;">
[% END %]
