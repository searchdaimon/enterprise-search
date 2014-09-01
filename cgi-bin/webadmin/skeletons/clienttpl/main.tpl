<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <title>[% "Enterprise search" | i18n %]</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <script type="text/javascript" src="js/common/jquery.js"></script>
  <script type="text/javascript" src="js/common/jquery.autocomplete.js"></script>
  <script type="text/javascript" src="js/common/suggest.js"></script>
  <link rel="stylesheet" href="css/common/suggest.css" type="text/css" />

	<script type="text/javascript">
	$("#queryField").ready(function() {
			addAutocomplete($("#queryField"), $("#queryButton"));
			$("#queryField").focus();
	});
	</script>

</head>
<body>
<h1>[% "Enterprise search" | i18n %]</h1>
<div>
<form action="?" method="get">
	<div id="query_div" style="position : relative;">
        	<input type="text" name="query" id="queryField" />
		<input type="submit" value="[% "Search" | i18n %]" id="queryButton" />
	[% FOREACH p IN query_params %]
        	<input type="hidden" name="[% p.key %]"  value="[% p.value | html %]" />
	[% END %]
	</div>
</form>
</div>
</body>
</html>
