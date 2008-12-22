[% PROCESS 'head.tpl' %][% INCLUDE header %]

<div id="errors">
	[% FOREACH e IN errors %]
		<p>[% e | html %]</p>
	[% END %]
</div>

[% PROCESS 'foot.tpl' %]
