//void searchSimple (int *TeffArrayElementer, struct iindexFormat *TeffArray,int *TotaltTreff, query_array *queryParsed,struct queryTimeFormat *queryTime,struct subnamesFormat subnames[], int nrOfSubnames, int languageFilterNr, int languageFilterAsNr[],char orderby[],int dates[]);
void searchSimple (int *TeffArrayElementer, struct iindexFormat *TeffArray,int *TotaltTreff,
                query_array *queryParsed, struct queryTimeFormat *queryTime,
                struct subnamesFormat subnames[], int nrOfSubnames,int languageFilterNr,
                int languageFilterAsNr[], char orderby[], int dates[],
                struct filtersFormat *filters,
		struct filteronFormat *filteron);

int searchFilterCount(int *TeffArrayElementer, struct iindexFormat *TeffArray, struct filtersFormat *filters,struct subnamesFormat subnames[], int nrOfSubnames,struct filteronFormat *filteron);
