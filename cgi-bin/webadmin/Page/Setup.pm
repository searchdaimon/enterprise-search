package Page::Setup;
use strict;
use warnings;
use Carp;
use CGI;
use File::stat;
use Data::Dumper;
use Sql::Connectors;
use Sql::Shares;
use Sql::Config;
BEGIN {
	#push @INC, "Modules";
	push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::Infoquery;
use config qw(%CONFIG);
use Page::Abstract;
our @ISA = qw(Page::Abstract);

use constant DEBUG => 0;

#TODO: Move all the methods in more suited classes, and deprecate this class.
sub _init {
	my $self = shift;
        my $dbh = $self->{dbh};
	$self->{'sqlConnectors'} = Sql::Connectors->new($dbh);
	$self->{'sqlShares'} 	 = Sql::Shares->new($dbh);
	$self->{'sqlConfig'}	 = Sql::Config->new($dbh);
	$self->{'infoQuery'}	 = Boitho::Infoquery->new($CONFIG{infoquery});
}

sub show_start_scan($$) {
	my ($self, $vars) = @_;
	return ($vars, "setup_start.html");
}

sub show_scanning($$$) {
	my ($self, $vars, $status) = @_;
	my $template_file;
	$template_file = "setup_scan_scanning_top.html"
		if ($status eq 'begin');
	$template_file = "setup_scan_scanning_bottom.html"
		if ($status eq 'end');
		
	carp "unknown status: $status" unless $template_file;
	
	return ($vars, $template_file);
}

## Shows list of found shares. User selects which ones 
## to add as collections.
sub show_process_form($$) {
	my ($self, $vars, $results_id_ptr) = @_;
	my $template_file = "setup_scan_process_results.html";
	
	$vars->{'results'} 
		= $self->_get_all_scan_results(@$results_id_ptr);
	return ($vars, $template_file);
}

## Last step in the wizard.
## Shows collections added.
sub show_complete_step($$) {
	my ($self, $vars) = @_;
	my $template_file = "setup_complete.html";
	return ($vars, $template_file);
}

# Runs scan for all available connectors.
# Prints scanning process in HTML format.
sub start_auto_scan($$) {
	my ($self, $vars) = @_;
	my $scan = $self->{'scan'};
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $connector_list_ptr = $sqlConnectors->get_all_names();
	my @result_ids = ();
	
	# Run autoscan for all types of connectors. 
	foreach my $connector (@$connector_list_ptr) {
		if ($scan->is_supported_connector($connector)) {
			$self->_html_connector("begin", $connector);
			
			my %scan_parameters;
			
			if (DEBUG) {
				%scan_parameters = (
					'range'     => "213.179.58.120",
					'username'  => 'Boitho',
					'password'  => '1234Asd',
					'connector' => $connector,
				);
			}
			else {
				%scan_parameters = (
					'range'		=> undef,
					'username'	=> undef,
					'password'	=> undef,
					'connector'	=> $connector,
				);
			}
			push @result_ids, $scan->scan_start(\%scan_parameters);
			$self->_html_connector("end", $connector);
		}
	}
		
	$vars->{'result_ids'} = \@result_ids;

	return $vars;
}

## Fetch and return all results from array of result-ID's
sub _get_all_scan_results($@) {
	
	## local function to add connector to all results.
	sub __add_connectors($$) {
		my ($connector, $results_ptr) = @_;
		for my $i (0..@$results_ptr) {
			next unless $results_ptr->[$i];
			$results_ptr->[$i]{'connector'} = $connector;	
		}
	}

	my ($self, @result_ids) = @_;
	my $sqlConnectors = $self->{'sqlConnectors'};
	my $scan = $self->{'scan'};
	
	my @all_results;
	foreach my $id (@result_ids) {
		my ($connector, $results_ptr) = $scan->process($id);
		$connector = $sqlConnectors->get_name($connector);
		&__add_connectors($connector, $results_ptr);
		@all_results = (@all_results, @$results_ptr);
		
	}
	return \@all_results;
}

# Get's selected shares from the process results form.
# Adds shares as collections to the database.
sub add_collections($$$) {
	my ($self, $vars, $shares_ptr, $checked_ptr) = @_;
	my $sqlShares = $self->{'sqlShares'};
	my $sqlConnectors = $self->{'sqlConnectors'};
	
	my @collections_added;
	
	foreach my $share_ptr (@$shares_ptr) {
	
		my %share = %$share_ptr;
		
		my $id = $share{'id'};
		next unless  #Skip shares not selected
			grep /^$id$/, @$checked_ptr;
		
		my $connector_id 
			= $sqlConnectors->get_id($share{'connector'});

		my $resource
			= $self->_generate_resource($share{'connector'}, \%share);
	
		my $success = 1;
		my $errmsg;
		eval {
			$success = $sqlShares->insert_share(
			{
				'connector'       => $connector_id,
				'resource'        => $resource,
				'active'          => 1, #Crawl it
				'smb_name'        => $share{'smb_name'},
				'smb_workgroup'   => $share{'smb_workgroup'},
				'domain'          => $share{'domain'},
				'collection_name' => $share{'collection_name'},
				'auth_id'         => $share{'auth_id'},
			});
		};
		if ($@) {
			$errmsg = $@;
			$success = 0;
		}
		
		my $added = {
			'name'		=> $share{'collection_name'},
			'success'	=> $success,
			'errmsg'   => $errmsg,
		};
		
		push @collections_added, $added;
	}

	$vars->{'collections_added'} = \@collections_added;
	return $vars;
}


## Helper function for add_collection
## Generates correct resource field for type of connector, unless resource is already set.
## Defaults to IP.
sub _generate_resource($$) {
	my ($self, $connector, $share_ptr) = @_;
	my %share = %$share_ptr;
	
	return $share{'resource'}
			if $share{'resource'};
	
	return "//" . $share{'addr'} . "/" . $share{'smb_name'}
		if ($connector eq "SMB");
	
	return $share{'addr'};
}


## HTML is added to the logic code, to avoid javascript usage.
## This might or might not be a good thing.
sub _html_connector($$$) {
	my ($self, $state, $connector) = @_;
	if ($state eq 'begin') {
		print "
		<li>
			<label title=\"Show details\" 
				onClick=\"show('${connector}_details')\"
				for=\"${connector}_span\">
				<span id=\"${connector}_span\">Scanning for $connector</span>
			</label>
			<div class=\"scan_results\" style=\"display : none;\" id=\"${connector}_details\">
		";
	}
	elsif ($state eq 'end') {
		print "</div> ...<img alt=\"\" src=\"file.cgi?i=agt_action_success&ampsize=22x22\" /></li>";
	}
	1;
}






1;
