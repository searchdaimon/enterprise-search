
#!/usr/bin/perl

use Time::HiRes;
use Getopt::Std;
use LWP::Simple;
getopts('u:w', \%opts);

#print "w : $opts{'w'}, ARGV $ARGV[0]\n";

my $host = shift(@ARGV) or die "must specify a host";
my $file = shift(@ARGV) or die "must specify a queryfile";

my $user = "";

if (exists ($opts{'u'}) ) {
	$user = $opts{'u'} . '@';
}


open(INF,$file) or die("can't open $file: $!");
my @arr = <INF>;
close(INF);

print "Server: \t$host\n";
print "Query file: \t$file\n";
print "User: \t\t$user\n";

print "----------------------------------------------------------------\n";
printf "| %-40s | %-7s | %-7s |\n",  "Query", "Time", "Hits";
print "----------------------------------------------------------------\n";

foreach my $i (@arr) {
	chomp($i);

	$query = $i;
	$query =~ s/([^A-Za-z0-9])/sprintf("%%%02X", ord($1))/seg;


	my $url = "http://$user$host/webclient/?query=$query&tpl=";
	#print "$url\n";
	
	our $StartTime  = Time::HiRes::time;
	my $content = get $url;
	my $time = Time::HiRes::time - $StartTime;
  	warn "Couldn't get $url" unless defined $content;

	my $hits = $content;
	#$hits =~ s/av totalt (\d+) resultater//g;
	$hits =~ s/av totalt ([\d ]+) resultater//g;
	$hits = $1;
	#print "#####################################################\n";
	#print $content, "\n";
	#print "#####################################################\n";
	#print $hits;
	printf "| %-40s | %.5f | %7s |\n",  $i, $time, $hits;

	#print "$i\t$time\n";	
}
print "----------------------------------------------------------------\n";

