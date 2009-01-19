[% USE HTML %]

[% BLOCK nav_page %]
    [% SET n = page_nav %]
    [% SET total_res = res_info.total_res %]
    [% SET per_page = n.num_results %]

    [% page_start = n.page - ((n.show_pages / 2) - (n.show_pages / 5))  %]
    [% IF page_start < 1 %]
        [% page_start = 1 %]
    [% END %]

    [% IF total_res > 0 %]
	<ul id="navWrapper">
    	<li id="navWrapperLeft">&nbsp;</li>
	<li id="liNavMenu">
            [% IF n.page > 1 %]
                <li id="prevNav"><a href="[% query | query_url %]&amp;page=[% n.page - 1 %]">
			[% "Prev" | i18n %]</a>
		</li>
	    [% ELSE %][%# legacy %]
	    	<li id="prevNav">&nbsp;</li>
            [% END %]

            [% SET p = page_start %]
            [% WHILE p < (n.show_pages + page_start) %]
	    	[% IF p == n.page %]
			<li id="navCurrent">[% p %]</li>
		[% ELSE %]
			<li>
                    		<a href="[% query | query_url %]&amp;page=[% p %]">[% p %]</a>
                	</li>
		[% END %]
                [% LAST IF (p * per_page) > total_res %]
                [% p = p + 1 %]

            [% END %]
            
            [% IF (n.page * per_page) < total_res %]
                <li id="nextNav"><a href="[% query | query_url %]&amp;page=[% n.page + 1 %]">[% "Next" | i18n %]</a></li>
	    [% ELSE %][%# legacy %]
	    	<li id="nextNav">&nbsp;</li>
            [% END %]
        </li>
    	<li id="navWrapperRight">&nbsp;</li>
	</ul>

    [% END %]
[% END %]


