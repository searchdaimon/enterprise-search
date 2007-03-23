#!/usr/bin/perl

use URI::URL;
use Digest::SHA1  qw(sha1);
#use IR qw(ResulveUrl);
use common qw(gyldig_url);

# land liste, data fra http://www.din.de/gremien/nas/nabd/iso3166ma/codlstp1/ , og honde modifisert av runar

if (scalar(@ARGV) < 2) {
	print "Usage:\n\t perl dmoz_laster.pl dmozdump nyeurlfil\n\n";
	exit;
}

%country_list = 	(			


"AFGHANISTAN" , "AF",
"ALBANIA" , "AL",
"ALGERIA" , "DZ",
"AMERICAN SAMOA" , "AS",
"ANDORRA" , "AD",
"ANGOLA" , "AO",
"ANGUILLA" , "AI",
"ANTARCTICA" , "AQ",
"ANTIGUA AND BARBUDA" , "AG",
"ARGENTINA" , "AR",
"ARMENIA" , "AM",
"ARUBA" , "AW",
"AUSTRALIA" , "AU",
"AUSTRIA" , "AT",
"AZERBAIJAN" , "AZ",
"BAHAMAS" , "BS",
"BAHRAIN" , "BH",
"BANGLADESH" , "BD",
"BARBADOS" , "BB",
"BELARUS" , "BY",
"BELGIUM" , "BE",
"BELIZE" , "BZ",
"BENIN" , "BJ",
"BERMUDA" , "BM",
"BHUTAN" , "BT",
"BOLIVIA" , "BO",
"BOSNIA AND HERZEGOVINA" , "BA",
"BOTSWANA" , "BW",
"BOUVET ISLAND" , "BV",
"BRAZIL" , "BR",
"BRITISH INDIAN OCEAN TERRITORY" , "IO",
"BRUNEI DARUSSALAM" , "BN",
"BULGARIA" , "BG",
"BURKINA FASO" , "BF",
"BURUNDI" , "BI",
"CAMBODIA" , "KH",
"CAMEROON" , "CM",
"CANADA" , "CA",
"CAPE VERDE" , "CV",
"CAYMAN ISLANDS" , "KY",
"CENTRAL AFRICAN REPUBLIC" , "CF",
"CHAD" , "TD",
"CHILE" , "CL",
"CHINA" , "CN",
"CHRISTMAS,ISLAND" , "CX",
"COCOS (KEELING) ISLANDS" , "CC",
"COLOMBIA" , "CO",
"COMOROS" , "KM",
"CONGO" , "CG",
"CONGO, THE DEMOCRATIC REPUBLIC OF, THE" , "CD",
"COOK ISLANDS" , "CK",
"COSTA RICA" , "CR",
"COTE D IVOIRE" , "CI",
"CROATIA" , "HR",
"CUBA" , "CU",
"CYPRUS" , "CY",
"CZECH REPUBLIC" , "CZ",
"DENMARK" , "DK",
"DJIBOUTI" , "DJ",
"DOMINICA" , "DM",
"DOMINICAN REPUBLIC" , "DO",
"EAST TIMOR" , "TP",
"ECUADOR" , "EC",
"EGYPT" , "EG",
"EL SALVADOR" , "SV",
"EQUATORIAL GUINEA" , "GQ",
"ERITREA" , "ER",
"ESTONIA" , "EE",
"ETHIOPIA" , "ET",
"FALKLAND ISLANDS,(MALVINAS)" , "FK",
"FAROE ISLANDS" , "FO",
"FIJI" , "FJ",
"FINLAND" , "FI",
"FRANCE" , "FR",
"FRENCH,GUIANA" , "GF",
"FRENCH POLYNESIA" , "PF",
"FRENCH SOUTHERN TERRITORIES" , "TF",
"GABON" , "GA",
"GAMBIA" , "GM",
"GEORGIA" , "GE",
"GERMANY" , "DE",
"GHANA" , "GH",
"GIBRALTAR" , "GI",
"GREECE" , "GR",
"GREENLAND" , "GL",
"GRENADA" , "GD",
"GUADELOUPE" , "GP",
"GUAM" , "GU",
"GUATEMALA" , "GT",
"GUINEA" , "GN",
"GUINEA-BISSAU" , "GW",
"GUYANA" , "GY",
"HAITI" , "HT",
"HEARD ISLAND AND MCDONALD ISLANDS" , "HM",
"HOLY SEE (VATICAN CITY STATE)" , "VA",
"HONDURAS" , "HN",
"HONG,KONG" , "HK",
"HUNGARY" , "HU",
"ICELAND" , "IS",
"INDIA" , "IN",
"INDONESIA" , "ID",
"IRAN, ISLAMIC REPUBLIC OF" , "IR",
"IRAQ" , "IQ",
"IRELAND" , "IE",
"ISRAEL" , "IL",
"ITALY" , "IT",
"JAMAICA" , "JM",
"JAPAN" , "JP",
"JORDAN" , "JO",
"KAZAKSTAN" , "KZ",
"KENYA" , "KE",
"KIRIBATI" , "KI",
"KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF" , "KP",
"KOREA, REPUBLIC OF" , "KR",
"KUWAIT" , "KW",
"KYRGYZSTAN" , "KG",
"LAO PEOPLE'S DEMOCRATIC REPUBLIC" , "LA",
"LATVIA" , "LV",
"LEBANON" , "LB",
"LESOTHO" , "LS",
"LIBERIA" , "LR",
"LIBYAN ARAB JAMAHIRIYA" , "LY",
"LIECHTENSTEIN" , "LI",
"LITHUANIA" , "LT",
"LUXEMBOURG" , "LU",
"MACAU" , "MO",
"MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF" , "MK",
"MADAGASCAR" , "MG",
"MALAWI" , "MW",
"MALAYSIA" , "MY",
"MALDIVES" , "MV",
"MALI" , "ML",
"MALTA" , "MT",
"MARSHALL ISLANDS" , "MH",
"MARTINIQUE" , "MQ",
"MAURITANIA" , "MR",
"MAURITIUS" , "MU",
"MAYOTTE" , "YT",
"MEXICO" , "MX",
"MICRONESIA, FEDERATED STATES OF" , "FM",
"MOLDOVA, REPUBLIC OF" , "MD",
"MONACO" , "MC",
"MONGOLIA" , "MN",
"MONTSERRAT" , "MS",
"MOROCCO" , "MA",
"MOZAMBIQUE" , "MZ",
"MYANMAR" , "MM",
"NAMIBIA" , "NA",
"NAURU" , "NR",
"NEPAL" , "NP",
"NETHERLANDS" , "NL",
"NETHERLANDS ANTILLES" , "AN",
"NEW CALEDONIA" , "NC",
"NEW ZEALAND" , "NZ",
"NICARAGUA" , "NI",
"NIGER" , "NE",
"NIGERIA" , "NG",
"NIUE" , "NU",
"NORFOLK ISLAND" , "NF",
"NORTHERN MARIANA ISLANDS" , "MP",
"NORWAY" , "NO",
"NORSK" , "NO",
"OMAN" , "OM",
"PAKISTAN" , "PK",
"PALAU" , "PW",
"PALESTINIAN TERRITORY ,OCCUPIED" , "PS",
"PANAMA" , "PA",
"PAPUA NEW GUINEA" , "PG",
"PARAGUAY" , "PY",
"PERU" , "PE",
"PHILIPPINES" , "PH",
"PITCAIRN" , "PN",
"POLAND" , "PL",
"PORTUGAL" , "PT",
"PUERTO RICO" , "PR",
"QATAR" , "QA",
"REUNION" , "RE",
"ROMANIA" , "RO",
"RUSSIAN FEDERATION" , "RU",
"RUSSIAN" , "RU",
"RWANDA" , "RW",
"SAINT HELENA" , "SH",
"SAINT KITTS AND NEVIS" , "KN",
"SAINT LUCIA" , "LC",
"SAINT PIERRE AND MIQUELON" , "PM",
"SAINT VINCENT AND THE GRENADINES" , "VC",
"SAMOA" , "WS",
"SAN,MARINO" , "SM",
"SAO TOME AND PRINCIPE" , "ST",
"SAUDI ARABIA" , "SA",
"SENEGAL" , "SN",
"SEYCHELLES" , "SC",
"SIERRA LEONE" , "SL",
"SINGAPORE" , "SG",
"SLOVAKIA" , "SK",
"SLOVENIA" , "SI",
"SOLOMON ISLANDS" , "SB",
"SOMALIA" , "SO",
"SOUTH AFRICA" , "ZA",
"SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS" , "GS",
"SPAIN" , "ES",
"SRI LANKA" , "LK",
"SUDAN" , "SD",
"SURINAME" , "SR",
"SVALBARD AND JAN MAYEN" , "SJ",
"SWAZILAND" , "SZ",
"SWEDEN" , "SE",
"SWITZERLAND" , "CH",
"SYRIAN ARAB REPUBLIC" , "SY",
"TAIWAN, PROVINCE OF CHINA" , "TW",
"TAJIKISTAN" , "TJ",
"TANZANIA, UNITED REPUBLIC OF" , "TZ",
"THAILAND" , "TH",
"TOGO" , "TG",
"TOKELAU" , "TK",
"TONGA" , "TO",
"TRINIDAD AND TOBAGO" , "TT",
"TUNISIA" , "TN",
"TURKEY" , "TR",
"TURKMENISTAN" , "TM",
"TURKS AND CAICOS ISLANDS" , "TC",
"TUVALU" , "TV",
"UGANDA" , "UG",
"UKRAINE" , "UA",
"UNITED ARAB EMIRATES" , "AE",
"UNITED KINGDOM" , "GB",
"UNITED STATES" , "US",
"UNITED STATES MINOR OUTLYING ISLANDS" , "UM",
"URUGUAY" , "UY",
"UZBEKISTAN" , "UZ",
"VANUATU" , "VU",
"VENEZUELA" , "VE",
"VIET NAM" , "VN",
"VIRGIN ISLANDS, BRITISH" , "VG",
"VIRGIN ISLANDS, U.S." , "VI",
"WALLIS AND FUTUNA" , "WF",
"WESTERN,SAHARA" , "EH",
"YEMEN" , "YE",
"YUGOSLAVIA" , "YU",
"ZAMBIA" , "ZM",
"ZIMBABWE" , "ZW",

);

#############################################################################################################################################
# Henter data fra dmoz filen
#############################################################################################################################################


$dmos_dump_fil = $ARGV[0];





open(INF,$dmos_dump_fil) or die("Can't open $dmos_dump_fil: $!");


open(NYEURLER,">$ARGV[1]") or die("Cant open $ARGV[1]: $!");
flock(NYEURLER,2) or die ("Can't lock lock file: $!");
# Reset the file pointer to the end of the file, in case 
# someone wrote to it while we waited for the lock...
seek(NYEURLER,0,2);


#inaliserer
our $terminated = 0;

$dmoz_category = "";
$url = "";
$titel = "";
$description = "";

#begynner på 1 slik at vi slipper problemer med at vi deler på 0
$count = 1;
$count_vis = 1;

#hvor fote vi skal printe ut status
$NprintStatus = 100000;

my $count_url_gyldig = 0;
my $count_url_Ikkegyldig = 0;


#finner start time
$start_time = time;
$SIG{INT} = $SIG{TERM} = $SIG{HUP} = \&signal_handler;

#kjør dene loopen til end of file (det ikke komer mede data)
while ((!eof(INF)) && (!$terminated)) {

$linje = "";
$linje = <INF>;       # reads one line from the file into the scalar $a
#print "linjer: $linje";

#tar bort elle entere (\n)

#$linje =~ s/\n//;

	
	#hvis vi finner at linjen er en kategori så:
	if ($linje eq "") {
		print "blank linje\n";
	}
	
	elsif ($linje =~ /Topic r:id/) {
	
		$dmoz_category = $linje;
		
		#print "start data: $dmoz_category\n";

		($garbage,$dmoz_category) = split(/<Topic r:id="/,$dmoz_category); 
		
		#print "1. dmoz_category: $dmoz_category, garbage: $garbage\n";

		($dmoz_category,$garbage) = split(/">/,$dmoz_category); 
		
	}
	elsif ($linje =~ /ExternalPage about/) {
	
	
		$url = $linje;
		
		($garbage,$url) = split(/<ExternalPage about="/,$url); 
		
		($url,$garbage) = split(/">/,$url); 
	
		
	
	}
	elsif ($linje =~ /d:Description/) {
		$description = $linje;
		
		($garbage,$description) = split(/<d:Description>/,$description); 
		
		($description,$garbage) = split(m#</d:Description>#,$description); 
		
		
	
	}
	elsif ($linje =~ /d:Title/) {
		$titel = $linje;
		
		($garbage,$titel) = split(/<d:Title>/,$titel); 
		
		($titel,$garbage) = split(m#</d:Title>#,$titel); 
		
	
	}
	
	elsif ($linje =~ m#</ExternalPage>#) {
	
	
		#sjekker om det er norsk data
		#if (($dmoz_category =~ /norsk/i) 	
		#if (($dmoz_category =~ /Top\/Arts/i) 	
		#or ($dmoz_category =~ /Top\/Games/i) 
		#or ($dmoz_category =~ /Top\/Shopping/i) 
		#or ($dmoz_category =~ /Top\/Health/i)
		#or ($dmoz_category =~ /Top\/Home/i)
		#if ((!($dmoz_category =~ /Top\/World/i)) && (!($dmoz_category =~ /Top\/Adult/i))
		
			#or ($dmoz_category =~ /norge/i) 
			#or ($dmoz_category =~ /Norsk/i) 
			#or ($dmoz_category =~ /norway/i)
			#or ($description =~ /norsk/i)
			#or ($description =~ /norge/i)
			#or ($description =~ /norway/i)
			#or ($titel =~ /norsk/i)
			#or ($titel =~ /norge/i)
			#or ($titel =~ /norway/i)
			#or ($url =~ /norsk/i)
			#or ($url =~ /norge/i)
			#or ($url =~ /norway/i)
			
			if ((1)
			
			) {
			
		#if ($dmoz_category =~ /film/i)
		# {
		 
	
		
		#print ".";
		if ($count == $count_vis) {
			$count_vis = $count + $NprintStatus;
			
			#finner hvor lenge det er siden
			$difference = time - $start_time;

			#finner brukt tip pr side
			$pr_side = $difference / $count;
			
			$seconds    =  $difference % 60;
			$difference = ($difference - $seconds) / 60;
			$minutes    =  $difference % 60;
			$difference = ($difference - $minutes) / 60;
			$hours      =  $difference % 24;
			$difference = ($difference - $hours)   / 24;
			$days       =  $difference % 7;
			$weeks      = ($difference - $days)    /  7;



			print "komet til: $count, brukt tid: $weeks weeks, $days days, $hours:$minutes:$seconds, tid pr side: $pr_side\n";
		}
		
		
			#print "{\n";
			#print "dmoz_category: $dmoz_category\n";
			#print "URL: $url\n";
			#print "titel: $titel\n";	
			#print "description: $description\n";	
			
			#@key_words1 = lexer("$titel");
			#@key_words2 = lexer("$description");
			
			
			#@key_words1 = vekting("20",@key_words1);
			#@key_words2 = vekting("40",@key_words2);
			

			#setter @key_words lik den første arrayen med nøkkelord, de neste må puejes
			#@key_words = @key_words1;
			#push(@key_words,@key_words2);
			
			#stoppord sjeker
			#@key_words = stoppord(@key_words);
			

			
			
			#print "}\n";
			
			$count++;
			#laster inn i databsen
			
			#$Sist_Oppdatert = SQL_datetime();
			#$rv = $dbh->do(qq{insert into Sider values("$url","$User_Name","$description","$titel","$dmoz_category","$Sist_Oppdatert")}) or dienice("can´t do statment: ",$dbh->errstr);

			$kilde = 1;
			$sprok = '';
			
			#hvis vi har Top/World prøver vi å mappe landet
			if ($dmoz_category =~ /^Top\/World\//i) {
				$sprok = $dmoz_category;
				$sprok =~ s/^Top\/World\/.*\///;
				#anokterer (henter det vi tokk port)
				$sprok = $1;
				#gjør om til opper case
				$sprok = uc($sprok);
				$sprok = $country_list{$sprok};
			}
			
			
			#gjør om litt på fomatet slik at vi får dmoz: i steden for Top/
			$dmoz_category =~ s/^Top\//dmoz: /;
			
			
		
			
			#add_url($url,$titel,$description,$dmoz_category,$sprok,$kilde);


			#resolver URLen, slik at man får ting som / på slutten der det skal være det.
			$url = ResulveUrl('http://www.dmoz.org/',$url);

			# Format på nye urlfilen
			#struct update_block
			#{
			#    unsigned char       sha1[20];
			#    unsigned char       url[200];
			#    unsigned char       linktext[50];
			#};

			#if (gyldig_url($url)) {			
			#	#print NYEURLER pack('A20 A200 A50',sha1($url),$url,$titel);			
				print NYEURLER "$url\n";
				++$count_url_gyldig;
			#}
			#else {
			#	#print "Url ugyldig: $url\n";
			#	++$count_url_Ikkegyldig;
			#}	

			#print "$url - $titel\n";
	


	
			$url = "";
			$titel = "";
			$description = "";
	
	
			#print "\n\n";
		} #if

	
	}
	
}


			



close(NYEURLER);
close(INF);

print "gyldig urls: $count_url_gyldig, Ikke gyldig urls: $count_url_Ikkegyldig\n";


############################################################################################################################
# hånterer signaler
############################################################################################################################
sub signal_handler {	
	if (not $terminated) {
		$terminated = 1;

		print "\n\aOk, begynner og avslutte\n\n";
	}
	else {
		die("Motok killsignal nr to, dør");
	}
}

sub ResulveUrl {
        my($BaseUrl,$NyUrl) = @_;

        $link = new URI::URL $NyUrl;
        $FerdigUrl = $link->abs($BaseUrl);

        return $FerdigUrl;
}

