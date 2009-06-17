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
	<table class="genericAttributes">
		[% PROCESS _attr_row a = attr.creator title="Creator" IF attr.creator %]
		[% PROCESS _attr_row a = attr.author title="Author" IF attr.author %]
		[% k="last saved by"; PROCESS _attr_row a = attr.$k title="Last saved by" IF attr.$k %]
		[% PROCESS _attr_row a = attr.language title="Language" IF attr.language %]
		[%# PROCESS _attr_row a = attr.size title="Size" IF attr.size  # crashes with vmethod size %]
		[% PROCESS _attr_row a = attr.resolution title="Resolution" IF attr.resolution %]
		[% k="page count"; PROCESS _attr_row a = attr.$k title="Page count" IF attr.$k %]
	</table>
[% END %]

[% BLOCK sharepoint_attributes %]
	<table class="genericAttributes">
		[% PROCESS _attr_row a = attr.parent title="Item" IF attr.sptype.value == "file" && attr.parent %]
		[% PROCESS _attr_row a = attr.List title="List" IF attr.List %]
		[% PROCESS _attr_row a = attr.Site title="Site" IF attr.Site %]
	</table>
[% END %]

[% BLOCK _attr_row %]
	<tr>
		<td style="width : 7em">[% title | i18n %]</td>
		<td>
			<a href="[% a.query | query_url %]">[% a.value %]</a> 
			[% PROCESS filter_icon q = a.attr_query %]
		</td>
	</tr>
[% END %]
