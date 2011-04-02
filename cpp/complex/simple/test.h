#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <cstdio>
#include <string>

#define TEST_CASE Test::testCaseStart(); {
#define TEST_CASE_A(info) Test::testCaseStart(info); {
#define TEST_CASE_END Test::testCaseEnd(); }
#define TEST_ASSERT(res) (Test::testLine = __LINE__, testAssert(res))
#define TEST_ASSERT_A(res, info) (Test::testLine = __LINE__, testAssert(res, info))

namespace Test{



bool waitOnTestCaseEnd = false;
bool waitOnTestFail = false;

int testCaseNumber;
bool isCaseFailed;
int testLine;

std::ostream& output = std::cout;

using std::string;
using std::endl;

inline void testCaseInfoOutput(const string &info);
inline void testCaseStart(const string &info = "");
inline void testCaseEnd();
inline bool testAssert(bool res, const string &info = "");

void testCaseStart(const string& info){
	testCaseNumber++;
	testCaseInfoOutput(info);
	isCaseFailed = false;
}

void testCaseInfoOutput(const string& info){
	output << endl << "Test " << testCaseNumber << ": " << info << endl;
}

void testCaseEnd(){
	output << (isCaseFailed ? "FAILED" : "PASSED") << endl;
	if(waitOnTestCaseEnd) getchar();
}

bool testAssert(bool res, const string &info){
	isCaseFailed |= !res;
	if(info.size()) output << "\t" << info;
	if(!res){
		output << "\tfailed on line" << testLine << endl;
		if(waitOnTestFail) getchar();
	}
}



}; //namespace Test

#endif //TEST_H
