count=1;
for i in /home/boitho/cvstestdata/lot/*/*/anchors;
do
        echo $count;

        perl IndexerLotAnchors.pl $count;
	#sleep 30; #sover litt så maskinen får mulighet til å hente seg inn hvis det er noe annet som også må gjøres

        count=`expr $count + 1`;
done

