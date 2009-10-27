package SharepointCrawler;

use strict;
use warnings;
use SOAP::Lite;
use Data::Dumper;

use Time::HiRes;

sub new {
	my $self = shift;
	my $class = ref($self) || $self;
	my ($_service,$url, $extraaction) = @_;

	return $self if ref $self;

	$self = bless {
		_url => $url,
		_service => $_service,
		_extraaction => $extraaction,
	} => $class;

	my $service = $self->{_service};

	my $serializer = $service->serializer();
	my $ns = 'http://schemas.microsoft.com/sharepoint/soap/';
	if ($extraaction) {
		$ns .= $extraaction . "/";
	}
	$serializer->register_ns( $ns, 's0' );
	$service->encoding(undef);

	return $self;
}

sub do_query {
	my ($self, $name, $data) = @_;

	my $service = $self->{_service};

	#print "Proxy: ".$self->{_url}."\n";

	my $action = "http://schemas.microsoft.com/sharepoint/soap/";
	if ($self->{_extraaction}) {
		$action .= $self->{_extraaction} . "/";
	}
	$action .= "$name";
	$service->on_action( sub { $action });
	return $service->proxy($self->{_url}, (keep_alive => 1))->call("s0:$name" => (ref $data eq 'ARRAY' ? @{$data} : $data));
}

# Lists

sub get_list_collection {
	my ($self) = @_;

	return $self->do_query('GetListCollection');
}

sub get_list {
	my ($self, $name) = @_;

	my $data = SOAP::Data->new(name => 'listName', type => 's:string', attr => {}, prefix => 's0', value => $name);
	return $self->do_query('GetList', $data);
}

sub get_list_items {
	my ($self, $name, $pager, $lastcrawl) = @_;

	my $startg = Time::HiRes::time;
	my $q;
	$self->{lastcrawl} = $lastcrawl;
	if (defined $self->{lastcrawl} and $self->{lastcrawl} > 0) {
		my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($self->{lastcrawl});

		my $timestr = (1900 + $year) . "-";
		$mon += 1;
		$timestr .= ($mon < 10 ? '0' . $mon : $mon) . "-";
		$timestr .= ($mday < 10 ? '0' . $mday : $mday) . "T";

		$timestr .= ($hour < 10 ? '0' . $hour : $hour) . ":";
		$timestr .= ($min < 10 ? '0' . $min : $min) . ":";
		$timestr .= ($sec < 10 ? '0' . $sec : $sec) . "Z";

		$q = SOAP::Data->new(name => 'Where', value => [
			SOAP::Data->new(name => 'Geq', value => [
				SOAP::Data->new(name => 'FieldRef', attr => {Name => 'Modified'}),
				SOAP::Data->new(name => 'Value', attr => {Type => 'DateTime'}, value => $timestr),
			])
		]);
	} else {
		$q = undef;
	}

	my $data = [
		SOAP::Data->new(name => 'listName', type => 's:string', prefix => 's0', value => $name),
		SOAP::Data->new(name => 'viewName', type => 's:string', prefix => 's0', value => ''),
		# XXX: Find a good number
		SOAP::Data->new(name => 'rowLimit', type => 's:int', prefix => 's0', value => '200'),
		SOAP::Data->new(name => 'viewFields', prefix => 's0', value => [
			# If this is empty, we get all the fields
			SOAP::Data->new(name => 'ViewFields', attr => {}, value => [
				])
			]
		),
		SOAP::Data->new(name => 'queryOptions', prefix => 's0', value => [
			SOAP::Data->new(name => 'QueryOptions', value => [
					SOAP::Data->new(name => 'Paging', attr => {ListItemCollectionPositionNext=>$pager}),
					SOAP::Data->new(name => 'ViewAttributes', attr => {Scope=>'Recursive'}), 
					SOAP::Data->new(name => 'IncludeMandatoryColumns', value => 'TRUE'), 
					SOAP::Data->new(name => 'IncludeAttachmentUrls', value => 'TRUE'), 
				])
			]),
		SOAP::Data->new(name => 'query', prefix => 's0', value => [
			SOAP::Data->new(name => 'Query', value => [ $q, ]),
		]),
	];
	my $endg = Time::HiRes::time;

	my $startg = Time::HiRes::time;
	my $r = $self->do_query('GetListItems', $data);
	my $endg = Time::HiRes::time;

	print STDERR "Get took:". ($endg-$startg)."\n";

	return $r;
}

sub get_attachment_collection {
	my ($self, $listname, $id) = @_;

	print "Listname: $listname, Itemid: $id\n";

#	my $guid = $listname;
#	$guid =~ s/^{//;
#	$guid =~ s/}$//;

	my $data = [
		SOAP::Data->new(name => 'listName', prefix => 's0', value => $listname),
		SOAP::Data->new(name => 'listItemID', prefix => 's0', value => $id),
	];

	return $self->do_query('GetAttachmentCollection', $data);
}

# Permissions

sub get_permission_collection {
	my ($self, $name, $type) = @_;

	my $data = [
		SOAP::Data->new(name => 'objectName', type => 's:string', prefix => 's0', value => $name),
		SOAP::Data->new(name => 'objectType', type => 's:string', prefix => 's0', value => $type),
	];

	return $self->do_query('GetPermissionCollection', $data);
}

# User Group

sub get_usercollection_from_group {
	my ($self, $name) = @_;

	my $data = [
		SOAP::Data->new(name => 'groupName', type => 's:string', prefix => 's0', value => $name),
	];

	return $self->do_query('GetUserCollectionFromGroup', $data);
}

sub get_groupcollection_from_role {
	my ($self, $name) = @_;

	my $data = [
		SOAP::Data->new(name => 'roleName', type => 's:string', prefix => 's0', value => $name),
	];

	return $self->do_query('GetGroupCollectionFromRole', $data);
}

sub get_usercollection_from_role {
	my ($self, $name) = @_;

	my $data = [
		SOAP::Data->new(name => 'roleName', type => 's:string', prefix => 's0', value => $name),
	];

	return $self->do_query('GetUserCollectionFromRole', $data);
}

sub get_groupcollection_from_user {
	my ($self, $name) = @_;

	my $data = [
		SOAP::Data->new(name => 'userLoginName', type => 's:string', prefix => 's0', value => $name),
	];

	return $self->do_query('GetGroupCollectionFromUser', $data);
}

# Versions

sub get_versions {
	my ($self, $file) = @_;

	my $data = [
		SOAP::Data->new(name => 'fileName', type => 's:string', prefix => 's0', value => $file),
	];

	return $self->do_query('GetVersions', $data);
}

# Webs

# XXX: Not working
#sub get_web {
#	my ($self, $site) = @_;
#
#	my $data = [
#		SOAP::Data->new(name => 'webUrl', type => 's:string', prefix => 's0', value => $site),
#	];
#
#	return $self->do_query('GetWeb', $data);
#}

sub get_web_collection {
	my ($self) = @_;

	return $self->do_query('GetAllSubWebCollection', []);
}

package Perlcrawl;

use Carp;
use Data::Dumper;
use strict;
use warnings;

use lib '../../Modules/';
use lib '../Modules/';

use Crawler;
our @ISA = qw(Crawler);

use SDSharepointDeserializer;
use sdMimeMap;

use LWP::Simple qw(get);
use LWP::UserAgent;
use LWP::Authen::Ntlm;
use LWP::Debug;
use HTTP::Request::Common;
use URI::Escape;

use HTML::Strip;

use SD::Crawl;

use SOAP::Lite(readable => 1);
use Date::Parse;

my $DEBUG;
$DEBUG = 0;
($DEBUG & 1) && SOAP::Lite->import(trace => 'debug');
($DEBUG & 2) && LWP::Debug->import(level => '+');

$SOAP::Constants::PATCH_HTTP_KEEPALIVE = 1;

my $sharepoint_version = undef;

my ($SOAP_ERROR) = (undef);

my ($the_user, $the_pass);

sub SOAP::Transport::HTTP::Client::get_basic_credentials {
        return $the_user => $the_pass;
}
sub LWP::UserAgent::get_basic_credentials {
        return $the_user => $the_pass;
}


# Override the service file fetcher in SOAP::Lite, we might want ntlm auth here
sub SOAP::Schema::access {
	my $self = shift->new;
	my $url = shift || $self->schema_url || Carp::croak 'Nothing to access. URL is not specified';

	my $ua = LWP::UserAgent->new(keep_alive => 1);

	my $req = GET $url;
	my $resp = $ua->request($req);

	$sharepoint_version = $resp->header('MicrosoftSharePointTeamServices');
	
	$resp->is_success ? $resp->content : die "Service description '$url' can't be loaded: ",  $resp->status_line, "\n";
}

sub soap_fault_handler {
        my ($soap, $res) = @_;


        if( ref( $res ) ) {
                chomp( my $err = $res->faultstring );
                $SOAP_ERROR = "SOAP FAULT: $err";
                print STDERR "SOAP FAULT: $err\n";
        }
        else
        {
                chomp(my $err = $soap->transport->status);
                $SOAP_ERROR = "TRANSPORT ERROR: $err";
                print STDERR "TRANSPORT ERROR: $err\n";
        }

        return new SOAP::SOM;
}

sub new_service {
	my ($self, $ip, $user, $pass, $name, $site, $extraaction) = @_;

	my $url = $self->{proto}."://$user:$pass\@$ip";
	if (defined $site and $site !~ /\s+/ and $site ne '') {
		$site =~ s/^\/+//;
		$site =~ s/\/+$//;
		$url .= "/$site";
	}
	$url .= "/_vti_bin/$name.asmx";
	print "Getting: $name.asmx?WSDL\n";
	my $service = new SOAP::Lite->service($url."?WSDL");
	#$service->proxy($url);
	#my $service = new SOAP::Lite->proxy($url);
	$service->deserializer(SDSharepointDeserializer->new);
	#$service->on_fault(\&soap_fault_handler);
	$service->on_nonserialized(sub { die "$_[0]?!?" });

	return new SharepointCrawler($service, $url, $extraaction);
}

sub is_sp2007 {
	my ($self) = @_;

	return 1 if ($self->{majorversion} == 12);
	return undef;
}

sub is_sp2003 {
	my ($self) = @_;

	return 1 if ($self->{majorversion} == 6);
	return undef;
}

# Cache roles
my %roles;
my %groups;

sub group_to_users {
	my ($self, $group) = @_;

	return $groups{$group} if exists $groups{$group};

	my @sids = ();

	my $ref = $self->{userservice}->get_usercollection_from_group($group)->{Users};

	$ref = [ $ref ] if (ref $ref eq 'HASH');

	if (ref $ref eq 'ARRAY') {
		foreach my $r (@{ $ref }) {
			push @sids, $r->{Sid} if exists $r->{Sid};
		}
	}

	$groups{$group} = \@sids;
	return \@sids;
}

sub role_to_groups {
	my ($self, $role, $userservice) = @_;

	return $roles{$role} if exists $roles{$role};
	
	print STDERR "Role: $role\n";
	my @sids = ();
	my $ref = $userservice->get_usercollection_from_role($role)->{Users};
	if (ref($ref) eq 'ARRAY') {
		@sids = map { $_->{Sid}; } @{ $ref };
	}
	$ref = $userservice->get_groupcollection_from_role($role);
	if (ref $ref eq 'ARRAY') {
		foreach my $r (@{ $ref }) {
			push @sids, $r->{Sid};
		}
	}

	$roles{$role} = \@sids;

	return \@sids;
}

sub handle_listitem_attachment_worker {
	my ($self, $url, $allowedstr, $parent, $p_attributes) = @_;

	# Some urls are not wanted, dwp for instance.
	# dwp are xml files describing a specific page on the sharepoint server, we generate our own pages so ignore these
	return if ($url =~ /\.dwp$/i); # v2
	return if ($url =~ /\.webpart$/i); # v3


	my $get_url = $url;

	my $req = HEAD $get_url;
	my $ua = LWP::UserAgent->new(keep_alive => 1);
	my $res = $ua->request($req);
	if ($res->status_line =~ /^401/) {
		# XXX: Send basic auth somehow else
		$get_url =~ s/^http(s|)?:\/\//http$1:\/\/$the_user:$the_pass@/;

		$req = HEAD $get_url;
		$res = $ua->request($req);
	}

	my $lastmodified = str2time($res->header('Last-Modified'));
	$lastmodified = 0 unless defined $lastmodified;

	#print "Resource status: " . $res->status_line ." for $url\n";
	if (($res->status_line =~ /^2\d\d/) and (not $self->document_exists($url, $lastmodified, $res->header('Content-Length')))) {
		my $req = GET $get_url;
		my $res = $ua->request($req);
		my $type = mapMimeType($res->header('Content-Type'));

		my $title = $url;
		$title = $1 if $title =~ /([^\/]+)$/;
		$title = uri_unescape($title);

		my @attributes = @{ $p_attributes };
		push @attributes, "sptype=file";
		push @attributes, "parent=$parent" if defined $parent;

		# TODO: change $p_attributes from array to hash
		my %attr = map { split "=", $_, 2 } @attributes;

		$self->add_document(
				url => $url,
				title => $title,
				content => $res->content,
				last_modified => $lastmodified,
				type => $type,
				acl_allow => $allowedstr,
				attributes => \%attr,
				);
	}

	# For when we want to support several versions of the same document
	#my $filename = substr($url, length($self->{base}));
	#$filename =~ s/^\/+//;
	#print Dumper($self->{versionsservice}->get_versions($filename));
}

sub handle_listitem_attachment {
	my ($self, $listid, $listname, $itemid, $itemurl, $allowedstr, $p_attributes) = @_;

	my $ref = $self->{listsservice}->get_attachment_collection($listname, $itemid);

	#print Dumper($ref);

	return unless (ref $ref->{Attachments} eq 'ARRAY');

	foreach my $url (@{$ref->{Attachments}}) {
		print $url."\n";
		$self->handle_listitem_attachment_worker($url, $allowedstr, $itemurl, $p_attributes);
	}

}

sub handle_value {
	my ($value) = @_;

	# Remove html
	my $hs = HTML::Strip->new();
	my $value = $hs->parse( $value );
	$hs->eof;

	if ($value =~ /(\d+;#)/) {
		$value = substr($value, length $1);
	} elsif  ($value =~ /(\d+)\.0+/) {
		$value = $1;
	}

	return $value;
}


sub handle_listitem_worker {
	my ($self, $items, $listid, $dispurl, $allowedstr, $pattributes_l, $listname) = @_;

	my @attributes_l = @{ $pattributes_l };
	my @attributes_a = @{ $pattributes_l };

	#my $starttime = Time::HiRes::time;
	print STDERR "Number of items this round: " . scalar (@{ $items->{listitems}->{'rs:data'}->{'z:row'} })."\n";
	foreach my $item (@{ $items->{listitems}->{'rs:data'}->{'z:row'} }) {
		my $doc = '';
		my $title = '';
		my $modifiedstr = '0';
		my $id = '';
		my $encodedabsurl = undef;

		my @attributes = @attributes_l;
		push @attributes, 'sptype=listitem';

		my $s1 = Time::HiRes::time;
		# XXX: Filter away some keys?
		$doc .= "<table>\n";
		foreach my $key (keys %{$item}) {
			my $pkey = $key;
			$pkey =~ s/^ows_//;

			# Drop if:
			# Double ows
			next if $pkey =~ /^ows/;
			# Start with underscore
			next if $pkey =~ /^_/;

			$title = $item->{$key} if ($pkey eq 'Title');
			$modifiedstr = $item->{$key} if ($pkey eq 'Modified');
			$id = $item->{$key} if ($pkey eq 'ID');
			$encodedabsurl = $item->{$key} if ($pkey eq 'EncodedAbsUrl');

			$pkey =~ s/x0020_//;

			# Misc drop
			my @misc_drop_keys = qw(
				LinkFilenameNoMenu
				LinkTitleNoMenu
				LinkTitle
				Attachments
				Editor
				Author
				Deleted
				FileLeafRef
				GroupEdit
				FileRef
				ServerUrl
				FSObjType
				WorkflowVersion
				BaseName
				ContentTypeDisp
				ContentType
				ProgId
				SelectTitle
				NameWithPicture
				NameWithPictureAndDetails
				IsSiteAdmin
				LinkFilename
				EncodedAbsUrl
				IsActive
				Created
				Modified
				UserSelection
				MetaInfo
				FileDirRef
				PictureDisp
				PermMask
				Order
				Title
				Title
				Last_Modified
				Created_Date
			);
			next if (grep $_ eq $pkey, @misc_drop_keys);

			# Drop ids
			next if $pkey =~ /id$/i;

			$pkey =~ s/_/ /;

			$doc .= "<tr>\n";
			$doc .= "<td>$pkey</td>\n";
			$doc .= "<td>".handle_value($item->{$key})."</td>\n";
			$doc .= "</tr>\n";
		}
		$doc .= "</table>\n";
		#my $e1 = Time::HiRes::time;
		#print "Stage1 took: ".($e1-$s1)."\n";

		#my $s2 = Time::HiRes::time;
		my $unixtime = str2time($modifiedstr);
		#my $e2 = Time::HiRes::time;
		#print "Stage2 took: ".($e2-$s2)."\n";
		#my $s3 = Time::HiRes::time;
		my $path = $dispurl . "ID=". $id;
		# Try harder to find a title
		if ($title eq '' and exists $item->{ows_BaseName}) {
			$title = $item->{ows_ContentType} . ': ' if (exists $item->{ows_ContentType});
			$title .= $item->{ows_BaseName};
		}

		# Type
		if (exists($item->{'ows_EventDate'})) {
			push @attributes, 'Appointment=Sharepoint';
		}
		elsif (exists($item->{'ows_ContentType'}) and $item->{'ows_ContentType'} eq 'Person') {
			push @attributes, 'Person=Sharepoint';
		}

		# Author
		if (exists($item->{'ows_Editor'})) {
			my $a = $item->{'ows_Editor'};
			$a =~ s/^\d+;#//;
			push @attributes, ('author=' . $a);
		} elsif (exists($item->{'ows_Author'})) {
			my $a = $item->{'ows_Author'};
			$a =~ s/^\d+;#//;
			push @attributes, ('author=' . $a);
		}

		push @attributes, "snippet=db";

		#my $e3 = Time::HiRes::time;
		#print "Stage3 took: ".($e3-$s3)."\n";
		#my $s4 = Time::HiRes::time;
		if (!$self->document_exists($path, $unixtime, length($doc))) {

			# TODO: change $p_attributes from array to hash
			my %attr = map { split "=", $_, 2 } @attributes;

			$self->add_document(
					url => $path,
					title => $title,
					content => $doc,
					last_modified => $unixtime,
					type => 'html',
					acl_allow => $allowedstr,
					attributes => \%attr,
					);
		}
		#my $e4 = Time::HiRes::time;
		#print "Stage4 took: ".($e4-$s4)."\n";
		#my $s5 = Time::HiRes::time;

		my $child_title = $title;
		$child_title =~ s/^[^:]+:\s*//;

		if (defined $encodedabsurl) {
			# XXX: Getting 404 on quite a bit of the files that end in .\d\d\d, skip them for now
			if ($encodedabsurl !~ /\.\d+$/ and not defined($item->{'ows_WebPartDescription'})) {
				$self->handle_listitem_attachment_worker($encodedabsurl, $allowedstr, undef, \@attributes_a);
			}
		}
		#my $e5 = Time::HiRes::time;
		#print "Stage5 took: ".($e5-$s5)."\n";
		#my $s6 = Time::HiRes::time;
		$self->handle_listitem_attachment($listid, $listname, $id, $child_title, $allowedstr, \@attributes_a);
		#my $e6 = Time::HiRes::time;
		#print "Stage6 took: ".($e6-$s6)."\n";
	}
	#my $endtime = Time::HiRes::time;

	#print STDERR "Add took: " . ($endtime - $starttime) . "\n";
}

sub handle_listitem {
	my ($self, $listid, $listurl, $allowedstr, $pattributes_l, $listname) = @_;

	my $items;

	# XXX: Verify
	my $dispurl = $listurl;
	if (not ($dispurl =~ s/[^\/]+.aspx$/DispForm.aspx?/)) {
		print STDERR "Unable to handle listurl: $listurl\n";
		return;
	}

	# More items in the list?
	my $first = 1;
	while ($first or exists($items->{listitems}->{'rs:data'}->{ListItemCollectionPositionNext})) {
		my $pager;

		if ($first) {
			$pager = undef;
		} else {
			$pager = $items->{listitems}->{'rs:data'}->{ListItemCollectionPositionNext};
		}

		print "Got pager: $pager\n" unless $first;
		$items = $self->{listsservice}->get_list_items($listid, $pager, $self->{lastcrawl});
#		if ($listurl =~ /users/) {
#			print STDERR Dumper($items);
#		}
		return unless exists $items->{listitems}->{'rs:data'}->{'z:row'};
#		return unless ($listurl =~ /users/);

		$self->handle_listitem_worker($items, $listid, $dispurl, $allowedstr, $pattributes_l, $listname);
		$first = undef;
	}
}

sub handle_lists {
	my ($self, $pattributes_p) = @_;

	my @attributes_p = @{ $pattributes_p } ;

	my $lists = $self->{listsservice}->get_list_collection();


	#print STDERR Dumper(map { $_->{Title} } @{ $lists->{Lists} });

	my $ref = [];
	$ref = $lists->{Lists} if ref $lists->{Lists} eq 'ARRAY';
	foreach my $list (@{$ref}) {

		print "Title: ".$list->{Title}."\n";
		#print "Desc: ".$list->{Description}."\n";
		#print STDERR Dumper($list);
		my @allowed = ();
		# XXX: Temporary hack
		if ($self->{ignoreperms} eq '0') {
			# XXX: Find a way to handle sharepoint sites we can't grab permissions from
			my $permref = $self->{permservice}->get_permission_collection($list->{ID}, 'List')->{Permissions};
			#print STDERR "Permref: " . Dumper($permref);
			if (ref $permref eq 'HASH') {
				$permref = [ $permref ];
			}
			foreach my $perm (@{ $permref }) {
				my $rolename = undef;
				#print STDERR "perm: " . Dumper($perm);

				next unless exists $perm->{Mask};
				if (exists $perm->{MemberIsUser} and $perm->{MemberIsUser} eq 'True') {
					my $username = $perm->{UserLogin};
					# XXX: resolve sid
					if ($perm->{Mask} eq '-1' or ($perm->{Mask} & 1) == 1) {
						push @allowed, $username;
					}
					next;
				
				}

				if ($self->is_sp2007) {
					$rolename = $perm->{GroupName};
				} elsif ($self->is_sp2003) {
					$rolename = $perm->{RoleName};
				} else {
					die 'Unknown sharepoint version';
				}

				if ($perm->{Mask} eq '-1' or ($perm->{Mask} & 1) == 1) {
					my $r;
					if ($self->is_sp2007) {
						$r = $self->group_to_users($rolename, $self->{userservice});
					} elsif ($self->is_sp2003) {
						$r = $self->role_to_groups($rolename, $self->{userservice});
					}
					foreach my $r2 (@{$r}) {
						push @allowed, $r2;
					}
				} else {
					if ($self->is_sp2007) {
						print "Group ".$rolename." does not have access to list\n";
					} elsif ($self->is_sp2003) {
						print "Role ".$rolename." does not have access to list\n";
					}
				}
			}
		} else {
			@allowed = ('Everyone');
		}

		my $datestr = $list->{Modified};
		my $unixtime = str2time($datestr);

		my $doc = '';
		foreach my $x (qw(Title Created Modified Description)) {
			$doc .= $x . ": ". $list->{$x}."\n";
		}

		my $path = $self->{url} . $list->{DefaultViewUrl};

		my %seenallowed;
		my $allowedstr = join(',', grep { ! $seenallowed{$_}++ } @allowed);
		if (!$self->document_exists($path, $unixtime, length($doc))) {
			my @attributes_l = @attributes_p;
			push @attributes_l, "sptype=list";


			# TODO: change $p_attributes from array to hash
			my %attr = map { split "=", $_, 2 } @attributes_l;

			$self->add_document(
				url => $path,
				title => $list->{Title},
				content => $doc,
				last_modified => $unixtime,
				type => 'txt',
				acl_allow => $allowedstr,
				attributes => \%attr,
			);
		}

		my @attributes = @attributes_p;
		push @attributes, 'List=' . $list->{Title};
		$self->handle_listitem($list->{ID}, $path, $allowedstr, \@attributes, $list->{Title});
	}
}

sub handle_sites {
	my ($self, $sites) = @_;

	#print Dumper($sites);

	foreach my $site (@{$sites}) {
		my $url = $site->{Url};
		if ($url =~ /^(\w+):\/\/([^\/]+)(\/.*)?$/) {
			my $proto = $1;
			my $ip = $2;
			# Check for toplevel site
			my $subsite = defined $3 ? $3 : '';

			$subsite =~ s/^\/+//;

			print "Trying site: " . ($site->{Title} or 'undefined') . "\n";

			my $userservice = $self->new_service($ip, $self->{username},
			    $self->{password}, "usergroup", $subsite, 'directory');
			my $listsservice = $self->new_service($ip, $self->{username}, $self->{password}, "lists", $subsite);
			my $versionsservice = $self->new_service($ip, $self->{username}, $self->{password}, "versions", $subsite);
			my $permservice = $self->new_service($ip, $self->{username},
			    $self->{password}, "permissions", $subsite, 'directory');

			$self->{userservice} = $userservice;
			$self->{listsservice} = $listsservice;
			$self->{permservice} = $permservice;
			$self->{versionsservice} = $versionsservice;

			$self->{proto} = $proto;
			$self->{ip} = $ip;
			$self->{url} = $proto . '://'.$ip;
			$self->{base} = $site->{Url};

			my @attributes;

			push @attributes, "source=sharepoint";
			#push @attributes, "site=$url";
			my $_subsite = $subsite;
			$_subsite = 'Home' if (not defined $_subsite or $_subsite eq '' or $_subsite =~ /^\s{0,}$/);
			push @attributes, "Site=$_subsite";
			#print Dumper(\@attributes);

			$self->handle_lists(\@attributes);
		} else {
			warn "Url on unknown format: $url";
		}
#		print $site->{Url} ."\n";
	}
}

##
# Main loop for a crawl update.
# This is where a resource is crawled, and documents added.
sub crawl_update {
	my (undef, $self, $opt) = @_;

	my $username = $the_user = $opt->{user};
	my $password = $the_pass = $opt->{password};
	my $ip = $opt->{ip};
	my $site = $opt->{site};
	my $repository = $opt->{collection_name};
	my $ignoreperms = $opt->{ignoreperms};

	# XXX: Support https
	$self->{proto} = 'http';

	my $webservice = $self->new_service($ip, $username, $password, "Webs", $site);

	# XXX: Detect sharepoint version
	print "Sharepoint version: $sharepoint_version\n";
	my @version = split(/\./, $sharepoint_version);
	$self->{majorversion} = $version[0];


	my $lastcrawl = $self->get_last_crawl_time();
	print "Last crawl? $lastcrawl\n";

	$self->{ip} = $ip;
	$self->{username} = $username;
	$self->{password} = $password;
	$self->{ignoreperms} = $ignoreperms;
	$self->{lastcrawl} = $lastcrawl;

#	print Dumper($sites);

	$site =~ s/^\s+//;
	$site =~ s/\s+$//;

	my $sites = $webservice->get_web_collection()->{Webs};
	$sites = [$sites] if (ref $sites eq 'HASH');

	if ($site eq '') {
		print Dumper($sites);

		$self->handle_sites($sites);
	} else {
		print "Sites:\n";
		my @want_sites;

		foreach my $r (@{ $sites }) {
			my $name = $r->{Url};
			$name =~ s/^http(s)?:\/\///;
			$name =~ s/^[^\/]+//;
			print $name;

			# Special case for the top level site
			if ($name eq '' and $site eq '/') {
				push @want_sites, $r;
				next;
			}
			$name =~ s/^\///;
			my @subnames = split("/", $name);
			if ($subnames[0] eq $site) {
				push @want_sites, $r;
				next;
			}
		}

		print Dumper(\@want_sites);

		my $url = $self->{proto} . "://$ip";
		if (defined $site and $site !~ /\s+/ and $site ne '') {
			my $site2 = $site;
			$site2 =~ s/^\/+//;
			$site2 =~ s/\/+$//;
			$url .= "/$site2";
		}

		$self->handle_sites(\@want_sites);
	}
}


sub path_access {
    my ($undef, $self, $opt) = @_;
# print Data::Dumper->Dump([$opt]);
    my $user = $opt->{"user"};
    my $passw  = $opt->{"password"};
    my $url = $opt->{"resource"};

    return 1; # Authenticated
}

1;
