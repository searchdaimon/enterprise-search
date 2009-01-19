[% PROCESS 'head.tpl' hide_infobar_ok = 1 %][% INCLUDE header %]

<div id="errors">
	[% FOREACH e IN errors %]
		<p>[% e | html %]</p>
	[% END %]
</div>

[% PROCESS 'foot.tpl' %]
