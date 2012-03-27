#ifndef HOMEWORKS_HASH
#define HOMEWORKS_HASH

#include <cstddef>

namespace Homeworks{

namespace HashF{

	//Cormen said that Uncle Knuth said that this const demonstrates pretty good distribution
	//it's a shorthand for ( (sqrt(5) - 1) / 2 ) - that is, golden ratio minus one
	static const double HASH_MULTIPLIER = 0.61803398874;

	template<typename T>
	int hashf(const T &key, int size){
		double val = 0;
		for(size_t i = 0; i < sizeof(T); i++){
			val += ((unsigned char*)&key)[i] * (i + 1);
		}
			
		
		double f = HASH_MULTIPLIER * val;
		int res = (int) ((f - (int)f) * (size - 1));
		if(res < 0 || res > size) throw res;
		return res;
	}

};
int count = 0;
template<typename Key, typename Val>
class HashMap{
int name;
	inline int hashf(const Key &key) const{
		return Homeworks::HashF::hashf<Key>(key, bucketSize);
	}

	static const int MIN_BUCKET_SIZE = 2;
	int bucketSize;

public:
	struct Pair;

private:
	struct Node{
		Node *n;
		Pair p;
		Node(const Key &k, const Val v, Node *n = NULL): p(k, v), n(n){}
	};
	typedef Node *PNode;

	PNode *bucket;
	int _size;

	PNode findNode(const Key &k) const{
		PNode n = bucket[hashf(k)];
		while(n != NULL && n->p.key != k) n = n->n;
		return n;
	}

	void init(){
		name = ++count;
		if(bucketSize < MIN_BUCKET_SIZE) throw EBucketSizeTooSmall();
		for(int i = 0; i < bucketSize; i++) bucket[i] = NULL;
	}

public:
	class EKeyNotFound{};
	class EBucketSizeTooSmall{};

	struct Pair{
		const Key key;
		Val val;
		Pair(const Key &k, const Val &v): key(k), val(v){}
		Pair(const Pair& x): key(x.key), val(x.val){}
	};

	class Iterator{
		friend class HashMap;
		HashMap &m;	
		int pos;
		PNode n;
		Iterator(HashMap *m, int pos, PNode n): m(*m), pos(pos), n(n){}

		void advance(){
			if(pos == m.bucketSize) throw EOutOfRange();

			if(n == NULL || n->n == NULL){
				do{
					pos++;
					if(pos == m.bucketSize){
						n = NULL;
						return;
					}
				} while(m.bucket[pos] == NULL);
				n = m.bucket[pos];
			}
			else
				n = n->n;
		}
	public:
		class EOutOfRange{};
		class ENotDereferenceable{};

		Iterator(const Iterator& i): m(i.m), pos(i.pos), n(i.n) {}

		inline Iterator& operator++() throw(EOutOfRange){
			advance();
			return *this;
		}

		inline Iterator& operator++(int) throw(EOutOfRange){
			return ++(*this);
		}

		Pair& operator*() throw(ENotDereferenceable){
			if (n == NULL) throw ENotDereferenceable();
			return n->p;
		}

		Pair* operator->() throw(ENotDereferenceable){
			if (n == NULL) throw ENotDereferenceable();
			return &n->p;	
		}

		const Pair& operator*() const{
			if (n == NULL) throw ENotDereferenceable();
			return n->p;
		}

		const Pair* operator->() const{
			if (n == NULL) throw ENotDereferenceable();
			return &n->p;	
		}

		bool operator==(const Iterator& i) const{
			return n == i.n && pos == i.pos;
		}
		bool operator!=(const Iterator& i) const{
			return !(i == *this);
		}
	};

	Iterator begin() const{
		int pos = 0;
		for(pos; pos < bucketSize && bucket[pos] == NULL; pos++);

		return Iterator(const_cast<HashMap*>(this), pos, pos == bucketSize ? NULL : bucket[pos]);
	}

	Iterator end() const{
		return Iterator(const_cast<HashMap*>(this), bucketSize, NULL);
	}

	inline int size() const{
		return _size;
	}

	inline bool empty() const{
		return !_size;
	}

	bool exists(Key &k) const{
		return findNode(k) != NULL;
	}

	Val& operator[](const Key &key){
		int pos = hashf(key);
		PNode n = bucket[pos];
		while(n != NULL && n->p.key != key) n = n->n;

		if(n == NULL){
			n = new Node(key, Val(), bucket[pos]);
			bucket[pos] = n;
			_size++;
		}
		return n->p.val;
	}

	const Val& operator[](const Key &key) const{
		return findNode(key)->p.val;
	}

	bool exists(const Key &key) const{
		return findNode(key) != NULL;
	}

	HashMap& insert(const Key &k, const Val &v){
		int pos = hashf(k);
		PNode n = bucket[pos];
		while(n != NULL && n->p.key != k) n = n->n;
		if(n == NULL){
			bucket[pos] = new Node(k, v, bucket[pos]);
			_size++;
		} else
			n->p.val = v;
		return *this;
	}

	HashMap& remove(const Key &k) throw(EKeyNotFound){
		int pos = hashf(k);
		PNode n = bucket[pos], prev = NULL;
		while(n != NULL && n->p.key != k) prev = n, n = n->n;
		if(n == NULL) throw EKeyNotFound();
		if(prev == NULL) bucket[pos] = n->n;
		else prev->n = n->n;
		delete n;
		n = NULL;
		_size--;
		return *this;
	}

	HashMap& clear(){
		for(int i = 0; i < this->bucketSize; i++){
			while(bucket[i] != NULL){
				PNode n = bucket[i];
				bucket[i] = n->n;
				delete n;
				n = NULL;
			}
		}
		_size = 0;
		return *this;
	}

	bool operator==(const HashMap &m) const{
		if(_size != m.size()) return false;
		for(HashMap::Iterator i = begin(); i != end(); i++){
			if(!m.exists(i->key) || i->val != m[i->key]) return false;
		}
		return true;
	}

	HashMap& update(const HashMap &m){
		for(HashMap::Iterator i = m.begin(); i != m.end(); i++)
			insert(i->key, i->val);
		return *this;
	}

	HashMap& operator=(const HashMap &m){
		return clear().update(m);
	}

	static const int DEFAULT_BUCKET_SIZE = 9;
	HashMap(const int bucketSize = DEFAULT_BUCKET_SIZE) throw(EBucketSizeTooSmall)
			: _size(0), bucketSize(bucketSize), bucket(new PNode[bucketSize]){
		init();	
	}

	HashMap(const HashMap &m, const int bucketSize = DEFAULT_BUCKET_SIZE) throw(EBucketSizeTooSmall)
			: _size(0), bucketSize(bucketSize), bucket(new PNode[bucketSize]){
		init();
		update(m);
	}

	~HashMap(){
		clear();
		delete[] bucket;
	}
};


namespace HashF{

	template<class T>
	int iterableSTLHashf(const T &key, int size){
		unsigned val = 0;
		int i = 0;
		for(typename T::const_iterator it = key.begin(); it != key.end(); it++, i++)
			val += ((unsigned char)*it) * (i + 1);

		double f = HASH_MULTIPLIER * val;
		return (int) ((f - (int)f) * (size - 1));
	}

};
};
#endif