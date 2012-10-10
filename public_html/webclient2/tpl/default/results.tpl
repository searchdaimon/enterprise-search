[% PROCESS 'head.tpl' %][% WRAPPER header %]
    <!--[if lt IE 8]>
    	<style type="text/css">
	.duplicates ul { margin-top : -0.1em; }

    	/* IE fix: removes gap between items */
    	.duplicates li { display : inline-block; }
    	.duplicates li { display : block; }
	</style>
    <![endif]-->
 
	<script type="text/javascript">

	$('#filterText').ready(function() {
		$('#filterText').corner();
	});
        $('#nav').ready(function() { 
		$('#nav').corner(); 
	});
        $(document).ready(function() {
    
            $("#navTree").treeview({
		// animated : 10 // render error when using with z-index in IE

            });
	    $("#navTree").show();

        });


	</script>
[% END %]

[% USE HTML %]
[% USE Dumper %]
[% PROCESS 'nav_left.tpl' %]
[% PROCESS 'nav_page.tpl' %]
[% PROCESS 'attribute_blocks.tpl' %]

  <noscript>
           <style type="text/css">
                   #navTree, #navTree ul, #navTree li {
	                           display : block !important;
	                           background : #dee7f0 !important;
	                   }
	           </style>
	 </noscript>




<div id="filterText">
         <h4>[% "Filtering" | i18n %]</h4>
</div>
<div id="nav">
	<div style="margin-left : 0.5em; font-size : medium; margin-top : 0.5em; ">
		<a style="text-decoration : none;  color : blue;" href="[% navigation.return_query | query_url %]">&#171;&nbsp;[% "All results" | i18n %]</a>
	</div>


	[% PROCESS nav_left nav = navigation %]
</div>
</div>

<ul id="collectionNav">
	<li class="collectionTab[% " selected" UNLESS coll_info.selected %]">
            <a href="[% coll_info.all_query | query_url %]">[% "All collections" | i18n %]</a>&nbsp;<span class="collectionRes" /><div class="collRight">&nbsp;</div>
        </li>
	[% FOREACH c IN coll_info.coll %]
		<li class="collectionTab[% " selected" IF coll_info.selected == c.name %]">
            		<a href="[% c.query | query_url %]">[% c.name | html | replace(' ', '&nbsp') %]</a>[% "&nbsp;" 
			%]<span class="collectionRes">([% 
				IF c.results; 
					s = "1 hits"; 
					s.i18n(c.results);
				ELSE;
					"no hits" | i18n;
				END;
			%])</span><div class="collRight">&nbsp;</div>
        	</li>
	[% END %]
</ul>

<div style="clear : both; margin-left : 16em;" class="lineBlue">&nbsp;</div>

<div id="messages">
	[% IF errors %] [%# TODO: Handle errors better %]
    		<div id="errorText">
		[% FOREACH e IN errors %]
			<p>[% e | html %]</p>
		[% END %]
    		</div>
	[% END %]
[% IF res_info.spelling.text %]
    <div id="spellcheck">
    	[% s = "Did you mean: 1?"; 
	   s.i18n(HTML.escape(res_info.spelling.text), res_info.spelling.query.query_url) 
	 %]
	</div> 
[% END %]

</div>

<div id="sortBar">
	[% INCLUDE sort_bar %]
</div>


<div id="results">
	[% UNLESS results.size %]
		<p id="noResultsText">[%   
			s = 'Found no results for query "1".';
			s.i18n(query) | html %]
		</p>
	[% ELSE %]
		<ul>
			[% FOREACH r IN results %]
				[% PROCESS show_res r = r %]
			[% END %]
		</ul>
	[% END %]
</div>

[% IF res_info.filtered %]
<div id="filtered">
 	[% str = "(filtered_msg)"; 
	   str.i18n(res_info.filtered) %]
</div>
[% END %]

[% INCLUDE nav_page %]
[% PROCESS 'foot.tpl' %]

[%##
  # blocks 
  %]
[% BLOCK show_res %]
	<li>
		<table><tr>
		<td>
		<div class="title"><img src="[% icon_url(r.icon) %]" width="14" height="14" alt="" />&nbsp;<a href="[% r.url | html %]">[% r.title | html %]</a></div>
		[% IF r.attributes.size && r.filetype == "eml" %]
			[% INCLUDE email_attributes attr = r.attributes %]
		[% END %]
		<div class="snippet">
			[% SET s = r.snippet.snippet.pop %]
			[% IF s.type == "db" %]
				<table class="dbSnippet">
				[% FOREACH tr IN s.table.pop.tr %]
					<tr>
					[% FOREACH td IN tr.td %]
						<td [% IF loop.first %]class="first"[% END %]>[% td %]</td>
					[% END %]
					</tr>
				[% END %]
				</table>
			[% ELSE %]
				[% r.snippet %]
			[% END %]
		</div>
			[% IF r.attributes.source && r.attributes.source.value == "sharepoint" %]
				[% INCLUDE sharepoint_attributes attr = r.attributes %]
			[% END %]

			[% INCLUDE generic_attributes attr = r.attributes %]
		<div>
			<span class="url" title="[% r.fulluri | html %]">[% r.uri | html %]</span>
			<span class="details">
				[% IF r.cache %]
				<a href="[% gen_cache_url(r.cache) %]">[% "cache" | i18n %]</a>
				[% END %]
				[% IF r.age %]
				 - [% r.age | html %]
				[% END %]
			</span>
			[% PROCESS show_dupes dupes = r.dupes IF r.dupes.size %]
		</div>
		</td>
		<td style="width : 110px;">
			[% IF r.thumb %]
				<a href="[% r.url %]"><img src="[% r.thumb %]" class="thumb" alt="thumb" width="100" height="100"/></a>
			[% END %]
		</td>
		</tr>
		</table>
	</li>
[% END %]

[% BLOCK show_dupes %]
	[% SET dupe_id = 0 UNLESS dupe_id %]
	[% dupe_id = dupe_id + 1 %]
	<div class="duplicates" style="margin-top : 0.5em;" >
		<strong>[% s = "Found 1 other copies";
			   s.i18n(dupes.size) %]
			(<a  onclick="$('#dupe[% dupe_id %]').toggle(200); return 1;">[% "show" | i18n %]</a>)
		</strong>
		<ul class="duplicateList" id="dupe[% dupe_id %]" style="display: none;">
			[% FOR d IN dupes %]
				<li>
					<a title="[% d.fulluri | html %]" href="[% d.url | html %]">[% d.uri | html %]</a>
				</li>
			[% END %]
		</ul>
	</div>
[% END %]
			
[% BLOCK sort_bar %]
	<span id="sortTitle">[% "Sort" | i18n %]:</span>&nbsp;&nbsp;[% 
		INCLUDE _sort_link l = 'newest' %]&nbsp;&nbsp;[%
		INCLUDE _sort_link l = 'oldest' %]&nbsp;&nbsp;[%
		INCLUDE _sort_link l = 'relv'   %]
[% END %]

[% BLOCK _sort_link %][% FILTER collapse %]
	[% SET query = sort_info.query.$l %]

	[% SET txt = { newest => "newest first", 
		   oldest => "oldest first",
		   relv   => "relevance" } %]

	[% IF l == sort_info.current %]
		[% txt.$l | i18n | strong %]
	[% ELSE %]
		<a href="[% query | query_url %]">[% txt.$l | i18n %]</a>
	[% END %]
[% END %][% END %]
