<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <title>[% "Searchdaimon" | html %] [% "search" | i18n %]</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <script type="text/javascript" src="js/common/jquery.js"></script>
  <script type="text/javascript" src="js/common/jquery.autocomplete.js"></script>
  <script type="text/javascript" src="js/common/suggest.js"></script>
  <link rel="stylesheet" href="css/common/suggest.css" type="text/css" />


	<style lang="text/css">
	body {
		font-family : arial, serif;	
	}

A:link { color : blue; }
A:visited { color : blue; }
A:active { color : blue; }
	

	#queryField, #queryButton {
		border : 1px solid;
		border-color : silver;
		
		
	}
	
	#queryField {
		font-size : large;
		font-weight: bold;
		background : white;
		width : 490px;
		height : 28px;
		color : #333333;
	}
	
	#queryButton {
		
		padding-right : 20px;
                padding-left : 20px;
		height : 32px;
		font-family: Arial, sans-serif;
		font-weight: bold;
                background : #014f99;
		/* background : white no-repeat scroll right;
		background-image : url(/glass.png); */
		color : white;
	}
	table {
		border : 0px solid;
		width : 600px;
		text-align : left;
		margin-top : 1em;
	}
	
	table td {
		width : 0px;
		vertical-align : top;
	}
	
	table td ul {
		list-style-type : none;
		margin : 0;
		padding : 0;
		font-size : small;
		margin-left : 0.5em;
	}
	#suggest_div {
		position : absolute; 
		top : 34px; 
		width : 352px; 
		left : -1px; 
		font-size : x-small;
	}
	</style>
    	
	<script type="text/javascript">
	$("#queryField").ready(function() {
			addAutocomplete($("#queryField"), $("#queryButton"));
			$("#queryField").focus();
	});
	</script>



</head>
<body>
<center>
<div>
<img src="img/common/logo_smaller.png" alt="Searchdaimon" title="Searchdaimon" />
<form action="#" method="get">
	<table><tr>
		<td>
		    <div id="query_div" style="position : relative;">

            		<input type="text" name="query" id="queryField" />
			
		    </div>
		</td>
		<td>
			<input type="submit" value="[% "Search" | i18n %]" id="queryButton" />
		</td>
		<td style="width : 100%;">
		</td>
	</tr></table>
	[% IF tpl %]
        	<input type="hidden" name="tpl" value="[% tpl | html %]" />
	[% END %]
	</div>

</form>
</center>
</body>
</html>
