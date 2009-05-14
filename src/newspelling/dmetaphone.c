#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>

int
strstrs(wchar_t *a, int len, ...)
{
	va_list va;
	wchar_t *t;

	va_start(va, len);
	while ((t = va_arg(va, wchar_t *))) {
		if (t[0] == '\0')
			break;
		if (wcsncmp(a, t, len) == 0) {
			va_end(va);
			return 1;
		}
	}
	va_end(va);

	return 0;
}

int
is_vowel(wchar_t ch)
{
	switch(ch) {
		case L'A': case L'E': case L'I': case L'O': case L'U': case L'Y':
			return 1;
	}
	return 0;
}

int
slavo_germanic(wchar_t *word)
{
	// XXX
        if(wcsstr(word, L"W") || wcsstr(word, L"K") || wcsstr(word, L"CZ") || wcsstr(word, L"WITZ"))
		return 1;

	return 0;
}

#define DMPAdd(_p, c) do { \
		*(_p) = c; \
		(_p)++; \
	} while (0)

#if 0
bool MString::SlavoGermanic()
{       
        if((Find(L'W') > -1) OR (Find(L'K') > -1) OR (Find(L"CZ") > -1) OR (Find("WITZ") > -1))
                return TRUE;

        return FALSE;
}
#endif


wchar_t *
dmetaphone(wchar_t *word)
{
	size_t len = wcslen(word);
	wchar_t *_w = malloc((2*len+10) * sizeof(wchar_t)), *w;
	wchar_t *p = malloc((2*len+10) * sizeof(wchar_t));
	wchar_t *p2 = p;
	int offset = 0;
	int last = len - 1;
	int i;

	int GetAt(int i) {
		return w[i];
	}

	memset(_w, '\0', (2*len+10) * sizeof(wchar_t));
	memset(p, '\0', (2*len+10) * sizeof(wchar_t));
	//p2 += 4;

	w = _w + 4;
	wcscpy(w, word);

	for (i = 0; w[i]; i++)
		w[i] = toupper(w[i]);

	/* Skip GN, KN, PN, WR, PS when they start a word */
	if (strstrs(w, 2, L"GN", L"PN", L"WR", L"PS", NULL)) {
		//printf("Skiping start of word: %.2ls\n", w);
		offset += 2;
	}

	// X at start of a word is pronounced Z
	// Z maps to S
	if (w[0] == L'X') {
		//printf("Xavier?\n");
		DMPAdd(p2, L'S');
		offset++;
	}

	for (;;) {
		if (offset >= len)
			break;
		switch (w[offset]) {
			case L'A': case L'E': case L'I': case L'O': case L'U': case L'Y':
				// All starting vowels map to A
				if (offset == 0)
					DMPAdd(p2, L'A');
				offset++;
				break;
			case L'B':
				DMPAdd(p2, L'P');
				offset++;
				if (w[offset] == L'B')
					offset++;
				break;
			case L'C':
                                //various germanic
                                if((offset > 1) &&
				    !is_vowel(w[offset-2]) &&
				    strstrs(&w[offset-1], 3, L"ACH", NULL) &&
				    ((w[offset+2] != L'I') &&
				     ((w[offset+2] != L'E') &&
				      strstrs(&w[offset-2], 6, L"BACHER", L"MACHER", NULL)))) {
                                        DMPAdd(p2, L'K');
                                        offset += 2;
                                        break;
                                }
				/* Special handling for caesar */
				if (offset == 0 && wcsncmp(w, L"CAESAR", 6) == 0) {
					DMPAdd(p2, L'S');
					offset +=2;
					break;
				}
				//italian 'chianti'
				if(wcsncmp(&w[offset], L"CHIA", 4) == 0) {
					DMPAdd(p2, L'K');
					offset +=2;
					break;
				}

				if (wcsncmp(&w[offset], L"CH", 2) == 0) {
                                        //find 'michael'
                                        if(offset > 0 && strstrs(&w[offset], 4, L"CHAE", NULL)) {
                                                //DMPAdd(p2, L"K", "X");
                                                DMPAdd(p2, L'K');
                                                offset += 2;
                                                break;
                                        }

                                        //greek roots e.g. 'chemistry', 'chorus'
                                        if (offset == 0 &&
                                            (strstrs(&w[offset+1], 5, L"HARAC", L"HARIS", NULL)  ||
                                             strstrs(&w[offset+1], 3, L"HOR", L"HYM", L"HIA", L"HEM", NULL)) &&
                                             !strstrs(w, 5, L"CHORE", NULL)) {
                                                DMPAdd(p2, L'K');
                                                offset += 2;
                                                break;
                                        }

                                        //germanic, greek, or otherwise 'ch' for 'kh' sound
                                        if((strstrs(w, 4, L"VAN ", L"VON ", NULL) || strstrs(w, 3, L"SCH", NULL))
                                                // 'architect but not 'arch', 'orchestra', 'orchid'
                                                || strstrs(&w[(offset - 2)], 6, L"ORCHES", L"ARCHIT", L"ORCHID", NULL)
                                                        || strstrs(&w[(offset + 2)], 1, L"T", L"S", NULL)
                                                                || ((strstrs(&w[(offset - 1)], 1, L"A", L"O", L"U", L"E", NULL) || (offset == 0))
                                                                        //e.g., 'wachtler', 'wechsler', but not 'tichner'
                                                                        && strstrs(&w[(offset + 2)], 1, L"L", L"R", L"N", L"M", L"B", L"H", L"F", L"V", L"W", L" ", NULL))) {
                                        
                                                DMPAdd(p2, L'K');
                                        }else {  
                                                if(offset > 0) {
                                                
                                                        if(strstrs(w, 2, L"MC", NULL))
                                                                //e.g., L"McHugh"
                                                                DMPAdd(p2, L'K');
                                                        else
                                                                //DMPAdd(p2, L'X', L'K');
                                                                DMPAdd(p2, L'X');
                                                } else {
                                                        DMPAdd(p2, L'X');
						}
                                        }
                                        offset += 2;
                                        break;
                                }
                                //e.g, 'czerny'
                                if(strstrs(&w[offset], 2, L"CZ", NULL) && !strstrs(&w[(offset - 2)], 4, L"WICZ", NULL)) {
                                        DMPAdd(p2, L'S');
                                        //DMPAdd(p2, L"S", "X");
                                        offset += 2;
                                        break;
                                }

                                //e.g., 'focaccia'
                                if(strstrs(&w[(offset + 1)], 3, L"CIA", NULL)) {
                                        DMPAdd(p2, L'X');
                                        offset += 3;
                                        break;
                                }

                                //double L'C', but not if e.g. 'McClellan'
                                if(strstrs(&w[offset], 2, L"CC", NULL) && !((offset == 1) && (w[0] == L'M'))) {
                                        //'bellocchio' but not 'bacchus'
                                        if(strstrs(&w[(offset + 2)], 1, L"I", L"E", L"H", NULL) && !strstrs(&w[(offset + 2)], 2, "HU", NULL)) {
                                                //'accident', 'accedeL' 'succeed'
                                                if(((offset == 1) && (w[(offset - 1)] == L'A')) 
                                                                || strstrs(&w[(offset - 1)], 5, L"UCCEE", L"UCCES", NULL)) {
                                                        DMPAdd(p2, L'K');
							DMPAdd(p2, L'S');
                                                //'bacci', 'bertucci', other italian
                                                } else {
                                                        DMPAdd(p2, L'X');
						}
                                                offset += 3;
                                                break;
                                        }else{//Pierce's rule
                                                DMPAdd(p2, L'K');
                                                offset += 2;
                                                break;
                                        }
				}

                                if(strstrs(&w[offset], 2, L"CK", L"CG", L"CQ", NULL)) {
                                
                                        DMPAdd(p2, L'K');
                                        offset += 2;
                                        break;
                                }

                                if(strstrs(&w[offset], 2, L"CI", L"CE", L"CY", NULL)) {
                                
                                        //italian vs. english
                                        if(strstrs(&w[offset], 3, L"CIO", L"CIE", L"CIA", NULL))
                                                //DMPAdd(p2, L"S", "X");
                                                DMPAdd(p2, L'S');
                                        else
                                                DMPAdd(p2, L'S');
                                        offset += 2;
                                        break;
                                }

                                //else
                                DMPAdd(p2, L'K');
                                
                                //name sent in 'mac caffrey', 'mac gregor
                                if(strstrs(&w[(offset + 1)], 2, L" C", L" Q", L" G", NULL))
                                        offset += 3;
                                else
                                        if(strstrs(&w[(offset + 1)], 1, L"C", L"K", L"Q", NULL) 
                                                && !strstrs(&w[(offset + 1)], 2, L"CE", L"CI", NULL))
                                                offset += 2;
                                        else
                                                offset += 1;
                                break;
                        case L'D':
                                if(strstrs(&w[offset], 2, L"DG", NULL)) {
                                        if(strstrs(&w[(offset + 2)], 1, L"I", L"E", L"Y", NULL)) {
                                                //e.g. 'edge'
                                                DMPAdd(p2, L'J');
                                                offset += 3;
                                                break;
                                        }else{
                                                //e.g. 'edgar'
                                                DMPAdd(p2, L'T');
                                                DMPAdd(p2, L'K');
                                                offset += 2;
                                                break;
                                        }
				}

                                if(strstrs(&w[offset], 2, L"DT", L"DD", NULL)) {
                                        DMPAdd(p2, L'T');
                                        offset += 2;
                                        break;
                                }
                                
                                //else
                                DMPAdd(p2, L'T');
                                offset += 1;
                                break;
                        case L'F':
                                if(w[(offset + 1)] == L'F')
                                        offset += 2;
                                else
                                        offset += 1;
                                DMPAdd(p2, L'F');
                                break;

                        case L'G':
                                if(w[(offset + 1)] == L'H') {
                                
                                        if((offset > 0) && !is_vowel(w[offset - 1])) {
                                        
                                                DMPAdd(p2, L'K');
                                                offset += 2;
                                                break;
                                        }

                                        if(offset < 3) {
                                        
                                                //'ghislane', ghiradelli
                                                if(offset == 0) {
                                                 
                                                        if(w[(offset + 2)] == L'I')
                                                                DMPAdd(p2, L'J');
                                                        else
                                                                DMPAdd(p2, L'K');
                                                        offset += 2;
                                                        break;
                                                }
                                        }
                                        //Parker's rule (with some further refinements) - e.g., 'hugh'
                                        if(((offset > 1) && strstrs(&w[(offset - 2)], 1, L"B", L"H", L"D", NULL) )
                                                //e.g., 'bough'
                                                || ((offset > 2) && strstrs(&w[(offset - 3)], 1, L"B", L"H", L"D", NULL) )
                                                //e.g., 'broughton'
                                                || ((offset > 3) && strstrs(&w[(offset - 4)], 1, L"B", L"H", NULL) ) ) {
                                        
                                                offset += 2;
                                                break;
                                        }else{
                                                //e.g., 'laugh', 'McLaughlin', 'cough', 'gough', 'rough', 'tough'
                                                if((offset > 2) 
                                                        && (w[(offset - 1)] == L'U') 
                                                        && strstrs(&w[(offset - 3)], 1, L"C", L"G", L"L", L"R", L"T", NULL) ) {
                                                
                                                        DMPAdd(p2, L'F');
                                                }else
                                                        if((offset > 0) && w[(offset - 1)] != L'I')
                                                                DMPAdd(p2, L'K');

                                                offset += 2;
                                                break;
                                        }
                                }

                                if(w[(offset + 1)] == L'N') {
                                
                                        if((offset == 1) && is_vowel(w[0]) && !slavo_germanic(w)) {
                                        
                                                DMPAdd(p2, L'K');
                                                DMPAdd(p2, L'N');
                                                //DMPAdd(p2, L"KN", "N");
                                        }else
                                                //not e.g. 'cagney'
                                                if(!strstrs(&w[(offset + 2)], 2, L"EY", NULL) 
                                                                && (w[(offset + 1)] != L'Y') && !slavo_germanic(w)) {
                                                
                                                        DMPAdd(p2, L'N');
                                                        //DMPAdd(p2, L"N", "KN");
                                                }else {
                                                        DMPAdd(p2, L'K');
                                                        DMPAdd(p2, L'N');
						}
                                        offset += 2;
                                        break;
                                }

                                //'tagliaro'
                                if(strstrs(&w[(offset + 1)], 2, L"LI", NULL) && !slavo_germanic(w)) {
                                
                                        DMPAdd(p2, L'K');
                                        DMPAdd(p2, L'L');
                                        //DMPAdd(p2, L"KL", "L");
                                        offset += 2;
                                        break;
                                }

                                //-ges-,-gep-,-gel-, -gie- at beginning
                                if((offset == 0)
                                        && ((w[(offset + 1)] == L'Y') 
                                                || strstrs(&w[(offset + 1)], 2, L"ES", "EP", "EB", "EL", "EY", "IB", "IL", "IN", "IE", "EI", "ER", NULL)) ) {
                                
                                        DMPAdd(p2, L'K');
                                        //DMPAdd(p2, L"K", "J");
                                        offset += 2;
                                        break;
                                }

                                // -ger-,  -gy-
                                if((strstrs(&w[(offset + 1)], 2, L"ER", NULL) || (w[(offset + 1)] == L'Y'))
                                                && !strstrs(w, 6, L"DANGER", L"RANGER", L"MANGER", NULL)
                                                        && !strstrs(&w[(offset - 1)], 1, L"E", L"I", NULL) 
                                                                && !strstrs(&w[(offset - 1)], 3, L"RGY", L"OGY", NULL) ) {
                                
                                        DMPAdd(p2, L'K');
                                        //DMPAdd(p2, L"K", "J");
                                        offset += 2;
                                        break;
                                }

                                // italian e.g, 'biaggi'
                                if(strstrs(&w[(offset + 1)], 1, L"E", L"I", L"Y", NULL) || strstrs(&w[(offset - 1)], 4, L"AGGI", L"OGGI", NULL)) {
                                
                                        //obvious germanic
                                        if((strstrs(w, 4, L"VAN ", L"VON ", NULL) || strstrs(w, 3, L"SCH", NULL))
                                                || strstrs(&w[(offset + 1)], 2, L"ET", NULL))
                                                DMPAdd(p2, L'K');
                                        else
                                                //always soft if french ending
                                                if(strstrs(&w[(offset + 1)], 4, L"IER ", NULL))
                                                        DMPAdd(p2, L'J');
                                                else
                                                        //DMPAdd(p2, L"J", "K");
                                                        DMPAdd(p2, L'J');
                                        offset += 2;
                                        break;
                                }

                                if(w[(offset + 1)] == L'G')
                                        offset += 2;
                                else
                                        offset += 1;
                                DMPAdd(p2, L'K');
                                break;

                        case L'H':
                                //only keep if first & before vowel or btw. 2 vowels
                                if(((offset == 0) || is_vowel(w[offset - 1])) 
                                        && is_vowel(w[offset + 1])) {
                                
                                        DMPAdd(p2, L'H');
                                        offset += 2;
                                } else { //also takes care of 'HH'
                                        offset += 1;
				}
                                break;
                        case L'J':
                                //obvious spanish, 'jose', 'san jacinto'
                                if(strstrs(&w[offset], 4, L"JOSE", NULL) || strstrs(&w[0], 4, L"SAN ", NULL) ) {
                                
                                        if(((offset == 0) && (w[(offset + 4)] == L' ')) || strstrs(&w[0], 4, L"SAN ", NULL) )
                                                DMPAdd(p2, L'H');
                                        else {
                                        
                                                //DMPAdd(p2, L"J", "H");
                                                DMPAdd(p2, L'J');
                                        }
                                        offset += 1;
                                        break;
                                }

                                if((offset == 0) && !strstrs(&w[offset], 4, L"JOSE", NULL))
                                        DMPAdd(p2, L'J');//Yankelovich/Jankelowicz
                                        //DMPAdd(p2, L"J", "A");//Yankelovich/Jankelowicz
                                else
                                        //spanish pron. of e.g. 'bajador'
                                        if(is_vowel(w[offset - 1]) 
                                                && !slavo_germanic(w)
                                                        && ((w[(offset + 1)] == L'A') || (w[(offset + 1)] == L'O')))
                                                DMPAdd(p2, L'J');
                                                //DMPAdd(p2, L"J", "H");
                                        else
                                                if(offset == last)
                                                        DMPAdd(p2, L'J');
                                                        //DMPAdd(p2, L"J", " ");
                                                else
                                                        if(!strstrs(&w[(offset + 1)], 1, L"L", L"T", L"K", L"S", L"N", L"M", L"B", L"Z", NULL) 
                                                                        && !strstrs(&w[(offset - 1)], 1, L"S", L"K", L"L", NULL))
                                                                DMPAdd(p2, L'J');

                                if(w[(offset + 1)] == L'J')//it could happen!
                                        offset += 2;
                                else
                                        offset += 1;
                                break;

                        case L'K':
                                if(w[(offset + 1)] == L'K')
                                        offset += 2;
                                else
                                        offset += 1;
                                DMPAdd(p2, L'K');
                                break;

                        case L'L':
                                if(GetAt(offset + 1) == L'L') {
                                
                                        //spanish e.g. 'cabrillo', 'gallegos'
                                        if(((offset == (len - 3)) 
                                                && strstrs(&w[(offset - 1)], 4, L"ILLO", L"ILLA", L"ALLE", NULL))
                                                         || ((strstrs(&w[(last - 1)], 2, L"AS", L"OS", NULL) || strstrs(&w[last], 1, L"A", L"O", NULL)) 
                                                                && strstrs(&w[(offset - 1)], 4, L"ALLE", NULL)) ) {
                                        
                                                DMPAdd(p2, L'L');
                                                //DMPAdd(p2, L"L", " ");
                                                offset += 2;
                                                break;
                                        }
                                        offset += 2;
                                }else
                                        offset += 1;
                                DMPAdd(p2, L'L');
                                break;

                        case L'M':
                                if((strstrs(&w[(offset - 1)], 3, L"UMB", NULL) 
                                        && (((offset + 1) == last) || strstrs(&w[(offset + 2)], 2, L"ER", NULL)))
                                                //'dumbL','thumb'
                                                ||  (GetAt(offset + 1) == L'M') )
                                        offset += 2;
                                else
                                        offset += 1;
                                DMPAdd(p2, L'M');
                                break;

                        case L'N':
                                if(GetAt(offset + 1) == L'N')
                                        offset += 2;
                                else
                                        offset += 1;
                                DMPAdd(p2, L'N');
                                break;

#if  0
                        case L'Ñ':
                                offset += 1;
                                DMPAdd(p2, L"N");
                                break;
#endif

                        case L'P':
                                if(GetAt(offset + 1) == L'H') {
                                
                                        DMPAdd(p2, L'F');
                                        offset += 2;
                                        break;
                                }

                                //also account for L"campbell", "raspberry"
                                if(strstrs(&w[(offset + 1)], 1, L"P", L"B", NULL))
                                        offset += 2;
                                else
                                        offset += 1;
                                        DMPAdd(p2, L'P');
                                break;

                        case L'Q':
                                if(GetAt(offset + 1) == L'Q')
                                        offset += 2;
                                else
                                        offset += 1;
                                DMPAdd(p2, L'K');
                                break;

                        case L'R':
                                //french e.g. 'rogier', but exclude 'hochmeier'
                                if((offset == last)
                                        && !slavo_germanic(w)
                                                && strstrs(&w[(offset - 2)], 2, L"IE", NULL) 
                                                        && !strstrs(&w[(offset - 4)], 2, L"ME", L"MA", NULL)) {
                                        //DMPAdd(p2, L"", "R");
					;
                                } else {
                                        DMPAdd(p2, L'R');
				}

                                if(GetAt(offset + 1) == L'R')
                                        offset += 2;
                                else
                                        offset += 1;
                                break;

                        case L'S':
                                //special cases 'island', 'isle', 'carlisle', 'carlysle'
                                if(strstrs(&w[(offset - 1)], 3, L"ISL", L"YSL", NULL)) {
                                
                                        offset += 1;
                                        break;
                                }

                                //special case 'sugar-'
                                if((offset == 0) && strstrs(&w[offset], 5, L"SUGAR", NULL)) {
                                
                                        DMPAdd(p2, L'X');
                                        //DMPAdd(p2, L"X", "S");
                                        offset += 1;
                                        break;
                                }

                                if(strstrs(&w[offset], 2, L"SH", NULL)) {
                                
                                        //germanic
                                        if(strstrs(&w[(offset + 1)], 4, L"HEIM", L"HOEK", L"HOLM", L"HOLZ", NULL))
                                                DMPAdd(p2, L'S');
                                        else
                                                DMPAdd(p2, L'X');
                                        offset += 2;
                                        break;
                                }

                                //italian & armenian
                                if(strstrs(&w[offset], 3, L"SIO", L"SIA", NULL) || strstrs(&w[offset], 4, L"SIAN", NULL)) {
                                
                                        if(!slavo_germanic(w))
                                                DMPAdd(p2, L'S');
                                                //DMPAdd(p2, L"S", "X");
                                        else
                                                DMPAdd(p2, L'S');
                                        offset += 3;
                                        break;
                                }

                                //german & anglicisations, e.g. 'smith' match 'schmidt', 'snider' match 'schneider'
                                //also, -sz- in slavic language altho in hungarian it is pronounced L's'
                                if(((offset == 0) 
                                                && strstrs(&w[(offset + 1)], 1, L"M", L"N", L"L", L"W", NULL))
                                                        || strstrs(&w[(offset + 1)], 1, L"Z", NULL)) {
                                
                                        DMPAdd(p2, L'S');
                                        //DMPAdd(p2, L"S", "X");
                                        if(strstrs(&w[(offset + 1)], 1, L"Z", NULL))
                                                offset += 2;
                                        else
                                                offset += 1;
                                        break;
                                }

                                if(strstrs(&w[offset], 2, L"SC", NULL)) {
                                
                                        //Schlesinger's rule
                                        if(GetAt(offset + 2) == L'H') {
                                                //dutch origin, e.g. 'school', 'schooner'
                                                if(strstrs(&w[(offset + 3)], 2, L"OO", L"ER", L"EN", L"UY", L"ED", L"EM", NULL)) {
                                                
                                                        //'schermerhorn', 'schenker'
                                                        if(strstrs(&w[(offset + 3)], 2, L"ER", L"EN", NULL)) {
                                                        
                                                                DMPAdd(p2, L'X');
                                                                //DMPAdd(p2, L"X", "SK");
                                                        }else {
                                                                DMPAdd(p2, L'S');
                                                                DMPAdd(p2, L'K');
							}
                                                        offset += 3;
                                                        break;
                                                }else{
                                                        if((offset == 0) && !is_vowel(w[3]) && (w[3] != L'W'))
                                                                DMPAdd(p2, L'X');
                                                                //DMPAdd(p2, L"X", "S");
                                                        else
                                                                DMPAdd(p2, L'X');
                                                        offset += 3;
                                                        break;
                                                }
					}

                                        if(strstrs(&w[(offset + 2)], 1, L"I", L"E", L"Y", NULL)) {
                                        
                                                DMPAdd(p2, L'S');
                                                offset += 3;
                                                break;
                                        }
                                        //else
                                        DMPAdd(p2, L'S');
                                        DMPAdd(p2, L'K');
                                        offset += 3;
                                        break;
                                }

                                //french e.g. 'resnais', 'artois'
                                if((offset == last) && strstrs(&w[(offset - 2)], 2, L"AI", L"OI", NULL))
					;
                                        //DMPAdd(p2, L"", "S");
                                else
                                        DMPAdd(p2, L'S');

                                if(strstrs(&w[(offset + 1)], 1, L"S", L"Z", NULL))
                                        offset += 2;
                                else
                                        offset += 1;
                                break;

                        case L'T':
                                if(strstrs(&w[offset], 4, L"TION", NULL)) {
                                
                                        DMPAdd(p2, L'X');
                                        offset += 3;
                                        break;
                                }

                                if(strstrs(&w[offset], 3, L"TIA", L"TCH", NULL)) {
                                
                                        DMPAdd(p2, L'X');
                                        offset += 3;
                                        break;
                                }

                                if(strstrs(&w[offset], 2, L"TH", NULL) 
                                        || strstrs(&w[offset], 3, L"TTH", NULL)) {
                                
                                        //special case 'thomas', 'thames' or germanic
                                        if(strstrs(&w[(offset + 2)], 2, L"OM", L"AM", NULL) 
                                                || strstrs(&w[0], 4, L"VAN ", L"VON ", NULL) 
                                                        || strstrs(&w[0], 3, L"SCH", NULL)) {
                                        
                                                DMPAdd(p2, L'T');
                                        }else{
                                                DMPAdd(p2, L'0');
                                                //DMPAdd(p2, L"0", "T");
                                        }
                                        offset += 2;
                                        break;
                                }

                                if(strstrs(&w[(offset + 1)], 1, L"T", L"D", NULL))
                                        offset += 2;
                                else
                                        offset += 1;
                                DMPAdd(p2, L'T');
                                break;

                        case L'V':
                                if(GetAt(offset + 1) == L'V')
                                        offset += 2;
                                else
                                        offset += 1;
                                DMPAdd(p2, L'F');
                                break;

                        case L'W':
                                //can also be in middle of word
                                if(strstrs(&w[offset], 2, L"WR", NULL)) {
                                
                                        DMPAdd(p2, L'R');
                                        offset += 2;
                                        break;
                                }

                                if((offset == 0) 
                                        && (is_vowel(w[offset + 1]) || strstrs(&w[offset], 2, L"WH", NULL))) {
                                
                                        //Wasserman should match Vasserman
                                        if(is_vowel(w[offset + 1]))
                                                DMPAdd(p2, L'A');
                                                //DMPAdd(p2, L"A", "F");
                                        else
                                                //need Uomo to match Womo
                                                DMPAdd(p2, L'A');
                                }

                                //Arnow should match Arnoff
                                if(((offset == last) && is_vowel(w[offset - 1])) 
                                        || strstrs(&w[offset-1], 5, L"EWSKI", "EWSKY", "OWSKI", "OWSKY", NULL) 
                                                        || strstrs(w, 3, L"SCH", NULL)) {
				  
                                        //DMPAdd(p2, L"", "F");
                                        offset += 1;
                                        break;
                                }

                                //polish e.g. 'filipowicz'
                                if(strstrs(&w[offset], 4, L"WICZ", L"WITZ", NULL)) {
                                
                                        DMPAdd(p2, L'T');
                                        DMPAdd(p2, L'S');
                                        //DMPAdd(p2, L"TS", "FX");
                                        offset += 4;
                                        break;
                                }

                                //else skip it
                                offset += 1;
                                break;

                        case L'X':
                                //french e.g. breaux
                                if(!((offset == last) 
                                        && (strstrs(&w[offset-3], 3, L"IAU", "EAU", NULL) 
                                                        || strstrs(&w[offset - 2], 2, L"AU", "OU", NULL))) ) {
                                        //DMPAdd(p2, L"KS");
                                        DMPAdd(p2, L'K');
                                        DMPAdd(p2, L'S');
				}

                                if(strstrs(&w[(offset + 1)], 1, L"C", L"X", NULL))
                                        offset += 2;
                                else
                                        offset += 1;
                                break;

                        case L'Z':
                                //chinese pinyin e.g. 'zhao'
                                if(w[offset+1] == L'H') {
                                        DMPAdd(p2, L'J');
                                        offset += 2;
                                        break;
                                } else {
                                        if (strstrs(&w[(offset + 1)], 2, L"ZO", L"ZI", L"ZA", NULL) 
					    || (slavo_germanic(w) && ((offset > 0) && GetAt(offset - 1) != L'T'))) {
                                        
                                                DMPAdd(p2, L'S');
                                                //DMPAdd(p2, L"S", "TS");
                                        } else {
                                                DMPAdd(p2, L'S');
					}
				}

                                if(w[offset + 1] == L'Z')
                                        offset += 2;
                                else
                                        offset += 1;
                                break;
			default:
				offset++;
				break;
		}
	}


	DMPAdd(p2, L'\0');

	free(_w);

	return p;
}


#ifdef DMP_TEST
void
dmp_test(wchar_t *a)
{
	wchar_t *p;

	printf("Trying: %ls\n", a);
	p = dmetaphone(a);
	printf("%ls => %ls\n", a, p);
	free(p);
}

int
main(int argc, char **argv)
{
	int i;
	wchar_t buf[2048];

#if 0
	for (i = 1; i < argc; i++) {
		mbtowc(buf, argv[i], sizeof(buf));
		dmp_test(buf);
	}
#endif
	dmp_test(L"zxzxzxzxzxzxzxzxzxzxz");

	return 0;
}
#endif
