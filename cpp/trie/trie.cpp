#include "trie.h"
#include <string>
#include <fstream>
#include <iostream>
#include <map>
using namespace std;
using namespace Homeworks;

Trie<std::string, int> t;
map<string, int> m, mm;
int main(){
	ifstream f("m&m.txt");/*
	t.insert("Buckets", 3);
	t.insert("Burstsort", 3);
	t.insert("Balancing", 3);
	t.insert("Burstsort,", 3);
	t.remove("Buckets");
	t.remove("Burstsort,");
	t.remove("Burstsort");
	t.remove("Balancing");*/
	//clog << t.exists("abc") << t.size();
	while(f >> ws, !f.eof()){
		string s;
		f >> s;if(s == "\"parse")
			s = s;
		t.insert(s, 1);
		assert(t.exists(s));
		m[s] = 1;
	}
	int sz = 0;
	/*
	for(Trie<string, int>::Iterator i = t.begin(); i != t.end(); i++, sz++){
		assert(m.find(i.key()) != m.end());
	}
	assert(sz == m.size() && sz == t.size());
*/
	clog << t.size();
	mm = m;
	for(map<string, int>::iterator i = m.begin(); i != m.end(); i++){
		assert(t.exists(i->first));
		mm.erase(i->first);
		clog << i->first << endl;
		t.remove(i->first);
		assert(!t.exists(i->first));
		for(map<string, int>::iterator j = mm.begin(); j != mm.end(); j++)
			assert(t.exists(j->first));
	}
	for(map<string, int>::iterator i = m.begin(); i != m.end(); i++){
		if(t.exists(i->first)) cout << "oppaa-nna";
	}
	clog << t.size();
	return 0;
}