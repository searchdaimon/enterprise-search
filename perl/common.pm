package common;
require Exporter;
@common::ISA = qw(Exporter);
@common::EXPORT = qw();
@common::EXPORT_OK = qw(fin_domene MakePath count_handler gyldig_url GetImageLocation MAKE_PATH GetAllLotPaths find_domain_no_subname);

use strict;

use POSIX;


############################################################################################################################
# finds the domain for a url
############################################################################################################################
sub fin_domene {
	my($domene) = @_;
	
	$domene =~ s/http:\/\///;
	
	$domene =~ s/\/.*//g;
	
	return $domene;
}
############################################################################################################################
# finds the domain for a url without the subname
############################################################################################################################
sub find_domain_no_subname {
        my($domene) = @_;

        $domene =~ s/http:\/\///;

        $domene =~ s/\/.*//g;

        $domene =~ s/([^.]+\.[^.]+$)//;
        $domene = $1;

        return $domene;
}


############################################################################################################################
# creates the necessary directories
############################################################################################################################
sub MakePath {
	my($path) = @_;
	my($path_komet_til) = '';
	
	#print "oppretter $path\n";
	
	system("mkdir -p $path");

	#my @mapper = split('/', $path);
	#
	#reverse(@mapper);
	#
	#foreach my $i (@mapper) {
	#	$path_komet_til .= $i . '/';
	#	#print "$i \n";
	#	if (not -e $path_komet_til) {
	#		mkdir($path_komet_til) or die("kunne ikke opprette $path_komet_til: $!");
	#	}
	#}
}

############################################################################################################################
# increments and decrements counters, returns the new count. Use 0 as the update to get the value without updating
############################################################################################################################
sub count_handler {
	my($counter,$verdi) = @_;
	my($count) = 0;
	
		#print "$verdi \n";
		#find the file
		my $file = 'data/count/' . $counter;
		
		open(COUNTFILE,"+<$file") or open(COUNTFILE,">$file") or die("Can't open $file: $!");
		flock(COUNTFILE,2);
		binmode(COUNTFILE);
	
		#seek to the beginning
		seek(COUNTFILE,0,0) or die "Seeking: $!";
		
		#if the file has a value we read it, otherwise we set $count to 0
		if (not -z COUNTFILE) {
			read(COUNTFILE,$count,-s COUNTFILE)
		}
		else {
			$count = 0;
			warn('mymod-count_handler:',"\"$counter\" har ingen verdi, så sat den til 0");
		}
		
		
		#if we did not just want the value (0 as the value), we update it
		if ($verdi != 0) {
			#seek back to the beginning
			seek(COUNTFILE,0,0) or die "Seeking: $!";
		
			#add together
			$count = $count + $verdi;
			#write back to the counter file
			print COUNTFILE $count;
		}
		
		close(COUNTFILE);
		
		return $count;
}

###############################################################################################################
#
###############################################################################################################
sub gyldig_url {
        my($url) = @_;

	#find the domain
	my $domain = $url;
	$domain =~ s/http:\/\///;
        $domain =~ s/(\/.*)//g;
	my $path = $1;

	#last sub domain
	my $lastSub = $domain;
	$lastSub =~ s/(\..*)//g;

	my @domainElements = split(/\./,$domain);
	#find the tld, i.e. things like .com, .net (co.uk becomes .uk). Check this for whether
	#it contains numbers further down. If it does, it is an ip address.
	@domainElements = reverse(@domainElements);
	my $tdl = $domainElements[0];
	#print "tdl: $tdl - $url\n";

        #find the depth in this URL
        #my $url_for_testing = $url;
        #my $dybde = ($url_for_testing =~ tr/\///);

        #count the number of occurrences of "http:/" (not http:// with two //, since some, like kvasier, use / not // (incredibly enough)) to avoid urls that are just redirects to others
        my $httpcount = 0;
        my $url_for_testing = $url;
        while ($url_for_testing =~ /http:\//g) { $httpcount++ }

	my $equalcount = 0;
	        $url_for_testing = $url;
        while ($url_for_testing =~ /=/g) { $equalcount++ }

	if (($url =~ /=print/) || ($url =~ /print=/)) {
		#print "NO. hav =print in url\n";
		return 0;
	}
	elsif($lastSub =~ /[0-9]/) {
		#do not want to include pages with a number in the last subdirectory. This is typically
		#server X of several servers, e.g. www3.nrk.no, and www.nrk.no is the same
		#
		#ToDo: does this also remove pages with a number in the main domain? 8 Feb, Runarb: Yes, if we only have a domain, not a subdomain. Triggers on 24so.com, but not on www.24so.com
	
		#print "Number i last sub: $url\n";
		return 0;
	}
        elsif (
           (length($url) < 199) #no more than 199 characters, leaving room for \0 in the 200-character area we use
                && (length($url) > 10) #no fewer than 10 characters
                && ($url !~ /\.\./) #does not contain ..
		&& ($url !~ /#/) #does not contain #
                && ($url =~ /(\?|\/$|.cfm$|.htm$|.html$|.shtml$|.shtm$|.php$|.jsp$|.asp$|.stm$|.ece$|.aspx$|.do$)/) #prevents us from getting anything other than links ending in /, .htm or .html or dynamic ones
                && ($url =~ /^http:\/\//) #must begin with http://, not for example ftp:// or https://
                && ($httpcount == 1) #bare har 1 http:// i seg
		&& ($url !~ /sid=/i) #does not contain a session id (this surely does not catch everything), case insensitive
		&& ($equalcount < 2) #does not contain more than one =, otherwise it is too complex a dynamic page
		&& ($url !~ /[^a-zA-Z0-9\.\?=:\/\-_~&\+,]/) #contains characters other than a-z A-Z 0-9 . ? = : / - _ ~ &
		&& ($tdl =~ /[a-z]/) #is not an ip address. Checks whether the tdl contains anything other than a-z
		&& ($path !~ /index\./) #does not contain index. , could be things like index.html
		&& ($path !~ /default\./) # does not contain default. such as default.asp
		&& ($#domainElements > 1) #domain has only 2 parts. Like vg.no, which typically also has a www.vg.no
	        #&& ($url =~ /\.no\//) #must be Norwegian (have .no)
        ) {
                return 1;
        }
        else {
                return 0;
        }
}


###############################################################################################################
#
###############################################################################################################
sub GetImageLocation {
	my($DocID) = @_;
	
	my $path = 'data/images/' . fmod($DocID, 64) . '/' . fmod($DocID, 20);
	my $file = $DocID . '.jpg';
	
	return ($path,$file);
}

############################################################################################################################
# creates the necessary directories
############################################################################################################################
sub MAKE_PATH {
	my($path) = @_;
	my($path_komet_til) = '';
	
	#print "oppretter $path\n";
	
	my @mapper = split('/', $path);
	
	reverse(@mapper);
	
	foreach my $i (@mapper) {
		$path_komet_til .= $i . '/';
		#print "$i \n";
		if (not -e $path_komet_til) {
			mkdir($path_komet_til) or die("kunne ikke opprette $path_komet_til: $!");
		}
	}
}
############################################################################################################################
#returns the path to all the lots that exist in the lot system on this server.
############################################################################################################################
sub GetAllLotPaths {
	my $resurs = shift;
	use File::Find;

	my @allLots = ();

	my $maplistFile;
	if (getenv('MAPLIST') ne '') {
		$maplistFile = getenv('MAPLIST')
	}
	else {
		$maplistFile = '/home/boitho/config/maplist.conf';
	}

	open(INF,$maplistFile) or die("common.pm: Cant open $maplistFile: $!");

	my @a = <INF>;

	foreach my $MainlotPath (@a) {
		chomp($MainlotPath);
	
		#print "$MainlotPath\n";
	
		if (-e $MainlotPath) {
			#now a lot must have a repository to be included. Is that correct?
			my $comand = "ls -1 -d $MainlotPath/*/$resurs";
	
			#print "comand: $comand\n";
			my $output = `$comand`;
	
			my @lotpath = split("\n",$output);
	
			foreach my $lotPath (@lotpath) {
				$lotPath =~ s/\/$resurs//i;
				#print "-$lotPath-\n";
				push(@allLots,$lotPath);
			}
	
		}
	
	}
	close(INF);
	
	return @allLots;
}
