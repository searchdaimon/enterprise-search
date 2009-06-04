
#!/usr/bin/perl

use Time::HiRes;
use Getopt::Std;
use LWP::Simple;
getopts('u:wm:n:', \%opts);


my $host = shift(@ARGV) or die "must specify a host";
my $file = shift(@ARGV) or die "must specify a queryfile";

my $user = "";
my $max = 0;
my $ntimes = 1;

if (exists ($opts{'u'}) ) {
	$user = $opts{'u'} . '@';
}

if (exists ($opts{'m'}) ) {
	$max = $opts{'m'} . '@';
}

if (exists ($opts{'n'}) ) {
	$ntimes = $opts{'n'};
}

for (1..$ntimes) {
open(INF,$file) or die("can't open $file: $!");
my @arr = <INF>;
close(INF);

print "Server: \t$host\n";
print "Query file: \t$file\n";
print "User: \t\t$user\n";

print "----------------------------------------------------------------\n";
printf "| %-40s | %-7s | %-7s |\n",  "Query", "Time", "Hits";
print "----------------------------------------------------------------\n";

my $count = 0;
foreach my $i (@arr) {
	++$count;

	chomp($i);

	$query = $i;
	$query =~ s/([^A-Za-z0-9])/sprintf("%%%02X", ord($1))/seg;


	my $url = "http://$user$host/webclient2/?query=$query&tpl=";
	#print "$url\n";
	
	our $StartTime  = Time::HiRes::time;
	my $content = get $url;
	my $time = Time::HiRes::time - $StartTime;
  	warn "Couldn't get $url" unless defined $content;

	my $hits = $content;
	#webclient 1
	#$hits =~ s/av totalt (\d+) resultater//g;
	#webclient 2
	if ($hits =~ /Viser ingen resultater/) {
		$hits = 0;
	}
	else {
		if ($hits =~ s/av totalt ([\d ,]+) resultat//g) {
			$hits = $1;
			$hist = chop($hits);
		}
		else {
			$hits = "\033[1;32m Error \033[0m";
		}
	}
	#print "#####################################################\n";
	#print $content, "\n";
	#print "#####################################################\n";
	#print $hits;
	printf "| %-40s | %.5f | %7s |\n",  $i, $time, $hits;

	#print "$i\t$time\n";	

	if ( ($max != 0) && ($max == $count) ) {
		last;
	}
}
print "----------------------------------------------------------------\n";

}
