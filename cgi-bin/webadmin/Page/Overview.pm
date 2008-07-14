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
use Data::Collection;
use Page::Abstract;
BEGIN {
    push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::Infoquery;
use Common::Collection;
use Common::Data::Overview;
use Params::Validate;

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

    my $dataColl = Data::Collection->new($s->{dbh}, { id => $coll_id });
    my %form_data = $dataColl->form_data();
    my %coll_data = $dataColl->coll_data();

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
    validate_pos(@_, 1, 1, 1);
    my ($s, $vars, $share) = @_;

    my @users = grep { defined $_ } @{$share->{user}};
    my %attr;
    for my $key (%COLLECTION_ATTR) {
        if (defined $share->{$key}) {
            $attr{$key} = $share->{$key};
        }
    }
    $attr{users} = \@users;
    my $dataColl = Data::Collection->new($s->{dbh}, \%attr);

    if (not $attr{auth_id}) {
        $dataColl->set_auth($share->{username}, $share->{password});
    }

    	
    # Check for errors
    unless ($attr{resource} || $attr{host}) {
	$vars->{error_missing_share} = 1;
	$vars->{share} = $share;
	$vars->{input_fields} = {$dataColl->get_attr}->{input_fields};
	return;
    }
	
    # Continue with the submit.
    $dataColl->update();

    $vars->{edit_success} = 1;
    return 1;
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
	my ($s, $vars, $id) = @_;
	my $collection_name = 
		$sqlShares->get_collection_name($id);

	$vars->{recrawl_request} =
		$s->{infoQuery}->recrawlCollection($collection_name);
	$vars->{recrawl_error} = $s->{infoQuery}->error
		unless $vars->{recrawl_request};
	$vars->{id} = $id;

	return $s->manage_collection($vars, $id);
}

##
# User has confirmed a delete.
sub delete_collection_confirmed {
	my ($self, $vars, $id) = (@_);
	croak ("The operation must be a POST request to work.") 
		unless($ENV{'REQUEST_METHOD'} eq 'POST');
	my $infoquery = $self->{'infoQuery'};

	my $collection_name = $sqlShares->get_collection_name($id);

	my ($success) = $infoquery->deleteCollection($collection_name);
	
	$vars->{'delete_error'} = $infoquery->error
		unless $success;
	
	$vars->{'delete_request'} = $success;
        if ($success) {
            my $coll = Data::Collection->new($self->{dbh}, { id => $id });
            $coll->delete();
        }
	
	return $vars;
}

## User wants to delete a collection. Show confirm dialog.
sub delete_collection {
	my ($self, $vars, $id) = (@_);
	$vars->{'collection_name'} = $sqlShares->get_collection_name($id);
	$vars->{'id'} = $id;

 	return TPL_DELETE_COLL;
}


1;
