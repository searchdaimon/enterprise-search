/*****************************************************************************
*                                                                            *
*  ------------------------------- mgsort.c -------------------------------  *
*                                                                            *
*****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "mgsort.h"

/*****************************************************************************
*                                                                            *
*  --------------------------------- merge --------------------------------  *
*                                                                            *
*****************************************************************************/

static int merge(void *data, int esize, int i, int j, int k, int (*compare)
   (const void *key1, const void *key2)) {

char               *a = data,
                   *m;

int                ipos,
                   jpos,
                   mpos;

/*****************************************************************************
*                                                                            *
*  Initialize the counters used in merging.                                  *
*                                                                            *
*****************************************************************************/

ipos = i;
jpos = j + 1;
mpos = 0;

/*****************************************************************************
*                                                                            *
*  Allocate storage for the merged elements.                                 *
*                                                                            *
*****************************************************************************/

if ((m = (char *)malloc(esize * ((k - i) + 1))) == NULL)
   return -1;

/*****************************************************************************
*                                                                            *
*  Continue while either division has elements to merge.                     *
*                                                                            *
*****************************************************************************/

while (ipos <= j || jpos <= k) {

   if (ipos > j) {

      /***********************************************************************
      *                                                                      *
      *  The left division has no more elements to merge.                    *
      *                                                                      *
      ***********************************************************************/

      while (jpos <= k) {

         memcpy(&m[mpos * esize], &a[jpos * esize], esize);
         jpos++;
         mpos++;

      }

      continue;

      }

   else if (jpos > k) {

      /***********************************************************************
      *                                                                      *
      *  The right division has no more elements to merge.                   *
      *                                                                      *
      ***********************************************************************/

      while (ipos <= j) {

         memcpy(&m[mpos * esize], &a[ipos * esize], esize);
         ipos++;
         mpos++;

      }

      continue;

   }

   /**************************************************************************
   *                                                                         *
   *  Append the next ordered element to the merged elements.                *
   *                                                                         *
   **************************************************************************/

   if (compare(&a[ipos * esize], &a[jpos * esize]) < 0) {

      memcpy(&m[mpos * esize], &a[ipos * esize], esize);
      ipos++;
      mpos++;

      }

   else {

      memcpy(&m[mpos * esize], &a[jpos * esize], esize);
      jpos++;
      mpos++;

   }

}

/*****************************************************************************
*                                                                            *
*  Prepare to pass back the merged data.                                     *
*                                                                            *
*****************************************************************************/

memcpy(&a[i * esize], m, esize * ((k - i) + 1));

/*****************************************************************************
*                                                                            *
*  Free the storage allocated for merging.                                   *
*                                                                            *
*****************************************************************************/

free(m);

return 0;

}

int mgsort(void *data, int size, int esize, int (*compare) (const void *key1, const void *key2)) {

	int 		i = 0;
	int 		k = size -1;

	return mgsort_intern(data,size,esize,i,k,compare);
}

/*****************************************************************************
*                                                                            *
*  -------------------------------- mgsort --------------------------------  *
*                                                                            *
*****************************************************************************/

int mgsort_intern(void *data, int size, int esize, int i, int k, int (*compare)
   (const void *key1, const void *key2)) {

int            	j;

/*****************************************************************************
*                                                                            *
*  Stop the recursion when no more divisions can be made.                    *
*                                                                            *
*****************************************************************************/

if (i < k) {

   /**************************************************************************
   *                                                                         *
   *  Determine where to divide the elements.                                *
   *                                                                         *
   **************************************************************************/

   j = (int)(((i + k - 1)) / 2);

   /**************************************************************************
   *                                                                         *
   *  Recursively sort the two divisions.                                    *
   *                                                                         *
   **************************************************************************/

   if (mgsort_intern(data, size, esize, i, j, compare) < 0)
      return -1;

   if (mgsort_intern(data, size, esize, j + 1, k, compare) < 0)
      return -1;

   /**************************************************************************
   *                                                                         *
   *  Merge the two sorted divisions into a single sorted set.               *
   *                                                                         *
   **************************************************************************/

   if (merge(data, esize, i, j, k, compare) < 0)
      return -1;

}

return 0;

}
