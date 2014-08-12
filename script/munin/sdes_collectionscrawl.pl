#!/usr/bin/env perl
use strict;
use warnings;
use Carp;

####################################################################
#settup
#####
#MySQL settup
my $boithoData = '/boithoData/lot/1/1/';


# Open boithoData and saves all relevant folders into a array
my @collections;
opendir (DIR, $boithoData) or die ("Can't open $boithoData: $!");
    while (my $file = readdir(DIR)) {

	if ($file =~ /\.$/ or $file =~ /^_/) { next; }
	push(@collections, $file);

    }
closedir(DIR);


@collections = sort @collections;


use constant CFG => 
q{graph_info This graph shows number of new documents pr collection.
graph_title Number of new documents
graph_vlabel Documents / hours
graph_category Searchdaimon
graph_period hour
graph_args -l 0
};


if (my $a = shift) {
	if ($a eq 'autoconf') {
		#print "yes\n";
		exit 1;
	}
	elsif ($a eq 'config') {
		print CFG;
		foreach my $i (@collections) {
			print "_$i.label $i\n";
			print "_$i.type DERIVE\n";
			print "_$i.min 0\n";
		}
	}
	else {
		die "Usage $0 [autoconf|config]\n";
	}
	exit;
}


foreach my $i (@collections) {

	my $count = 0;
	my $file = $boithoData . $i . "/DocID";
	if (open(INF, $file)) {
		flock(INF,1);
		$count = <INF>;
		chomp($count);
		close(INF);
	}

	print "_$i.value $count\n";

}




