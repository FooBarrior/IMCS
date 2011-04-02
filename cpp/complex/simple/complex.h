#ifndef COMPLEX_H
#define	COMPLEX_H

#include <iostream>

namespace Homeworks{



class Complex{

private:
    double real, imag;

public:
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}

    inline double re() const{
        return real;
    }
    inline double im() const{
        return imag;
    }
    
	double radius() const;
	double angle() const;

    Complex operator+(const Complex &c) const;
    Complex operator-(const Complex &c) const;
    Complex operator*(const Complex &c) const;
    Complex operator/(const Complex &c) const;
    
	inline Complex& operator+=(const Complex &c){
		return *this = *this + c;
	}
    inline Complex& operator-=(const Complex &c){
		return *this = *this - c;
	}
    inline Complex& operator*=(const Complex &c){
		return *this = *this * c;
	}
    inline Complex& operator/=(const Complex &c){
		return *this = *this / c;
	}

    inline bool operator==(const Complex &c) const{
		return real == c.re() && imag == c.im();
	}
    inline bool operator!=(const Complex &c) const{
		return real != c.re() || imag != c.im();
	}
    
	friend std::ostream& operator<<(std::ostream &out, const Complex &c){
		return out << c.re() << (c.im() > 0 ? "+" : "-") << c.im() <<"i";
	}

}; //Complex



}; //Homeworks
#endif /* COMPLEX_H */

