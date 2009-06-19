
#!/usr/bin/perl

use Time::HiRes;
use Getopt::Std;
use LWP::Simple;
getopts('u:wm:n:', \%opts);


my $host = shift(@ARGV) or die "must specify a host";
my $user = shift(@ARGV) or die "must specify a user";
my $file = shift(@ARGV) or die "must specify a queryfile";

my $max = 0;
my $ntimes = 1;

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


	my $url = "http://$host/cgi-bin/suggest_webclient_oem?q=$query&user=$user";
	#print "$url\n";
	
	our $StartTime  = Time::HiRes::time;
	my $content = get $url;
	my $time = Time::HiRes::time - $StartTime;
  	warn "Couldn't get $url" unless defined $content;

	#print "$content\n";

	my $hits = $content;

	$hits = 0;
	#teller antall \n
	$hits += ($content =~ tr/\n//);

	# markerer at vi fikk 0 hoits
	if ($hits == 0) {
		$hits = "\033[1;32m      $hits\033[0m";
	}
	printf "| %-40s | %.5f | %7s |\n",  $i, $time, $hits;

	#print "$i\t$time\n";	

	if ( ($max != 0) && ($max == $count) ) {
		last;
	}
}
print "----------------------------------------------------------------\n";

}
