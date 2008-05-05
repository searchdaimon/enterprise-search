package Page::Overview;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Sql::Shares;
use Sql::Connectors;
use Sql::CollectionAuth;
use Sql::ShareGroups;
use Sql::ShareUsers;
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
use constant TPL_DEFAULT => 'overview.html';

my $sqlShares;
my $sqlConnectors;
my $sqlAuth;
my $sqlGroups;
my $sqlUsers;

sub new {
    my ($class, $dbh, $state) = @_;
	my $self = {};
	bless $self, $class;
	$self->_init($dbh, $state);
	return $self;
}

sub _init($$$) {
	my ($self, $dbh, $state) = (@_);
	$self->{'dbh'}		 = $dbh;
	$self->{'state'}	 = $state;
	$sqlShares     = Sql::Shares->new($dbh);
	$sqlConnectors = Sql::Connectors->new($dbh);
	$sqlAuth = Sql::CollectionAuth->new($dbh);
	$sqlGroups = Sql::ShareGroups->new($dbh);
        $sqlUsers = Sql::ShareUsers->new($dbh);
	$self->{'common'}     	 = Common::Generic->new;
	$self->{'dataOverview'} = Common::Data::Overview->new($dbh);
	$self->{'infoQuery'}   = Boitho::Infoquery->new($CONFIG->{'infoquery'});
}

## Sends a crawl collection request to infoquery.
sub crawl_collection($$) {
	my ($self, $vars, $id) = (@_);
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

sub list_collections {
	my ($s, $vars) = @_;
	$vars->{connectors} 
            = [ $s->{dataOverview}->get_connectors_with_collections() ];

        TPL_DEFAULT;
}



## 
# Method for displaying a form to edit a collection.
sub edit_collection {
    my ($self, $vars, $collection) = (@_);
    my $template_file = "overview_edit.html";
    my $state = $self->{'state'};

    my ($id, @input_fields)
        = $self->_get_collection_data($vars, $collection);

    # User may not edit coll.name
    @input_fields = grep { $_ ne 'collection' } @input_fields;
    $vars->{input_fields} = \@input_fields;

    if (defined $state->{'share'}) {
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
                if grep /^groups$/, @input_fields;
            $share->{'user'} = [ $sqlUsers->get_users($id) ]
                if grep /user/, @input_fields;

            $vars->{'share'} = $share;
        }
    }

    return ($vars, $template_file);
}

sub manage_collection {

	my ($self, $vars, $id) = (@_);
	my $collection_name = $sqlShares->get_collection_name($id);

	$vars->{'id'} = $id;
	$vars->{'share'} = {
		'collection_name' => $collection_name,
	};
	return ($vars, 'overview_manage.html');


}

sub submit_edit {
	my ($self, $vars, $share) = @_;

        croak "argument share not provided"
            unless defined $share;
	
	my $dbh = $self->{'dbh'};
	
	my $connector = $sqlShares->get_connector_name($share->{'id'});
	
	# Check for errors
	my $c_collection = Common::Collection->new($dbh);
	my ($valid, $msg) = $c_collection->validate($share, qw(share));

	unless ($valid) {
		$vars->{'share'} = $share;
		$vars->{'input_fields'} = $sqlConnectors->get_input_fields($connector);
		$vars->{$msg} = 1;
		return ($vars, $valid);
	}
	
	# Continue with the submit.
	$sqlShares->update_share($share);
	$sqlGroups->set_groups($share->{id}, $share->{group_member});
        my @users = grep { defined $_ } @{$share->{user}};
        $sqlUsers->set_users($share->{id}, \@users);

	$vars->{'edit_success'} = 1;
	return ($vars, $valid);
	
}

sub activate_collection($$$) {
	my ($self, $vars, $id) = (@_);
	$vars->{'success_activate'} = 1;
	$sqlShares->set_active($id);
	return $vars;
}


## 
# Forcing a full recrawl of a collection.
# 
sub recrawl_collection($$$) {
	my ($self, $vars, $submit_values) = (@_);
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

##
# User has confirmed a delete.
sub delete_collection_confirmed {
	my ($self, $vars, $id) = (@_);
	croak ("The operation must be a POST request to work.") 
		unless($ENV{'REQUEST_METHOD'} eq 'POST');
	my $infoquery = $self->{'infoQuery'};

	my $collection_name = $sqlShares->get_collection_name($id);

        my $success = 1;
	## Infoquery kommentert ut. Set tilbake når bug er fikset
	
	#my $success = $infoquery->deleteCollection($collection_name);
	
	#$vars->{'delete_error'} = $infoquery->error
	#	unless $success;
	
	$vars->{'delete_request'} = $success;
	$sqlShares->delete_share($id)
		if $success;
	
	return $vars;
}

## User wants to delete a collection. Show confirm dialog.
sub delete_collection {
	my ($self, $vars, $id) = (@_);
	$vars->{'collection_name'} = $sqlShares->get_collection_name($id);
	$vars->{'id'} = $id;

 	my $template_file = "overview_delete_share.html";
 	return ($vars, $template_file);
}



## Helper function for edit_collection
## Gets data regarding the collection.
sub _get_collection_data {
	my ($self, $vars, $collection) = (@_);

	my $id = $self->_find_collection_id($vars, $collection);
	return unless $id; #coll does not exist.

	# Figure out input fields for connector
	my $input_fields_ref = $sqlConnectors->get_input_fields(
					$sqlShares->get_connector_name($id));
        # Grab other options
	$self->_get_associated_data($vars, $input_fields_ref);
	
	return ($id, @{$input_fields_ref});
}


sub _get_associated_data($$$) {
	my ($self, $vars, $fields) = (@_);
	
	my $infoquery = $self->{'infoQuery'};
	
	if (grep { /^authentication$/ } @$fields) {
		my @authdata = $sqlAuth->get_all_auth();
		$vars->{'authentication'} = \@authdata;
	}

	$vars->{'connectors'} = $sqlConnectors->get_connectors()
		if grep /^connector$/, @$fields;
	$vars->{'group_list'} = $infoquery->listGroups
		if grep /^groups$/, @$fields;
	$vars->{'user_list'} = $infoquery->listUsers()
		if grep /^exchange_user_select$/, @$fields;

	return $vars;
}

## Helper function for edit_collection
## to find collection id.
sub _find_collection_id($$) {
	my ($self, $vars, $collection) = (@_);
	my $id;
	if ($collection =~ /^\d+$/) { 
		$id = $collection 
	}
	else { 	
		$id = $sqlShares->get_id_by_collection($collection);  
	}
	
	unless ($id) {
		$vars->{'error_collection_not_exist'} = 1;
		return;
	}
	
	return $id;
}

sub _get_connectors {
	croak "_get_connectors() is deprecated. Use method in class Common::Data::Overview";
}



1;
