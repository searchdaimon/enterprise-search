#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define SWAP(a, b) do { char *__tmp = (a); (a) = (b); (b) = (__tmp); } while(0)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MIN3(a, b, c) MIN(a, MIN(b, c))

int
min3(int a, int b, int c)
{
	int min = a;
	if (b < min)
		min = b;
	if (c < min)
		min = c;
	return min;
}
const unsigned int cost_del = 1;
const unsigned int cost_ins = 1;
const unsigned int cost_sub = 1;

/* XXX: Add transposition */

#if 0
int
levenshteindistance(wchar_t *s1, wchar_t *s2)
{
	int n1 = wcslen(s1);
	int n2 = wcslen(s2);
	int _p[n2+1], _q[n2+1];
	int *p = _p, *q = _q;
	int *r;

	p[0] = 0;
	for (int j = 1; j <= n2; ++j)
		p[j] = p[j-1] + cost_ins;

	for (int i = 1; i <= n1; ++i) {
		q[0] = p[0] + cost_del;
		for (int j = 1; j <= n2; ++j) {
			int d_del = p[j] + cost_del;
			int d_ins = q[j-1] + cost_ins;
			int d_sub = p[j-1] + (s1[i-1] == s2[j-1] ? 0 : cost_sub);

			q[j] = min3(d_del, d_ins, d_sub);
		}
		r = p;
		p = q;
		q = r;
	}

	return p[n2];
}
#endif

#if 0
int DamerauLevenshteinDistance(char str1[1..lenStr1], char str2[1..lenStr2])
   // d is a table with lenStr1+1 rows and lenStr2+1 columns
   declare int d[0..lenStr1, 0..lenStr2]
   // i and j are used to iterate over str1 and str2
   declare int i, j, cost
 
   for i from 0 to lenStr1
       d[i, 0] := i
   for j from 1 to lenStr2
       d[0, j] := j
 
   for i from 1 to lenStr1
       for j from 1 to lenStr2
           if str1[i] = str2[j] then cost := 0
                                else cost := 1
           d[i, j] := minimum(
                                d[i-1, j  ] + 1,     // deletion
                                d[i  , j-1] + 1,     // insertion
                                d[i-1, j-1] + cost   // substitution
                            )
           if(i > 1 and j > 1 and str1[i] = str2[j-1] and str1[i-1] = str2[j]) then
               d[i, j] := minimum(
                                d[i, j],
                                d[i-2, j-2] + cost   // transposition
                             )
                                
 
   return d[lenStr1, lenStr2]

#endif
#if 1

int
levenshteindistance(wchar_t *s1, wchar_t *s2)
{
	int l1 = wcslen(s1);
	int l2 = wcslen(s2);
	int d[l1+1][l2+1], i, j;
	int cost;

	//printf("Checking: %ls %ls\n", s1, s2);

	for (i = 0; i <= l1; i++)
		d[i][0] = i;
	for (j = 0; j <= l2; j++)
		d[0][j] = j;
	
	for (i = 1; i <= l1; i++) {
		for (j = 1; j <= l2; j++) {
			if (s1[i-1] == s2[j-1])
				cost = 0;
			else
				cost = 1;
			d[i][j] = min3(
				d[i-1][j] + 1,	// deletion
				d[i][j-1] + 1,	// insertion
				d[i-1][j-1] + cost // substituion
			);
			if (i > 1 && j > 1 && s1[i-1] == s2[j-2] && s1[i-2] == s2[j-1])
				d[i][j] = MIN(d[i][j], d[i-2][j-2] + cost);
		}
	}

	return d[l1][l2];
}
#endif
