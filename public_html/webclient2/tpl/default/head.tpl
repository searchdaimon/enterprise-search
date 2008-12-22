[% BLOCK header %]<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"    
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <title>[% query | html %] - Searchdaimon</title>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <link type="text/css" rel="stylesheet" href="css/common/suggest.css" />
    <link type="text/css" rel="stylesheet" href="css/common/jquery.treeview.css" />
    <link type="text/css" rel="stylesheet" href="css/default/searchpage.css" />

    <!--[if IE 6]>
        <link rel="stylesheet" type="text/css" href="css/default/searchpage_ie6.css" />
    <![endif]-->
    <script type="text/javascript" src="js/common/suggest.js"></script>
    <script type="text/javascript" src="js/common/jquery.js"></script>
    <script type="text/javascript" src="js/common/jquery.autocomplete.js"></script>
    <script type="text/javascript" src="js/common/jquery.cookie.js"></script>
    <script type="text/javascript" src="js/common/jquery.treeview.js"></script>
    <script type="text/javascript" src="js/common/jquery.corner.js"></script> 

    [% content %]

    <script type="text/javascript">
    $(document).ready(function() {
    	addAutocomplete($('#queryField'), $('#queryBtn'));
    });
    </script>
      </head>

<body>

<div id="topNav">
    
    <a href="?">
        <img src="img/common/logo_top.jpg" alt="Searchdaimon" />
    </a>


    <form method="get" action="?">
        <div>
            <input type="text" name="query" id="queryField" value="[% query | html %]" size="40" />
	    [% IF tpl %]
            	<input type="hidden" name="tpl" value="[% tpl | html %]" />
	    [% END %]

            <input type="image" id="queryBtn" 
                src="[% "(search_btn_img)" | i18n %]" title="[% "Start search" | i18n %]" alt="[% "Search" | i18n %]" />
        </div>
    </form>

</div>

<div id="infoBar">
    <span style="float : left;">
        <strong>[% "Results for" | i18n %]: <em>[% query %]</em></strong>
    </span>
    <span style="float : right; display : inline;">
	[% IF res_info.total_res %]

        	[% str = "Showing %1 - %2 out of %3 results. %4 sec."; 
	   	str.i18n(
			res_info.res_from,
			res_info.res_to,
			res_info.total_res, 
			res_info.search_time) 
		%]
    	[% ELSE %]
		[% 
			s = "Showing no results. %1 sec."; 
			s.i18n(res_info.search_time || 0) 
		%]
	[% END %]
    </span>
    <img src="img/default/infobar_right.jpg" alt="" />
</div>
<div style="clear : both; position : relative;">
[% END %]
