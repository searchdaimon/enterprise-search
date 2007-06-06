package Common::Generic;
use strict;
use warnings;
use Data::Dumper;
use Carp;
use Sql::Sql;
use Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw(init_root_page);

sub new {
 	my $class = shift;
 	my $self = {};
 	bless $self, $class;
 	return $self;
}

# This method checks if a side button was clicked (edit, delete)
# And returns the ID.
sub request {
	my ($self, $data_ref) = @_;
	grep { 
		return $_->{'id'} if (defined($_) and $_->{'submit'});
	} @$data_ref;
	return;
}

# Return auth_id.
# Adds new username/password if auth doesn't exist.
sub get_auth_id {
	my ($self, $dbh, $auth_id, $user, $pass) = @_;

	unless ($auth_id) {
		my $sqlAuth = Sql::CollectionAuth->new($dbh);
		$auth_id = $sqlAuth->add($user, $pass);
	}
	return $auth_id;
}

##
# Common init procedure for root pages.
#
# Attributes:
#	template_dir - Template dir for page.
#	page_class - Default page class for page
#
# Returns:
#	cgi - CGI instance
#	state_ptr - POST/GET parameters
#	vars - Template vars
#	dbh - Database handler
#	page - Page class instance
sub init_root_page {
	my ($template_dir, $page_class) = @_;
	my $cgi = CGI->new;
	my $state_ptr = CGI::State->state($cgi);
	
	my $vars = { };
	my $template = Template->new(
		{INCLUDE_PATH => "./templates:./templates/common:.$template_dir"}
	);

	my $sql = Sql::Sql->new();
	my $dbh = $sql->get_connection();
	
	my $page = $page_class->new($dbh);
	return ($cgi, $state_ptr, $vars, $template, $dbh, $page);
}
1;