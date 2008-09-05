####################################################################
#settup
#####
#MySQL settup
$user = "boitho";
$Password = "G7J7v5L5Y7";
#$server = "localhost";
$server = "web1.jayde.boitho.com";
$database = "boithoweb";



#$tabel = "submission_url";
#
#my $subname = 'freelistning';

use constant DEBUG => 1;

my $urlAtATime = 250;

my $orderby = 'order by rand()';
#my $orderby = 'order by id';


use IR qw(ResulveUrl);

use DBI; #bruker DBI databse interfase
use Compress::Zlib;
use Boitho::Reposetory;
use Boitho::Lot;



require LWP::Parallel::UserAgent;
  use HTTP::Request; 

  # persistent robot rules support. See 'perldoc WWW::RobotRules::AnyDBM_File'
  require WWW::RobotRules::AnyDBM_File;
  
  require LWP::Parallel::RobotUA;

  # establish persistant robot rules cache. See WWW::RobotRules for
  # non-permanent version. you should probably adjust the agentname
  # and cache filename.
  my $rules = new WWW::RobotRules::AnyDBM_File 'ParallelUA', '/tmp/boitho_robottxt_cache';




use Time::HiRes;

my $tabel = shift(@ARGV) or usage ("You must specify table");
my $subname = shift(@ARGV) or usage ("You must specify subname");


my $lasttime = Time::HiRes::time;
print "\n\nStarter å beansmarke\n";
my $count = 0;  

my @reqs;
#my $DocID;

		# dette kjøres hver gang mymod.pm kalles
		my $dbh = DBI->connect("DBI:mysql:database=$database;host=$server;port=3306",						#Kobler til databasen
                             $user, $Password) or warn("Can`t connect: $DBI::errstr");	#
							 
		
		if ($subname eq 'freelistning') {
			$sth = $dbh->prepare("select WWWDocID,url,id from $tabel WHERE WWWDocID is not NULL AND (last_indexed = 0 OR last_indexed is NULL) limit 500000") or dienice("Can`t prepare statment: ", $dbh->errstr);
			#$sth = $dbh->prepare("select WWWDocID,url,id from $tabel WHERE WWWDocID is not NULL limit 500000") or dienice("Can`t prepare statment: ", $dbh->errstr);
			$rv = $sth->execute;
		}
		else {
			$sth = $dbh->prepare("select WWWDocID,url,id from $tabel WHERE WWWDocID is not NULL limit 500000") or dienice("Can`t prepare statment: ", $dbh->errstr);
			$rv = $sth->execute;
		}


		while (($DocID,$url,$id) = $sth->fetchrow_array) {


			#bug: skal vi gjøre dette her, eller under innlegelsen?
			$url = ResulveUrl("http://www.boitho.com/addurl.html",$url);


			if ($subname eq 'freelistning') {
        	                $rv = $dbh->do(qq{
                                        update $tabel set last_indexed=NOW() where id="$id"
	                        }) or warn("can´t do statment: ",$dbh->errstr);	
			}
			else {
        	                $rv = $dbh->do(qq{
                                        update $tabel set crawler_fetched=NOW() where id="$id"
	                        }) or warn("can´t do statment: ",$dbh->errstr);	
			}

	    		print "$url, nr $count\n";


			my %element;
			$element{'url'} = $url;			
			$element{'DocID'} = $DocID;			
			$element{'ID'} = $id;			
		
			#$DocID++;

			push(@reqs,\%element);

			if ($count > $urlAtATime) {
				print "last. count $count\n";
				#last;
				eval {
					crawlSomeUrls($rules,\@reqs);
				}; #else eval/alarm
				if ($@) {
        				warn;   # propagate unexpected errors
				}




				@reqs = ();
				$count = 0;
			}

			$count++;

		}

		#og siste
		crawlSomeUrls($rules,\@reqs);

		#/laster inn urler


  print "done loading\n";


  print "Ferdig: " . (Time::HiRes::time - $lasttime) . "\n";




sub crawlSomeUrls {

	my ($rules,$reqs) = @_;

  	my ($req,$res);
  	my %DocIDToUrlHash;

	my $curentTime = time;

  	# create new UserAgent (actually, a Robot)
  	my $pua = new LWP::Parallel::RobotUA ("boitho.com-robot/3.2 ( http://www.boitho.com/bot.html )", 'abuse@boitho.com', $rules);


       	$pua->timeout   (5);  # in seconds
       	$pua->delay    ( 0);  # in seconds
       	$pua->max_req  ( 1);  # max parallel requests per server
       	$pua->max_hosts(20);  # max parallel servers accessed
       	$pua->redirect  (10);   #tilater ikke redirekts


  	# for our own print statements that follow below:
  	local($\) = ""; # ensure standard $OUTPUT_RECORD_SEPARATOR

  	# register requests
  	foreach $element (@{ $reqs }) {

		$req = new HTTP::Request(GET => %{ $element }->{'url'});
		$DocIDToUrlHash{%{ $element }->{'url'}} = %{ $element }->{'DocID'};

  		print "Registering '".$req->url."', DocID: ",$DocIDToUrlHash{%{ $element }->{'url'}},"\n";
  		$pua->register ($req , \&handle_answer);
  		#  Each request, even if it failed to # register properly, will
  		#  show up in the final list of # requests returned by $pua->wait,
  		#  so you can examine it # later.
  	}
  	print "done Registering\n";

  	# $pua->wait returns a pointer to an associative array, containing
  	# an '$entry' for each request made, sorted by its url. (as returned
  	# by $request->url->as_string)
  	my $entries = $pua->wait(); # give another timeout here, 25 seconds

	print "back from pua->wait()\n";

	# let's see what we got back (see also callback function!!)
	foreach (keys %$entries) {
    		$res = $entries->{$_}->response;

    		# examine response to find cascaded requests (redirects, etc) and
    		# set current response to point to the very first response of this
    		# sequence. (not very exciting if you set '$pua->redirect(0)')
    		my $r = $res; my @redirects;
    		while ($r) { 
			$res = $r; 
			$r = $r->previous; 
			push (@redirects, $res) if $r;
    		}
    
    
		#ToDO: $res->request er av og til "undef", hvorfor ? (sjekker for det nå, men hvorfor skjer det)
		if (defined $res->request) {
			# summarize response. see "perldoc HTTP::Response"
    			print "Answer for ",$res->request->url," DcoID: ", $DocIDToUrlHash{$res->request->url}," was \t '",$res->code,"': ", $res->message, "\n";
	
			#$ferdig_side{'url'} = $res->request->url;
			#$ferdig_side{content_type} = $res->content_type; 
			#$ferdig_side{'response_code'} = $res->code;
			#$ferdig_side{response_content} = $res->content;
		
			#print "<side>\n";
			#print "	<head>\n";
			#print "	URL: " . $res->request->url . "\n";
			#print "	content_type: ". $res->content_type . "\n";
			#print "	response_code: " . $res->code . "\n";
			#print "	</head>\n";
			#print "	<content>\n";
			#print  	$res->content . "\n";
			#print "	</content>\n";
			#print "</side>\n";

			my $html_compressed = compress($res->content);

			my $ipaddress = "0.0.0.0";
			my $clientapplicationversion = "0.1";
			my $userID = "internal";
			my $image = '';

			my $DocID = $DocIDToUrlHash{$res->request->url};
			my $response = $res->code;

			#hvis vi ikke har urlen i db sjekker vi om vi har url som er lik, men med en / . Kan ha blitt lagt på en /, uten at det er en redirect
			if ((!exists($DocIDToUrlHash{$res->request->url}) ) && ( exists($DocIDToUrlHash{$res->request->url . '/'}) ) ) {
				$DocIDToUrlHash{$res->request->url} .= '/';
			}

			if (!exists($DocIDToUrlHash{$res->request->url})) {
				print "don't have a DocID for ",$res->request->url,"\n";
				#exit;
			}

			print "appening DocID: ", $DocIDToUrlHash{$res->request->url}, ", url: ", $res->request->url, ", res: $response\n";

			Boitho::Reposetory::rApendPost($DocIDToUrlHash{$res->request->url},$res->request->url,'htm',
				$res->code,$ipaddress,$curentTime,$clientapplicationversion,$userID,$html_compressed,
				length($html_compressed),$image,length($image),$subname);

				if ($subname eq 'freelistning') {
				}
				else {
        	        	        $rv = $dbh->do(qq{
                	                        update $tabel set  http_response="$response" where WWWDocID="$DocID"
		                        }) or warn("can´t do statment: ",$dbh->errstr);	
				}

			
		}
		
	    	# print redirection history, in case we got redirected
	    	foreach (@redirects) {
			print "\t",$_->request->url, "\t", $_->code,": ", $_->message,"\n";
			print "        content_type: ". $res->content_type . "\n";
			print          $res->content . "\n";
	    	}
  	
	}


}  

  
  # our callback function gets called whenever some data comes in
  # (same parameter format as standard LWP::UserAgent callbacks!)
sub handle_answer {
    my ($content, $response, $protocol, $entry) = @_;

	if (DEBUG) {
    		print "Handling partial answer from '",$response->request->url,": ", length($content), " bytes, Code ", $response->code, ", ", $response->message,"\n";
	}
	
    if (length ($content) ) {
	# just store content if it comes in
	$response->add_content($content);
    } else {
        # Having no content doesn't mean the connection is closed!
        # Sometimes the server might return zero bytes, so unless
        # you already got the information you need, you should continue
        # processing here (see below)
        
	# Otherwise you can return a special exit code that will
        # determins how ParallelUA will continue with this connection.

	# Note: We have to import those constants via "qw(:CALLBACK)"!

	# return C_ENDCON;  # will end only this connection
			    # (silly, we already have EOF)
	# return C_LASTCON; # wait for remaining open connections,
			    # but don't issue any new ones!!
	# return C_ENDALL;  # will immediately end all connections
			    # and return from $pua->wait
    }

    # ATTENTION!! If you want to keep reading from your connection,
    # you should have a final 'return undef' statement here. Even if
    # you think that all data has arrived, it does not hurt to return
    # undef here. The Parallel UserAgent will figure out by itself
    # when to close the connection!

    return undef;	    # just keep on connecting/reading/waiting 
                            # until the server closes the connection. 
}






sub usage {
	my $message = shift;

	print "\n", $message, "\n";
	print "Usage: picrawl.pl table subname\n\n";
	print "eks picrawl.pl submission_url freelistning\n";
	print "eks picrawl.pl pi_sider paidinclusion\n";
	exit;
}

