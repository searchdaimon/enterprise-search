package common;
require Exporter;
@common::ISA = qw(Exporter);
@common::EXPORT = qw();
@common::EXPORT_OK = qw(fin_domene MakePath count_handler gyldig_url GetImageLocation MAKE_PATH GetAllLotPaths find_domain_no_subname);

use strict;

use POSIX;


############################################################################################################################
# finner domene for en url
############################################################################################################################
sub fin_domene {
	my($domene) = @_;
	
	$domene =~ s/http:\/\///;
	
	$domene =~ s/\/.*//g;
	
	return $domene;
}
############################################################################################################################
# finner domene for en url uten subname
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
# oppretter de nødvendige mappene
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
# øker og misker tellere, retunerer det nye antallet. Bruk 0 som oppdatering for å få verdien uten å oppdatere
############################################################################################################################
sub count_handler {
	my($counter,$verdi) = @_;
	my($count) = 0;
	
		#print "$verdi \n";
		#finner filen
		my $file = 'data/count/' . $counter;
		
		open(COUNTFILE,"+<$file") or open(COUNTFILE,">$file") or die("Can't open $file: $!");
		flock(COUNTFILE,2);
		binmode(COUNTFILE);
	
		#søker til begyndelsen
		seek(COUNTFILE,0,0) or die "Seeking: $!";
		
		#hvis filen har en verdif leser vi den, hvis ikke seter vi $count til 0
		if (not -z COUNTFILE) {
			read(COUNTFILE,$count,-s COUNTFILE)
		}
		else {
			$count = 0;
			warn('mymod-count_handler:',"\"$counter\" har ingen verdi, så sat den til 0");
		}
		
		
		#hvis vi ikke bare ville ha verdien (0 som verdi), oppdateerer vi
		if ($verdi != 0) {
			#søker tilbake til begyndelsen
			seek(COUNTFILE,0,0) or die "Seeking: $!";
		
			#legger sammen
			$count = $count + $verdi;
			#skriver tilabek til teller filen
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

	#finner domene
	my $domain = $url;
	$domain =~ s/http:\/\///;
        $domain =~ s/(\/.*)//g;
	my $path = $1;

	#last sub domain
	my $lastSub = $domain;
	$lastSub =~ s/(\..*)//g;

	my @domainElements = split(/\./,$domain);
	#finner tld, altså ting som .com, .net (co.uk blir .uk). Sjekker dette for om 
	#det inneholder tall lengere ned. Hvis det gjør det er det en ip adresse.
	@domainElements = reverse(@domainElements);
	my $tdl = $domainElements[0];
	#print "tdl: $tdl - $url\n";

        #finner dybden i denne URLen
        #my $url_for_testing = $url;
        #my $dybde = ($url_for_testing =~ tr/\///);

        #teller antal forekomster av "http:/" (ikke http:// med to // da noen, som kvasier bruker / ikke // (utrolig nokk)) får å ungå urler som bare er vidresendig til andre
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
		#vil ikke ha med sider der vi har et tall i siste subdirektory. Da dette typisk 
		#er server X av flere servere, Eks: www3.nrk.no, og www.nrk.no er det samme
		#
		#ToDo: fjerner dette også sider med tall i hoveddomenet ? 8.feb, Runarb: Ja, hvis vi bare her et domene, ikke somdomnene. Triger på 24so.com, men ikke på www.24so.com
	
		#print "Number i last sub: $url\n";
		return 0;
	}
        elsif (
           (length($url) < 199) #ikke på mere en 199 tegn, setter av plass til \0 i den 200 tegn område vi bruker
                && (length($url) > 10) #ikke på mindre en 10 tegn
                && ($url !~ /\.\./) #ikke har .. i seg
		&& ($url !~ /#/) #ikke har # i seg
                && ($url =~ /(\?|\/$|.cfm$|.htm$|.html$|.shtml$|.shtm$|.php$|.jsp$|.asp$|.stm$|.ece$|.aspx$|.do$)/) #forhindrer at i får noe annet en liknker som slutter på /, .htm eller .html eller dynamsike
                && ($url =~ /^http:\/\//) #må begynne på http://, ikke for eksempel ftp:// eller https://
                && ($httpcount == 1) #bare har 1 http:// i seg
		&& ($url !~ /sid=/i) #ikke har noe shesiaon id i seg (dette tar sikkert ikke alt), case insensetiv
		&& ($equalcount < 2) #ikke har flere en = i seg, dat er det en for komplisert dynamisk side
		&& ($url !~ /[^a-zA-Z0-9\.\?=:\/\-_~&\+,]/) #ineholder andre tegn en a-z A-Z 0-9 . ? = : / - _ ~ &
		&& ($tdl =~ /[a-z]/) #ikke er en ipadresse. Sjekker om tdl en inneholder noe anet en a-z
		&& ($path !~ /index\./) #ikke inneholder index. , Kan være ting som index.html da
		&& ($path !~ /default\./) # ikke ineholder default. som default.asp
		&& ($#domainElements > 1) #domene bare er på 2 deler. Slik som vg.no, er typisk en www.vg.no også da
	        #&& ($url =~ /\.no\//) #må være nosrsk (ha .no)
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
# oppretter de nødvendige mappene
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
#retunerer pathen til alle lottene som fins i lottsystemet på denne serveren.
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
			#nå må en lot ha reposetory får å bli med. Er det riktig ?
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
