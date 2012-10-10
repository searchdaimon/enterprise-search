[% USE Dumper %]

[% BLOCK filter_icon %]
	<a href="[% q | query_url %]" class="filterIcon"><img 
		src="img/common/viewmag.gif" alt="[% alt || "All documents with attribute" %]" />
	</a>
[% END %]

[% BLOCK email_attributes %]
	<div class="emailAttributes">


		[% IF attr.from %]
			[% "From" | i18n %]: <a href="[% attr.from.query | query_url %]">[% attr.from.value %]</a>
			[% PROCESS filter_icon q = attr.from.attr_query alt="All documents from this person" %]
			&nbsp;&nbsp;
		[% END %]

		[% IF attr.to %]
			[% "To" | i18n %]: <a href="[% attr.to.query | query_url %]">[% attr.to.value %]</a> 
			[% PROCESS filter_icon q = attr.to.attr_query alt="All documents to this person" %]
			[% IF attr.num_receivers.value > 1 %]
				<span>
					([% s="and 1 others"; s.i18n(attr.num_receivers.value - 1) %])
				</span>
			[% END %]
		[% END %]

	</div>
[% END %]

[% BLOCK generic_attributes %]
	<ul class="genericAttributes">
              	[% FOREACH k IN attr.keys %]
                        [% PROCESS _attr_row a = attr.$k title="$k" %]
                [% END %]
	</ul>
[% END %]

[% BLOCK sharepoint_attributes %]
	<ul class="genericAttributes">
		[% PROCESS _attr_row a = attr.parent title="Item" IF attr.sptype.value == "file" && attr.parent %]
		[% PROCESS _attr_row a = attr.List title="List" IF attr.List %]
		[% PROCESS _attr_row a = attr.Site title="Site" IF attr.Site %]
	</ul>
[% END %]

[% BLOCK _attr_row %]
	<li>
		[% title | i18n_nowarn FILTER ucfirst %]:
		<a href="[% a.query | query_url %]">[% a.value %]</a> 
		[% PROCESS filter_icon q = a.attr_query %]
	</li>
[% END %]
