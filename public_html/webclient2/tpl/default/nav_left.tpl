
[% BLOCK nav_left %]

	<ul id="navTree" class="filetree treeview-gray" style="display : none;"> 
		[% FOREACH group IN nav.groups %]
			[% INCLUDE nav_group g = group top_group = 1 %]
		[% END %]
	</ul>

[% END %]

[% BLOCK nav_group %]
	[% SET classes = [ ] %]

	[% IF !g.expanded %]
		[% classes.push("closed") %]
	[% END %]
	[% IF top_group %]
		[% classes.push("topGroup") %]
	[% END %]

	<!-- group -->
	<li class="[% classes.join(" ") %]"> 
		[%#TODO icon %]
		[% IF g.icon %]
			<img src="[% icon_url(g.icon) %]" alt="" />
		[% END %]
		[% IF top_group %]
			<div class="topGroupDiv">&nbsp;</div>
		[% END %]
		[% "<strong>" IF g.selected %]
		<a href="[% g.query | query_url %]">[% g.name | html %]</a>
		[% "</strong>" IF g.selected %]
		[% IF g.hits %]
			<span class="navHits">
				([% s="%1 hits"; s.i18n(g.hits) %])
			</span>
		[% END %]

		[% IF g.groups.size %]
			<ul>
			[% FOREACH sub_g IN g.groups %]
				[%# Using INCLUDE rather than
				    process to create a new scope. %]
				[% INCLUDE nav_group g = sub_g top_group = 0 %]
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
	[% hl_start = "" %]
	[% hl_end = "" %]
	[% IF i.selected %]
		[% hl_start = "<strong>" %]
		[% hl_end   = "</strong>" %]
	[% END %]
	[% SET icon = "" %]
	[% IF i.icon %]
		[% SET url = icon_url(i.icon) %]
		[% SET icon = "<img src=\"$url\" alt=\"\" />&nbsp;" %]
	[% END %]
	<!-- item -->
	<li>&nbsp;[% "<strong>" IF i.selected %] 
		<span>[% icon %]<a href="[% i.query | query_url %]">[% i.name | html %]</a>
		 </span>[% "</strong>" IF i.selected %]

		 <span class="navHits"> ([% 
		 	s="%1 hits";
			s.i18n(i.hits)
		%])</span>
	</li>
	<!-- end item -->
[% END %]
