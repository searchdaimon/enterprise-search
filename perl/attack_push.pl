use strict;
use warnings;
use threads;
use Thread::Queue;
use File::Slurp;
use LWP::UserAgent;
use HTTP::Request::Common;
use Getopt::Std;

my %opt;

getopts('t:', \%opt);




my $q = Thread::Queue->new();    # A new empty queue




if ($#ARGV != 2) {
	print "Please specify a folder, collection and server.\n\n";
 	print "Usage:\n\n";
 	print "perl/attack_push.pl folder collection server\n\n";
	exit;
}

$opt{folder} = shift(@ARGV) or die "must specify a folder";
$opt{collection} = shift(@ARGV) or die "must specify a collection";
$opt{server} = shift(@ARGV) or die "must specify a server";

if (!exists ($opt{'t'}) ) {
        $opt{'t'} = 3;
}

# Worker threads
my @thr;
for my $i (1..$opt{'t'}) {
	$thr[$i] = threads->create(
	        sub {
			my $ua = LWP::UserAgent->new;

	            	# Thread will loop until no more work
	            	while (defined(my $item = $q->dequeue())) {
				my $tid = threads->tid();

	                	# Do work on $item
				my $data = read_file( $item, binmode => ':raw', err_mode => 'carp' );
				unless ( $data ) {
					warn("Can't read file $item: $!");
					next;
				}

				my $url = "http://$opt{server}/documents/$opt{collection}$item";
		

				my $response = $ua->post($url, 
							REQUEST_METHOD => 'ADDDELAYED',
							Content_Type => 'multipart/form-data',
							Content => $data
				);


				print "t $tid: $url. ";
				print $response->content;

	            	}
	        }
	);
}

# Go thru all files
recdir($opt{folder});

# If we have an old version of Thread::Queue we will have to busy wait for it to be finished and do an ungracefully exit with the working thread still running.
# If we have version 3 or newer we can just call end() and gracefully wait for the threads to exit.
if (Thread::Queue->VERSION < 3) {

	while ($q->pending() > 0) {
		sleep(0.2);
	}

	# Wating a bit more....
	sleep(4);
	exit;
}
else {
	# Signal that there is no more work to be sent
	$q->end();

	# Join up with the thread when it finishes
	for my $i (1..$opt{'t'}) {
		$thr[$i]->join();
	}
}


sub recdir {
	my ($path) = @_;
	
	my $DIR;
	opendir($DIR, $path) or warn("can't opendir $path: $!") && return;

        while (my $file = readdir($DIR) ) {

                #skiping . and ..
                if ($file =~ /\.$/) {
                      next;
                }
                my $candidate = $path . "\/" . $file;

                if (-d $candidate) {
                        recdir($candidate);
                }
                else {
			# add $candidate
			#print "$candidate\n";
			$q->enqueue($candidate);
                }
      }

      closedir($DIR);

}
