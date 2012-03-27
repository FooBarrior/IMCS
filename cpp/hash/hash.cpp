#include <cstdio>
#include <iostream>

#include "hash.h"
#include "stdhashf/string.h"

using namespace Homeworks;
using namespace std;

int testNum;
template<class T> void test(const T& actual, const T& expected, const char* errMsg, int line) {
	testNum++;
	clog << "Test " << testNum << ": ";
	if (actual == expected)
		clog << "OK" << endl;
	else
		clog << "FAILED, " << errMsg << ", line " << line << endl;
}

int main(){
{
	HashMap<string, int> h;
	test(h.size(), 0, "HashMap(rep_func)", __LINE__);
	test(h.empty(), true, "empty() true", __LINE__);
	h["666"] = 823;
	test(h.empty(), false, "empty() false", __LINE__);
	h["123"] = 123;
	test(h.size(), 2, "size()", __LINE__);
	HashMap<string, int> h1(h);
	test(h, h1, "HashMap(const HashMap&)", __LINE__);
	h1["ababaca"] = 20;
	HashMap<string, int> h2;
	h2 = h1;
	test(h1, h2, "operator=(const HashMap&)", __LINE__);
	h2.clear();
	HashMap<string, int> h3;
	test(h2, h3, "clear()", __LINE__);
}
testNum = 0;
{
	HashMap<int, int> h;
	test(h.size(), 0, "HashMap()", __LINE__);
	test(h.begin() == h.end(), true, "begin() == end() true", __LINE__);
	test(h.begin() != h.end(), false, "begin() != end() false", __LINE__);
	h[666];
	test(h.begin() == h.end(), false, "begin() == end() false", __LINE__);
	test(h.begin() != h.end(), true, "begin() != end() true", __LINE__);
	test(++h.begin() == h.end(), true, "Iterator::operator++()", __LINE__);
	test((h.begin()++) == h.end(), true, "Iterator::operator++(int)", __LINE__);
	HashMap<int, int>::Iterator i = h.begin();
	i++;
	test(i == h.end(), true, "Iterator::operator++(int)", __LINE__);
}
testNum = 0;
{
	HashMap<string, int> h;
	test(h["qwer"], 0, "operator[] insert", __LINE__);
	test(h.size(), 1, "operator[] insert", __LINE__);
	test(h["qwer"] = 666, 666, "operator[] write", __LINE__);
	test(h["qwer"], 666, "operator[] read", __LINE__);
	h.insert("ababaca", 777);
	test(h.size(), 2, "insert(const KeyT&, const ValT&)", __LINE__);
	test(h["ababaca"], 777, "insert(const KeyT&, const ValT&)", __LINE__);
	h.remove("ababaca");
	test(h.size(), 1, "remove(const KeyT&)", __LINE__);
	test(h["ababaca"], 0, "remove(const KeyT&)", __LINE__);
}
	return 0;
}