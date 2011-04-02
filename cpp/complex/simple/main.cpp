#include "complex.h"
#include "test.h"
#include <cmath>

using namespace Test;
using namespace Homeworks;

#define TEST_ASSERT_EQ(c, a, b) TEST_ASSERT((c).re() == (a) && (c).im() == (b))

int main(){

TEST_CASE_A("Constructors")
	Complex c;
	TEST_ASSERT_EQ(c, 0, 0);
	int a = 1234, b = 5678;
	c = Complex(a, b);
	TEST_ASSERT_EQ(c, a, b);
TEST_CASE_END

TEST_CASE_A("const math operators")
	Complex c = Complex(12, 34);
	c = c + Complex(12, 21);
	TEST_ASSERT_EQ(c, 24, 55);

TEST_CASE_END

TEST_CASE_A("setting math operators")
	Complex c = Complex(1234, 5678);
	c += 9;
	TEST_ASSERT_EQ(c, 1243, 5678);
	c += Complex(-9, -5600);
	TEST_ASSERT_EQ(c, 1234, 78);
	c -= Complex(1200, 72);
	TEST_ASSERT_EQ(c, 34, 6);
	c *= Complex(-1, 1);
	TEST_ASSERT_EQ(c, -40, 28);
	c /= Complex(34, 6);
	TEST_ASSERT_EQ(c, -1, 1);
	c *= 6;
	TEST_ASSERT_EQ(c, -6, 6);
TEST_CASE_END

TEST_CASE_A("eq opers")
	Complex c = Complex(1234, 5678);
	TEST_ASSERT(c == Complex(1234, 5678));
	c *= 8765;
	TEST_ASSERT(c == Complex(1234 *8765, 5678 *8765));
	TEST_ASSERT(c != Complex(123, 234));
TEST_CASE_END

TEST_CASE_A("angle, radius")
	const double PI = 3.14159265;
	Complex c = Complex(1, 0);
	output << c.radius() << " " << c.angle() << endl;
	TEST_ASSERT(c.angle() == 0 && c.radius() == 1);
	c = Complex(5, 5);
	TEST_ASSERT(c.angle() - PI / 4 < 1E-5 && c.radius() - 5*sqrt(2) < 1E-5);
TEST_CASE_END

    return 0;
}

