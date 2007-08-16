use Compress::Zlib;

use Boitho::Reposetory;


require LWP::Parallel::UserAgent;
  use HTTP::Request; 

  # persistent robot rules support. See 'perldoc WWW::RobotRules::AnyDBM_File'
  require WWW::RobotRules::AnyDBM_File;
  
  require LWP::Parallel::RobotUA;

  # establish persistant robot rules cache. See WWW::RobotRules for
  # non-permanent version. you should probably adjust the agentname
  # and cache filename.
  my $rules = new WWW::RobotRules::AnyDBM_File 'ParallelUA', 'cache';


my $subname = 'freelistning';

my $DABUG = 0;
my $urlAtATime = 250;

use constant UrlQueuePostLength => 204;

use Time::HiRes;

my $lasttime = Time::HiRes::time;
print "\n\nStarter å beansmarke\n";
my $count = 0;  

my @reqs;
#my $DocID;

  		#laster inn urler
		open(INF,"submission_url.crawl") or die("Can't open inn.test: $!");

		#foreach $line (@ary) {

   		#	chomp($line);

		while (read(INF,$post,UrlQueuePostLength)) {

                        my ($url,$DocID) = unpack('A200,I',$post);


	    		print "$url, nr $count\n";
			if ($url eq '') {
				print "emty url in file\n";
				next;
			}


			my %element;
			$element{'url'} = $url;			
			$element{'DocID'} = $DocID;			
			#$DocID++;

			push(@reqs,\%element);

			if ($count > $urlAtATime) {
				print "last. count $count\n";
				#last;
				crawlSomeUrls($rules,\@reqs);

				@reqs = ();
				$count = 0;
			}

			$count++;

		}

		#/laster inn urler

		close(INF);

  print "done loading\n";


  print "Ferdig: " . (Time::HiRes::time - $lasttime) . "\n";


sub crawlSomeUrls {

	my ($rules,$reqs) = @_;

  	my ($req,$res);
  	my %DocIDToUrlHash;

	my $curentTime = time;

  	# create new UserAgent (actually, a Robot)
  	my $pua = new LWP::Parallel::RobotUA ("boitho.com-robot/3.1 ( http://www.boitho.com/bot.html )", 'abuse@boitho.com', $rules);

  	$pua->timeout   (5);  # in seconds
  	$pua->delay    ( 0);  # in seconds
  	$pua->max_req  ( 1);  # max parallel requests per server
  	$pua->max_hosts(20);  # max parallel servers accessed
  	$pua->redirect  (0);	#tilater ikke redirekts
 
  
  
  	# for our own print statements that follow below:
  	local($\) = ""; # ensure standard $OUTPUT_RECORD_SEPARATOR

  	# register requests
  	foreach $element (@{ $reqs }) {

		$req = new HTTP::Request(GET => %{ $element }->{'url'});
		$DocIDToUrlHash{%{ $element }->{'url'}} = %{ $element }->{'DocID'};

  		print "Registering '".$req->url."'\n";
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

Boitho::Reposetory::rApendPost($DocIDToUrlHash{$res->request->url},$res->request->url,'htm',
$res->code,$ipaddress,$curentTime,$clientapplicationversion,$userID,$html_compressed,
length($html_compressed),$image,length($image),$subname);
			
		}
		
	    	# print redirection history, in case we got redirected
	    	foreach (@redirects) {
			print "\t",$_->request->url, "\t", $_->code,": ", $_->message,"\n";
	    	}
  	
	}


}  

  
  # our callback function gets called whenever some data comes in
  # (same parameter format as standard LWP::UserAgent callbacks!)
sub handle_answer {
    my ($content, $response, $protocol, $entry) = @_;

	if ($DEBUG) {
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



