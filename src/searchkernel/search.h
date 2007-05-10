//void searchSimple (int *TeffArrayElementer, struct iindexFormat *TeffArray,int *TotaltTreff, query_array *queryParsed,struct queryTimeFormat *queryTime,struct subnamesFormat subnames[], int nrOfSubnames, int languageFilterNr, int languageFilterAsNr[],char orderby[],int dates[]);
void searchSimple (int *TeffArrayElementer, struct iindexFormat *TeffArray,int *TotaltTreff,
                query_array *queryParsed, struct queryTimeFormat *queryTime,
                struct subnamesFormat subnames[], int nrOfSubnames,int languageFilterNr,
                int languageFilterAsNr[], char orderby[],
                struct filtersFormat *filters,
		struct filteronFormat *filteron,
		query_array *search_user_as_query
		);

#ifdef BLACK_BOKS
int searchFilterCount(int *TeffArrayElementer, 
			struct iindexFormat *TeffArray, 
			struct filtersFormat *filters,
			struct subnamesFormat subnames[], 
			int nrOfSubnames,
			struct filteronFormat *filteron,
			int dates[],
			struct queryTimeFormat *queryTime
		);
#endif
