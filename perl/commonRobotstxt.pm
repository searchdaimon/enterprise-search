package commonRobotstxt;
require Exporter;
@commonRobotstxt::ISA = qw(Exporter);
@commonRobotstxt::EXPORT = qw();
@commonRobotstxt::EXPORT_OK = qw(is_RobotsAllowd);

use strict;
use DB_File;

use RobotRules;

# Rutine for å teste om vi kan laste ned en url. Retunerer 1 hvis vi kan, 0 hvis ikke
#
#
# $RobotName		Robotens navn, for eksempel Boitho.com-robot/1.0
# $robotstxt		Robots.txt filen vi vil teste mot
# $robotstxturl		the URL that was used to retrieve the /robots.txt file
# $url				Urlen vi vil teste om vi kan crawle
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
# u = unknown (ukjent status, vi har ikke lastet ned robots.txt fil)
# o = out (robots.txt fieln er sent ut til nedlasting)
# n = no, domene har ingen robots.txt fil
# y = yes, vi har robots.txt fil for domene
sub New {
	my($class) = @_;
	#my $class = '';
	my $self = {}; #allocate new hash for objekt
	bless($self, $class);
	open($self->{'LOCK'},">../data/robottxt.lock") or die ("Cant open data/robottxt.lock: $!");
	flock($self->{'LOCK'},2) or die ("Can't lock lock file: $!");
	print { $self->{'LOCK'} } $$;
	
	tie %{ $self->{'DBMrobottxt'} }, "DB_File", '../data/robottxt' or die("Can't open dbm ../data/robottxt: $!");
	
	
	
	return $self;
}
#laster infor om en url, slik at vi siden kan begynne å teste hva vi vet om den
sub LodeDomain {
	my($self,$domain) = @_;
	
	$self->{'domain'} = $domain;
	
	my $post = $self->{'DBMrobottxt'}->{$self->{'domain'}};
	
	
	if ($post ne '') {
		#hvis ikke posten er tom splitter vi status og data
		($self->{'domaindata'}->{'status'},$self->{'domaindata'}->{'data'}) = split(':',$post);
	}
	else {
		$self->{'domaindata'}->{'status'} = 'u';
	}
	
}
#om vi har lastet ned urlen
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
	#domene har ingen robots.txt fil, vi kn trykt crawle
		return 1;
	}
	
	elsif ($self->{'domaindata'}->{'status'} eq 'y') {
	#vi har robots.txt fil, så vi tester på den
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