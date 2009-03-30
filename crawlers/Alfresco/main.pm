package Perlcrawl;
use Carp;
use Data::Dumper;
use strict;
use warnings;

use Crawler;
our @ISA = qw(Crawler);

use SD::Entropysoft;
use sdMimeMap;

use Carp;
use Data::Dumper;
use LWP::Simple qw(get);

use SD::Crawl;

##
# Main loop for a crawl update.
# This is where a resource is crawled, and documents added.
sub crawl_update {
	my (undef, $self, $opt) = @_;

	print Dumper($opt);

	my $username = $opt->{"user"};
	my $password = $opt->{"password"};
	my $ip = $opt->{"ip"};
	my $repository = $opt->{collection_name};
	my $repotype = $opt->{"repotype"};
	my $repourl = $opt->{"repourl"};
	my $rpass  = $opt->{"rpassword"};
	my $ruser = $opt->{"rusername"};
	my $rdomain = $opt->{"rdomain"};
	my $counter = 0;

	my $crawler = new SD::Entropysoft($ip, $username, $password, $repository);

	$crawler->register_repository($repository, $repourl, $ruser, $rpass, $rdomain, $repotype);

	$self->remove_foreign_users();
	while (my ($key, $value) = each(%{$crawler->{usergroups} })) {
		$self->add_foreign_user($key);
		foreach my $group (@{ $value }) {
			$self->add_foreign_user($key, $group);
		}
	}
	
	$crawler->max_size(1024000);
	$crawler->document_callback(sub {
		my ($time, $size, $uid, $type, $properties, $getdata, $getpermissions) = @_;

		my $rtype = mapMimeType($type);
		my $path;
		my $title;

		if (!defined($path = SD::Entropysoft::find_in_mapentry_array('EncodedAbsUrl', $properties))) {
			$path = $uid->{uid};
		}

		if (!defined($title = SD::Entropysoft::find_in_mapentry_array('Title', $properties))) {
			if (!defined($title = SD::Entropysoft::find_in_mapentry_array('title', $properties))) {
				$title = $path;
			}
		}


		if ($path =~ /\.(\w+)$/) {
			$rtype = $1;
		} elsif ($path =~ /\.(\w+)\[\d+\]$/) {
			$rtype = $1;
		}

		#print "We got some  stuff here: $type / $rtype\n";

		if (!$self->document_exists($path, $time, $size)) {
			my $data = &$getdata();
			my @permissions = &$getpermissions();

			print "And we want to add it!\n";
			$self->add_document(
				url => $path,
				title => $title,
				content => $data,
				last_modified => $time,
				type => $rtype,
				acl_allow => join(',', @permissions),
				attributes => "");
		}
		$counter++;
	});

	$crawler->go($repository);
};


sub path_access {
    my ($undef, $self, $opt) = @_;
# print Data::Dumper->Dump([$opt]);
    my $user = $opt->{"user"};
    my $passw  = $opt->{"password"};
    my $url = $opt->{"resource"};

    return 1; # Authenticated
}

1;
