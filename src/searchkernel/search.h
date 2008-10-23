struct hashtable;

void searchSimple (int *TeffArrayElementer, struct iindexFormat *TeffArray,int *TotaltTreff,
//		int *unfilteredTeffArrayElementer, struct iindexFormat *unfilteredTeffArray,
                query_array *queryParsed, struct queryTimeFormat *queryTime,
                struct subnamesFormat subnames[], int nrOfSubnames,int languageFilterNr,
                int languageFilterAsNr[], char orderby[],
                struct filtersFormat *filters,
		struct filteronFormat *filteron,
		query_array *search_user_as_query,
		int ranking, struct hashtable **crc32maphash
		);

#ifdef BLACK_BOKS
char* searchFilterCount(int *TeffArrayElementer, 
			struct iindexFormat *TeffArray, 
			struct filtersFormat *filters,
			struct subnamesFormat subnames[], 
			int nrOfSubnames,
			struct filteronFormat *filteron,
			int dates[],
			struct queryTimeFormat *queryTime,
			struct fte_data *getfiletypep,
			attr_conf *showattrp,
			query_array *qa
		);

void searchFilterInit(struct filtersFormat *filters,int dates[]);
#endif
