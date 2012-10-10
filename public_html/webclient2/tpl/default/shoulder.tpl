
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />

    <link rel="shortcut icon" href="/webclient2/favicon.ico">
    <link rel="icon" type="image/ico" href="/webclient2/favicon.ico">

    <link rel="alternate" type="application/rss+xml" title="[% query | html %] - Searchdaimon search" href="api/opensearch/1.1/search?query=[% query | html %]" />
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

    [% IF anonymous!=1 %]
	    <script type="text/javascript">
	    $(document).ready(function() {
	    	addAutocomplete($('#queryField'), $('#queryBtn'));
	    });
	    </script>
    [% END %]
