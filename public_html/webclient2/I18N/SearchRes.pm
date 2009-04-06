package I18N::SearchRes;
use strict;
use warnings;
use base 'Locale::Maketext';

sub load_lang {
	my $tpl = shift;
	Locale::Maketext::Lexicon->import({
		'*' => [Gettext => "locale/$tpl/*/search.po"],
	});
	1;
}

use Locale::Maketext::Lexicon {
	#'*' => [Gettext => 'locale/*/search.po'],
	#_auto => 1, # fallback to another lang
	_decode => 1, # utf8
	_style => 'gettext',
};

1;
