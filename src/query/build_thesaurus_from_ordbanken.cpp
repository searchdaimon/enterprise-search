
/**
 *	build_thesaurus_from_ordbanken.cpp
 *
 *	(C) Copyright SearchDaimon AS 2008, Magnus Galåen (mg@searchdaimon.com)
 *
 *	Bygger en ordliste/thesaurus med ordbankens ordliste som input.
 *	I dag blir ord som inneholder ikke-søkbare tegn fjernet.
 */

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <set>

#include "../common/utf8-strings.h"

using namespace std;


// tword-flags:
const char tword_stem=1, tword_dup=2;

typedef struct
{
    string	text;
    int		id;
    int		flags;
} tword;


struct tword_compare_text
{
    bool operator()(const tword *a, const tword *b) const
	{
	    int		cmp = utf8_strcasecmp((const unsigned char*)a->text.c_str(), (const unsigned char*)b->text.c_str());
	    if (cmp < 0) return true;
	    if (cmp > 0) return false;
	    if (a->text < b->text) return true;
	    if (a->id < b->id) return true;
	    return false;
	}
};

struct tword_compare_id
{
    bool operator()(const tword *a, const tword *b) const
	{
	    if (a->id < b->id) return true;
	    if (a->id > b->id) return false;
	    int		cmp = utf8_strcasecmp((const unsigned char*)a->text.c_str(), (const unsigned char*)b->text.c_str());
	    if (cmp < 0) return true;
	    if (cmp > 0) return false;
	    if (a->text < b->text) return true;
	    return false;
	}
};


bool valid_word( unsigned char *word )
{
    // querytegn + '-' og ' ':
    char	valid[128] = {
		    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		    1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
		    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,
		    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0};

    for (int i=0; word[i]!='\0'; i++)
	if ((word[i]<128 && !valid[word[i]])
	    && !((word[i]>=192 && word[i]<=223) && (word[i+1]>=128 && word[i+1]<=191))
	    && !((word[i]>=224 && word[i]<=239) && (word[i+1]>=128 && word[i+1]<=191) && (word[i+2]>=128 && word[i+2]<=191))
	    && !((word[i]>=240 && word[i]<=247) && (word[i+1]>=128 && word[i+1]<=191) && (word[i+2]>=128 && word[i+2]<=191) && (word[i+3]>=128 && word[i+3]<=191))
	    )
	    return false;

    return true;
}


int main( int argc, char *argv[] )
{
    if (argc!=4)
	{
	    cout << "Usage: " << argv[0] << " <input:fullform_bm.txt> <output:thesaurus.text> <output:thesaurus.id>" << endl;
	    return 0;
	}

    ifstream	ordbanken(argv[1]);
    ofstream	text_out(argv[2]), id_out(argv[3]);
    char	line[1024];

    // <string, int>
    set<tword*, tword_compare_text>	W;
    set<tword*, tword_compare_id>	Id;

    int		id;
    string	grunnform, fullform;

    while (ordbanken.getline(line, 1023))
	{
	    // Skip linjer som ikke inneholder en entry:
	    if (line[0]<'0' || line[0]>'9')
		continue;

	    istringstream	iss(line);
	    char		buf[1024];
	    char		*temp;

	    iss.get(buf, 1023, '\t');
	    id = atoi(buf);
	    iss.get();
	    iss.get(buf, 1023, '\t');
	    temp = (char*)copy_latin1_to_utf8((unsigned char*)buf);

	    // Hvis fullform eller grunnform inneholder ugyldige tegn, hopp over ordet.
	    if (temp[0]=='-' || !valid_word((unsigned char*)temp))
		{
		    free(temp);
		    continue;
		}

	    grunnform = string(temp);
	    free(temp);
	    iss.get();
	    iss.get(buf, 1023, '\t');
	    temp = (char*)copy_latin1_to_utf8((unsigned char*)buf);

	    // Hvis fullform eller grunnform inneholder ugyldige tegn, hopp over ordet.
	    if (temp[0]=='-' || !valid_word((unsigned char*)temp))
		{
		    free(temp);
		    continue;
		}

	    fullform = string(temp);
	    free(temp);

//	    cout << id << "\t" << grunnform << "\t" << fullform << endl;

	    tword	*t = new tword;
	    t->text = fullform;
	    t->id = id;
	    t->flags = 0;
	    if (grunnform == fullform) t->flags|= tword_stem;

	    W.insert(t);
	    Id.insert(t);
	}

    text_out << W.size() << endl;	// Number of entries.
    set<tword*>::iterator	it=W.begin(), stop=W.end(), succ;
    for (; it!=stop;)
	{
	    succ = it;
	    ++succ;

	    if (succ!=stop)
		{
		    // Marker duplikat-ord:
		    if (!utf8_strcasecmp((const unsigned char*)(*succ)->text.c_str(), (const unsigned char*)(*it)->text.c_str()))
			(*it)->flags|= tword_dup;
		}

	    text_out << (*it)->text << ";" << (*it)->flags << ";" << (*it)->id << endl;

	    it = succ;
	}

    id_out << Id.size() << endl;	// Number of entries.
    it = Id.begin();
    stop = Id.end();
    for (; it!=stop; ++it)
	{
	    id_out << (*it)->id << ";" << (*it)->flags << ";" << (*it)->text << endl;
	}
}
