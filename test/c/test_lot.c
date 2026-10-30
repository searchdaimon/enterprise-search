/* Unit tests for the DocID <-> lot arithmetic in src/common/lot.c */
#include "tap.h"
#include "common/define.h"
#include "common/lot.h"

int main(void)
{
	unsigned int n = NrofDocIDsInLot;

	diag("NrofDocIDsInLot = %u", n);

	is_int(rLotForDOCid(0), 1, "DocID 0 is in lot 1");
	is_int(rLotForDOCid(1), 1, "DocID 1 is in lot 1");
	is_int(rLotForDOCid(n - 1), 1, "last DocID of lot 1");
	is_int(rLotForDOCid(n), 2, "first DocID of lot 2");
	is_int(rLotForDOCid(2 * n), 3, "first DocID of lot 3");
	is_int(rLotForDOCid(10 * n + 7), 11, "arbitrary DocID in lot 11");

	is_int(LotDocIDOfset(1), 0, "lot 1 starts at offset 0");
	is_int(LotDocIDOfset(2), n, "lot 2 starts at NrofDocIDsInLot");
	is_int(LotDocIDOfset(5), 4 * n, "lot 5 offset");

	/* Round-trip invariants between the two helpers. */
	{
		int lot, allok = 1;
		for (lot = 1; lot <= 50 && allok; lot++) {
			unsigned int first = LotDocIDOfset(lot);
			unsigned int last = first + n - 1;
			if (rLotForDOCid(first) != lot || rLotForDOCid(last) != lot ||
			    rLotForDOCid(last + 1) != lot + 1)
				allok = 0;
		}
		ok(allok, "every lot's offset range maps back to the same lot");
	}

	return done_testing();
}
