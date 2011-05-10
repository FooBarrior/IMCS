#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <cstdio>
#include <string>

#define TEST_CASE Test::testCaseStart(); {
#define TEST_CASE_A(info) Test::testCaseStart(info); {
#define TEST_CASE_END Test::testCaseEnd(); }
#define TEST_ASSERT(res) (Test::testLine = __LINE__, testAssert(res))
#define TEST_ASSERT_A(res, info) (Test::testLine = __LINE__, testInfoOutput(info), testAssert(res))

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
inline void testInfoOutput(const string &info);
bool testAssert(bool res);

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

void testInfoOutput(string &info){
	if(info.size()) output << "\t" << info << "...";
}

bool testAssert(bool res){
	isCaseFailed |= !res;
	if(!res){
		output << endl << "\t\tfailed on line" << testLine << endl;
		if(waitOnTestFail) getchar();
	} else
		output << " OK" << endl;
}

}; //namespace Test

#endif //TEST_H
