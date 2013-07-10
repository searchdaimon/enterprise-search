
#!/usr/bin/perl

use strict;

use Thread;
use Time::HiRes;
use Getopt::Std;
use LWP::Simple;

my %opts;
my %opt;

getopts('u:wm:n:rt:dshce:', \%opts);


if ($#ARGV != 1) {
	print "\n";
	print "Please specify a host and a file with queries.\n\n";
 	print "Usage:\n\n";
 	print "perl/attack.pl host queryfile\n\n";
	exit;
}

$opt{host} = shift(@ARGV) or die "must specify a host";
$opt{file} = shift(@ARGV) or die "must specify a queryfile";


$opt{max} = 0;
$opt{ntimes} = 1;
$opt{random} = 0;
$opt{wordwash} = 0;
$opt{diff} = 0;
$opt{notime} = 0;
$opt{haltOnError} = 0;

if (exists ($opts{'u'}) ) {
	$opt{user} = $opts{'u'} . '@';
}

if (exists ($opts{'m'}) ) {
	$opt{max} = $opts{'m'} . '@';
}

if (exists ($opts{'n'}) ) {
	$opt{ntimes} = $opts{'n'};
}

if (exists ($opts{'r'}) ) {
	$opt{random} = 1;
}
if (exists ($opts{'w'}) ) {
	$opt{wordwash} = 1;
}

if (exists ($opts{'t'}) ) {
	$opt{threads} = $opts{'t'};
}
if (exists ($opts{'d'}) ) {
	$opt{diff} = 1;
}
if (exists ($opts{'s'}) ) {
	$opt{notime} = 1;
}
if (exists ($opts{'h'}) ) {
	$opt{haltOnError} = 1;
}
if (exists ($opts{'c'}) ) {
	$opt{printContent} = 1;
}
if (exists ($opts{'e'}) ) {
	$opt{errorfile} = $opts{'e'};
}

open(INF,$opt{file}) or die("can't open " . $opt{file} . ": $!");
my @arr;
while (<INF>) {
	chomp;

	# Skip empty lines and comments that starts with "#" sign.
	if ($_ eq '' || $_ =~ /^#/) {
		next;
	}
	push(@arr, $_);
}
close(INF);


print "Server: \t" . $opt{host} . "\n";
print "Query file: \t" . $opt{file} . "\n";
print "User: \t\t". $opt{user} . "\n";

print "-----------------------------------------------------------------------\n";
printf "| %-40s | %-7s | %-9s | %-2s |\n",  "Query", $opt{diff} ? "" : "Time", "Hits", "T";
print "-----------------------------------------------------------------------\n";

my @threads;
my $EOUT;

if ($opt{threads}) {
	for my $i (1..$opt{threads}) {

		my $thr = new Thread \&attack, \@arr, \%opt, $i; 
		push(@threads, $thr);

	}

	#venter på trådene
	foreach my $thr (@threads) {
		$thr->join;
	}
}
else {
	attack(\@arr,\%opt,0);
}
print "-----------------------------------------------------------------------\n";

sub attack {
	my ($arr, $opt, $tid) = @_;

	if ($opt{threads} > 1) {
		#sover mellom 0 og 1 for å la den komme litt igang.
		my $random_number = rand(1);
		print "Veter i $random_number sec\n";
		sleep($random_number);
 	}

	if ($opt{errorfile}) {
		open($EOUT,">$opt{errorfile}") or warn("Cant open errorfile \"$opt{errorfile}\": $!");
	}

	for (1..$opt{ntimes}) {

		if ($opt{random}) {
			fisher_yates_shuffle( \@arr );    # permutes @array in place	
		}


		my $count = 0;
		foreach my $i (@$arr) {
			++$count;

			chomp($i);
			#fjerner alt etter første ugyldige tegn. Kjekt for å parse ordbøker og slikt
			if ($opt{wordwash}) {
				$i =~ /(^[\wæøåÆØÅ ]+)/;
				$i = $1;
			}

			my $query = $i;
			$query =~ s/([^A-Za-z0-9])/sprintf("%%%02X", ord($1))/seg;

			#skipper tomme querys hvis vi har på haltOnError, da det altid vil feile.
			if ( ($opt{haltOnError} == 1) && ($query eq "") ) {
				next;
			}

			my $url;
			if ($opt{user}) {
				$url = "http://" . $opt{user} . $opt{host} . "/webclient2/?query=$query&lang=en";
			}
			else {
				$url = "http://" . $opt{host} . "/webclient2/indexanon.pl?query=$query&lang=en";
			}
			#print "$url\n";
	
			my $StartTime  = Time::HiRes::time;
			my $content = get $url;
		  	warn "Couldn't get $url: $!" unless defined $content;

			my $time = Time::HiRes::time - $StartTime;


			if ($opt{diff}) {
				$time = -1;
			}
			if ($opt{notime}) {
				$time = 0;
			}
			


			my $hits = $content;
			
			if ($hits =~ /Showing no results/) {
				$hits = 0;
			}
			else {
				if ($hits =~ s/out of ([\d ,]+) result//g) {
					$hits = $1;
					chop($hits);
				}
				else {
					$hits = "\033[1;32m  Error \033[0m";
					if ($opt{errorfile}) {
						print $EOUT $content;
					}
					
				}
			}
			if ($opt{printContent}) {
				print "#####################################################\n";
				print $content, "\n";
				print "#####################################################\n";
				print $hits;
			}
			printf "| %-40s | %.5f | %9s | %2i |\n",  $i, $time, $hits, $tid;

			#print "$i\t$time\n";	

			if ( ($opt{haltOnError} == 1) && ($hits =~ /Error/) ) {
				print "haltOnError\n";
				exit;
			}
	
			if ( ($opt{max} != 0) && ($opt{max} == $count) ) {
				last;
			}
		}

	}

	if ($opt{errorfile}) {
		fclose($EOUT) or warn ("Can't clode $opt{errorfile}: $!");
	}

}
# array randomiserings funksjon. Se perl cookbook side 121
sub fisher_yates_shuffle {
         my $array = shift;
         my $i;
         for ($i = @$array; --$i; ) {
             my $j = int rand ($i+1);
             next if $i == $j;
             @$array[$i,$j] = @$array[$j,$i];
         }
}

