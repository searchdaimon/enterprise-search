static int cmp1(const void *p, const void *q);

int main (int argc, char *argv[]) {

	int i,y;       
	int NrOfLots;
	int *lots;

        if (argc < 5) {
                printf("Dette programet splitter opp en urdil pasert på hovedlot.\n\n\tUsage:\n\tSplittUdfileByLot udfile filmedLoter filutenlotter lotnomre...\n\n");
                exit(0);
        }

	NrOfLots = argc -4;

	printf("NrOfLots: %i\n",NrOfLots);

	//allokerer minne til en array over lottene
	lots = malloc(sizeof(int) * NrOfLots);	

	//legger inn lotene
	y = 0;
	for (i=4;i<argc;i++) {
		printf("lotnr %s\n",argv[i]);
	
		lots[y] = atoi(argv[i]);
		y++;	
	}

	//sorterer lottene
	qsort(lots,NrOfLots,sizeof(int),cmp1);

	for(i=0;i<NrOfLots;i++) {
		printf("lotsorted: %i\n",lots[i]);
	}

	free(lots);
}

static int cmp1(const void *p, const void *q)
{
   return *(const int *) p - *(const int *) q;
}

