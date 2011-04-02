#include "complex.h"
#include <cmath>

namespace Homeworks{



Complex Complex::operator+(const Complex &c) const{
	return Complex(real + c.re(), imag + c.im());
}

Complex Complex::operator-(const Complex &c) const{
	return Complex(real - c.re(), imag - c.im());
}

Complex Complex::operator*(const Complex &c) const{
	return Complex(real*c.re() - imag*c.im(), real*c.im() + imag*c.re());
}

Complex Complex::operator/(const Complex &c) const{//(ac+bd)/t + i(bc - ad)/t
	double t = c.re()*c.re() + c.im()*c.im();
	return Complex( (real*c.re() + imag*c.im()) / t, (imag*c.re() - real*c.im()) / t );
}

double Complex::radius() const{
	return sqrt(real*real + imag*imag);
}

double Complex::angle() const{
	return atan2(imag, real);
}



}//namespace Homeworks

