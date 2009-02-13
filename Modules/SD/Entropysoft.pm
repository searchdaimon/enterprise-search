#!/usr/bin/perl -w

package SD::Entropysoft;

use strict;
use warnings;

use Data::Dumper;
use Getopt::Long;
use Date::Parse;

use SOAP::Lite(readable => 1, outputxml => 1);

our $SOAP_ERROR;

my $DEBUG;
my $result = GetOptions ("debug" => \$DEBUG); # No 'my' for $DEBUG? 
$DEBUG && SOAP::Lite->import(trace => 'debug');

my ($the_user, $the_pass);

sub SOAP::Transport::HTTP::Client::get_basic_credentials { 
	return $the_user => $the_pass;
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


# Serialize ID
# <id xsi:type="ns2:ID"><repository xsi:type="xsd:string">entropysoft</repository><uid xsi:type="xsd:string"></uid></id>
# <id xsi:type="tns1:ID"><repository xsi:type="xsd:string">ftp</repository><uid xsi:type="xsd:string" /></id>
sub SOAP::Serializer::as_ID {
	my $self = shift;
	my($value, $name, $type, $attr) = @_;

	my $repository = SOAP::Data->new(name=>'repository', value=>$value->{repository}, type=>'string');
	my $uid = SOAP::Data->new(name=>'uid', value=>$value->{uid}, type=>'string');
	my $rval = [$name,
	   {'xsi:type'=>'ID', %$attr}, 
	   {
		   repository=>$repository,
		   uid=>$uid
	   }
	   ];
	return $rval;
}


# Serialize ID
# <credentials xsi:type="ns2:Credentials"><userName xsi:type="xsd:string">en</userName><password xsi:type="xsd:string" xsi:nil="true"/><domain xsi:type="xsd:string">sdtest2003</domain></credentials>
sub SOAP::Serializer::as_Credentials {
	my $self = shift;
	my($value, $name, $type, $attr) = @_;

	my $euser = SOAP::Data->new(name=>'userName', value=>$value->{userName}, type=>'string');
	my $epass = SOAP::Data->new(name=>'password', value=>$value->{password}, type=>'string');
	my $edomain = SOAP::Data->new(name=>'domain', value=>$value->{domain}, type=>'string');
	print Dumper($name);
	my $rval = ['credentials',##$name,
	   {'xsi:type'=>'Credentials', %$attr}, 
	   {
		   userName=>$euser,
		   password=>$epass,
		   domain=>$edomain
	   }
	   ];
	return $rval;
}

sub make_id {
	my ($repo, $uid) = @_;

	return { repository => $repo, uid => (defined $uid ? $uid : "") };
}

sub find_in_mapentry_array {
	my $name = shift;
	my $mes = shift;

	foreach my $entry (@{$mes}) {
		return $entry->{value} if ($entry->{key} eq $name);
	}

	return undef;
}

sub traverse_repository_file {
	my ($self, $uid, $service, $me) = @_;

	# XXX: Grab all pages
	if ($service->getPageCount($uid) > 1) {
		die "More than one page...";
	}

	my $size = find_in_mapentry_array('contentLength', $me->{properties});
	if (defined($size) && $size > (1024 * 1024)) {
		print STDERR "Skiping document, too big...\n";
		next;
	}

	if (not defined($self->{max_size}) or $self->{max_size} > $size) {
		my $time = 0;
		my $ts = undef;

		$ts = find_in_mapentry_array('updateDate', $me->{properties});
		if (defined($ts)) {
			$ts = find_in_mapentry_array('creationDate', $me->{properties});
		}

		my $type = find_in_mapentry_array('mimeType', $me->{properties});

		print Dumper($me);

		$time = str2time($ts) if (defined($ts));

		#my $data = $service->getContent($uid, 0);
		my $getdata = sub {
			return $service->getContent($uid, 0)->{data};
		};

		my $getpermissions = sub {
			print "Grabing permissions...\n";

			#print "Foo: " . Dumper($service->getPermissions($uid)) . "\n";
			#print "Bar: " . Dumper($service->getPermissionTypes($uid)) . "\n";

			return $service->getPermissions($uid);
		};

		if (defined($self->{found_document})) {
			$self->{found_document}($time, $size, $uid, $type, $me->{properties}, $getdata, $getpermissions);
		}
		#print Dumper($data);
	}
}

sub traverse_repository_folder {
	my ($self, $uid, $service) = @_;

	my $children = $service->getChildren($uid, 0, 0, 0, undef);

	print "Found " . $children->{totalRows} . " children.\n";

	foreach my $child (@{ $children->{results} }) {
		if (find_in_mapentry_array("mimeType", $child->{properties}) eq 'urn:entropysoft:folder') {
			traverse_repository_folder($self, make_id($child->{id}->{repository}, $child->{id}->{uid}), $service);
		} else {
			print "Found file: " . $child->{id}->{repository} . ":" . $child->{id}->{uid} ."\n";
			#print Dumper($child);
			traverse_repository_file($self, make_id($child->{id}->{repository}, $child->{id}->{uid}), $service, $child);
		}
	}
}

sub traverse_repository {
	my ($self, $repo, $service) = @_;

	my $root = $service->getRepositoryRootFolder($repo) || die "$SOAP_ERROR";

	print "Traversing: " . find_in_mapentry_array("name", $root->{properties}) . "\n";

	traverse_repository_folder($self, make_id($root->{id}->{repository}, $root->{id}->{uid}), $service);

	return 1;
}

sub register_repository {
	my ($self, $name, $host, $user, $pass, $domain, $type) = @_;

	my %factories = (
		sharepoint => 'net.entropysoft.eci.sharepoint.SharePointContentProviderFactory',
	);
	my $factory = $factories{$type} || die "Unknown factory for connector: $type";

	my $regservice = $self->{regservice};

	my $site;
	if ($host !~ /^http/) {
		$site = "http://" . $host;
	} else {
		$site = $host;
	}

	$regservice->createUpdateRepositoryXml(
		'
		<repository>
		<disabled>false</disabled>
		<displayName>'.$name . " " . $site.'</displayName>
		<factoryClassName>'.$factory.'</factoryClassName>
		<ignoreOtherProperties>false</ignoreOtherProperties>
		<mappings>
		<forceReadOnly>false</forceReadOnly>
		<hidden>false</hidden>
		<internalName>name</internalName>

		<providerName>FileLeafRef</providerName>
		</mappings>
		<mappings>
		<forceReadOnly>false</forceReadOnly>
		<hidden>false</hidden>
		<internalName>author</internalName>
		<providerName>Author</providerName>
		</mappings>
		<mappings>
		<forceReadOnly>false</forceReadOnly>
		<hidden>false</hidden>

		<internalName>contentLength</internalName>
		<providerName>File_x0020_Size</providerName>
		</mappings>
		<mappings>
		<forceReadOnly>false</forceReadOnly>
		<hidden>false</hidden>
		<internalName>creationDate</internalName>
		<providerName>Created</providerName>
		</mappings>
		<mappings>
		<forceReadOnly>false</forceReadOnly>

		<hidden>false</hidden>
		<internalName>updateDate</internalName>
		<providerName>Modified</providerName>
		</mappings>
		<name>'.$name.'</name>
		<properties>
		<key>net.entropysoft.services.timeToLive</key>
		</properties>
		<properties>
		<key>siteUrl</key>
		<value>'.$site.'</value>

		</properties>
		<properties>
		<key>handleGenericLists</key>
		<value>false</value>
		</properties>
		<properties>
		<key>httpChunkingEnabled</key>
		<value>true</value>
		</properties>
		<properties>
		<key>credentials.userName</key>
		</properties>

		<properties>
		<key>credentials.password</key>
		</properties>
		<properties>
		<key>credentials.domain</key>
		</properties>
		</repository>
		',
	{
		domain => $domain,
		password => $pass,
		userName => $user,
	},
	1);
}

sub init {
	my ($self, $ip, $user, $pass) = @_;

	$self->{the_user} = $user;
	$self->{the_pass} = $pass;

	my $service = new SOAP::Lite->service("http://$user:$pass\@$ip:8082/entropysoft/ws/ContentService?WSDL");
	my $regservice = new SOAP::Lite->service("http://$user:$pass\@$ip:8082/entropysoft/ws/RepositoryRegistry?WSDL");
	#my $regservice = new SOAP::Lite->service("file:./regserv.wsdl");
	$service->on_fault(\&soap_fault_handler);
	$service->on_nonserialized(sub { die "$_[0]?!?" });

	$regservice->on_fault(\&soap_fault_handler);
	$regservice->on_nonserialized(sub { die Dumper(@_)."?!?" });

	$the_user = $user;
	$the_pass = $pass;
	#$service->{user} = $user;
	#$service->{pass} = $pass;

	$self->{service} = $service;
	$self->{regservice} = $regservice;
}

sub new {
	my ($foo, $ip, $user, $pass) = @_;

	my $self = {};

	$self->{service} = undef;
	$self->{max_size} = undef;
	bless $self;
	$self->init($ip, $user, $pass);

	return $self;
}

sub go {
	my ($self, $repo) = @_;

	traverse_repository($self, $repo, $self->{service});
}

sub document_callback {
	my ($self, $func) = @_;

	$self->{found_document} = $func;
}

sub max_size {
	my $self = shift;

	if (@_) {
		$self->{max_size} = shift;
	}

	return $self->{max_size};
}

1;

#package Some;

#use Data::Dumper;

#my $crawler = new Entropysoft("localhost", "Admin", "");

#my $crawler = new SD::Entropysoft("localhost", "Admin", "");
#$crawler->register_repository('sp01', 'http://sp03.searchdaimon.com', 'en', '1234asd', 'sdtest2003', 'sharepoint');
#$crawler->register_repository("sharepoint-es", "sp01.searchdaimon.com", "en", "1234asd", "sdtest2003", "sharepoint");

#
#$crawler->max_size(1024000);
#$crawler->document_callback(sub { 
#	my ($time, $size, $uid, $type, $getdata, $getpermissions) = @_;
#
#	print "Document: size: " . $size . " time: " . $time . " type: $type uid: " . $uid->{uid} . "\n";
#	print "Permissions: " . Dumper(&$getpermissions()) . "\n";
##	print "Data is:\n" . &$getdata() . "\n";
#});
#$crawler->go("fileserver");
##$crawler->go("sp01");
