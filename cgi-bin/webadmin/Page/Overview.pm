package Page::Overview;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Sql::Shares;
use Sql::Connectors;
use Sql::CollectionAuth;
use Sql::ShareGroups;
use Sql::Config;
BEGIN {
	#push @INC, 'Modules';
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::Infoquery;
use Common::Collection;
use Common::Generic;
use Common::Data::Overview;

use config qw($CONFIG);

sub new($$$) {
	my $class = shift;
	my $dbh = shift;
	my $state = shift;
	my $self = {};
	bless $self, $class;
	$self->_init($dbh, $state);
	return $self;
}

sub _init($$$) {
	my ($self, $dbh, $state) = (@_);
	$self->{'dbh'}		 = $dbh;
	$self->{'state'}	 = $state;
	$self->{'sqlShares'}	 = Sql::Shares->new($dbh);
	$self->{'sqlConnectors'} = Sql::Connectors->new($dbh);
	$self->{'sqlAuth'}	 = Sql::CollectionAuth->new($dbh);
	$self->{'sqlGroups'}	 = Sql::ShareGroups->new($dbh);
	$self->{'common'}     	 = Common::Generic->new;
	$self->{'dataOverview'} = Common::Data::Overview->new($dbh);
	$self->{'infoQuery'}   = Boitho::Infoquery->new($CONFIG->{'infoquery'});
	#my $sqlConfig = Sql::Config->new($dbh);
	
	#$self->{'default_crawl_rate'} 
	#	= $sqlConfig->get_setting('default_crawl_rate');
	#$self->{'sqlConfig'}	 = $sqlConfig;
}

## Sends a crawl collection request to infoquery.
sub crawl_collection($$) {
	my ($self, $vars, $id) = (@_);
	my $sqlShares = $self->{'sqlShares'};
	my $iq = $self->{'infoQuery'};

	my $collection = $sqlShares->get_collection_name($id);
	
	# Check if collection exists
	unless($collection) {
		$vars->{'crawl_error_not_exist'} = 1;
		$vars->{'crawl_request'} = 0;
		return $vars;
	}
	
	# Submit crawl request.
	my $success = $iq->crawlCollection($collection);
	$vars->{'crawl_request'} = $success;
	$vars->{'crawl_error'} = $iq->get_error
		unless($success);
	return $vars;
}

sub list_collections($$) {
	my ($self, $vars) = (@_);
	my $dataOverview = $self->{'dataOverview'};
	my @connectors = $dataOverview->get_connectors_with_collections();	
	$vars->{'connectors'} = \@connectors;
	return ($vars, 'overview.html');
}



## Method for displaying a form to edit a collection.
sub edit_collection {
	my ($self, $vars, $collection) = (@_);
	my $template_file = "overview_edit.html";
	my $sqlShares = $self->{'sqlShares'};
	my $sqlGroups = $self->{'sqlGroups'};
	
	
	my $state = $self->{'state'};
	
	my ($input_fields, $id);
	($vars, $input_fields, $id) =
		$self->_get_collection_data($vars, $collection);
	
	if (defined($state->{'share'})) {
		# User tried to submit invalid values, showing form again.
		my $share = $state->{'share'};
		$share->{'connector_name'} = 
			$sqlShares->get_connector_name($share->{'id'});
		$vars->{'share'} = $share;
	}
	else {
		# First time editing a collection.
		unless ($sqlShares->id_exists($id)) {
			# Show error
			$vars->{'error_collection_not_exist'} = 1;
		}
 		else {
			# Collection exists, continue.
 			my $share =  $sqlShares->get_share($id);
 			$share->{'group_member'} = $sqlGroups->get_groups($id)
 				if grep /^groups$/, @$input_fields;
 			$vars->{'share'} = $share;
 		}
	}

	
	return ($vars, $template_file);
}

sub manage_collection {

	my ($self, $vars, $id) = (@_);
	my $sqlShares = $self->{'sqlShares'};
	my $collection_name = $sqlShares->get_collection_name($id);

	$vars->{'id'} = $id;
	$vars->{'share'} = {
		'collection_name' => $collection_name,
	};
	return ($vars, 'overview_manage.html');


}

sub submit_edit {
	my ($self, $vars, $share) = @_;

	my $sqlShares     = $self->{'sqlShares'};
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $sqlGroups     = $self->{'sqlGroups'};

	

	unless (defined $share) {
		croak "argument share not provided";
	}
	
	my $dbh = $self->{'dbh'};
	
	my $connector = $sqlShares->get_connector_name($share->{'id'});
	
	# Check for errors
	my $c_collection = Common::Collection->new($dbh);
	my ($valid, $msg) = $c_collection->validate($share, 
		qw(collection_name share));


	unless ($valid) {
		$vars->{'share'} = $share;
		$vars->{'input_fields'} = $sqlConnectors->get_input_fields($connector);
		$vars->{$msg} = 1;
		return ($vars, $valid);
	}
	
	# Contine with the submit.
	$sqlShares->update_share($share);
	$sqlGroups->set_groups($share->{'id'}, $share->{'group_member'});

	$vars->{'edit_success'} = 1;
	return ($vars, $valid);
	
}

sub activate_collection($$$) {
	my ($self, $vars, $id) = (@_);
	my $sqlShares = $self->{'sqlShares'};
	$vars->{'success_activate'} = 1;
	$sqlShares->set_active($id);
	return $vars;
}


## User is forcing a full recrawl of a collection.
## Do it, and return him to the collection management form
sub recrawl_collection($$$) {
	my ($self, $vars, $submit_values) = (@_);
	my $sqlShares = $self->{'sqlShares'};
	my $iq        = $self->{'infoQuery'};
	my $common    = $self->{'common'};
	my $id = $common->request([$submit_values]);
	my $collection_name = 
		$sqlShares->get_collection_name($id);

	$vars->{'recrawl_request'} =
		$iq->recrawlCollection($collection_name);
	$vars->{'recrawl_error'} = $iq->error
		unless $vars->{'recrawl_request'};
	$vars->{'id'} = $id;

	return $self->manage_collection($vars, $id);
}

## User has confirmed a delete. Delete share, and show a success message.
sub delete_collection_confirmed {
	my ($self, $vars, $id) = (@_);
	croak ("The operation must be a POST request to work.") 
		unless($ENV{'REQUEST_METHOD'} eq 'POST');
	my $sqlShares = $self->{'sqlShares'};
	my $infoquery = $self->{'infoQuery'};

	my $collection_name = $sqlShares->get_collection_name($id);
	
	## Infoquery kommentert ut. Set tilbake når bug er fikset
	
	#my $success = $infoquery->deleteCollection($collection_name);
	
	#$vars->{'delete_request'} = $success;
	#$vars->{'delete_error'} = $infoquery->error
	#	unless $success;
	
	$sqlShares->delete_share($id)
		if 1; #$success;
	
	return $vars;
}

## User wants to delete a collection. Show confirm dialog.
sub delete_collection {
	my ($self, $vars, $id) = (@_);
	my $sqlShares = $self->{'sqlShares'};
	$vars->{'collection_name'} = $sqlShares->get_collection_name($id);
	$vars->{'id'} = $id;

 	my $template_file = "overview_delete_share.html";
 	return ($vars, $template_file);
}



## Helper function for edit_collection
## Gets data regarding the collection.
sub _get_collection_data($$$) {
	my ($self, $vars, $collection) = (@_);
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $sqlShares = $self->{'sqlShares'};

	# Figure out collection id
	my $id;
	{;
		my $exists;
		($vars, $exists, $id) = $self->_find_collection_id($vars, $collection);
		return $vars unless $exists;
	}
	
	# Figure out input fields for connector
	my $input_fields = $sqlConnectors->get_input_fields(
					$sqlShares->get_connector_name($id));
	$input_fields = $self->_add_collection_field($input_fields);
	$vars->{'input_fields'} = $input_fields;
	
	# Grab other options
	$vars = $self->_get_associated_data($vars, $input_fields);
	
	return ($vars, $input_fields, $id);
}


## Helper function for edit_collection
## This is data that is not directly used by the collection
## but needs to be listed up as options for the user.
sub _get_associated_data($$$) {
	my ($self, $vars, $fields) = (@_);
	
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $sqlAuth = $self->{'sqlAuth'};
	my $infoquery = $self->{'infoQuery'};
	
	if (grep { /^authentication$/ } @$fields) {
		my @authdata = $sqlAuth->get_all_auth();
		$vars->{'authentication'} = \@authdata;
	}

	$vars->{'connectors'} = $sqlConnectors->get_connectors()
		if grep /^connector$/, @$fields;
	$vars->{'group_list'} = $infoquery->listGroups
		if grep /^groups$/, @$fields;
	return $vars;
}

## Helper function for edit_collection
## to find collection id.
sub _find_collection_id($$) {
	my ($self, $vars, $collection) = (@_);
	my $sqlShares = $self->{'sqlShares'};

	my $id;
	if ($collection =~ /\d+/) { 
		$id = $collection 
	}
	else { 	
		$id = $sqlShares->get_id_by_collection($collection);  
	}
	
	unless ($id) {
		$vars->{'error_collection_not_exist'} = 1;
		return ($vars, 0, undef);
	}
	
	return ($vars, 1, $id);
}

## Helper function for show_edit_form
## to add the collection field unless it's added.
sub _add_collection_field($$) {
	my ($self, $fields) = (@_);
	my $added = grep /^collection$/, @$fields;
	unshift(@$fields, 'collection')
		unless($added);
	return $fields;
}

sub _get_connectors {
	croak "_get_connectors() is deprecated. Use method in class Common::Data::Overview";
}



1;
