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
use Page::Abstract;
BEGIN {
	#push @INC, 'Modules';
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::Infoquery;
use Common::Collection;
use Common::Generic;
use Common::Data::Overview;

our @ISA = qw(Page::Abstract);

use config qw($CONFIG);
use constant TPL_DEFAULT     => 'overview.html';
use constant TPL_EDIT        => 'overview_edit.html';
use constant TPL_DELETE_COLL => 'overview_delete_share.html';

my $sqlShares;
my $sqlConnectors;
my $sqlAuth;
my $sqlGroups;
my $sqlUsers;

sub _init {
	my $self = shift;
        my $dbh = $self->{dbh};
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
sub crawl_collection {
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
    my ($s, $vars, $coll_id) = (@_);
    croak "not a valid 'coll_id'"
        unless $coll_id and $coll_id =~ /^\d+$/;

    unless ($sqlShares->id_exists($coll_id)) {
        $vars->{'error_collection_not_exist'} = 1;
        return TPL_EDIT;
    }
    
    my @input_fields = @{$sqlConnectors->get_input_fields(
            $sqlShares->get_connector_name($coll_id))};

    my $c_coll = Common::Collection->new($s->{dbh});
    my %form_data = $c_coll->coll_form_data(@input_fields);
    my %coll_data = $c_coll->coll_data($coll_id, @input_fields);

    while (my ($k, $v) = each %form_data) {
        $vars->{$k} = $v;
    }
    $vars->{share} = \%coll_data;

    return TPL_EDIT;
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
	$sqlShares->set_active($id);
	$vars->{'success_activate'} = 1;
	return $vars;
}


## 
# Forcing a full recrawl of a collection.
sub recrawl_collection {
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

 	return TPL_DELETE_COLL;
}

sub _get_form_data { croak "Use Common::Collection instead" }


sub _get_coll_data {
croak "use Common::Collection instead" }


sub _get_connectors {
	croak "_get_connectors() is deprecated. Use method in class Common::Data::Overview";
}



1;
