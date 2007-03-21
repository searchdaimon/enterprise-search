package Page::Overview;
use strict;
use warnings;
use Carp;
use Data::Dumper;
use Time::Local;
use Sql::Shares;
use Sql::Connectors;
use Sql::CollectionAuth;
use Sql::ShareGroups;
use Sql::Config;
BEGIN {
    push @INC, 'Modules';
}
use Boitho::Infoquery;
use Common::Collection;
use Common::Generic;

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
	$self->{'infoQuery'}	 = Boitho::Infoquery->new;
	$self->{'common'}     	 = Common::Generic->new;
	
	my $sqlConfig = Sql::Config->new($dbh);
	
	$self->{'default_crawl_rate'} 
		= $sqlConfig->get_setting('default_crawl_rate');
	$self->{'sqlConfig'}	 = $sqlConfig;
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
	$vars->{'connectors'} = $self->_get_connectors;
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
	my ($self, $vars) = (@_);

	my $sqlShares = $self->{'sqlShares'};
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $sqlGroups = $self->{'sqlGroups'};
	
	my $state = $self->{'state'};
	my $share = $state->{'share'};	
	my $dbh = $self->{'dbh'};
	
	my $connector = $sqlShares->get_connector_name($share->{'id'});
	
	# Check for errors
	my ($valid, $msg) = Collection::validate($dbh, $share, 
		qw(collection_name share));
	
	unless ($valid) {
		
		#$template_file = 'resources_edit.html';
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
	
	$vars->{'authentication'} = $sqlAuth->get_authentication
		if grep /^authentication$/, @$fields;
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

## Get all connectors and their shares.
sub _get_connectors {
	my $self = shift;
	my $sqlConnectors = $self->{'sqlConnectors'};
	
	my $connectors = $sqlConnectors->get_with_shares;
	
	for my $c (0..$#{@{$connectors}}) {
		my $shares = $connectors->[$c]{'shares'};
		next unless($shares);
		
		for my $s (0..$#{@{$shares}}) {
			# Add some extra info for each share.
			
			# Not showing info for a disabled share,
			# no need to get it.
			next unless $shares->[$s]{'active'};
				
			$shares->[$s] 
				= $self->_add_smarter_rate($shares->[$s]);
			$shares->[$s] 
				= $self->_add_next_crawl($shares->[$s]);
			$shares->[$s] 
				= $self->_add_documents_in_collection($shares->[$s]);
		}

		$connectors->[$c]{'shares'} = $shares;
	
	}
	
	return $connectors;
}


## Add "smart rate" to a share. This adds a variable with 
## recrawl rate in text. Example: "Crawled every 12 hours."
sub _add_smarter_rate($$) {
	my ($self, $share)	= @_;
	my $default_rate	= $self->{'default_crawl_rate'};
	my $rate 		= $share->{'rate'};

	$rate = $default_rate unless($rate);

	my $smart_rate  	= $self->_minutes_to_text($rate);
	$share->{'smart_rate'}  = $smart_rate;

	return $share;
}

## Add variable with next crawl in text.
## Example: "Should have been crawled 10 days ago."
sub _add_next_crawl {
	my ($self, $share) = @_;
	my $default_rate   = $self->{'default_crawl_rate'};

	my $last = $share->{'last'};
	my $rate = $share->{'rate'};
	$rate = $default_rate unless $rate;

	return $share unless $last; # need to know last to find next
	
	my $next_crawl		= $self->_get_next_crawl($rate, $last);
	$share->{'next_crawl'}	= $next_crawl;

	return $share;
}

## Add a variable with document count for a share.
sub _add_documents_in_collection {
	my ($self, $share)  = @_;
	my $infoquery 	    = $self->{'infoQuery'};
	my $collection_name = $share->{'collection_name'};

	my $doc_count 
		= $infoquery->documentsInCollection($collection_name);
	$share->{'doc_count'} = $doc_count;

	return $share;
}

## Given crawl rate, and time of last crawl, this function returns
## a string saying when next crawl happens (or should have happened).
sub _get_next_crawl {
	my $self = shift;
	my ($rate, $last) = (@_);
	unless($last and $rate) {
		carp "Missing eather \$last or \$rate.";
		return;
	}
	
	#make $last into unixtime, if it isn't.
	unless ($last =~ /^\d+$/) {
		$last = $self->_mysql_to_unixtime($last);
	}
	
	my $time_ago = time - $last;
	my $time_left = ($rate * 60) - $time_ago; # * 60 to get seconds.
	
	my $to_text = $self->_minutes_to_text(abs($time_left / 60));
	if ($time_left < 0) { return "Should have been crawled $to_text ago"; }
	return "Next crawl in $to_text";

}

## Change to mysql time string to unixstamp
sub _mysql_to_unixtime {
	my $self = shift;
	my $mysql_time = shift;
	return unless $mysql_time;
	#/(\d\d\d\d)-?(\d\d)-?(\d\d) ?(\d\d):?(\d\d):?(\d\d)/;
	my ($year, $month, $day, $hour, $minute, $second) 
		= $mysql_time 
		=~ /(\d\d\d\d)-(\d\d)-(\d\d) (\d\d):(\d\d):(\d\d)/;
	my $unixtime = eval { timelocal($second, $minute, $hour, $day, 
			 ($month - 1), ($year - 1900)) };
	carp $@ if $@;
	
	$unixtime;
}

## Method to change seconds into text.
## Example outputs: "1 day", "5 minutes", "12 hours"
sub _minutes_to_text {
	my $self = shift;
	my $minutes = shift;
	return unless $minutes;
	if ($minutes < 60) { # less than an hour
		$minutes = int $minutes;
		return "$minutes minute" if ($minutes == 1);
		return "$minutes minutes";
	}
	elsif ($minutes < 1440) { #less than an day
		my $hours = int($minutes / 60);
		return "$hours hour" if ($hours == 1);
		return "$hours hours";
	}
	
	my $days = int($minutes / 60 / 24);
	return "$days day" if ($days == 1);
	return "$days days";
}


1;
