package Page::Overview;
use strict;
use warnings;

use Carp;
use Data::Dumper;
use URI::Escape;

use Sql::Config;
use Sql::Shares;
use Sql::Connectors;
use Sql::CollectionAuth;
use Sql::ShareGroups;
use Sql::ShareUsers;
use Sql::Config;
use Sql::ShareResults;
use Sql::SessionData;
use Sql::System;
use Data::Collection;
use Page::Abstract;
BEGIN {
    push @INC, $ENV{'BOITHOHOME'} . '/Modules';
}
use Boitho::Infoquery;
use Common::Collection;
use Params::Validate;
use Time::Local qw(timelocal);

our @ISA = qw(Page::Abstract);

use config qw(%CONFIG);
use constant TPL_DEFAULT     		=> 'overview.html';
use constant TPL_EDIT        		=> 'overview_edit.html';
use constant TPL_DELETE_COLL 		=> 'overview_delete_share.html';
use constant TPL_CUSTOMIZE   		=> 'overview_customize.html';
use constant TPL_ADV_MANAGEMENT 	=> 'overview_manage.html';
use constant TPL_ACCESS_LEVEL		=> 'overview_accesslevel.html';
use constant TPL_GRAPHS			=> 'overview_graphs.html';
use constant TPL_CONSOLE		=> 'overview_console.html';
use constant TPL_DOCUMENTS		=> 'overview_documents.html';
use constant TPL_RREAD			=> 'overview_rread.html';
use constant TPL_PAGEINFO		=> 'overview_pageinfo.html';
use constant TPL_HTMLDUMP		=> 'overview_htmldump.html';

Readonly::Scalar my $META_CONN_PUSH => "Pushed collections";
Readonly::Scalar my $CRAWL_STATUS_OK => qr{^(OK\.|Ok)$};

Readonly::Hash my %VALID_CUST => map { $_ => 1 } qw(
	rank_url_main_word filter_same_crc32 filter_same_domain rank_title_first_word
	rank_body_array filter_same_url rank_author_array rank_headline_array summary
	rank_title_array filter_response filter_TLDs rank_url_array cache_link);

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
	$self->{'infoQuery'}   = Boitho::Infoquery->new($CONFIG{'infoquery'});
	$self->{sqlConfig} = Sql::Config->new($dbh);
}

##
# Sends a crawl collection request to infoquery.
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
	my $success = $iq->crawlCollection($collection, logfile => $CONFIG{'coll_log_path'} . $collection);
	$vars->{'crawl_request'} = $success ? 1 : 0;
	$vars->{'crawl_error'} = $iq->get_error
		unless($success);
	return $vars;
}

sub show { shift->list_collections(@_) }
sub list_collections {
	my ($s, $vars, $from_setup) = @_;
	$vars->{connectors} 
            = [ $s->gen_collection_list() ];

	$s->_post_setup_check($vars)
		if $from_setup;

        TPL_DEFAULT;
}

sub _post_setup_check {
	my ($s, $vars) = @_;

	# warn if integration failed.
	my $sys = Sql::System->new($s->{dbh});
	return if $s->{infoQuery}->countUsers($sys->primary_id) != -1;
	
	$vars->{error_usersys}{msg} = $s->{infoQuery}->error;
	$vars->{error_usersys}{sys_id} = $sys->primary_id;
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

	my $dataColl = Data::Collection->new($self->{dbh}, { id => $id });
	my %form_data = $dataColl->form_data();
	my %coll_data = $dataColl->coll_data();

	$vars->{'id'} = $id;
	$vars->{'share'} = \%coll_data;

	return TPL_ADV_MANAGEMENT;
}

sub show_access_level {
	my ($self, $vars, $id) = (@_);

	my $dataColl = Data::Collection->new($self->{dbh}, { id => $id });
	my %form_data = $dataColl->form_data();
	my %coll_data = $dataColl->coll_data();
	my $iq = Boitho::Infoquery->new($CONFIG{'infoquery'});

	$vars->{'id'} = $id;
	$vars->{'share'} = \%coll_data;

	# look up if we have a user system. If we don't we will only disply ananymus
    	my $sys = Sql::System->new($self->{dbh});
	if ($sys->have_system()) {

		$vars->{'levels'} = [
			{
				value => 'acl',
				string => 'System permissions',
				description => "Ordinary permission checking against the indexed system.",
			},
			{
				value => 'user',
				string => 'User',
				description => "Allow access for all logged in users.",
			},
			{
				value => 'group',
				string => 'Group',
				description => "Allow access to all members of a group.",
			},
			{
				value => 'anonymous',
				string => 'Anonymous ( public available )',
				description => "Anonymous search, available to search from public search page.",
			},
		];

	}
	else {

		$vars->{'levels'} = [
			{
                                value => 'anonymous',
                                string => 'Anonymous ( public available )',
                                description => "Anonymous search, available to search from public search page.",
                        },
                ];

	}

	$vars->{'groups'} = $iq->listGroups();

	foreach my $r (@{ $vars->{'levels'} }) {
		if ($r->{value} eq $coll_data{'accesslevel'}) {
			$r->{checked} = 1;
		}
	}

	return TPL_ACCESS_LEVEL;
}

sub show_customize {
	my ($s, $vars, $id, $no_db_fetch) = @_;
	my $sqlRes = Sql::ShareResults->new($s->{dbh});

	$vars->{id} = $id;
	$vars->{share}{collection_name} 
		= $sqlShares->get_collection_name($id);

	unless ($no_db_fetch) {
		# Create w/ defaults
		$sqlRes->insert({ share => $id })
			unless $sqlRes->exists({share => $id});

		my %res = %{$sqlRes->get({ share => $id })};


		map { $vars->{share}{$_} = $res{$_} } keys %res;
	}


	#croak Dumper($vars);

	return TPL_CUSTOMIZE;
}

sub show_graphs {
	my ($s, $vars, $id) = @_;

	my $sqlShares = Sql::Shares->new($s->{dbh});
	my $sess = Sql::SessionData->new($s->{dbh});
	my $sessid = $sess->insert('crawled documents' => '');
	$vars->{sessid} = $sessid;
	$vars->{id} = $id;
	my $collection_name = $sqlShares->get_collection_name($id);
	$vars->{collection_name} = $collection_name;
	
	return TPL_GRAPHS;
}

sub show_console {
        my ($s, $vars, $id) = @_;

        my $sqlShares = Sql::Shares->new($s->{dbh});
        my $sess = Sql::SessionData->new($s->{dbh});
        my $sessid = $sess->insert('crawled documents' => '');
        $vars->{sessid} = $sessid;
        $vars->{id} = $id;
        my $collection_name = $sqlShares->get_collection_name($id);
        $vars->{collection_name} = $collection_name;

        return TPL_CONSOLE;
}

sub show_documents {
        my ($s, $vars, $id) = @_;

        my $sqlShares = Sql::Shares->new($s->{dbh});
        my $sess = Sql::SessionData->new($s->{dbh});
        my $sessid = $sess->insert('crawled documents' => '');
        $vars->{sessid} = $sessid;
        $vars->{id} = $id;
        my $collection_name = $sqlShares->get_collection_name($id);
        $vars->{collection_name} = $collection_name;
	$vars->{collection_doc_count} = $s->{infoQuery}->documentsInCollection($collection_name);
	$vars->{collection_lot_count} = $s->{infoQuery}->lotsInCollection($collection_name);;

	my @lotlinks;
	for(my $count = 1; $count <= $vars->{collection_lot_count}; $count++) {
		push(@lotlinks,($count==1?"":", ") . "<a href='overview.cgi?action=rread&amp;id=$id&amp;lot=$count&amp;offset=0'>$count</a>");
	}

	$vars->{collection_lot_links} = \@lotlinks;

        return TPL_DOCUMENTS;
}

sub show_rread {
        my ($s, $vars, $id, $lot, $offset) = @_;

        my $sqlShares = Sql::Shares->new($s->{dbh});
        my $sess = Sql::SessionData->new($s->{dbh});
        my $sessid = $sess->insert('crawled documents' => '');
        $vars->{sessid} = $sessid;
        $vars->{id} = $id;
        my $collection_name = $sqlShares->get_collection_name($id);
        $vars->{collection_name} = $collection_name;
        $vars->{lot} = $lot;

	$vars->{rread} = $s->{infoQuery}->repositoryRead($lot,$collection_name,$offset);

	foreach my $i (@{ $vars->{rread}->{docs} }) {
		$i->{PageInfoUrl} = "overview.cgi?action=pageinfo&amp;id=$id&amp;offset=$i->{radress}&amp;DocID=$i->{DocId}&amp;htmlSize=$i->{htmlsize}&amp;imagesize=$i->{imagesize}";
	}

	if ($vars->{rread}->{Offset_last}) {
		$vars->{next_link} = "overview.cgi?action=rread&amp;id=$id&amp;lot=$lot&amp;offset=$vars->{rread}->{Offset_last}";
	}
        return TPL_RREAD;
}

sub show_pageinfo {
        my ($s, $vars, $id, $lot, $DocID, $offset, $htmlSize, $imagesize) = @_;

        my $sqlShares = Sql::Shares->new($s->{dbh});
        my $sess = Sql::SessionData->new($s->{dbh});
        my $sessid = $sess->insert('crawled documents' => '');
        $vars->{sessid} = $sessid;
        $vars->{id} = $id;
        my $collection_name = $sqlShares->get_collection_name($id);
        $vars->{collection_name} = $collection_name;


	$vars->{pageinfo} = $s->{infoQuery}->repositoryPageInfo($offset, $DocID, $htmlSize, $imagesize, $collection_name, 0);

	$vars->{pageinfo}->{acl_allow} =~ s/,/<br>/g;
	$vars->{pageinfo}->{acl_denied} =~ s/,/<br>/g;

	$vars->{pageinfo}->{url_unescaped} = uri_unescape( $vars->{pageinfo}->{Url} );

	$vars->{htmldump} = "overview.cgi?action=htmldump&amp;id=$id&amp;offset=$offset&amp;DocID=$DocID&amp;htmlSize=$htmlSize&amp;imagesize=$imagesize";

	if ($vars->{attributes} eq ", ") {
		$vars->{attributes} = '';
	}

        return TPL_PAGEINFO;
}

sub show_htmldump {
        my ($s, $vars, $id, $lot, $DocID, $offset, $htmlSize, $imagesize) = @_;

        my $sqlShares = Sql::Shares->new($s->{dbh});
        my $sess = Sql::SessionData->new($s->{dbh});
        my $sessid = $sess->insert('crawled documents' => '');
        $vars->{sessid} = $sessid;
        $vars->{id} = $id;
        my $collection_name = $sqlShares->get_collection_name($id);
        $vars->{collection_name} = $collection_name;


	$vars->{pageinfo} = $s->{infoQuery}->repositoryPageInfo($offset, $DocID, $htmlSize, $imagesize, $collection_name, 1);


        return TPL_HTMLDUMP;
}

sub submit_edit {
    validate_pos(@_, 1, 1, 1);
    my ($s, $vars, $share) = @_;

    my @users = grep { defined $_ } @{$share->{users}};
    my %attr;
    for my $key (%COLLECTION_ATTR) {
        if (defined $share->{$key}) {
            $attr{$key} = $share->{$key};
        }
    }
    $attr{users} = \@users;
    my $dataColl = Data::Collection->new($s->{dbh}, \%attr);

    if ($attr{auth_id} eq 'new_values') {
        $dataColl->set_auth($share->{username}, $share->{password});
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
# Force  full recrawl of a collection.
sub recrawl_collection {
	my ($s, $vars, $id) = @_;
	my $collection_name = 
		$sqlShares->get_collection_name($id);

	my $succs = $s->{infoQuery}->recrawlCollection($collection_name, logfile => $CONFIG{'coll_log_path'} . $collection_name);
	if ($succs) {
		$vars->{coll_succs} = "Crawl request sent.";
	}
	else {
		$vars->{coll_error} = $s->{infoQuery}->error();
	}

	$vars->{id} = $id;
	return $s->manage_collection($vars, $id);
}

sub test_crawl_coll {
	my ($s, $vars, $id, $num_docs) = @_;
	
	$vars->{num_docs} = $num_docs;
	
	$num_docs =~ s/(^\s+|\s+$)//g;
	if ($num_docs !~ /^\d+$/) {
		$vars->{coll_error} = "Invalid number of documents.";
		return $s->manage_collection($vars, $id);
	}

	my $name = $sqlShares->get_collection_name($id);
	
	my $succs = $s->{infoQuery}->recrawlCollection($name, 
		documents_to_crawl => $num_docs, logfile => $CONFIG{'coll_log_path'} . $name);
	if ($succs) {
		$vars->{coll_succs} = "Crawl request sent.";
	}
	else {
		$vars->{coll_error} = $s->{infoQuery}->error();
	}

	return $s->manage_collection($vars, $id);
}

sub del_collection {
	my ($self, $vars, %opt) = (@_);
	croak  "The operation must be a POST request to work."
		unless $ENV{REQUEST_METHOD} eq 'POST';
	my $iq = $self->{infoQuery};
	my $del_name;

	if ($opt{pushed}) {
		my $exists = $sqlShares->collection_name_exists($opt{coll});
		if ((not defined $opt{coll}) or $exists) {
			$vars->{delete_error} 
				= "Collection is not a pushed collection.";
			$vars->{delete_request} = 0;
			return;
		}
		$del_name = $opt{coll};
	}
	else {
		$del_name = $sqlShares->get_collection_name($opt{id});
	}

	if ($iq->deleteCollection($del_name)) {
		$vars->{delete_request} = 1;
		unless ($opt{pushed}) {
			my $coll = Data::Collection->new($self->{dbh}, { id => $opt{id} });
            		$coll->delete();
		}
	}
	else {
		$vars->{delete_error} = $iq->error;
		$vars->{delete_request} = 0;
	}
	
	return $vars;
}

sub del_confirm {
	my ($self, $vars, %opt) = @_;
	if ($opt{pushed}) {
		$vars->{push_del} = 1;
		$vars->{collection_name} = $opt{name};
	}
	else {
		$vars->{collection_name} 
			= $sqlShares->get_collection_name($opt{id});
		$vars->{id} = $opt{id};
	}

 	return TPL_DELETE_COLL;
}

sub stop_crawl {
	validate_pos(@_, 1, 1, { regex => qr(^\d+$) });
	my ($s, $vars, $id) = @_;
	my $pid = $sqlShares->get({ id => $id }, 'crawl_pid')->{crawl_pid};
	
	if (!$pid) {
		$vars->{error} = "Collection is not being crawled.";
		return $s->show();
	}

	if (!$s->{infoQuery}->killCrawl($pid)) {
		$vars->{error} = "Unable to stop crawl. Asuming it's not running.";
		$sqlShares->update({ crawl_pid => undef }, { id => $id });
	}
	else {
		$vars->{success} = "Crawl has been stopped.";
	}
	return $s->show();
}

sub list_pushed {

	open my $ph, "$CONFIG{list_collections_path} |"
		or croak "list collections util: ", $!;

	my @colls;
	for my $line (<$ph>) {
		chomp $line;
		$line =~ /^subname: (.*)$/;
		my $coll = $1;
		
		# Asuming it's pushed if not in db.
		next if $sqlShares->collection_name_exists($coll);

		push @colls, $coll;
	}

	return @colls;
}

sub upd_customization {
	my ($s, $vars, $id, %params_raw) = @_;
	my $sqlRes = Sql::ShareResults->new($s->{dbh});

	croak "Collection '$id' does not exist"
		unless $sqlShares->exists({ id => $id });

	# Mysql 3.20 doesn't seem to support SET = DEFAULT(field)
	# Must therefore delete and insert 
	# TODO: Update in a less error prone manner when
	#       we update mysql.

	my %params;
	
	# Clean and validate.
	for my $p (keys %params_raw) {
		next unless $VALID_CUST{$p};
		$params{$p} = $params_raw{$p};
		$params{$p} =~ s/\s+//g;
	}

	if (my @errors = $s->_validate_custom(%params)) {
		$vars->{errors} = \@errors;
		while (my ($k, $v) = each %params) {
			$vars->{share}{$k} = $v;
		}
		return $s->show_customize($vars, $id, 1);
	}

	# Delete empty fields
	# to use db defaults instead.
	for my $p (keys %params) {
		delete $params{$p} if $params{$p} eq "";
	}

	my @checkboxes = qw(
		filter_same_crc32 filter_same_domain filter_same_url 
		filter_TLDs filter_response cache_link);
	$params{$_} = $params{$_} ? 1 : 0 for @checkboxes;
	
	# Update db.
	$sqlRes->delete({ share => $id });
	$params{share} = $id;
	$sqlRes->insert(\%params);

	$vars->{succs} = "Customizations updated";

	return $s->show_customize($vars, $id);
}


sub _validate_custom {
	my ($s, %params) = @_;

	my @errors;
	my @comma_seperated = qw(
		rank_author_array rank_title_array 
		rank_body_array rank_url_array
		rank_headline_array
	);
	my @single_number = qw(rank_url_main_word rank_title_first_word);

	for my $p (@comma_seperated) {
		next if $params{$p} eq q{};

		push @errors, "'$p' has should be comma seperated list"
			unless $params{$p} =~ /^(?:-?\d+|-?\d+,?)+$/;
	}
	for my $p (@single_number) {
		next if $params{$p} eq q{};

		push @errors, "'$p' should be a number.",
			unless $params{$p} =~ /^-?\d+$/;
	}
	push @errors, "Invalid summary option."
		if $params{summary} and $params{summary} !~ /^(snippet|start)$/;
	
	return @errors;
}

##
# Return format
# { connector_name => [ collections ], }
sub gen_collection_list {
	my $s = shift;

	my $iq = $s->{infoQuery};
	my $default_crawl_rate
		= $s->{sqlConfig}->get_setting('default_crawl_rate');

	my @connectors = $sqlConnectors->get({});
	for my $conn (@connectors) {
		my @collections = $sqlShares->get({ connector => $conn->{id}});
		next unless @collections;
		
		$conn->{collections} = \@collections;
		
		my $test_coll = sprintf $CONFIG{test_coll_name}, $conn->{name};
		for my $coll (@collections) {

			if ($coll->{collection_name} eq $test_coll) {
				$coll->{is_test_coll} = 1;
				$conn->{has_test_coll} = 1;
			}

			next unless $coll->{active};

			my $rate = $coll->{rate} || $default_crawl_rate;
			
			$coll->{last_crawl} = $s->_mysql_to_unixtime($coll->{last});
			delete $coll->{last}; # 'last' conflicts with tt2 vmethod,
			                      # and creates magic.

			$coll->{smart_rate} = $s->_minutes_to_text($rate);
			$coll->{next_crawl} = $s->_get_next_crawl($rate, $coll->{last_crawl});
			$coll->{doc_count}  = $iq->documentsInCollection(
				$coll->{collection_name});

			$coll->{warn} = $coll->{crawler_message}
				if $coll->{crawler_message} !~ $CRAWL_STATUS_OK;
		}

	}
	#croak Dumper(\@connectors);

	# Add XML pushed collections
	my @pushed_names = $s->list_pushed();
	my @pushed_colls;
	for my $n (@pushed_names) {
		push @pushed_colls, { 
			collection_name => $n,
			doc_count       => $iq->documentsInCollection($n),
			is_pushed       => 1,
		};
	}
	push @connectors, { 
		name => $META_CONN_PUSH,
		collections => \@pushed_colls,
		active => $#pushed_colls > 0 ? 1 : undef,
	};
	#carp Dumper(\@connectors);
	return @connectors;
}


sub _get_next_crawl {
	my $self = shift;
	my ($rate, $last) = @_;
	return unless $last; # Need last to find next.
	

	my $time_ago = time - $last;
	my $time_left = ($rate * 60) - $time_ago; # * 60 to get seconds.
	
	my $to_text = $self->_minutes_to_text(abs($time_left / 60));
	if ($time_left < 0) { return "Should have been crawled $to_text ago"; }
	return "Next crawl in $to_text";

}



##
# Change to mysql time string to unixstamp
# TODO: Fetch with MySQL function UNIX_TIMESTAMP
sub _mysql_to_unixtime {
	my $self = shift;
	my $mysql_time = shift;
	return unless $mysql_time;
	return if $mysql_time eq q{0000-00-00 00:00:00}; # legacy

	my ($year, $month, $day, $hour, $minute, $second) 
		= $mysql_time 
		=~ /(\d\d\d\d)-(\d\d)-(\d\d) (\d\d):(\d\d):(\d\d)/;
	my $unixtime = eval { timelocal($second, $minute, $hour, $day, 
			 ($month - 1), ($year - 1900)) };
	if ($@) {
		carp $@;
		return;
	}
	
	$unixtime;
}

##
# Method to change seconds into text.
# Example outputs: "1 day", "5 minutes", "12 hours"
sub _minutes_to_text {
	my ($self, $minutes) = @_;
        return unless defined $minutes;
    
        return "now" if $minutes == 0;

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
