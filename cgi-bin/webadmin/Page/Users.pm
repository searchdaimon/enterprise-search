package Page::Users;
use strict;
use warnings;
use Carp;
use File::stat;
use Data::Dumper;

our %CONFIG;
require 'config.pl';

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}
1;
