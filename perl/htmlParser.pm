package htmlParser;
require Exporter;
@htmlParser::ISA = qw(Exporter);
@htmlParser::EXPORT = qw();
@htmlParser::EXPORT_OK = qw(strip_html);


#annen mudul er denne, bør kasnje bytte navn ?
use HTML::Parser ();

use constant MinLinjeLengde => 0;

#####################################################################################################
# stripper html
#####################################################################################################
sub strip_html {
	my($text) = @_;
	my($title_ferdig,$body_ferdig,$description_ferdig,$a_link);
	
	$text =~ s/<br>//g;
	$text =~ s/(\n+| +)/ /g; #fjerner \n'er da da ikke har betyding, så vi får blank space problem og kan ikke beregne innhold | fjerner doble spacer
	
#use strict;

#my $body = 0;
#my $title = 0;
#my @body;
#my @title;
#my $title_ferdig;
#my $body_ferdig;
#my $description_ferdig;
#my @tags;
#my %body_attr;

$body = 0;
$title = 0;

$a_link = 0;

@body = ();
@title = ();
@headlines = ();
@bold = ();
@innhold = ();
@urler = ();
@bgcolor = ();
@spam = ();

%ReturnVerdier = {};	#hashen som vi skal returnere

$TabellHeight = 0;
$title_ferdig = '';
$body_ferdig = '';
$description_ferdig = '';
$innhold_ferdig = '';
$iSpam = 0;				#Er vi i spam eller ikke ?
$iLitenFont = 0;		#er vi i et område med <= 1 font ?
$iNoscript = 0;			#er vi i en <noscript> område ? Bare de uten script ser dette, og det er få så her jømmes det spam
$iNoframe = 0;			#er vi i en <noframe> område ? Bare de uten fram suport ser dette, og det er få så her jømmes det spam
$iTextArea = 0;			#skal ikke ha elementer fra tekst aerie form elemenet
$harFrameset = 0;		#hvis vi har framset er all tekst spam
$iUrl = 0;				#holder om vi er i en <a> tag.
$iDivVisibilityHidden = 0; #holder om vi er i en <div> tag med "visibility:hidden" som atributt
$iBlackTextSpam = 0;
$iDivSpanSmalFont = 0;

$linjer_siden_innhold = 0; #oversikt over hvor mange linjer det er siden sist vi så noe inhold, slik at vi kan filtrere ut høyre meny støy

%meta_hash = ();

@tags = ();
%body_attr = ();
$headlines_ferdig = '';
$bold = '';

my $html = HTML::Parser->new(api_version => 3,
                                text_h  => [\&text,'dtext'],
                                start_h => [\&open_tag,'tagname,attr'],
                                end_h   => [\&close_tag,'tagname']);

$html->ignore_elements(qw(script comment style));
$html->parse($text);
$html->eof;


#titel
$title_ferdig = join(" ",@title);
$title_ferdig =~ s/<[^>]+>//gi;		#fjerner eventuelle html tagger fra titelen
$ReturnVerdier{'title'} = $title_ferdig;



#finner innhold
$innhold_ferdig = join(" \n",@innhold);

my $title_ferdigtemp = $title_ferdig;
$title_ferdigtemp =~ s/[^a-zA-Zøæå. ,!]//g;

$innhold_ferdig =~ s/^$title_ferdigtemp//g;	#fjerner tittelen hvis den er en del av innholdet. Typisk på grunn av at de har brukt overskriften som tittel
$innhold_ferdig =~ s/ +/ /g;
$ReturnVerdier{'innhold'} = $innhold_ferdig;

#body
$body_ferdig = join(" ",@body);
#gjør flere spaser om til en, og \n om til spacer
$body_ferdig =~ s/( +|\n)/ /g;
$ReturnVerdier{'body'} = $body_ferdig;

#headlines
$ReturnVerdier{'headlines'} = join(" ",@headlines);

#bold
$ReturnVerdier{'bold'} = join(" ",@bold);

#spam
$ReturnVerdier{'spam'} = join(" ",@spam);

#finner innhold, som er definert med linjer på mere en 30 tegn
#foreach my $i	(@innhold) {
#	if ((length($i) > 30) && (($i =~ tr/ //) > 10)) {
#		print "$i\n";
#		$innhold_ferdig .= $i . "\n";
#	}
#}
#finner beskrivlesen
$ReturnVerdier{'description'} = $meta_hash{'description'};
$ReturnVerdier{'keywords'} = $meta_hash{'keywords'};

#print "innhold $innhold_ferdig\n";
#I forbindelse med tagen <frameset> kan man legge inn uten bruk av <noframes></noframes> tagger. Dette er ikke korekt html kode, men vises som frames i IE.
#Når vi har <frameset> tagger skal vi ikke ta noe body tekst fra siden.
if ($harFrameset) {
	$ReturnVerdier{'spam'} .= 'Har framsett så all tekst er spam: ' . $ReturnVerdier{'innhold'} . "\n";
	$ReturnVerdier{'innhold'} = '';
}

#Spam sjekk: sjekker om Bodyens bakrunsfarge og tekst farge er like. Hvis det er det skal All tekst på siden regnes som spam og blir ikke med i indeksen.
if ((exists $body_attr{'bgcolor'}) && (exists  $body_attr{'text'})) {	#sjekker om disse tagene er satt, hvis begge mangler er de jo like, men det er ikke spam av det. Kan ikke ha noe av det.
	if ($body_attr{'bgcolor'} eq $body_attr{'text'}) {
		$ReturnVerdier{'spam'} .= $ReturnVerdier{'innhold'};
		$ReturnVerdier{'innhold'} = '';
	}
}

#debug: viser alle meta teggane
#foreach $i (keys %meta_hash) {
#	print "$i = $meta_hash{$i}\n";
#}
#/debug: viser alle meta teggane




#print $title_ferdig;
###
##fjerner html tagger
#$description_ferdig =~ s/<[^>]+>//gi;
##fjerner annet
#$description_ferdig =~ s/(<|>)//gi;
##erstater hvit sapes med en enkelt sace
#$description_ferdig =~ s/\s/ /gi;
##erstater fels spacer med en
#$description_ferdig =~ s/ +/ /g;
###


$ReturnVerdier{urler} = \@urler;

return %ReturnVerdier;

##
sub text{
        my $text = shift;
		
        return unless($text =~ /\w/);
		
		#if (($iSpam) || ($iLitenFont) || ($iNoscript) || ($iNoframe) || ($iTextArea) || ($iDiv))  {
		#	#debug, print ut spamen
		#	#print "SPAM: $text\n";
		#	push(@spam,$text);
		#}
		
		# spam bekjembeles
		if ($iSpam) {
			push(@spam,'Spam: ' . $text . "<br>\n");
		}
		elsif ($iLitenFont) {
			push(@spam,'LitenFont: ' . $text . "<br>\n");
		}
		elsif ($iNoscript) {
			push(@spam,'Noscript: ' . $text . "<br>\n");
		}
		elsif ($iNoframe) {
			push(@spam,'Noframe: ' . $text . "<br>\n");
		}
		elsif ($iDivVisibilityHidden) {
			push(@spam,'DivVisibilityHidden: ' . $text . "<br>\n");
		}
		elsif ($iBlackTextSpam) {
			push(@spam,'BlackTextSpam: ' . $text . "<br>\n");
		}
		elsif ($iDivSpanSmalFont) {
			push(@spam,'DivSpanSmalFont: ' . $text . "<br>\n");
		}
		elsif ($iTextArea) {
			push(@spam,'TextArea: ' . $text . "<br>\n");
		}
		elsif ($iUrl) {
			#print "Url tekst= $text\n";
			$urler[$#urler][1] .= $text;
		}
		#/spam bekjembeles
		else {	#Hvis dette ikk er spam
			if (!$a_link) {
        		if($title){
               		push(@title,$text);
        		}elsif($headlines){
                	push(@headlines,$text);
        		}elsif($bold){
                	push(@bold,$text);
        		}elsif($body){
                	push(@body,$text);
        		}
			}
		
		if (
	#			(
						((length($text) > MinLinjeLengde) 
					&& 	(($text =~ tr/ //) > 10))
					||	($headlines)
	#			)
	#			&& ($linjer_siden_innhold < 10) #bruker ikke dette, skaper mere problemer en det løser, da man ofte bare får litte grane av en side
		) {	
			push(@innhold,$text);
			$linjer_siden_innhold = 0;
		}
		else {
			push(@spam,'Kort linje: ' . $text . "<br>\n");
		}
		#	else {
		#		#print "$text\n";
		#		if ($#innhold != -1) {
		#			$linjer_siden_innhold++;
		#		}
		#	}
		} #else
}

sub open_tag{

        my $tagname = shift;
        my $attr    = shift;

		$a_link = 1 if($tagname eq 'a');
        $title = 1 if($tagname eq 'title');
		$bold = 1 if($tagname eq 'b');
		$headlines = 1 if(($tagname eq 'h1') || ($tagname eq 'h2') || ($tagname eq 'h3') || ($tagname eq 'h4'));

        if($tagname eq 'body'){
                $body = 1;
                while(my($key,$value) = each %{$attr}){
                        $body_attr{$key} = "'$value'";
                }				
        }elsif($body){
                push(@tags,"<$tagname>");
				#print "tags: $tagname\n";
        }
		
		#print "$tagname\n";
		
		
		if ($tagname eq 'meta') {
			while(my($value,$key) = each %{$attr}){
                       # $body_attr{$key} = "'$value'";
					   #print "$key = $value\n";
					#   if (lc($key) eq 'description') {
					#   		#print "\naaaaaaaaaaaa : $forige_key\n";
					#		$description = $forige_key;
					   #}
					   $meta_hash{lc($key)} = $forige_key;
					 #  print "aa: $key : $forige_key\n";
					   
					   $forige_key = $key;
					   #print "ppppp: $forige_key \n";
                }
		
		}
		elsif ($tagname eq 'a') {
			while(my($value,$key) = each %{$attr}){
				#print "$value,$key\n";
				if ($value eq 'href') {
					#print "$key\n";
					#push(@urler,$key);
					$urler[$#urler +1][0] = $key;
					$urler[$#urler][1] = ''; #inaliserer tilfele vi ikke finer noe test til denne urlen
					$iUrl = 1;
				}
			}
		}
		elsif ($tagname eq 'noscript') {
			$iNoscript = 1;
		}
		elsif (($tagname eq 'noframe') || ($tagname eq 'noframes')) {
			$iNoframe = 1;
		}
		elsif ($tagname eq 'textarea') {
			$iTextArea = 1;
		}
		#<div> og <span> deler denne. Men å slå av må gjøres separat da vi kan ha <span> i <div>
		elsif (($tagname eq 'div') || ($tagname eq 'span')) {
			#leter etter bruken av visibility:hidden; i styl i <div> tager, da det er spam
			#print "er i div\n";
			
			while(my($key,$value) = each %{$attr}){
				#print "\t$key = $value\n";
				if ($key eq 'style') {
				
					#man kan bruke så mange spacer man vil her ser det ut til, så vi fjerner alle, slik at vi får samme splitt altid
					$value =~ s/ //g;
					#atribyttene likker som key:value separert med ";", slpiller derfor disse og tester
					my @StyleAtributter = split(';',$value);
					
					#går gjenom alle atributtene
					foreach my $i (@StyleAtributter) {
						#print "\t$i\n";
						my ($AtribKey,$AtribValue) =split(':',$i);
						
						if (($AtribKey eq 'visibility') && ($AtribValue eq 'hidden')) {
							#print "fant visibility:hidden\n";
							$iDivVisibilityHidden = 1;
						}
						#ser etter font-size en er satt for lite
						if ($AtribKey eq 'font-size') {
							#sjekker om størelsen er angitt i "pt"
							if ($AtribValue =~ /pt$/) {
								$AtribValue =~ s/pt$//g;
								#sjeker om den er mindre en 7
								if ($AtribValue <= 7) {
									$iDivSpanSmalFont = 1;
								}
							}
						}
					}
					
				}
			}
		}
		elsif ($tagname eq 'frameset') {
			$harFrameset = 1;
		}
		
		#exit;
		#elsif (($tagname eq 'a') && ($tagname eq 'a')) {
		#	#print "$tagname : $attr\n";
		#}
		
		if (($tagname eq 'td') || ($tagname eq 'table')) {
			$TabellHeight++; #øker tabel høyden
		}
		
		#print keys(%{$attr}) . "\n";
		#hvis vi ikke har noen atributter
		if (keys(%{$attr}) == 0) {
		
			#print "\nIngen attr\n";
			#sjekker om bakrunsfargen er sort, og tekst fargen ikke er redefinert i <body>. Hvis den ikke er det er den sort som standar, så dette err spam
			if (($bgcolor[$#bgcolor] eq '#000000') && (not exists $body_attr{'text'})) {
				#print "spam!\n";
				$iBlackTextSpam = 1;
			}
		}
		else {
			$iBlackTextSpam = 0;
		}
		#spam detektering
		while(my($key,$value) = each %{$attr}){ 
		
		#print "$key : $value\n";
			if ($key eq 'color') {
				#print "$key - $value ";
				#print "bgcolor " . $bgcolor[$#bgcolor] . " " . $#bgcolor . "\n";
				if ($value eq $bgcolor[$#bgcolor]) {
					#print "$key - $value ";
					#print "bgcolor " . $bgcolor[$#bgcolor] . " " . $#bgcolor . "\n";
					#print "Spam Spam Spam Spam?\n";
					#exit;
					$iSpam = 1;
				}
				else {
					#print "\t$value eq $bgcolor[$#bgcolor]\n";
				}
			}
			elsif ($key eq 'bgcolor') {
				#print "$key - $value\n";
				push(@bgcolor, $value);
			}
			elsif ((($key eq 'size') || ($key eq 'font-size')) && ($tagname eq 'font')) {
				if ($value <= 1) {
					#print "$key = $value\n";
					#exit;
					$iLitenFont = 1;
				}
			}
			
		}
		#print "$tagname : $attr\n";
}

sub close_tag{

        my $tagname = shift;

		$a_link = 0 if($tagname eq 'a');
        $title = 0 if($tagname eq 'title');
        $body  = 0 if($tagname eq 'body');
		$bold = 0 if($tagname eq 'b');
		$headlines = 0 if(($tagname eq 'h1') | ($tagname eq 'h2') | ($tagname eq 'h3') | ($tagname eq 'h4'));

        push(@tags,"</$tagname>") if($body);
		
		#forhindrer spam
		#tar bort et elemenet fra staken som har oversikt over plassene der man kan ha bakrun
		if (($tagname eq 'td') || ($tagname eq 'table')) {
			
			if ($#bgcolor == $TabellHeight) {
				pop(@bgcolor);
				#print "pop\n";
			}
			$TabellHeight--; #øker tabel høyden
			$iSpam = 0;	#Vi er ute av område som kansje er spam.
		}
		elsif ($tagname eq 'font') {
			$iLitenFont = 0;
		}
		elsif ($tagname eq 'noscript') {
			$iNoscript = 0;
		}
		elsif (($tagname eq 'noframe') || ($tagname eq 'noframes')) {
			$iNoframe = 0;
		}
		elsif ($tagname eq 'textarea') {
			$iTextArea = 0;
		}
		elsif ($tagname eq 'span') {
			$iDivSpanSmalFont =0;
		}
		elsif ($tagname eq 'div') {
			$iDivVisibilityHidden = 0;
		}
		elsif ($tagname eq 'a') {
			$iUrl = 0; 
		}

}

}
