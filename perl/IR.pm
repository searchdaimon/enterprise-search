
package IR;
require Exporter;
@IR::ISA = qw(Exporter);
@IR::EXPORT = qw();
@IR::EXPORT_OK = qw(lexer stoppord ResulveUrl adultFilter);


use URI::URL;

#########################################################################################################
#
#########################################################################################################
sub lexer {
	my($tegn,$TypePosition,%termer) = @_;
	my($term);


	#convert characters
	
	$tegn =~ s/&aring;/ו/g;
	$tegn =~ s/&aelig;/ז/g;
	$tegn =~ s/&oslash;/ר/g;

	#$tegn =~ s/&lt;/\</g;
	#$tegn =~ s/&gt;/\>/g;
	#\21 Oct 2005: removing this since it takes too much time
#	$tegn =~ s/&amp;/&/g;
#	$tegn =~ s/&quot;/"/g;
#	$tegn =~ s/&nbsp;/ /g;
	#/
	
	#convert hyphen to space
	#$tegn =~ s/-/ /g;
	#$tegn =~ s/_/ /g;
	
	#convert to lowercase
	$tegn = lc($tegn);
	
	#strip out everything except a-z and Norwegian letters
	#$tegn =~ s/[^a-zרזו ]//g;
	
	#replace runs of consecutive spaces with a single one
	$tegn =~ s/ +/ /g;
	
	
	my @mulige_ord = split(/[^\wרזו]/, $tegn); #split on non-alphanumeric characters (\w)
	
	
	#add position info
	%termer;
	my $count = 0;
	for my $i (@mulige_ord) {
		#print "$i - $count\n";
		
		${ $termer{$i} }[$#{ $termer{$i} } +1][0] = $count;	#where in the document
		${ $termer{$i} }[$#{ $termer{$i} }][1] = $TypePosition;	#which type: body, url, title or....
		
		$count++;
	}

	
return %termer;
}

sub stoppord {
	my($language,$minimum_term_lengde,$maksimum_term_lengde,%mulige_ord) = @_;
	my(@ord) = ();

	########################################################################
	#check whether the correct stop word list is loaded, if not we load it
	if (not exists $stoppord_liste{$language}) {
		#try to load the list
		my $fil = '../data/stoppord/' . $language . '.txt';
		open(INF,"$fil") or warn("Can't open $fil: $!");
		my @a = <INF>;       # reads the ENTIRE FILE into hash %b, loaded with \n at the end, so \n must be included when we compare
		close(INF);
		
		foreach my $i (@a) {
			chomp($i);
			$b{$i} = 1;
			#print "$i\n";
		}
		
 		$stoppord_liste{$language} = {%b};
		
		#list out the ones we have
		#foreach $i (keys %b) {
		#	print "$i\n";
		#}
	}
	########################################################################
	foreach my $term (keys %mulige_ord) {
		#remove the last character if it is anything other than a-z and Norwegian letters, so that periods, commas, etc disappear.
		#$term = $i->[0];
		#$posisjon = $i->[1];
		#$term =~ s/[^a-zרזו0-9]$//g;
		
		if (
				($term =~ /[^a-zרזו0-9]/) 						#contains something other than alphanumeric characters
			|| 	($term eq '')									#is blank
			||	(length($term) < $minimum_term_lengde) 			#is shorter than we want
		    	||	(length($term) > $maksimum_term_lengde) 			#is longer than we want
			||	(exists $stoppord_liste{$language}->{$term})	#in the stop word list
			) {
			#print "-$term-\n";
			#push(@ord,$term);
			#$ord[$#ord++][0] = $term;
			#$ord[$#ord][1] = $posisjon;
			delete $mulige_ord{$term};
		}
		
	}

	#exit;
	return %mulige_ord;
}

###################################################################################################################
# convert a relative url to a full url.
# Returns the full url
#
#	Input values
#	$BaseUrl	: the url the new url came from	Ex: http://www.domene.com/ 
#	$NyUrl		: new url to be resolved. Ex: ../path/side.htm 
#
#	This procedure is much cheaper to use than URI::URL since it does not start up a whole lot, but does not have as many
#	functions so it has to call URI::URL for about 1/4 of the urls. But this happens automatically without the user needing
#	to care.
###################################################################################################################
sub ResulveUrl {
	my($BaseUrl,$NyUrl) = @_;
#		if ($NyUrl =~ /^http:\/\//) { #if this is not a full url starting with http:// and not a relative one starting with ../
#			$FerdigUrl = $NyUrl;
#			
#		}
#		elsif ((($NyUrl !~ /^\.|^\//) || ($BaseUrl =~ /$\//)) && ($NyUrl !~ /:/)){ #if the url does not begin with . or / and the base url ends with / and the new url does not contain a : (https: mailto: ftp: etc) it can be appended
#			if ($BaseUrl =~ /$\//) { #if we start with a / remove it
#				$temp = $BaseUrl;
#				chop($temp);
#				
#				#print "1 $temp / $NyUrl\n";
#				$FerdigUrl = $temp . '/' . $NyUrl;
#			}
#			else {
#				if (($NyUrl !~ /^https/) && ($NyUrl !~ /^mailto/)) {	#if we begin with neither https nor mailto
#					#print "2 $BaseUrl $NyUrl\n";
#					$FerdigUrl = $BaseUrl . $NyUrl;
#					
#				}
#			}
#		}
#		else {
#			#$AntallUrler++;
			$link = new URI::URL $NyUrl;
			$FerdigUrl = $link->abs($BaseUrl);
#		}
#		
	return $FerdigUrl;
}
###################################################################################################################
#	check how many words in a hash appear in data/AdultWords.txt and data/AdoldHardCoreWords.txt in order to calculate a
#	weight for how likely it is that this is porn
#
#	data/AdultWords.txt - words that often appear on porn sites, but also elsewhere, have a double meaning
#	data/AdoldHardCoreWords.txt - words that have no non-porn meaning
#	data/AdultFraser.txt - 2-word phrases that recur a lot on porn sites
###################################################################################################################
BEGIN {

	warn("have disabled AdultWords.txt loading");
	return;

	my $adultWords;	#persistent variable. Stays here between each time we call the subroutine

		#try to load the list
		my $fil = '../data/AdultWords.txt';
		open(INF,"$fil") or die("Can't open $fil: $!");
		my @a = <INF>;       # reads the ENTIRE FILE into hash %b, loaded with \n at the end, so \n must be included when we compare
		close(INF);
		
		my %b = {};
		foreach my $i (@a) {
			chomp($i);
			$b{$i} = 1;
			#print "$i\n";
		}
		
 		$adultWords{'AdultWords'} = {%b};
		
		#list out the ones we have
		foreach $i (keys %b) {
			#print "$i\n";
		}
		
		#try to load the list
		my $fil = '../data/AdultHardCoreWords.txt';
		open(INF,"$fil") or die("Can't open $fil: $!");
		my @a = <INF>;       # reads the ENTIRE FILE into hash %b, loaded with \n at the end, so \n must be included when we compare
		close(INF);
		
		my %b = {};
		foreach my $i (@a) {
			chomp($i);
			$b{$i} = 1;
			#print "$i\n";
		}
		
 		$adultWords{'AdoldHardCoreWords'} = {%b};
		
		#list out the ones we have
		#foreach $i (keys %b) {
		#	print "$i\n";
		#}
		##############################
		#	AdultFraser.txt
		#############################
		#try to load the list
		my $fil = '../data/AdultFraser.txt';
		open(INF,"$fil") or die("Can't open $fil: $!");
		my @a = <INF>;       # reads the ENTIRE FILE into hash %b, loaded with \n at the end, so \n must be included when we compare
		close(INF);
		
		my %b = {};
		foreach my $i (@a) {
			chomp($i);
			($ord1, $ord2) = split(' ',$i,2);
			push(@{ $b{$ord1} },$ord2);
			#print "$ord1, $ord2\n";
		}
		
 		$adultWords{'AdultFraser'} = {%b};
		#exit;
		
		
sub adultFilter {
	my(%TermHash) = @_;
	
	my $verdi = 0;
	#print "verdi $verdi\n";
	
	foreach my $i (keys %TermHash) {
		if (exists $adultWords{'AdoldHardCoreWords'}->{$i}) {
			#print "ah: $i\n";
			$verdi += 20;
		}
		if (exists $adultWords{'AdultWords'}->{$i}) {
			#print "an: $i\n";
			$verdi += 3;
		}
		if (exists $adultWords{'AdultFraser'}->{$i}) {
			#print "\nFins $i kombinert med ";
			#loop through all the words that come after the first word; together the two words make up the phrase we are looking for
			foreach my $x (@{ $adultWords{'AdultFraser'}->{$i} }) {
			#	print "$x eller ";
				if (exists $TermHash{$x}) {	#if one of the phrase words also exists
					foreach my $y (@{ $TermHash{$x} }) {
					#	print "\n\t$x i $y->[1], $y->[0]";
						#now we have found the last word in the phrase, so we loop through and check whether it is combined with the first word anywhere
						foreach my $h (@{ $TermHash{$i} }) {
						#	print "\nh: $i $h->[1] $h->[0]";
							#if (($h->[1] eq $y->[1]) && ($h->[0] == ($y->[0] -1))) { #subtract 1, then the positions are equal if they are adjacent.
							if ($h->[1] eq $y->[1]) {
								my $distance = $y->[0] - $h->[0];	#find the distance
								#print "\ndistance $distance\n";
								if ($distance == 1) {
								#print "\nfant: $i:$h->[0] $x: $y->[0] \n";
								$verdi += 20;
								#exit;
								}
							}
						}
						
						#foreach my $l (@{ $adultWords{'AdultFraser'}->{$x} }) {
						#	print "l $l $l->[0] $l->[1] ";
						#}
						#exit;
					}
				}
				#foreach my $y (@{ $TermHash{$i} }) {
				#print "\n$y->[1], $y->[0]\n";
				#}
			}
			#exit;
			
		}
	}
	
	#print "verdi $verdi\n";
	
	return $verdi;
}#sub

}#BEGIN
