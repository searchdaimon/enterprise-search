package htmlParser;
require Exporter;
@htmlParser::ISA = qw(Exporter);
@htmlParser::EXPORT = qw();
@htmlParser::EXPORT_OK = qw(strip_html);


#this is another module, maybe should change the name ?
use HTML::Parser ();

use constant MinLinjeLengde => 0;

#####################################################################################################
# strips html
#####################################################################################################
sub strip_html {
	my($text) = @_;
	my($title_ferdig,$body_ferdig,$description_ferdig,$a_link);
	
	$text =~ s/<br>//g;
	$text =~ s/(\n+| +)/ /g; #removes \n's since they have no meaning, otherwise we get blank space problems and cannot compute content | removes double spaces
	
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

%ReturnVerdier = {};	#the hash that we are going to return

$TabellHeight = 0;
$title_ferdig = '';
$body_ferdig = '';
$description_ferdig = '';
$innhold_ferdig = '';
$iSpam = 0;				#Are we in spam or not ?
$iLitenFont = 0;		#are we in an area with <= 1 font ?
$iNoscript = 0;			#are we in a <noscript> area ? Only those without script see this, and they are few so spam is hidden here
$iNoframe = 0;			#are we in a <noframe> area ? Only those without frame support see this, and they are few so spam is hidden here
$iTextArea = 0;			#should not have elements from the textarea form element
$harFrameset = 0;		#if we have a frameset then all text is spam
$iUrl = 0;				#tracks whether we are in an <a> tag.
$iDivVisibilityHidden = 0; #tracks whether we are in a <div> tag with "visibility:hidden" as an attribute
$iBlackTextSpam = 0;
$iDivSpanSmalFont = 0;

$linjer_siden_innhold = 0; #count of how many lines since we last saw any content, so we can filter out right-hand menu noise

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


#title
$title_ferdig = join(" ",@title);
$title_ferdig =~ s/<[^>]+>//gi;		#removes any html tags from the title
$ReturnVerdier{'title'} = $title_ferdig;



#finds content
$innhold_ferdig = join(" \n",@innhold);

my $title_ferdigtemp = $title_ferdig;
$title_ferdigtemp =~ s/[^a-zA-Zøæå. ,!]//g;

$innhold_ferdig =~ s/^$title_ferdigtemp//g;	#removes the title if it is part of the content. Typically because they have used the heading as the title
$innhold_ferdig =~ s/ +/ /g;
$ReturnVerdier{'innhold'} = $innhold_ferdig;

#body
$body_ferdig = join(" ",@body);
#turns multiple spaces into one, and \n into spaces
$body_ferdig =~ s/( +|\n)/ /g;
$ReturnVerdier{'body'} = $body_ferdig;

#headlines
$ReturnVerdier{'headlines'} = join(" ",@headlines);

#bold
$ReturnVerdier{'bold'} = join(" ",@bold);

#spam
$ReturnVerdier{'spam'} = join(" ",@spam);

#finds content, which is defined as lines of more than 30 characters
#foreach my $i	(@innhold) {
#	if ((length($i) > 30) && (($i =~ tr/ //) > 10)) {
#		print "$i\n";
#		$innhold_ferdig .= $i . "\n";
#	}
#}
#finds the description
$ReturnVerdier{'description'} = $meta_hash{'description'};
$ReturnVerdier{'keywords'} = $meta_hash{'keywords'};

#print "innhold $innhold_ferdig\n";
#With the <frameset> tag one can include content without using <noframes></noframes> tags. This is not correct html code, but is shown as frames in IE.
#When we have <frameset> tags we should not take any body text from the page.
if ($harFrameset) {
	$ReturnVerdier{'spam'} .= 'Har framsett så all tekst er spam: ' . $ReturnVerdier{'innhold'} . "\n";
	$ReturnVerdier{'innhold'} = '';
}

#Spam check: checks whether the body background color and text color are equal. If so, all text on the page is treated as spam and is not included in the index.
if ((exists $body_attr{'bgcolor'}) && (exists  $body_attr{'text'})) {	#checks whether these tags are set; if both are missing they are equal, but that is not spam. Can have neither.
	if ($body_attr{'bgcolor'} eq $body_attr{'text'}) {
		$ReturnVerdier{'spam'} .= $ReturnVerdier{'innhold'};
		$ReturnVerdier{'innhold'} = '';
	}
}

#debug: shows all the meta tags
#foreach $i (keys %meta_hash) {
#	print "$i = $meta_hash{$i}\n";
#}
#/debug: shows all the meta tags




#print $title_ferdig;
###
##removes html tags
#$description_ferdig =~ s/<[^>]+>//gi;
##removes other things
#$description_ferdig =~ s/(<|>)//gi;
##replaces whitespace with a single space
#$description_ferdig =~ s/\s/ /gi;
##replaces multiple spaces with one
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
		
		# spam fighting
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
			#print "Url text= $text\n";
			$urler[$#urler][1] .= $text;
		}
		#/spam fighting
		else {	#If this is not spam
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
	#			&& ($linjer_siden_innhold < 10) #not using this, it creates more problems than it solves, since you often only get a tiny bit of a page
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
					$urler[$#urler][1] = ''; #initializes in case we do not find any text for this url
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
		#<div> and <span> share this. But turning it off must be done separately since we can have <span> inside <div>
		elsif (($tagname eq 'div') || ($tagname eq 'span')) {
			#looks for the use of visibility:hidden; in the style of <div> tags, since that is spam
			#print "in div\n";
			
			while(my($key,$value) = each %{$attr}){
				#print "\t$key = $value\n";
				if ($key eq 'style') {
				
					#it seems you can use as many spaces as you like here, so we remove them all, so we always get the same split
					$value =~ s/ //g;
					#the attributes are like key:value separated by ";", so we split on these and test
					my @StyleAtributter = split(';',$value);
					
					#goes through all the attributes
					foreach my $i (@StyleAtributter) {
						#print "\t$i\n";
						my ($AtribKey,$AtribValue) =split(':',$i);
						
						if (($AtribKey eq 'visibility') && ($AtribValue eq 'hidden')) {
							#print "fant visibility:hidden\n";
							$iDivVisibilityHidden = 1;
						}
						#looks for whether font-size is set too small
						if ($AtribKey eq 'font-size') {
							#checks whether the size is given in "pt"
							if ($AtribValue =~ /pt$/) {
								$AtribValue =~ s/pt$//g;
								#checks whether it is less than 7
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
			$TabellHeight++; #increases the table height
		}
		
		#print keys(%{$attr}) . "\n";
		#if we do not have any attributes
		if (keys(%{$attr}) == 0) {
		
			#print "\nIngen attr\n";
			#checks whether the background color is black, and the text color is not redefined in <body>. If it is not, it is black by default, so this is spam
			if (($bgcolor[$#bgcolor] eq '#000000') && (not exists $body_attr{'text'})) {
				#print "spam!\n";
				$iBlackTextSpam = 1;
			}
		}
		else {
			$iBlackTextSpam = 0;
		}
		#spam detection
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
		
		#prevents spam
		#removes an element from the stack that tracks the places where a background can occur
		if (($tagname eq 'td') || ($tagname eq 'table')) {
			
			if ($#bgcolor == $TabellHeight) {
				pop(@bgcolor);
				#print "pop\n";
			}
			$TabellHeight--; #decreases the table height
			$iSpam = 0;	#We are out of the area that might be spam.
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
