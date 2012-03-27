#include <cstddef>
#include <cassert>
#include <map>
#include <vector>

namespace Homeworks{

#define RELEASEVAR(x) ((delete (x)), x = NULL)
#define RELEASEBUF(b) ((delete[] (b)), b = NULL)
namespace TrieTraits{
	
	template<typename Key>
	struct STLTrait{
		typedef typename Key::value_type Item;

		static typename Key::size_type size(const Key &k){return k.size();}
		//static idx(const Key &k, int i){return k[i];}
		static const size_t card = sizeof(typename Key::value_type) * 255;
		typedef typename Key::const_iterator KeyIter;
		static KeyIter begin(const Key &k){return k.begin();}
		static KeyIter end(const Key &k){return k.end();}
		template<typename Iter>
		static Key key(Iter a, Iter b){return Key(a, b);}
	};
};

template<typename Key, typename Val, typename Trait = TrieTraits::STLTrait<Key> >
class Trie{
	typedef Val *PVal;
	typedef typename Trait::KeyIter KeyIter;
	typedef typename Trait::Item Item;
	static const size_t card = Trait::card;

public:
	class EKeyNotFound{};

private:
	struct Node{
		typedef Node *PNode;

		struct KeyVal{
			PNode n;
			PVal v;
			int len;
			Item pos, *chunk;

			inline void split(int i, PNode owner, Item pos){
				assert(n != NULL || v != NULL);
				PNode node = new Node(owner, pos), child = n;
				Item *start = chunk + i + 1, *end = chunk + len;
				node->arr[chunk[i]] = new KeyVal(start, end, len - i - 1, v, child);
				node->size++;
				v = NULL;
				n = node;
			}

			template<typename Iter>
			inline int seek(Iter &start, const Iter &end, int &dif){
				int i = 0;
				while(i < len && chunk[i] == *start && dif)
					assert(start != end), start++, i++, dif--;
				assert((start == end) == (dif == 0));
				return i;
			}

			template<typename Iter>
			inline PVal& allocate(Iter &start, const Iter &end, int dif, Item pos, PNode owner){
				int i = seek(start, end, dif);

				if(i != len) split(i, owner, pos);
				else if(dif && n == NULL) n = new Node(owner, pos);
				PVal &res = dif ? n->allocate(start, end, dif) : v;

				if(i != len){
					Item *newChunk = new Item[i];
					for(int j = 0; j < i; j++)
						newChunk[j] = chunk[j];
					delete[] chunk;
					chunk = newChunk;
					len = i;
				}
				return res;
			}

			template<typename Iter>
			inline int match(Iter &start, const Iter &end, int &dif){
				int i = seek(start, end, dif);
				return (i < len || (dif ? (void*)n : (void*)v) == NULL) ? -1 : i;
			}

			template<typename Iter>
			KeyVal(Iter &start, const Iter &end, int len, PVal v = NULL, PNode n = NULL): n(n), v(v), len(len){		
				chunk = new Item[len];
				for(int i = 0; i < len; i++, start++)
					assert(start != end), chunk[i] = *start;
				assert(start == end);
			}
			~KeyVal(){
				RELEASEVAR(n);
				RELEASEVAR(v);
				RELEASEBUF(chunk);
			}
		};
		typedef KeyVal *PKeyVal;

		PKeyVal arr[card];
		Node *parent;
		int posInParent, size;

		template<typename Iter>
		PVal& allocate(Iter &start, const Iter &end, int dif){
			Item item = *start++;
			dif--;
			PKeyVal &kv = arr[item];
			bool isKeyEmpty = kv == NULL;
			if(isKeyEmpty){
				size++;
				kv = new KeyVal(start, end, dif);
			}
			return isKeyEmpty ? kv->v : kv->allocate(start, end, dif, item, this);
		}

		template<typename Iter>
		bool find(Iter &start, const Iter &end, int dif){
			PKeyVal &kv = arr[*start++];
			return kv != NULL && kv->match(start, end, --dif) != -1 && (!dif || kv->n->find(start, end, dif));
		}

		void remove(KeyIter &start, const KeyIter &end, int dif){
			Item itm = *start++;
			PKeyVal &kv = arr[itm];
			int i = 0;
			if(kv == NULL || (i = kv->match(start, end, --dif)) == -1)
				throw EKeyNotFound();

			if(dif)
				kv->n->remove(start, end, dif);
			else{
				assert(start == end);
				if(kv->n == NULL){
					RELEASEVAR(kv);
					size--;
					return;
				}
				RELEASEVAR(kv->v);
			}
			if(kv->v == NULL && kv->n->size == 1){//merge
				int i = 0;
				while(i < card && kv->n->arr[i] == NULL) i++;
				assert(i < card);
				int childLen = kv->n->arr[i]->len;
				int newLen = kv->len + childLen + 1;
				Item *newBuf = new Item[newLen];
				for(int j = 0; j < kv->len; j++) newBuf[j] = kv->chunk[j];
				newBuf[kv->len] = (Item)i;
				int ofs = kv->len + 1;
				for(int j = 0; j < childLen; j++) newBuf[j + ofs] = kv->n->arr[i]->chunk[j];

				kv->v = kv->n->arr[i]->v;
				PNode n = kv->n->arr[i]->n;
				kv->n->arr[i]->n = NULL;
				delete kv->n;
				kv->n = n;

				RELEASEBUF(kv->chunk);
				kv->chunk = newBuf;
				kv->len = newLen;
			}
		}

		Node(Node *parent, int posInParent): size(0), parent(parent), posInParent(posInParent){
			for(PKeyVal *i = arr; i != arr + card; *i++ = NULL);
		}
	};

	typedef typename Node::PNode PNode;

	PNode root;
	int _size;


public:
	int size(){return _size;}

	class Iterator{
		friend class Trie;

		enum State{
			IT_BEFORE_FIRST,
			IT_PAST_REAR,
			IT_NORMAL,
		};
		int pos;
		PNode n;
		State state;

		static void skipFront(Iterator *i){
			while(++i->pos < card && i->n->arr[i->pos] == NULL);
		}
		
		static void skipBack(Iterator *i){
			while(--i->pos >= 0 && i->n->arr[i->pos] == NULL);
		}

		template<void skip(Iterator*), bool isFront>
		void move(){
			assert(n != NULL);
			bool found = false;
			while(!found){
				skip(this);
				if(pos == isFront ? card : -1){
					pos = n->posInParent;
					n = n->parent;
					if(n == NULL){
						state = isFront ? IT_PAST_REAR : IT_BEFORE_FIRST;
						return;
					}
				} else{
					found = n->arr[pos]->v != NULL;
					assert(!found && n->arr[pos]->n == NULL);
					n = n->arr[pos]->n;
				}
			}
			state = IT_NORMAL;
		}

		Iterator(int pos, PNode n, State state): pos(pos), n(n), state(state){}
	public:
		class ENotDereferenceable{};

		Key key() const{
			if(n->arr[pos]->v) throw ENotDereferenceable();
			std::vector<Item> v;
			PNode node = n;
			int npos = pos;
			while(node != NULL){
				typename Node::PKeyVal &kv = node->arr[npos];

				for(int i = 0; i < kv->len; i++) v.push_back(kv->chunk[kv->len - i]);
				v.push_back(npos);
				npos = node->posInParent;
				node = node->parent;
			}
			return Trait::key(v.reverse());
		}

		Val& operator*(){
			PVal v = n->arr[pos]->v;
			if(v == NULL) throw ENotDereferenceable();
			return *v;
		}
		Iterator& operator++(){
			move<skipFront, true>();
			return *this;
		}
		Iterator& operator++(int){
			return ++(*this);
		}

		Iterator& operator--(){
			move<skipBack, false>();
			return *this;
		}
		Iterator& operator--(int){
			return (*this)--;
		}

		Iterator& operator=(const Iterator &i){
			pos = i.pos;
			state = i.state;
			n = i.n;
			return *this;
		}
		bool operator==(const Iterator &i) const{
			return pos == i.pos && n == i.n;
		}
		bool operator!=(const Iterator &i) const{
			return !(*this == i);
		}
	};

	Iterator begin(){
		return ++Iterator(1, root, Iterator::IT_BEFORE_FIRST);
	}

	Iterator end(){
		return Iterator(card, root, Iterator::IT_PAST_REAR);
	}

	Val& operator[](const Key &k){
		KeyIter i = Trait::begin(k);
		PVal& v = root->allocate(i, Trait::end(k), Trait::size(k));
		if(v == NULL){
			v = new Val();
			_size++;
		}
		return *v;
	}

	Trie& insert(const Key &k, const Val &val){
		KeyIter i = Trait::begin(k);
		PVal& v = root->allocate(i, Trait::end(k), Trait::size(k));
		if(v == NULL){
			v = new Val(val);
			_size++;
		} else *v = Val(val);
		return *this;
	}

	bool exists(const Key &k){
		KeyIter i = Trait::begin(k);
		return root->find(i, Trait::end(k), Trait::size(k));
	}

	Trie& remove(const Key &k){
		KeyIter i = Trait::begin(k);
		root->remove(i, Trait::end(k), Trait::size(k));
		_size--;
		return *this;
	}

	Trie(): _size(0), root(new Node(NULL, 0)){}
	~Trie(){delete root;}
};

};
