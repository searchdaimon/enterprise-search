package I18N::SearchRes;
use strict;
use warnings;
use base 'Locale::Maketext';

sub nbsp_quant {
    my($handle, $num, @forms) = @_;

    return $num if @forms == 0; # what should this mean?
    return $forms[2] if @forms > 2 and $num == 0; # special zeroth case

    # Normal case:
    # Note that the formatting of $num is preserved.
    return( $handle->numf($num) . '&nbsp;' . $handle->numerate($num, @forms) );
    # Most human languages put the number phrase before the qualified phrase.
}


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
