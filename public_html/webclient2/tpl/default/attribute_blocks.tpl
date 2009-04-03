[% USE Dumper %]
[% BLOCK email_attributes %]
	<div class="emailAttributes">


		[% IF attr.from %]
			[% "From" | i18n %]: <a href="[% attr.from.query | query_url %]">[% attr.from.value %]</a>
		[% END %]

		[% IF attr.to %]
			[% "To" | i18n %]: <a href="[% attr.to.query | query_url %]">[% attr.to.value %]</a> 
			[% IF attr.num_receivers.value > 1 %]
				<span>
					([% s="and %1 others"; s.i18n(attr.num_receivers.value - 1) %])
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

[% BLOCK _attr_row %]
	<tr>
		<td style="width : 7em">[% title | i18n %]</td>
		<td>
			<a href="[% a.query | query_url %]">[% a.value %]</a> 
			<a href="[% a.attr_query | query_url %]"><img src="img/common/filter.png" alt="All documents with attribute" /></a>
		</td>
	</tr>
[% END %]
