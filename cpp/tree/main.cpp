#include "b-tree.h"
#include "../test.h"

using namespace Homeworks;
using namespace Test;

int main(){
TEST_CASE_A("Initialization test and simple checks")
	typedef BTreeMap<int, int> BII;
	typedef BII::Iterator ITER;
	BII bm;
	TEST_ASSERT_A(!bm.getSize(), "size validation");
	ITER it = bm.first();
	TEST_ASSERT_A(it.isPastRear(), "iterator of first() of empty struct must point to past-rear element");
	bm[231] = 123;
	TEST_ASSERT_A(bm.getSize() == 1, "checking size of tree with 1 elem");
	int a = bm[231];
	TEST_ASSERT_A(a == 123 && bm[231] == 123, "chacking value");
	TEST_ASSERT(bm.first()->value);
	it = bm.first();
	TEST_ASSERT(!it.isPastRear() && !it.isBeforeFirst());
	it++;
	TEST_ASSERT(!it.isBeforeFirst() && it.isPastRear());
	return 0;
TEST_CASE_END
}

