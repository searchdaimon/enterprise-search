package Boitho::Infoquery;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use constant VERSION => 1.0;
use constant VERBOSE => 0;

# This class is a wrapper for Boitho's tool infoquery.

my $infoquery_path;

eval {
	# Get's path to infoquery from a config file.
	# The path should be in $CONFIG->{'infoquery'}
	require config;
	import config qw($CONFIG);
	our $CONFIG;
	$infoquery_path = $CONFIG->{'infoquery'};
};
if ($@) { # Warn if verbose
	carp "Could not load config file, 
	path must be provided when creating an instance. $@" if VERBOSE;
}

my $error = "";

sub new {
	my $class = shift;
	$infoquery_path = shift if @_;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub get_error {
	return $error;
}
sub error { $_[0]->get_error; } #alias

sub crawlCollection($$) {
	my $self = shift;
	my $collection = shift;

	unless($collection) {
		$error = "Didn't get a collection to crawl.";
		return undef;
	}

	my ($retval, $output) =
		$self->_infoquery_exec("crawlCollection \Q$collection\E");

	return $retval;
}


sub authTest($$) {
	my ($self, $method, @args) = @_;
	
	unless ($method) {
		my $error = "No integration method selected";
		return (0, $error);
	}
	# TODO: Implement in infoquery.
	# error example: 
		#return (0, "Username/password is wrong.");
	return 1;
	
	#my $param .= "\Q$_\E" foreach @args;
	#return $self->
	#	_infoquery_exec("authTest $param");
}


sub recrawlCollection($$) {
	my $self = shift;
	my $collection = shift;

	unless($collection) {
		$error = "Didn't get a collection to crawl.";
		return 0;
	}

	my ($retval, $output) =
		$self->_infoquery_exec("recrawlCollection \Q$collection\E");

	return $retval;
}


sub deleteCollection($$) {
 	return 1; # Until Infoquery is fixed.
#	my ($self, $collection) = @_;
# 	
# 	unless($collection) {
# 		$error = "Didn't get a collection to crawl.";
# 		return 0;
# 	}
# 	
#	return $self->_infoquery_exec("deleteCollection \Q$collection\E");
}

sub listUsers {
	my $self = shift;
	return $self->_getList('listUsers', 'user');
}

sub listGroups {
	my $self = shift;
	return $self->_getList('listGroups', 'group');
}

sub scan($$$$$) {
	my $self = shift;
	my $connector = shift;
	my ($host, $username, $password) = (@_);
	
	unless ($host) {
		$error = "Infoquery didn't get any host".
		return 0;
	}
	
	my $param = "scan \Q$connector\E \Q$host\E";
		$param .= " \Q$username\E" if $username;
		$param .= " \Q$password\E" if $password;
		
	return $self->_getList($param, 'share');
}

# Returns groups and collections of a given username.
# The returned datastructure looks like this:
# { 	'group name #1' => [
# 			'collection name #1',
# 			'collection name #2',
# 			],
# 	'group name #2' => [],
# };
sub groupsAndCollectionForUser($$) {
	my ($self, $username) = @_;
	my %groups = ();
	
	unless ($username) {
		$error = "Infoquery didn't get a username";
		return 0;
	}
	
	my ($success, @output) 
		= $self->_infoquery_exec("groupsAndCollectionForUser \Q$username\E");
	return undef unless $success;

	carp Dumper(\@output);

	my $last_group = '';
	foreach my $line (@output) {
		my ($key, $value) = split ": ", $line;
		next unless $value;
		if ($key eq 'group') { #$value contains groupname
			$groups{$value} = [] unless $groups{$value};
			$last_group = $value;
		}
		
		if ($key eq 'collection') { # $value contains collection name
			push @{$groups{$last_group}}, $value;
		}
	}
	
	return \%groups;
}

sub documentsInCollection($$) {
	my ($self, $collection) = (@_);
	unless ($collection) {
		carp "Didn't get a collection name.";
		return;
	}
	
	my $doc_count_ref = $self->_getList("documentsInCollection \Q$collection\E", 'Documents');

	#runarb, legger til at infoquery kan feile, og ikke gi liste
	if (not defined($doc_count_ref)) {
		return -1; #returnerer -1 som en feilkode
	}
	else {
		return $doc_count_ref->[0];
	}
}

# Execute infoquery, fetch the error if there is one.
# Returns: (success, @output). On error, get it with $iq->get_error;
#
# Important: This method does not escape
# its parameters. They must be escaped in the method
# calling this one.
sub _infoquery_exec($$) {
	my ($self, $parameters) = (@_);
	my $success = 1;

	croak  "Infoquery path not defined. Provide path when 
		creating an instance (new->(PATH)) or use a config file."
		unless $infoquery_path;

	my $exec_string = "$infoquery_path $parameters";
	open my $iq, "$exec_string |"
		or carp "Unable to execute infoquery, $!";
	my @output = <$iq> if $iq;
	close $iq or $success = 0;
	unless ($success) {
		$error = $output[0];
		return 0;
	}

	return (1, @output);
}


# Helper method to parse simple lists from infoquery.
sub _getList {
	my ($self, $parameter, $keyword) = @_;
	my @list = ();

	my ($success, @output) 
		= $self->_infoquery_exec($parameter);

	return undef unless $success;
	
	foreach my $line (@output) {
		my ($key, $value) = split(": ", $line);
		next unless ($value);
		chomp($value);
		push @list, $value if ($key eq $keyword);
	}
	@list = sort { lc($a) cmp lc($b) } @list; #sort the human way.
	return \@list;
}


1;
