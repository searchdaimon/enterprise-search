package Page::Add;
use strict;
use warnings;
use Sql::Shares;
use Sql::Connectors;
use Sql::CollectionAuth;
use Sql::ShareGroups;
#use Modules::Boitho::Infoquery;
use Boitho::Infoquery;
use Data::Dumper;
use Common::Collection;
use Carp;

# Helper class with functions used with add.cgi
# Function used to add shares.
#

sub new {
	my $class = shift;
	my $dbh = shift;
	my $state = shift;
	my $self = {};
	bless $self, $class;
	
	$self->_init($dbh, $state);

	return $self;
}

sub _init {
	my ($self, $dbh, $state) =  (@_);
	$self->{'dbh'} = $dbh;
	$self->{'state'} = $state;
	$self->{'sqlShares'} = Sql::Shares->new($dbh);
	$self->{'sqlConnectors'} = Sql::Connectors->new($dbh);
	$self->{'sqlAuth'} = Sql::CollectionAuth->new($dbh);
	$self->{'collection'} = Common::Collection->new($dbh);
	
}

# Adds a given share to the database.
sub add_share {
	my $self = shift;
	my $share = shift;
	my $sqlShares = $self->{'sqlShares'};
	my $sqlGroups = Sql::ShareGroups->new($self->{'dbh'});
	$share->{'active'} = 1 if ($share->{'active'}); # HTML form sets it to "on"
	my $id = $sqlShares->insert_share($share);
	$sqlGroups->set_groups($id, $share->{'group_member'});
	
	return 1;
}

# Print the html for the first form used when adding a share.
sub show_first_form {
	my ($self, $vars) = (@_);
	my $sql = $self->{'sqlConnectors'};
	my $template = $self->{'template'};

	$vars->{'connectors'} = $sql->get_connectors();
	return ($vars, 'add.html');

}

# Print the html for the second form used when adding a share.
sub show_second_form {
	my ($self, $vars, $connector) = (@_);
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $sqlAuth = $self->{'sqlAuth'};
	my $template = $self->{'template'};
	my $state = $self->{'state'};
	my $iq = new Boitho::Infoquery;
	
	my $share = $state->{'share'};
	if (!$share->{'connector_name'}) {
		$share->{'connector_name'} = $sqlConnectors->get_name($connector)
	}
	$vars->{'share'} = $share;
	$vars->{'authentication'} = $sqlAuth->get_authentication;
	$vars->{'group_list'} = $iq->listGroups;
	$vars->{'input_fields'} = $sqlConnectors->get_input_fields($connector);
	$vars->{'from_scan'} = $state->{'from_scan'}
		if $state->{'from_scan'}; # Coming from scan result. Contain's id.
	
	return ($vars, 'add_details.html');
}

## Handles what to do when the user submits the first form.
sub submit_first_form($$$) {
	my ($self, $vars, $share) = (@_);
	my $dbh        = $self->{'dbh'};
	my $state      = $self->{'state'};
	my $collection = $self->{'collection'};
	my ($valid, $msg) = $collection->validate($share, 
			qw(collection_name connector));
			
	my $template_file;
	
	unless($valid) {
		# User didn't provide valid input.
		# Show the first form again.
		$vars->{$msg} = 1;
		$vars->{'share'} = $share;
		($vars, $template_file) =
			$self->show_first_form($vars);
	}
	else {
		# All OK. Show the second form.
		$vars->{'state'} = $state;
		($vars, $template_file) = 
			$self->show_second_form($vars, $share->{'connector'});
	}
	
	return ($vars, $template_file);
}

## Handles what to do when the user submits the second form.
## The "add" wizard is complete, so add all info to the database.
sub submit_second_form($$$) {
	my ($self, $vars, $share) = (@_);
	
	my $state = $self->{'state'};
	my $dbh = $self->{'dbh'};
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $collection = $self->{'collection'};
	
	
	# Check for any errors
	my ($valid, $msg) = $collection->validate(
		$share,	qw(collection_name share connector));
				
	my $template_file;
	
	unless($valid) {
		# Input from the form was not valid. Show it again.
		$vars->{$msg} = 1; # Add the error to vars
		$vars->{'share'} = $share;
		$vars->{'state'} = $state;
		($vars, $template_file) = 
			$self->show_second_form($vars, $share->{'connector'});
	}
	else {
		# Input was all OK.
		# Add share to database.
		$self->add_share($share);
		
		$vars->{'success'} = 1;

		if (defined($state->{'from_scan'})) {
			# User was adding a collection from scan results.
			# Let's allow him to return to it.
			$vars->{'result_id'} = $state->{'from_scan'};
		}

		($vars, $template_file) = $self->show_first_form($vars);

	}
	
	return ($vars, $template_file);
}

sub add_from_scan($$$) {
	my $self = shift;
	my ($vars, $state) = @_;
	my ($host, $path) = $self->_get_host_path($state->{'add_collection'});

	unless ($host and $path) {
		$vars->{'error_scan_missing_host_path'} = 1;
		carp "Ooops! missing host path from scan";
		return;
	}

	my $connector = $state->{'connector'};
			
	my $share = {
		'connector' => $connector,
		'host' => $host,
		'path' => $path };

	# Smb exception
	my $sqlConnectors = $self->{'sqlConnectors'};
	if ($sqlConnectors->get_name($connector) eq 'SMB') {
		$share->{'resource'} = "\\\\$host\\$path";
	}

	$vars->{'share'} = $share;
	$vars->{'from_scan'} = $state->{'result_id'};
	return $vars;
}

## Helper function for add_from_scan
sub _get_host_path($$) {
	my $self = shift;
	my $data = shift;
	map { return ($_->{'host'}, $_->{'path'}) if $_->{'submit'} } @$data;
}
1;
