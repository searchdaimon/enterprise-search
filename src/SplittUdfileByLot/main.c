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

	//allocate memory for an array of the lots
	lots = malloc(sizeof(int) * NrOfLots);	

	//insert the lots
	y = 0;
	for (i=4;i<argc;i++) {
		printf("lotnr %s\n",argv[i]);
	
		lots[y] = atoi(argv[i]);
		y++;	
	}

	//sort the lots
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

