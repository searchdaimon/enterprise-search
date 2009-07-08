package Page::Synonym;
use strict;
use warnings;
use Carp;

use Page::Abstract;
our @ISA = qw(Page::Abstract);

sub _init {
	my $s = shift;
	$s->{sqlsyn} = Sql::Synonym->new($s->{dbh});
	$s->{sqlsyngroup} = Sql::SynonymGroup->new($s->{dbh});
}
1;

