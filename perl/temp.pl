my $comand = 'env MAPLIST=/home/boitho/config/maplistNODE1.conf ./LotInvertetIndexMaker.pl';

@array1 = ();
@array2 = ();
@array3 = ();
@array4 = ();


for my $i (23 .. 50) {

	my $mod = $i % 4;
	$mod++;
	if ($mod == 1) {
		push(@array1,$i);
	}
	elsif ($mod == 2) {
		push(@array2,$i);
	} 
	elsif ($mod == 3) {
		push(@array3,$i);
        }
	elsif ($mod == 4) {
		push(@array4,$i);
        }
	else {
		print "error \n";
		exit;
	}

	
}

print "----------------- 1 --------------------\n\n";
foreach	my $i (@array1){
	print "$comand $i\n";
}

print "----------------- 2 --------------------\n\n";
foreach my $i (@array2){
        print "$comand $i\n";
}

print "----------------- 3 --------------------\n\n";
foreach my $i (@array3){
        print "$comand $i\n";
}

print "----------------- 4 --------------------\n\n";
foreach my $i (@array4){
        print "$comand $i\n";
}


