<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"    
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <title>[% query | html %] - [% "Enterprise search" | i18n %]</title>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <link type="text/css" rel="stylesheet" href="css/common/suggest.css" />

    <script type="text/javascript" src="js/common/suggest.js"></script>
    <script type="text/javascript" src="js/common/jquery.js"></script>
    <script type="text/javascript" src="js/common/jquery.autocomplete.js"></script>

    <script type="text/javascript">
    $(document).ready(function() {
    	addAutocomplete($('#queryField'), $('#queryBtn'));
    });
    </script>
      </head>

<body>

[% USE HTML %]

<h1>[% "Enterprise search" | i18n %]</h1>
<div>
<form action="?" method="get">
	<div id="query_div" style="position : relative;">
        	<input type="text" name="query" id="queryField" value="[% query | html %]" />
		<input type="submit" value="[% "Search" | i18n %]" id="queryButton" />
	
		[% FOREACH p IN query_params %]
        		<input type="hidden" name="[% p.key %]"  value="[% p.value | html %]" />
		[% END %]
	</div>
</form>
</div>

<div><h3>Collections</h3>
	<ul>
		<li class="[% "selected" UNLESS coll_info.selected %]">
            		<a href="[% coll_info.all_query | query_url %]">[% "All collections" | i18n %]</a>
		</li>
		[% FOREACH c IN coll_info.coll %]
		<li class="[% "selected" IF coll_info.selected == c.name %]">
            		<a href="[% c.query | query_url %]">[% c.name | html %]</a>
			([% 
				IF c.results; 
					s = "n hits"; 
					s.i18n(c.results);
				ELSE;
					"no hits" | i18n;
				END;
			%])
		</li>
		[% END %]
	</ul>
</div>


<div><h3>Navigation</h3>
	<a href="[% navigation.return_query | query_url %]">[% "All results" | i18n %]</a>

	[% PROCESS nav_left nav = navigation %]
</div>


<div><h3>Sort</h3>
	<ul>
	[% FOREACH sort IN ['newest', 'oldest', 'relv' ] %]
		<li>
		[% IF sort == sort_info.current %]
			<strong>[% sort | i18n %]</strong>
		[% ELSE %]
			<a href="[% sort_info.query.$sort | query_url %]">[% sort | i18n %]</a>
		[% END %]
		</li>
	[% END %]
	</ul>
</div>

<div>
	[% IF errors %]
	<div>
	<h4>Errors</h4>
		[% FOREACH e IN errors %]
			<p>[% e | html %]</p>
		[% END %]
	</div>
	[% END %]
	[% IF res_info.spelling.text %]
	<div>
	<h4>Suggestion</h4>
    	[% s = "Did you mean: s?"; 
	   s.i18n(
	   	HTML.escape(res_info.spelling.text), 
		res_info.spelling.query.query_url
	   ) 
	 %]
	</div> 
[% END %]
</div>


<div><h3>Results</h3>
	[% UNLESS results.size %]
		<p>[%   
			s = 'Found no results for query "s".';
			s.i18n(query) | html %]
		</p>
	[% ELSE %]
	[% SET i = 0 %]
			[% FOREACH r IN results %]
				[% SET i = i + 1 %]
				<h4>Result [% i %]</h4>
				<table border="1">
					<tr><td>Title</td><td>[% r.title | html %]</td></tr>
					<tr><td>Icon</td><td><img src="[% icon_url(r.icon) %]" alt="" /></td></tr>
					<tr><td>Attributes</td><td>
						<ul>
						[% FOREACH a IN r.attributes %]
							<li>
							[% a.key | html %]: [% a.value.value | html %] &mdash;
							(<a href="[% a.value.query | query_url %]">filter</a>)
							(<a href="[% a.value.attr_query | query_url %]">search attribute</a>)
							</li>
						[% END %]
						</ul>
					</td></tr>
					<tr><td>Snippet</td><td>[% r.snippet %]</td></tr>
					<tr><td>Short&nbsp;URI</td>
						<td><a href="[% r.url %]" title="Long: [% r.fulluri | html %]">[% r.uri | html %]</a>
					</td></tr>
					<tr><td>Cache</td>
					    <td>
						[% IF r.cache %]
							<a href="[% gen_cache_url(r.cache) %]">[% "cache" | i18n %]</a>
						[% ELSE %]
							No cache.
						[% END %]
					</td></tr>
					<tr><td>Timestamp</td>
						<td>
							[% IF r.age %]
								[% r.age | html %]
							[% ELSE %]
								No timestamp.
							[% END %]
						</td>
					</tr>
					<tr><td>Duplicates</td>
					    <td>
						<ul>
					    	[% FOREACH d IN r.dupes %]
							<li>
							<a title="[% d.fulluri | html %]" 
							   href="[% d.url | html %]">
							   	[% d.uri | html %]
							</a>
							</li>
						[% END %]
						</ul>
					</td></tr>
					<tr><td>Thumb</td>
					<td>
					[% IF r.thumb %]
						<img src="[% r.thumb %]" alt="thumb" />
					[% ELSE %]
						No thumbnail.
					[% END %]
					</td></tr>
				</table>
			[% END %]
	[% END %]
</div>

[% IF res_info.filtered %]
<div>
	<h3>Filter message</h3>
 	[% str = "(filtered_msg)"; 
	   str.i18n(res_info.filtered) %]
</div>
[% END %]

[% INCLUDE nav_page %]
</body></html>



[%# build navigation tree %]
[% BLOCK nav_left %]
	<ul> 
		[% nav.preprocess_nav %]
		[% FOREACH group IN nav.groups %]
			[% INCLUDE nav_group g = group %]
		[% END %]
	</ul>

[% END %]

[% BLOCK nav_group %]
	<!-- group -->
	<li> 
		[% IF g.icon %]
			<img src="[% icon_url(g.icon) %]" alt="" />
		[% END %]
		[% "<strong>" IF g.selected %]
		<a href="[% g.query | query_url %]">[% g.name | html %]</a>
		[% "</strong>" IF g.selected %]
		
		[% IF g.hits %]
			([% s="n hits"; s.i18n(g.hits) %])
		[% END %]

		[% IF g.groups.size %]
			<ul>
			[% FOREACH sub_g IN g.groups %]
				[%# Using INCLUDE rather than
				    process to create a new scope. %]
				[% INCLUDE nav_group g = sub_g %]
			[% END %]
			</ul>
		[% END %]

		[% IF g.items.size %]
			<ul>
			[% FOREACH item IN g.items %]
				[% INCLUDE nav_item i = item %]
			[% END %]
			</ul>
		[% END %]

	</li>
	<!-- end group -->

[% END %]

[% BLOCK nav_item %]
	[% SET icon = "" %]
	[% IF i.icon %]
		[% SET url = icon_url(i.icon) %]
		[% SET icon = "<img src=\"$url\" alt=\"\" />&nbsp;" %]
	[% END %]
	<!-- item -->
	<li>
		[% "<strong>" IF i.selected %] 
		[% icon %]
		<a href="[% i.query | query_url %]">[% i.name | html %]</a>
		[% "</strong>" IF i.selected %]

		([% 
		 	s="n hits";
			s.i18n(i.hits)
		%])
	</li>
	<!-- end item -->
[% END %]


[%# Page navigation %]
[% BLOCK nav_page %]
	[% SET n = page_nav %]
	[% SET total_res = res_info.total_res %]
	[% SET per_page = n.num_results %]

	[% page_start = n.page - ((n.show_pages / 2) - (n.show_pages / 5))  %]
		[% IF page_start < 1 %]
		[% page_start = 1 %]
	[% END %]

	[% IF total_res > 0 %]
		<ul>
		[% IF n.page > 1 %]
			<li><a href="[% query | query_url %]&amp;page=[% n.page - 1 %]">
			[% "Prev" | i18n %]</a>
			</li>
		[% END %]

		[% SET p = page_start %]
		[% WHILE p < (n.show_pages + page_start) %]
			[% IF p == n.page %]
				<li><strong>[% p %]</strong></li>
			[% ELSE %]
			<li>
				<a href="[% query | query_url %]&amp;page=[% p %]">[% p %]</a>
			</li>
			[% END %]
			[% LAST IF (p * per_page) > total_res %]
			[% p = p + 1 %]
		[% END %]

		[% IF (n.page * per_page) < total_res %]
			<li><a href="[% query | query_url %]&amp;page=[% n.page + 1 %]">[% "Next" | i18n %]</a></li>
		[% END %]

		</ul>

	[% END %]
[% END %]


