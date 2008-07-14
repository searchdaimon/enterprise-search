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
