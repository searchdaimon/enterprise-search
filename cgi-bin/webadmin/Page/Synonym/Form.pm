package Page::Synonym::Form;
use strict;
use warnings;
BEGIN { 
	push @INC, $ENV{BOITHOHOME} . "/Modules"; 
}
use Readonly;
use Data::Dumper;
use Carp;

use SD::ISO639 qw(%ISO639_TWO_LETTER);
use Page::Synonym;
our @ISA = qw(Page::Synonym);

Readonly::Scalar my $TPL_LIST => "synonym_list.html";

sub show_list {
	my ($s, $tpl_vars) = @_;
	my @languages = map { 
		[ $_, $ISO639_TWO_LETTER{$_} ] 
	} keys %ISO639_TWO_LETTER;

	@languages = sort { $a->[1] cmp $b->[1] } @languages;
	$tpl_vars->{languages} = [ 
		[ "_all", "All languages" ],
		@languages
	];
	return $TPL_LIST;
}
1;
