package Sql::SynonymGroup;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use Readonly;
use Params::Validate;

use Sql::Webadmin;

our @ISA = qw(Sql::Webadmin);

Readonly::Scalar our $TBL => "synonymGroup";


my $lang_q = "SELECT language FROM $TBL WHERE id = ?";
sub get_lang {
	my ($s, $lang) = @_;
	return $s->sql_single($lang_q, $lang);
}

sub exists { shift->SUPER::exists($TBL, 'id', @_) }
sub get { shift->SUPER::get($TBL, @_) }
sub insert { shift->SUPER::insert($TBL, @_) }
sub update { shift->SUPER::update($TBL, @_) }
sub delete { shift->SUPER::delete($TBL, @_) }


1;

