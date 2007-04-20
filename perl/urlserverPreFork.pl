#!/usr/bin/perl

use DB_File;
use commonDemon qw(daemonis opt_handler);
use LWP::Simple;

%opts = opt_handler('dl',$help);


if ($opts{d}) {
	daemonis(0,1,'lagringsserver');
}

my $terminated = 0;
my $waitingForConection = 0; #holder om vi står å vente på en tilkobling slik at vi kan avslutte med en gang

$url_server_port = 8080;



use IO::Select;
use IO::Socket;

use Time::HiRes;

use Socket;

use commonRobotstxt qw(is_RobotsAllowd);


use XML::Simple;
use XML::Writer;
use XML::Writer::String;

use MySQLDcConnect;

use MIME::Base64 ();
use Compress::Zlib;

use Boitho::Reposetory;
use Boitho::Lot;

use common qw{fin_domene};


use constant MinUrlInQue => 2500; #url, responskode, data type, html
use constant MinClientApplicationVersion => 0.1;

use constant RobotsTxtStorageServer => '213.179.58.99'; #hvos vi skal sende robots.txt filene vi laster ned ?

use constant subname => 'www';

use constant UrlQueuePostLength => 204;

use constant RobotName => 'boitho.com-dc';

use constant SocketTimeout => 600; #10 minn timeout

#use constant MakeThumbnale => 1; #om vi skal lage bilder for sidene vi laster ned. 1= skall, 0=skall ikke
use constant MakeThumbnale => 0; #om vi skal lage bilder for sidene vi laster ned. 1= skall, 0=skall ikke

#my $UrlQueue = Thread::Queue->new;


#oppner robot.txt databasen

		
#laster blokklisten
%DomainBlockList = loadeBaqnnList();

#cashe over lagringsservere for loter
my %lotStorageServerCashe = {};
		
#oppreter treaden som vil hontre urlkøen
#$UrlQueueTreadHA = threads->new(\&UrlQueueTread);

Boitho::Lot::lotlistLoad();
	
###############################################################################################################################
#!/usr/bin/perl
# preforker - server who forks first
use IO::Socket;
use Symbol;
use POSIX;

# establish SERVER socket, bind and listen.
$server = IO::Socket::INET->new(LocalPort => $url_server_port,
                                Type      => SOCK_STREAM,
                                Proto     => 'tcp',
                                Reuse     => 1,
                                Listen    => 10 )
  or die "making socket: $@\n";

# global variables
$PREFORK                = 10;       # number of children to maintain
$MAX_CLIENTS_PER_CHILD  = 5;        # number of clients each child should process
%children               = ();       # keys are current child process IDs
$children               = 0;        # current number of children

sub REAPER {                        # takes care of dead children
    $SIG{CHLD} = \&REAPER;
    my $pid = wait;
    $children --;
    delete $children{$pid};
}

sub HUNTSMAN {                      # signal handler for SIGINT
    local($SIG{CHLD}) = 'IGNORE';   # we're going to kill our children
    kill 'INT' => keys %children;
    exit;                           # clean up with dignity
}

    
# Fork off our children.
for (1 .. $PREFORK) {
    make_new_child();
}

# Install signal handlers.
$SIG{CHLD} = \&REAPER;
$SIG{INT}  = \&HUNTSMAN;

# And maintain the population.
while (1) {
    sleep;                          # wait for a signal (i.e., child's death)
    for ($i = $children; $i < $PREFORK; $i++) {
        make_new_child();           # top up the child pool
    }
}

sub make_new_child {
    my $pid;
    my $sigset;
    
    # block signal for fork
    $sigset = POSIX::SigSet->new(SIGINT);
    sigprocmask(SIG_BLOCK, $sigset) or die "Can't block SIGINT for fork: $!\n";
    
    die "fork: $!" unless defined ($pid = fork);
    
    if ($pid) {
        # Parent records the child's birth and returns.
        sigprocmask(SIG_UNBLOCK, $sigset)
            or die "Can't unblock SIGINT for fork: $!\n";
        $children{$pid} = 1;
        $children++;
        return;
    } else {
        # Child can *not* return from this subroutine.
        #$SIG{INT} = 'DEFAULT';      # make SIGINT kill us as it did before
    		

 	#our $terminated = 0;
        #our $waitingForConection = 0; #holder om vi står å vente på en tilkobling slik at vi kan avslutte med en gang


		
        # unblock signals
        sigprocmask(SIG_UNBLOCK, $sigset)
            or die "Can't unblock SIGINT for fork: $!\n";
    

	#instalerer signal handler, slik at vi ikke dør mitt i ting, og vi får tid til å lagre
        $SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;
	
		###########################################################
		@UrlQueue = ();
		#while (scalar(@UrlQueue) <= MinUrlInQue -1) {
			my $lastetUrler = 0;
	 		open(UrlQueueFA,"+<../data/UrlQueue") or warn("Can't open UrlQueue: $!");
			binmode(UrlQueueFA);
			flock(UrlQueueFA,2);
		
			my $FileSize = (stat(UrlQueueFA))[7];
	
		
			#if ($FileSize > (MinUrlInQue * UrlQueuePostLength)) {	
			if ($FileSize != 0) {
				#Ok, filen er stor nokk, vi leser
			
			
				#finner antall urler vi skal lese
				my $urlsToRead = 0;
				if ($FileSize > (MinUrlInQue * UrlQueuePostLength)) {
					$urlsToRead = MinUrlInQue;
				}
				else {
					$urlsToRead = ($FileSize / UrlQueuePostLength);
				}

				#finner ut til hvor vi skal lese til
				my $StartToRead = $FileSize - ($urlsToRead * UrlQueuePostLength);
		
				seek(UrlQueueFA,$StartToRead,0) or warn("cant seek to $StartToRead $!");



				print "will read $urlsToRead urls\n";				

				for (1 .. $urlsToRead) {
					read(UrlQueueFA,$post,UrlQueuePostLength) or warn($!);
		
					#my ($url,$DocID) = unpack('A200,I',$post);
				
					#push(@UrlQueue,"$DocID,$url");
					push(@UrlQueue,$post);				

					#print "$url,$DocID\n";
					$lastetUrler++;
				}
			
				#Sletter de urlene vi nettop leste
				truncate(UrlQueueFA,$StartToRead) or warn("Cant truncate $!. StartToRead is $StartToRead");
			
				print "Lastet $lastetUrler til UrlQueue som ble på " . scalar(@UrlQueue) . "\n";
		
			}
			else {
				print "UrlQueue har ferre en " . MinUrlInQue . "elementer.\n";
			#	sleep(10);
			}
		
		close(UrlQueueFA);
		#}
		###########################################################

		
		
	

		#hvis vi ikke har noen urler kobler vi oss til, og lar "new_connection" ta seg av problemet
		if (scalar(@UrlQueue) == 0) {
				# handle connections until we've reached $MAX_CLIENTS_PER_CHILD
				#temp:
				#for (my $i=0; (($i < $MAX_CLIENTS_PER_CHILD) && (not $terminated)); $i++) {
					$waitingForConection = 1;
					$client = $server->accept()     or last;
					$waitingForConection = 0;

					our $StartTime  = Time::HiRes::time;
            				# do something with the connection
					
						new_connection($client);
					

					print "RunTime" . (Time::HiRes::time - $StartTime) . "\n\n";
					close $client;
				#}
		}
		#hvis vi har URLer skal vi godta nye tilkoblinger får så lenge vi har det
		else {

			%sett = {};

			while ((scalar(@UrlQueue) > 0) && (not $terminated)) {
				$waitingForConection = 1;
        			    	$client = $server->accept()     or last;
				$waitingForConection = 0;
		        
			    	# do something with the connection
				new_connection($client);
			
				close $client;
				#$ones=1;
       			}
	   	}
    

    
		#lagerer eventuell data
		SaveQueues();

#temp:
#	my $overflowData = <$client>;
#		
#	print "overflowData: -$overflowData-\n";

	#shutdown($client,2);	
	#close($client);
        # this exit is VERY important, otherwise the child will become
        # a producer of more and more children, forking yourself into
        # process death.
	print "exiting\n";
        exit;
    }
}
###############################################################################################################################

############################################################################################################################
# Lagerer URLKøen
############################################################################################################################
sub SaveQueues {

	if (scalar(@UrlQueue) != 0) {
	
		print "UrlQueue have " . scalar(@UrlQueue) .  " elements, saving\n";
		
		open(UrlQueueFA,">>../data/UrlQueueSERVEROWERFLOW") or warn($!);
		binmode(UrlQueueFA);
		flock(UrlQueueFA,2);
		# Reset the file pointer to the end of the file, in case 
		# someone wrote to it while we waited for the lock...
		seek(UrlQueueFA,0,2);
	
		foreach my $element (@UrlQueue) {
			if (length($element) == UrlQueuePostLength) {
				print UrlQueueFA $element;
			}
			else {
				warn("url element is not " .UrlQueuePostLength. "but " .length($element). "\n");
			}
		}
	
		close(UrlQueueFA);
	}
}

############################################################################################################################
# hånterer signaler
############################################################################################################################
sub signal_handler {	
	my $signame = shift;

	local $SIG{INT} = 'IGNORE';


	warn "got sig $signame";

	if ($waitingForConection) {
		warn "Waiting for conection. Can exit safe\n";
		
		SaveQueues();
		
		
		exit;
	}
	else {
		if (not $terminated) {
			++$terminated;

			warn "Ok, begynner og avslutte";
		}
		else {
			warn("Motok killsignal nr $terminated, dør");
		}
	}
}

sub new_connection {
  	my $fh = shift;

  	binmode $fh;

	print "start of new_conection" . (Time::HiRes::time - $StartTime) . "\n\n";  
  	
	#print "i tread\n";
  	#skriv ut datene vi fikk inn
  	#print <$fh> . "\n";
	#exit;
  	#$fh->recv($data_read, $maxlen_read);
	
	#print "leser\n";
	#my $data_read = <$fh>;
	#chomp(@data_read = <$client>);
	#$data_read = join(" ", @data_read);
	
	#leser første 10 bytes for å finne ut hvor mye vi skal lese
	#my $maxlen_read = 0;
	#read($fh,$maxlen_read,10);
	
	#temp:recv($fh,$maxlen_read,8,MSG_WAITALL);
	#read($fh,$maxlen_read,8);

	#temp:recv($fh,$protokollversjon,2,MSG_WAITALL);
	#read($fh,$protokollversjon,2);

	#print "maxlen_read: $maxlen_read. protokollversjon: -$protokollversjon-\n";


	#lengden er total beskjedslengde, så i må trekke fra 10 som er hedderen på beskjeden
	#$maxlen_read = $maxlen_read -10;
	
	#print "lest første 10 bytes" . (Time::HiRes::time - $StartTime) . "\n\n";
	
	#print "Leser lengde: $maxlen_read\n";

	#read($fh,$data_read,$maxlen_read);

    #setter en timer slik at vi ikke henger her for altid hvis noe skulle klikke
    my $maxlen_read = 0;
    eval {
        local $SIG{ALRM} = sub { die "alarm\n" }; # NB: \n required
        alarm SocketTimeout;


      
	#leser lengde
        read($fh,$maxlen_read,8);

	#lengden er total beskjedslengde, så i må trekke fra 10 som er hedderen på beskjeden
        $maxlen_read = $maxlen_read -10;

	#leser protokol
        read($fh,$protokollversjon,2);
	print "lest første 10 bytes" . (Time::HiRes::time - $StartTime) . "\n\n";

	#leser data
	read($fh,$data_read,$maxlen_read);

#########
alarm 0;
}; #else eval/alarm 
if ($@) {
        warn unless $@ eq "alarm\n";   # propagate unexpected errors
        #die unless $@ eq "alarm\n";   # propagate unexpected errors
        # timed out

}
#########


	print "leste " . length($data_read) . " of $maxlen_read\n";

  
  	if ($data_read eq '') { die("Motok ingen inn data") };



	if (($protokollversjon < 2) || ($protokollversjon eq '')) {


	print "begynne å parse xml" . (Time::HiRes::time - $StartTime) . "\n\n";  
  	my $jobb = XMLin("$data_read", forcearray=>1);

	print "parset xml" . (Time::HiRes::time - $StartTime) . "\n\n";
	
	#kjører gjenom jobbene
	#for my $jobb (@{$cust_xml->{'boithodc'}}) {
		
		print "Motok komando: $jobb->{'komando'}->[0]\n";
		
		my $userName = $jobb->{'user'}->[0];
		my $userPassword = $jobb->{'password'}->[0];
		
		my $clientapplicationversion = $jobb->{'applicationversion'}->[0];
		##################################################################################################################
		# Sjekker klientens aplikasjon versjon
		#
		##################################################################################################################
		if ($jobb->{'applicationversion'}->[0] < MinClientApplicationVersion) {
			my $output = XML::Writer::String->new();

			my $writer = new XML::Writer(OUTPUT => $output);

			$writer->xmlDecl();

			$writer->startTag('xml');
			$writer->startTag('error');
						$writer->startTag('popuperror');
							
							$writer->startTag('sleep');
								$writer->characters('60');
							$writer->endTag();
							$writer->startTag('comment');
								$writer->characters('Sorry, cant give you any urls because your client is outdated, pleas go to www.boitho.com and dwonloade a newer version.');
							$writer->endTag();
							
						$writer->endTag();	
			$writer->endTag();
			$writer->endTag();

			$output = $writer->getOutput();
			$writer->end();
			
			print $output->value; 
			
			socketSend($fh,$output->value);
		}
		##################################################################################################################
		#	Honterer forespørsel om status
		#
		###################################################################################################################
		elsif ($jobb->{'komando'}->[0] eq 'status') {
			print $fh "Ok\n";
		}
		##################################################################################################################
		#	Honterer forespørsel om URLer for kombinert bilde og side nedlasting fra integrerte klienter
		#
		###################################################################################################################
		elsif ($jobb->{'komando'}->[0] eq 'get_sider_for_integrert_nedlasting') {
			#deklarere svar XMLet
			
			
			
			my $output = XML::Writer::String->new();

			my $writer = new XML::Writer(OUTPUT => $output);

			$writer->xmlDecl();

			$writer->startTag('xml');
			
			for my $data (@{$jobb->{'data'}}) {
			print "ba om $data->{'antall'}->[0] sider\n";
				#####################################
				#honterer å sende data til klienten
				if (scalar(@UrlQueue) <= 0){
				#if (1) {
					print "Har bare " . scalar(@UrlQueue) . "urler på kø, klienten får såve\n";
					#$writer->startTag('svar', 'komando' => 'crawl');
					$writer->startTag('error');
						$writer->startTag('generalerror');
							
							$writer->startTag('sleep');
								$writer->characters(5);
							$writer->endTag();
							$writer->startTag('comment');
								$writer->characters('Url server has no more urls. Please come back later.');
							$writer->endTag();
							
						$writer->endTag();	
					$writer->endTag();
				}
				else {
			
					#låser, slik at bare vi har tilgang til databasen
					open(LOCK,">../data/robottxt.lock") or die ("Cant open data/robottxt.lock: $!");
					#print "locking robottxt.lock\n";
					flock(LOCK,2) or die ("Can't lock lock file: $!");
					print LOCK $$;
					
					open(OVERFLOW,">>../data/UrlQueueOVERFLOW") or die ("Cant open ../data/UrlQueueOVERFLOW: $!");
					binmode(OVERFLOW);
					#print "locking UrlQueueOVERFLOW\n";
					flock(OVERFLOW,2) or die ("Can't lock lock file: $!");
					# Reset the file pointer to the end of the file, in case 
					# someone wrote to it while we waited for the lock...
					seek(OVERFLOW,0,2);

					open(BANNET,">>../data/UrlQueueBANNET") or die ("Cant open ../data/UrlQueueBANNET: $!");
                                        binmode(BANNET);
					#print "locking UrlQueueBANNET\n";
                                        flock(BANNET,2) or die ("Can't lock lock file: $!");
                                        # Reset the file pointer to the end of the file, in case
                                        # someone wrote to it while we waited for the lock...
                                        seek(BANNET,0,2);

					open(UDOWN,">>../data/UrlQueueDOWN") or die ("Cant open ../data/UrlQueueDOWN: $!");
					binmode(UDOWN);
                                        #print "locking UrlQueueDOWN\n";
                                        flock(UDOWN,2) or die ("Can't lock lock file: $!");
                                        # Reset the file pointer to the end of the file, in case
                                        # someone wrote to it while we waited for the lock...
                                        seek(UDOWN,0,2);
					
					open(FORBIDDEN,">>../data/UrlQueueFORBIDDEN") or die ("Cant open ../data/UrlQueueFORBIDDEN: $!");
                                        binmode(FORBIDDEN);
					flock(FORBIDDEN,2) or die ("Can't lock lock file: $!");
                                        ## Reset the file pointer to the end of the file, in case
                                        ## someone wrote to it while we waited for the lock...
                                        seek(FORBIDDEN,0,2);

					#opner databasen
					#dbmopen %DBMrobottxt, 'data/robottxt', 0666 or die("Can't open dbm data/robottxt: $!");
					tie %DBMrobottxt, "DB_File", '../data/robottxt' or die("Can't open dbm ../data/robottxt: $!");
					
					#$writer->startTag('svar', 'komando' => 'crawl');
					$writer->startTag('crawlthis');
					
					
					my $count = 0;
					while ($count < $data->{'antall'}->[0]) {
					
						#my $UrlElement = $UrlQueue->dequeue();
						my $UrlElement = pop(@UrlQueue) or last; #hvis vi er tom for urler sender vi de vi eventuelt har
						
						#print "UrlElement: $UrlElement " . scalar(@UrlQueue) . "\n";
				
						#my ($DocID,$url) = split(',',$UrlElement,2);
						my ($url,$DocID) = unpack('A200,I',$UrlElement);			
	
						my $domene = fin_domene($url);
					
						my $DomenesRobottxt = $DBMrobottxt{$domene};
						
						my ($RobotsTxtType,$RobotsTxtData) = unpack('A A*',$DomenesRobottxt);

						print "\ndomene: $domene, ";
						print "RobotsTxtType: $RobotsTxtType ";

						if (length($UrlElement) != UrlQueuePostLength) {
                                                        print "Bad UrlElement only " . length($UrlElement) . " bytes long\n";
                                                }
						elsif (IsBlocketByDomain($domene)) {
							#honterer at en side er blokert
							print " siden ere på blokklisten ";
							print BANNET $UrlElement;
						}
						elsif (exists $sett{$domene}) {
							print ", sett før ($sett{$domene} times)";
							$sett{$domene} = $sett{$domene} +1;

							#putt på overflow
							print OVERFLOW $UrlElement;
							
						}
						elsif ($RobotsTxtType eq '6') {
							print ", Domene var nede når vi spurte om robots.txt fil";
							print UDOWN $UrlElement;
						}
						#print "DomenesRobottxt: $DomenesRobottxt\n";
						#sjekker at vi har sjekket om at domene har robot txt fil, og at det ikke var noen
						elsif (($DomenesRobottxt eq '0') || ($RobotsTxtType eq '3')) {
							
							if ($DomenesRobottxt eq '0') {
								print ", Har testet robot.txt som ikke var der. Crawler";
							}
							elsif ($RobotsTxtType eq '3') {
								print ", har testet robots.txt fil som var for stor. Sikkert bare en 404, crawler ";
							}

							$writer->startTag('crawl');
							$writer->startTag('url');
								$writer->characters($url);
							$writer->endTag();
							#$writer->startTag('dato');
							#		$writer->characters($i->[1]);
							#$writer->endTag();
							$writer->startTag('DocID');
								$writer->characters($DocID);
							$writer->endTag();
							
							$writer->startTag('storage');
								$writer->characters(calkualtStorageServer($DocID));
							$writer->endTag();
							
							#setter om vi skal lage thumbnale eller ikke for siden
                                                        if (MakeThumbnale) {
                                                                $writer->startTag('MakeThumbnale');
                                                                        $writer->characters('TRUE');
                                                                $writer->endTag();

                                                        }
                                                        else {
                                                                $writer->startTag('MakeThumbnale');
                                                                        $writer->characters('FALSE');
                                                                $writer->endTag();
                                                        }


							$writer->endTag();
							
							$count++;
							$sett{$domene} = 1;
						}
						elsif ($RobotsTxtType eq '1') {
						#domene har robots.txt fil, og vi har den på disk
							print "domene har robots.txt fil, og vi hr den på disk, tetster den\n";
	
							my $robotstxturl = "http://$domene/robots.txt";
						
							if (is_RobotsAllowd(RobotName,$RobotsTxtData,$robotstxturl,$url)) {
								
								print "$robotstxturl is_RobotsAllowd for $url\n";
								#print "robots.txt: $RobotsTxtData\n\n";
								
								$writer->startTag('crawl');
								$writer->startTag('url');
									$writer->characters($url);
								$writer->endTag();
								#$writer->startTag('dato');
								#		$writer->characters($i->[1]);
								#$writer->endTag();
								$writer->startTag('DocID');
									$writer->characters($DocID);
								$writer->endTag();
							
								$writer->startTag('storage');
									$writer->characters(calkualtStorageServer($DocID));
								$writer->endTag();
							
								#setter om vi skal lage thumbnale eller ikke for siden
								if (MakeThumbnale) {
									$writer->startTag('MakeThumbnale');
                                                                		$writer->characters('TRUE');
                                                        		$writer->endTag();

								}
								else {
							        	$writer->startTag('MakeThumbnale');
                                                                        	$writer->characters('FALSE');
                                                                	$writer->endTag();
								}
								$writer->endTag();
							
								$count++;
								$sett{$domene} = 1;
							
							}
							else {
								#Blokkert av robots.txt fil. Lagrer i blokkert filen
								print FORBIDDEN $UrlElement;
							}
						}
						elsif (($DomenesRobottxt eq '')  || (($RobotsTxtType eq '2') && ($RobotsTxtData < (time - 3600)))) { #hvis vi ikke har robots.txt fil, eller vi ikke har fåttilbake en (60 min siden vi sente den ut).
							#print "2: $RobotsTxtData < " . (time - 3600) . "\n";

print ", har ikke robots.txt på disk, DomenesRobottxt : $DomenesRobottxt";
print ", time: $RobotsTxtData < " . (time - 3600);

							#print "Har ikke lastet ned robot.txt filen for $domene enda\n";
							#Gir klienten beskjed om å skaffe den
							#skriver post som betyr at vi har sent den ut til fetshin. Slik at ingen andre gjør det.
#print "writeing domain info: $domene\n";
							$DBMrobottxt{$domene} = pack('A A*','2',time);
						
							#ber klienten hente robot.txt filen
							$writer->startTag('robottxt');
							$writer->startTag('url');
								$writer->characters('http://' . $domene . '/robots.txt');
							$writer->endTag();
							
							$writer->startTag('storage');
								$writer->characters(RobotsTxtStorageServer);
							$writer->endTag();
							
							$writer->endTag();
						
							#putter domene ut på køen igjen
							#$UrlQueue->enqueue("$DocID,$url");
							#push(@UrlQueue,"$DocID,$url");
							print OVERFLOW $UrlElement;							

							$count++;
							$sett{$domene} = 1;
						}
						elsif ($RobotsTxtType eq '2') {
						#hvis vi har sent ut beskjed om å fetshe den, men ikke fåt noen
							#temp: print "Venter på robots.txt for $domene, time: $RobotsTxtData < " . (time - 3600) . " overflover";
							print OVERFLOW $UrlElement;
						
						}
						else {
							print "Ble ikke fanget opp. Noe er feil ?\n";
							#skal på over flow
							print OVERFLOW $UrlElement;
						}

						#$sett{$domene} = 1;
					}
				
					#stenger databsen
					#dbmclose (%DBMdatabase) or warn("cant dbmclose: $!");
					untie %DBMrobottxt or die("Can't close dbm ../data/robottxt: $!");
					#undefine %DBMdatabase;
					close(LOCK) or warn ("Canr close: $!");
					
					close(OVERFLOW);
					close(BANNET);
					close(UDOWN);
					close(FORBIDDEN);

					$writer->endTag();
				}
			#####################################
			}
			$writer->endTag();

			$output = $writer->getOutput();
			$writer->end();
			
			#print $output->value; 
			
			socketSend($fh,$output->value);
			
		}
		##################################################################################################################
		# Hontere lagring 
		##################################################################################################################
		elsif ($jobb->{'komando'}->[0] eq 'oppdater') {
		
			print "utfører oppdater\n";
			print "utfører oppdater" . (Time::HiRes::time - $StartTime) . "\n\n";
			
			#opner Reposetory'et
			#Boitho::Reposetory::ropen();
		
			
			my $countCrawl = 0;
			my $countRobotstxt = 0;
			my $countOKCrawl = 0;			
			
			
						
			#finner tiden, siden dette er kostbart kjør vi det bare en gang, i steden for en gang pr side

			my $curentTime = time;
			
			warn "robot.s scalar". scalar(@{$jobb->{'robottxt'}});
						
			#hvis vi har robots.txt filer
			if (scalar(@{$jobb->{'robottxt'}}) != 0) {
				#loser, slik at bare vi har tilgang til databasen
                        	open(LOCK,">../data/robottxt.lock") or die ("Cant open ../data/robottxt.lock: $!");
                        	flock(LOCK,2) or die ("Can't lock lock file: $!");
                        	print LOCK $$;

                        	#opner databasen
                                tie %DBMrobottxt, "DB_File", '../data/robottxt' or die("Can't open dbm ../data/robottxt: $!");

                        

                        	print "kjører gjenom robottxtfiler\n";
			
				for my $robottxtha (@{$jobb->{'robottxt'}}) {
				
					$countRobotstxt++;
					#for robottxt filer
					print "$robottxtha->{'url'}->[0]\n";
				
					my $domene = fin_domene($robottxtha->{'url'}->[0]);
				
					print "Motot data om robots.txt for $domene\n";	
			
					print "Respons: $robottxtha->{'response'}->[0]\n";
					print "Url: $robottxtha->{'url'}->[0]\n";
				
					#ToDo ha med user id
                                        my $userID = 0;
				
			
					if (($robottxtha->{'response'}->[0] >= 199) && ($robottxtha->{'response'}->[0] <= 299)) {
						#er i 200 område, vi har robot.txt fil
				
				
						#dekoder
						#$robottxtha->{'html'}->[0] = uncompress(MIME::Base64::decode($robottxtha->{'html'}->[0]));
						$robottxtha->{'html'}->[0] = MIME::Base64::decode($robottxtha->{'html'}->[0]) or warn('Cant Base64 decode: ' . $!);
						$robottxtha->{'html'}->[0] = uncompress($robottxtha->{'html'}->[0]) or warn("Cant uncompress: $! length vas: " . length($robottxtha->{'html'}->[0]) . " respons was: " . $robottxtha->{'response'}->[0] . " user: " . $userName . ' domain: ' . $robottxtha->{'url'}->[0]);
				
						#hvis robot.txtene er minder en 800 bytes lagres den idatabasen, hvis ikke er det ikke plass, er en grense på 1009 byt for value og key.
						if (length($robottxtha->{'html'}->[0]) < 800) {
							$DBMrobottxt{$domene} = pack('A A*','1',$robottxtha->{'html'}->[0]) or warn($!);
							print "Mindre en 800 bytes, lagrer\n";
						}
						else {
							$DBMrobottxt{$domene} = pack('A','3') or warn($!);
							print "Størren en 800 bytes,\n";
						}
				
						#print "Har robot.txt file: -" . $robottxtha->{'html'}->[0] . "-\n";
						#exit;
					}
					elsif ($robottxtha->{'response'}->[0] >= 600){
						warn("$robottxtha->{'response'}->[0] $robottxtha->{'errormessage'}->[0] svar for $robottxtha->{'url'}->[0]");
						$DBMrobottxt{$domene} = pack('A A*','6',$curentTime) or warn($!);
					}
					else {
						print "404, lagrer som ikke har\n";
					#ingen robot txt fil, 404 303 eller tilsvarende error
						$DBMrobottxt{$domene} = pack('A','0');
				
					}
				}
			
			

			
				#stenger databsen
				untie %DBMrobottxt or die("Can't close dbm ../data/robottxt: $!");
				
				close(LOCK) or warn ("Canr close: $!");
			} #enf if vi har robots.txt's

			print "scalar crawl" . scalar(@{$jobb->{'crawl'}}) . "\n";
			if (scalar(@{$jobb->{'crawl'}}) != 0) {
				#for crawlede sider


				@{$jobb->{'crawl'}} = sort byDocID @{$jobb->{'crawl'}};
				print "etter sort by DocID" . (Time::HiRes::time - $StartTime) . "\n\n";


				for my $crawlha (@{$jobb->{'crawl'}}) {
				
					#print "crawlet $countCrawl " . (Time::HiRes::time - $StartTime) . "\n\n";
					
					$countCrawl++;
					if (($crawlha->{'response'}->[0] >= 199) && ($crawlha->{'response'}->[0] <= 299)) {
						print "motot crawler data for DocID " . $crawlha->{'docid'}->[0] . " html var " . length($crawlha->{'html'}->[0]) ." bytes\n";


						$countOKCrawl++;
			
						#behandler html
						$crawlha->{'html'}->[0] = MIME::Base64::decode($crawlha->{'html'}->[0]) or warn('Cant unbase64: ' . $!);
						#$crawlha->{'html'}->[0] = uncompress($crawlha->{'html'}->[0]) or warn('Cant uncompress ' . $!);
					
						
						#tester om IP adressen er slått opp, hvis den ikke er det slår vi den opp
						if ($crawlha->{'ipaddress'}->[0] eq '') {
							my $ipInBinary = inet_aton($domene);
							
							#tester lengde for å se om vi fikk til å slå den opp
							if (length($ipInBinary) == 4) {
								$crawlha->{'ipaddress'}->[0] = inet_ntoa($ipInBinary);
							}
							else {
								$crawlha->{'ipaddress'}->[0] = '';
							}
						}
					
					
					
					
							
						#tester om vi har noe bilde data. Lagrer hvis vi har
						if ($crawlha->{'data'}->[0] eq '') {
						warn ("did get html, but no image for $crawlha->{'docid'}->[0]\n");
						} 
						else {
							#behandler bilde
                                        		$crawlha->{'data'}->[0] = MIME::Base64::decode($crawlha->{'data'}->[0]) or warn($!);

                                	        	#fra verjon 0.4 bruker vi ikke xlib på bildene
                        	                	if ($clientapplicationversion <= 0.3) {
                	                        	        $crawlha->{'data'}->[0] = uncompress($crawlha->{'data'}->[0]) or warn($!);
        	                                	}
	
						}
					
						print " image length " . length($crawlha->{'data'}->[0]);
				
						Boitho::Reposetory::rApendPost($crawlha->{'docid'}->[0],$crawlha->{'url'}->[0],'htm',$crawlha->{'response'}->[0],$crawlha->{'ipaddress'}->[0],$curentTime,$clientapplicationversion,$userID,$crawlha->{'html'}->[0],length($crawlha->{'html'}->[0]),$crawlha->{'data'}->[0],length($crawlha->{'data'}->[0]),subname);

						#exit;
					}
					else {
						#ikke 200 side
						
						Boitho::Reposetory::rApendPost($crawlha->{'docid'}->[0],$crawlha->{'url'}->[0],'htm',$crawlha->{'response'}->[0],$crawlha->{'ipaddress'}->[0],$curentTime,$clientapplicationversion,$userID,'',0,'',0,subname);
					


					}
				}
			
				#stenger Reposetory'et
				Boitho::Reposetory::rclose();

			}			

			#oppdater statestikk over antall sider dennne brukeren har crawlet
			print "Trying to conecting to mysql server\n";
			my $dbh = MySQLDcConnect::GetHandler();
			print "conectet\n";

			$rv = $dbh->do(qq{
					update downloaded set crawl=crawl + "$countCrawl", robotstxt=robotstxt + "$countRobotstxt" where user= "$userName"
			}) or warn("can´t do statment: ",$dbh->errstr);
			print "query finsis\n";
			
			
		}
		else {
				print "Ukjent komando $jobb->{'komando'}->[0]\n";
				#exit;
		}
	#}

	}	
	else {
		print "protokollversjon: $protokollversjon !\n";

		$totallest = 0;

		#finner tiden, siden dette er kostbart kjør vi det bare en gang, i steden for en gang pr side
                my $curentTime = time;

		$userID = 0;

		if ($protokollversjon == 2) {
			my $header = substr($data_read,0,40);
                	$totallest += 40;

			($komando, $clientapplicationversion,$userName,$userPassword) = unpack('A10 A6 A12 A12',$header);
		}
		elsif ($protokollversjon == 3) {
			my $header = substr($data_read,0,51);
                	$totallest += 51;
			($komando, $clientapplicationversion,$userName,$userPassword, $DedicatedServer, $ServerName) = unpack('A10 A6 A12 A12 A1 A10',$header);
			if ($DedicatedServer eq '1') {
				print "is DedicatedServer, ServerName $ServerName\n";
				get "http://bbh-001.boitho.com/cgi-bin/hartbip/bip.cgi?user=$userName&node=$ServerName";
			}
			else {
				print "is NOT DedicatedServer: $DedicatedServer, ServerName $ServerName\n";
				print "header $header\n";
			}
			$GroupName = '';
		}
		elsif ($protokollversjon == 4) {
			my $header = substr($data_read,0,66);
                	$totallest += 66;
			($komando, $clientapplicationversion,$userName,$userPassword, $DedicatedServer, $ServerName, $GroupName) = unpack('A10 A6 A12 A12 A1 A10 A15',$header);
			if ($DedicatedServer eq '1') {
				print "is DedicatedServer, ServerName $ServerName\n";
				get "http://bbh-001.boitho.com/cgi-bin/hartbip/bip.cgi?user=$userName&node=$ServerName";
			}
			else {
				print "is NOT DedicatedServer: $DedicatedServer, ServerName $ServerName\n";
				print "header $header\n";
			}


		}
		elsif ($protokollversjon == 5) {
			my $header = substr($data_read,0,71);
                	$totallest += 71;
			($komando, $clientapplicationversion,$userName,$userPassword, $DedicatedServer, $ServerName, $GroupName) = unpack('A10 A6 A12 A12 A1 A10 A20',$header);
			if ($DedicatedServer eq '1') {
				print "is DedicatedServer, ServerName $ServerName\n";
				get "http://bbh-001.boitho.com/cgi-bin/hartbip/bip.cgi?user=$userName&node=$ServerName";
			}
			else {
				print "is NOT DedicatedServer: $DedicatedServer, ServerName $ServerName\n";
				print "header $header\n";
			}


		}
		else {
			die("ukjent protokolversjon: $protokollversjon");
		}

		print "hedder: $komando, $clientapplicationversion,$userName,$userPassword\n";

			if ($komando eq 'oppdater') {

        	                print "utfører oppdater " . (Time::HiRes::time - $StartTime) . "\n";


                        	my $countCrawl = 0;
                        	my $countRobotstxt = 0;
				my $countOKCrawl = 0;

				$robotstxtinalisert = 0;
				$crawlinalisert = 0;
				while ($totallest < $maxlen_read) { 
			
					$sideheder = substr($data_read,$totallest,253);
					$totallest += 258;

		
					my %side = {};
					 ($side{type}, $side{DociD}, $side{Respons}, $side{Url}, $side{IPAddress}, $side{htmllength}, $side{imagelength}) = unpack('A10 A10 A3 A200 A15 A10 A10', $sideheder);	

					if ($side{htmllength} != 0) {		
						$html = substr($data_read,$totallest,$side{htmllength});
						$totallest += $side{htmllength};
					}

					if ($side{imagelength} != 0) {
						$image = substr($data_read,$totallest,$side{imagelength});
						$totallest += $side{imagelength};
	
					}
	
					print "type: $side{type}\n, DociD: $side{DociD}\n, Respons: $side{Respons}\n, Url: $side{Url}\n IPAddress: $side{IPAddress}\n htmllength: $side{htmllength}\n magelength: $side{imagelength}\n";
			
					if ($side{type} eq 'crawl') {

                                        
						$crawlinalisert = 1;

                                        	$countCrawl++;
                                        	if (($side{Respons} >= 199) && ($side{Respons} <= 299)) {
                                                	print "motot crawler data for DocID " . $side{DociD};

							$countOKCrawl++;
	                                                #tester om vi har noe bilde data. Lagrer hvis vi har
                                                
                	                        
							print "image length $side{imagelength} ,Faktisn inage length: " . length($image) ."\n";
						}
                        	                else {
                                	                #ikke 200 side
                                                
							
							#Boitho::Reposetory::rApendPost($side{DociD},$side{Url},'htm',$side{Respons},$side{IPAddress},$curentTime,$clientapplicationversion,$userID,'',0,'',0);


                                        	}

							print "rApendPost start\n";
							#lagrer info uanset hva status er. 301 og 302 putter info i htmlen om redirekt adresse
							Boitho::Reposetory::rApendPost($side{DociD},$side{Url},'htm',$side{Respons},,$side{IPAddress},$curentTime,$clientapplicationversion,$userID,$html,$side{htmllength},$image,$side{imagelength},subname);
					
							print "rApendPost end\n";

                                	}
					elsif ($side{type} eq 'robottxt') {
						if (!$robotstxtinalisert) {

							#loser, slik at bare vi har tilgang til databasen
                                			open(LOCK,">../data/robottxt.lock") or die ("Cant open ../data/robottxt.lock: $!");
                                			flock(LOCK,2) or die ("Can't lock lock file: $!");
                                			print LOCK $$;

                                			#opner databasen
                                			tie %DBMrobottxt, "DB_File", '../data/robottxt' or die("Can't open dbm ../data/robottxt: $!");

			
							$robotstxtinalisert = 1;
						}

                                		$countRobotstxt++;

                                        	#for robottxt filer

                                        	my $domene = fin_domene($side{Url});

                                        	print "Motot data om robots.txt for $domene\n";

                                        	print "Respons: $side{Respons}\n";
                                        	print "Url: $side{Url}\n";

						if (($side{Respons} >= 199) && ($side{Respons} <= 299)) {
                                                	#er i 200 område, vi har robot.txt fil

							#robots.txt filen er komprimert, og må unkomprimeres først
							$html = uncompress($html) or warn("cant uncompress: $!");

                                                	#hvis robot.txtene er minder en 800 bytes lagres den idatabasen, hvis ikke er det ikke plass, 
							#så vi lagrer bare de første 799 bytene. Dette får å takle store robots.txt filer, som 
							#webmasterworld, som har en blog i sin
							#fra og med 11.04.07 2024 bytes
                                                	if ($side{htmllength} < 2024) {
                                                        	$DBMrobottxt{$domene} = pack('A A*','1',$html) or warn($!);
                                                        	print "Mindre en 2024 bytes, lagrer\n";
                                                	}
                                                	else {
                                                        	#$DBMrobottxt{$domene} = pack('A','3') or warn($!);
                                                        	$DBMrobottxt{$domene} = pack('A A2024','1',$html) or warn($!);
 								print "Størren en 2024 bytes,\n";
                                                	}

       
                                        	}
                                        	elsif ($side{Respons} >= 600){
             
                                                	$DBMrobottxt{$domene} = pack('A A*','6',$curentTime) or warn($!);
                                        	}
                                        	else {
                                                	print "404, lagrer som ikke har\n";
                                        		#ingen robot txt fil, 404 303 eller tilsvarende error
                                                	$DBMrobottxt{$domene} = pack('A','0');

                                        	}

	                                }
					else {
						warn ("ukjent side type \"$side{type}\"");
					}

				}

				#hvis vi har opnet robots.txt ting stenger vi de
				if ($robotstxtinalisert) {
					#stenger databsen
	                                untie %DBMrobottxt or die("Can't close dbm ../data/robottxt: $!");

        	                        close(LOCK) or warn ("Canr close: $!");

				}

				if ($crawlinalisert) {
					#stenger Reposetory'et
        	                        Boitho::Reposetory::rclose();
				}
				#oppdater statestikk over antall sider dennne brukeren har crawlet
                        		print "Trying to conecting to mysql server\n";
                        	my $dbh = MySQLDcConnect::GetHandler();
                        		print "conectet\n";

                        	$rv = $dbh->do(qq{
                                        update downloaded set crawl=crawl + "$countCrawl", robotstxt=robotstxt + "$countRobotstxt" where user= "$userName"
                        	}) or warn("can´t do statment: ",$dbh->errstr);

				$rv = $dbh->do(qq{
					insert into okdownloaded values(null,"$userName","$countOKCrawl","$countRobotstxt",now())
				}) or warn("can´t do statment: ",$dbh->errstr);


				if ($GroupName ne '') {
					$rv = $dbh->do(qq{
                                        	update groupsdownloaded  set crawl=crawl + "$countCrawl", robotstxt=robotstxt + "$countRobotstxt" where user="$userName" AND groupname="$GroupName"
                                	}) or warn("can´t do statment: ",$dbh->errstr);
				}
                        	print "query finsis\n";


			}
	}

#temp:
#       my $overflowData = <$fh>;
#
#       print "overflowData: -$overflowData-\n";

	#sover litt så vi er sikker på at alt ble sent fra bufferene.
	#sleep(1);
#stenger ned sokketen,
#close $fh;
	
	#dør pent, uten dette får vi memory lekasje da anatal tråder hoper seg opp. De ser ikke ut til å dø selv om de går ut av scop
	#$self = threads->self;
	#$self->detach;
	#exit;

#alarm 0;
#}; #else eval/alarm 
#if ($@) {
#        warn unless $@ eq "alarm\n";   # propagate unexpected errors
#        #die unless $@ eq "alarm\n";   # propagate unexpected errors
#        # timed out
#
#}

}

sub socketSend {
	my($socket,$data) = @_;
	
	
	#print $socket pack('i',length($data)) . $data;
	
	#print $socket sprintf('%10i', length($data) + 10) . $data;
	#bruker eval for å forhindre at vi klikker hvis send ikke gikk. Fikk "send: Cannot determine peer address at mymod.pm line 146" feil en skjelden gang her
	#eval { $socket->send(sprintf('%10i', length($data) + 10) . $data) }; warn $@ if $@;
	my $out = sprintf('%10i', length($data) + 10) . $data;
	#print "sender: $out\n";
	
	eval { print $socket $out}; warn $@ if $@;
	
	sleep(5); #gir tid til å skrive ut data
	
}

#sub calkualtStorageServer {
#	my $DocID = shift;
#	my $lot = Boitho::Lot::rLotForDOCid($DocID);
#
#	#husk vi bruker IP her, ikke hostname !!!!!
#
#	if ($lot > 595) {
#		#bbs-002.boitho.com
#                return '213.179.58.55';
#	}
#	elsif ($lot > 383) {
#                #bbs-003.boitho.com
#                return '213.179.58.56';
#        }
#	elsif ($lot > 252) {
#		#bbs-002.boitho.com
#		return '213.179.58.55';
#	}
#	else {	
#		#bbs-001.boitho.com
#		return '213.179.58.52';
#	}
#
#	#return Boitho::Lot::lotlistGetServer($lot);
#
#}

###############################################################################
# baserer seg på å ha en global cashe av StorageServer fro loter. Følgen må erklæres i main programet
#my %lotStorageServerCashe = {};
#
sub calkualtStorageServer {
        my $DocID = shift;
        my $lot = Boitho::Lot::rLotForDOCid($DocID);

        my $ipaddr = '';

        if (not exists $lotStorageServerCashe{$lot}) {

                #husk vi bruker IP her, ikke hostname !!!!!
                my $hostname =  Boitho::Lot::lotlistGetServer($lot);

                #henter ip
                $ipaddr = inet_ntoa(inet_aton($hostname));

                $lotStorageServerCashe{$lot} = $ipaddr;

        }
        else {
                print "got lot Storage Server ip from cashe\n";
                $ipaddr = $lotStorageServerCashe{$lot};
        }

        return $ipaddr;

}



sub byDocID {
	$a->{'docid'}->[0] <=> $b->{'docid'}->[0];
}

###################################################################################
# Rutine for å teste om et domene er i bannlista
###################################################################################
sub IsBlocketByDomain() {
        my $domain = shift;
        my $ReturnStatus = 0;

        my @elements = split(/\./,$domain);     #splitter opp domene i enkeltdele
        @elements = reverse(@elements);

        my $forTest = '';
        foreach my $s (@elements) {
                #print "$s\n";

                if ($forTest ne '') {
                        $forTest = $s . '.' . $forTest;
                }
                else {
                        $forTest = $s;
                }

                if ($DomainBlockList{$forTest}) {
                        $ReturnStatus = 1;
                        print "$forTest er bannet\n";
                }
                #print "forTest $forTest\n";
        }

        return $ReturnStatus;
}
#########################################################################
#laster listen over sider vi blokkerer
#########################################################################
sub loadeBaqnnList {
        my %DomainBlockList = {};

        #leser inn alle sidene som er blokert
        open(INF,'../data/bann_list.txt') or die($!);
        while (<INF>) {

                chomp;
                s/#.*//;        #no comments
                s/^\s+//;       #no leading white
                s/\s+$//;       #no traling white
                #print "-$_- \n";

                $DomainBlockList{$_} = 1;
        }
        close(INF);

        return %DomainBlockList;
}

