package Sql::Synonym;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Readonly;
use Params::Validate;

use Sql::Webadmin;

our @ISA = qw(Sql::Webadmin);

Readonly::Scalar our $TBL => "synonym";

sub all_lists {
	my $s = shift;
	my %groups;
	for my $res ($s->get({}, [qw(word group)])) {
		push @{$groups{$res->{group}}}, $res->{word};
	}
	return map { { 
		group => $_, 
		list  => $groups{$_},
	} } keys %groups;
}

sub exists { shift->SUPER::exists($TBL, '`group`', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }



1;
