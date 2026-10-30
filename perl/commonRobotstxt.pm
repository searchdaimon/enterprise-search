package commonRobotstxt;
require Exporter;
@commonRobotstxt::ISA = qw(Exporter);
@commonRobotstxt::EXPORT = qw();
@commonRobotstxt::EXPORT_OK = qw(is_RobotsAllowd);

use strict;
use DB_File;

use RobotRules;

# Routine to test whether we can download a url. Returns 1 if we can, 0 if not
#
#
# $RobotName		The robot's name, for example Boitho.com-robot/1.0
# $robotstxt		The robots.txt file we want to test against
# $robotstxturl		the URL that was used to retrieve the /robots.txt file
# $url				The url we want to test whether we can crawl
#
#
sub is_RobotsAllowd {
	my ($RobotName,$robotstxt,$robotstxturl,$url) = @_;
	
	
	my $allowed;
	my $rules = WWW::RobotRules->new($RobotName);
	
	$rules->parse($robotstxturl, $robotstxt);

	if($rules->allowed($url)) {
    	$allowed = 1;
 	}
	else {
		$allowed = 0;
	}
	
	undef $rules;
	
	return $allowed;
}
#
#
# u = unknown (unknown status, we have not downloaded the robots.txt file)
# o = out (the robots.txt file has been sent out for download)
# n = no, the domain has no robots.txt file
# y = yes, we have a robots.txt file for the domain
sub New {
	my($class) = @_;
	#my $class = '';
	my $self = {}; #allocate new hash for object
	bless($self, $class);
	open($self->{'LOCK'},">../data/robottxt.lock") or die ("Cant open data/robottxt.lock: $!");
	flock($self->{'LOCK'},2) or die ("Can't lock lock file: $!");
	print { $self->{'LOCK'} } $$;
	
	tie %{ $self->{'DBMrobottxt'} }, "DB_File", '../data/robottxt' or die("Can't open dbm ../data/robottxt: $!");
	
	
	
	return $self;
}
#load info about a url, so that we can then start testing what we know about it
sub LodeDomain {
	my($self,$domain) = @_;
	
	$self->{'domain'} = $domain;
	
	my $post = $self->{'DBMrobottxt'}->{$self->{'domain'}};
	
	
	if ($post ne '') {
		#if the entry is not empty we split status and data
		($self->{'domaindata'}->{'status'},$self->{'domaindata'}->{'data'}) = split(':',$post);
	}
	else {
		$self->{'domaindata'}->{'status'} = 'u';
	}
	
}
#whether we have downloaded the url
sub TestetForRobotstxt {
	my $self = shift;
	
	if ($self->{'domaindata'}->{'status'} eq 'u') {
		return 0;
	}
	else {
		return 1;
	}
}
sub CanCrawl {
	my($self,$RobotName,$url) = @_;
	
	
	if ($self->{'domaindata'}->{'status'} eq 'n') {
	#the domain has no robots.txt file, we can safely crawl
		return 1;
	}
	
	elsif ($self->{'domaindata'}->{'status'} eq 'y') {
	#we have a robots.txt file, so we test against it
		my $robotstxturl = "http://$self->{'domain'}/robots.txt";
		
		print "robots.txt: " . $self->{'domaindata'}->{'data'} . "\n";
		
		if (is_RobotsAllowd($RobotName,$self->{'domaindata'}->{'data'},$robotstxturl,$url)){
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}
}
sub addRobotstxtFile {
	my($self,$robotstxtfile) = @_;
	
	print "adding: $robotstxtfile\n";
	
	my $post = 'y:' . $robotstxtfile;
	
	$self->{'DBMrobottxt'}->{ $self->{'domain'} } = $post;
	
	$self->{'domaindata'}->{'status'} = 'y';
	$self->{'domaindata'}->{'data'} = $robotstxtfile;
}
sub Close {
	my $self = shift;
	
	untie %{ $self->{'DBMrobottxt'} } or die("Can't close dbm ../data/robottxt: $!");
	close($self->{'LOCK'}) or warn ("Cant close lock file: $!");
}