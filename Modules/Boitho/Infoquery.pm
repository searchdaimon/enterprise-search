# Class: Boitho::Infoquery
# Wrapper for Boitho's tool infoquery.

package Boitho::Infoquery;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use constant VERSION => 1.0;
use constant VERBOSE => 0;

my $infoquery_path;
my $error = "";

sub new {
	my $class = shift;
	$infoquery_path = shift if @_;
    
	croak "Infoquery path ($infoquery_path) does not exist."
	    unless -e $infoquery_path;
	
	my $self = {};
	bless $self, $class;
	return $self;
}

sub get_error {
	return $error;
}
sub error { $_[0]->get_error; } #alias

sub crawlCollection { shift->_crawl("crawlCollection", @_)  }
sub recrawlCollection { shift->_crawl("recrawlCollection", @_)  }



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




sub deleteCollection($$) {
	my ($self, $collection) = @_;
 	
 	unless($collection) {
 		$error = "Didn't get a collection to delete.";
 		return 0;
 	}
 	
	return $self->_infoquery_exec("deleteCollection \Q$collection\E");
}

sub listUsers {
	my $self = shift;
	return $self->_getList('listUsers', 'user');
}

sub listMailUsers {
	my $self = shift;
	return $self->_getList('listMailUsers', 'user');
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

sub killCrawl {
    my ($s, $pid) = @_;
    return $s->_infoquery_exec("killCrawl \Q$pid\E");
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

sub _crawl  {
	my ($self, $crawl_cmd, $collection, %opt) = @_;

	unless ($collection) {
		$error = "Didn't get a collection to crawl.";
		return;
	}
	my $exec = "$crawl_cmd \Q$collection\E";
	
	if ($crawl_cmd eq 'recrawlCollection') {
		my $num_docs = $opt{documents_to_crawl} 
			? $opt{documents_to_crawl} : -1;
		$exec .= " \Q$num_docs\E";
	}

	$exec .= " \Q$opt{logfile}\E" 
		if defined $opt{logfile};

	return $self->_infoquery_exec($exec, $opt{callb});
}

##
# Execute infoquery, fetch the error if there is one.
# Get error with $iq->get_error;
#
# Important: This method does not escape
# its parameters. They must be escaped in the method
# calling this one.
sub _infoquery_exec {
    my ($self, $parameters, $callb) = (@_);
    my $success = 1;

    my $exec_string = "$infoquery_path $parameters";
    open my $iq, "$exec_string |"
        or carp "Unable to execute infoquery, $!";

    my @output;
    while (my $o = <$iq>) {
        $callb ? &$callb($o) : push @output, $o;
    }
    close $iq or $success = 0;
    unless ($success) {
        $error = join "\n", @output;
	warn("was unable to succesfuly execute infoquery: \"$exec_string\"\nGot error: $error");
        return;
    }

    return wantarray ? (1, @output) : 1;
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
