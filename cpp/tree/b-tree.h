#ifndef B_TREE_H
#define B_TREE_H

#include <cstdlib>
#include <cassert>
#include <cstring>

namespace Homeworks{

const int DEFAULT_NODE_MODIFIER = 3;

template <typename Key, typename Value, int treeModifier = DEFAULT_NODE_MODIFIER>
class BTreeMap{

	class BasicNode;
	typedef class BasicNode *BasicNodePtr;

	typedef unsigned int Size;

	BasicNodePtr root;
	Size size;

	struct Element{
		Key key;
		Value value;
		Element(const Key &key, const Value &value) : key(key), value(value){}
	};
	typedef Element *ElementPtr; 

	typedef enum FallDownBehave{
		FD_UNDETERMINED,
		FD_BY_LEFT_SIDE,
		FD_BY_RIGHT_SIDE,
	} FallDownBehave;
	
	struct KeyPos{
		BasicNodePtr node;
		int pos;
		KeyPos(BasicNodePtr &n, int &p) : node(n), pos(p){}
	};

	class InternalNode;
	typedef InternalNode *InternalNodePtr;
	typedef BasicNode LeafNode;

	static BasicNodePtr buildNode(const bool isLeaf, const Size keyCount, InternalNodePtr parent){
		return isLeaf? new LeafNode(keyCount, parent) : new InternalNode(keyCount, parent);
	}

	class BasicNode{
	public:
		inline virtual bool isLeaf(){ return true; } //OMG :'(
		Size keyCount;
		static const int MIN_KEY_COUNT = treeModifier - 1;
		static const int MAX_KEY_COUNT = treeModifier * 2 - 1;
		ElementPtr items[MAX_KEY_COUNT];
		InternalNode *parent;

		BasicNode(const Size keyCount = 0, InternalNode *parent = NULL)
			: keyCount(keyCount), parent(parent){};

		void insertKey(const Size pos, const ElementPtr x){
			assert(pos >= 0 && pos <= keyCount && keyCount < MAX_KEY_COUNT);
			ElementPtr *from = items + pos;
			memmove(from + 1, from, sizeof(ElementPtr) * (keyCount - pos));
			items[pos] = x;
			keyCount++;
		}
		
		void excludeKey(const Size pos, const bool needFree){
			assert(pos >= 0 && pos < keyCount);
			//assert(keyCount > MIN_KEY_COUNT); root???
			if(needFree) delete items[pos];
			ElementPtr *from = items + pos;
			memmove(from, from + 1,	sizeof(ElementPtr) * (keyCount - pos - 1));
			items[--keyCount] = NULL;
		}
		
		inline bool isRoot() const{ return NULL == parent; }
		inline virtual void insertNode
        	(const Size pos, const BasicNodePtr node, const Size subtreeKeyCount){ return; }
		inline virtual void excludeNode
			(const Size pos, const bool needFree){ return; }
		inline bool isTreeConditionFulfilled(){
			return
				(isRoot() || keyCount >= MIN_KEY_COUNT) &&
				keyCount <= MAX_KEY_COUNT &&
				isTreeNodeChildCountValid()
			;
		}
		inline virtual bool isTreeNodeChildCountValid(){ return true; }
		
		int getItemOrNext(const Key key){
			int pos = 0;
			for(pos = 0; pos < keyCount && items[pos]->key < key; pos++);
			return pos;
		}
		bool testIfKeyFound(const int pos, const Key key){
			assert(pos <= keyCount);
			return pos < keyCount && items[pos]->key == key;
		}

		bool alterKey(ElementPtr key){
			assert(key != NULL);
			int pos = getItemOrNext(key->key);
			bool keyFound = testIfKeyFound(pos, key->key);
			if(keyFound){
				items[pos]->value = key->value;
				return false;
			}
			return doAlterKey(key, pos);
		}


		virtual bool doAlterKey(ElementPtr key, int pos){
			insertKey(pos, key);
			return true;
		}

		virtual void pourElements(BasicNodePtr _src, int start){
			BasicNode &nsrc = *_src;
			Size count = nsrc.keyCount - start;
			assert(keyCount + count <= MAX_KEY_COUNT);
			//assert(isTreeConditionFulfilled()); /*unreliable*/
			//assert(isTreeNodeChildCountValid());/*unreliable too*/

			memcpy(items + keyCount, nsrc.items + start, count * sizeof(ElementPtr));
			keyCount += count;

			for(int i = start; i < nsrc.keyCount; i++) nsrc.items[i] = NULL;
			nsrc.keyCount = start - 1;
		}

		virtual bool deleteKey(Key key, int pos, bool needFree){
		//Cormen: 1
			if(!testIfKeyFound(pos, key)) return false;
			excludeKey(pos, needFree);
		}

		virtual Size getSubtreeKeyCount(){ return keyCount; }

		virtual bool splitChild(int x){assert(false);};
		virtual void mergeChildrenToLeft(int lPos){assert(false);};
		virtual int balanceNode(int pos){assert(false);};

	};

	class InternalNode: public BasicNode{
	public:
		inline virtual bool isLeaf(){ return false; } //OMG=(
		Size nodesCount;
		typedef BasicNode Parent; //abstraction, wha!
		static const int MIN_CHILD_COUNT = Parent::MIN_KEY_COUNT + 1;
		static const int MAX_CHILD_COUNT = Parent::MAX_KEY_COUNT + 1;

		InternalNode(const Size keyCount = 0, const InternalNode *parent = NULL)
			: nodesCount(!keyCount ? 0 : keyCount + 1){}

		BasicNodePtr nodes[MAX_CHILD_COUNT];
		Size subtreeKeyCounts[MAX_CHILD_COUNT];
		inline Size getSize() const{ return size; }

		virtual void insertNode(const Size pos, const BasicNodePtr node, const Size subtreeKeyCount){
			assert(isTreeNodeChildCountValid());
			assert(pos >= 0 && pos <= nodesCount);

			BasicNodePtr *fromN = nodes + pos;
			memmove(fromN + 1, fromN, sizeof(BasicNodePtr) * (nodesCount - pos));
			Size *fromS = subtreeKeyCounts + pos;
			memmove(fromS + 1, fromS, sizeof(Size) * (nodesCount - pos));

			node->parent = this;
			nodes[pos] = node;
			subtreeKeyCounts[pos] = subtreeKeyCount;
			nodesCount++;
		}
		
		virtual void excludeNode(Size pos, bool needFree){
		//    assert(isTreeNodeChildCountValid(node)); //why not?
			assert(pos >= 0 && pos < nodesCount);
			if(needFree) delete nodes[pos];

			BasicNodePtr *fromN = nodes + pos;
			memmove(fromN, fromN + 1, sizeof(BasicNodePtr) * (nodesCount - pos - 1));
			Size *fromS = subtreeKeyCounts + pos;
			memmove(fromS, fromS + 1, sizeof(Size) * (nodesCount - pos - 1));

			nodesCount--;
			nodes[nodesCount] = NULL;
			subtreeKeyCounts[nodesCount] = 0;
		}
		
		inline bool isTreeNodeChildCountValid(){
			    return
					(Parent::isRoot() || nodesCount >= MIN_CHILD_COUNT) &&
					nodesCount <= MAX_CHILD_COUNT;
				;
		}

		virtual Size getSubtreeKeyCount(){
		    Size res = 0;
			if(!isLeaf())
				for(int i = 0; i < nodesCount; i++) res += subtreeKeyCounts[i];
			return res + Parent::keyCount;
		}

		virtual void pourElements(BasicNodePtr _src, int start){
			Parent::pourElements(_src, start);
			InternalNode &nsrc = *static_cast<InternalNodePtr>(_src);
			Size count = nsrc.nodesCount - start;
			for(int i = start; i < nsrc.nodesCount; i++) nsrc.nodes[i]->parent = this;
			memcpy(nodes + nodesCount, nsrc.nodes + start, count * sizeof(BasicNodePtr));
			memcpy(subtreeKeyCounts + nodesCount, nsrc.subtreeKeyCounts + start, count * sizeof(Size));
			for(int i = start; i < nsrc.nodesCount; i++){
				nsrc.nodes[i] = NULL;
				nsrc.subtreeKeyCounts[i] = 0;
			}
		
			nodesCount += count;
			nsrc.nodesCount = start;
		}

		virtual bool splitChild(int x){
		    assert(Parent::MAX_KEY_COUNT != Parent::keyCount);
			assert(x >= 0 && x <= Parent::keyCount && Parent::keyCount >= 0);

			BasicNodePtr child = nodes[x];
			assert(child != NULL);

			BasicNodePtr newNode = BTreeMap::buildNode(Parent::isLeaf(), Parent::keyCount, Parent::parent);

			newNode->pourElements(child, treeModifier);
			//insert mediane key in node
			insertKey(x, child->items[treeModifier - 1]);
			child->items[treeModifier - 1] = NULL;
			insertNode(x + 1, newNode, newNode->getSubtreeKeyCount());
			subtreeKeyCounts[x] -= subtreeKeyCounts[x + 1] + 1;

			return true;
		}

		virtual void mergeChildrenToLeft(int lPos){
			assert(lPos >= 0 && lPos < Parent::keyCount && Parent::keyCount <= Parent::MAX_KEY_COUNT);
			BasicNodePtr x = nodes[lPos], y = nodes[lPos + 1];
			assert(x->keyCount < treeModifier && y->keyCount < treeModifier);

			x->items[x->keyCount++] = Parent::items[lPos];

			x->pourElements(y, 0);

			Parent::excludeKey(lPos, false);


			subtreeKeyCounts[lPos] += subtreeKeyCounts[lPos + 1] + 1;
			excludeNode(lPos + 1, true); //and free it, yep
		}
		
		virtual bool doAlterKey(ElementPtr key, int pos){
			if(nodes[pos]->keyCount == BasicNode::MAX_KEY_COUNT){
				if(!splitChild(pos)) return false;
				//after we splitted, a chance, that a key
				//added in node is less then a new key, is apperaring
				assert(BasicNode::items[pos] != NULL);
				if(BasicNode::items[pos]->key < key->key) pos++;
				assert(pos == BasicNode::keyCount || BasicNode::items[pos]->key > key->key);
			}
			bool res = nodes[pos]->alterKey(key);
			if(res) subtreeKeyCounts[pos]++;
			return res;
		}

		virtual int balanceNode(int pos){
			if(nodes[pos]->keyCount != Parent::MIN_KEY_COUNT) return pos;

			BasicNodePtr mc = nodes[pos];

			BasicNodePtr lc = pos > 0 ? nodes[pos - 1] : NULL;

			BasicNodePtr rc = pos + 1 < nodesCount ? nodes[pos + 1] : NULL;

			int lkp = pos - 1, rkp = pos;
			assert(lc != NULL || rc != NULL);

			if(lc != NULL && lc->keyCount > Parent::MIN_KEY_COUNT){

				assert(lc->isTreeConditionFulfilled());

				int lastKey = lc->keyCount - 1;
				mc->insertKey(0, Parent::items[lkp]);
				Parent::items[lkp] = lc->items[lastKey];

				lc->excludeKey(lastKey, false);
				subtreeKeyCounts[pos - 1]--;
				subtreeKeyCounts[pos]++;

				if(!lc->isLeaf()){
					InternalNode &ilc = *static_cast<InternalNodePtr>(lc);
					int lastNode = ilc.nodesCount - 1;
					Size kc = ilc.subtreeKeyCounts[lastNode];
					subtreeKeyCounts[pos - 1] -= kc;
					subtreeKeyCounts[pos] += kc;

					static_cast<InternalNodePtr>(mc)->insertNode(0, ilc.nodes[lastNode], kc);
					ilc.excludeNode(lastNode, false);
				}
				
				return pos;
			}

			if(rc != NULL && rc->keyCount > Parent::MIN_KEY_COUNT){

				assert(rc->isTreeConditionFulfilled());

				mc->insertKey(mc->keyCount, Parent::items[rkp]);
				Parent::items[rkp] = rc->items[0];

				rc->excludeKey(0, false);
				subtreeKeyCounts[pos + 1]--;
				subtreeKeyCounts[pos]++;

				if(!rc->isLeaf()){
					InternalNode &irc = *static_cast<InternalNodePtr>(rc);
					Size kc = irc.subtreeKeyCounts[0];
					subtreeKeyCounts[pos + 1] -= kc;
					subtreeKeyCounts[pos] += kc;

					static_cast<InternalNodePtr>(mc)->insertNode(static_cast<InternalNodePtr>(mc)->nodesCount, irc.nodes[0], kc);
					irc.excludeNode(0, false);
				}

				return pos;
			}

			if(lc != NULL){
				mergeChildrenToLeft(lkp);
				//this is the specific rule, because node is poured to left
				pos--;
			}
			else if(rc != NULL) mergeChildrenToLeft(rkp);
			else assert(false);
		
			return pos;
		}

		bool internalKeyDelete(int pos, int shift, FallDownBehave fb){
			int childNodePos = pos + shift;
			BasicNodePtr child = nodes[childNodePos];
			if(childNodePos >= nodesCount || child->keyCount < treeModifier) return false;

			free(Parent::items[pos]);
			Parent::items[pos] = fallDown(child, fb);
			
			Key key = Parent::items[pos]->key;
			assert(child->deleteKey(key, child->getItemOrNext(key), false));
			subtreeKeyCounts[childNodePos]--;

			return true;
		}


		virtual bool deleteKey(Key key, int pos, bool needFree){
		//ALWAYS, on successful (with true on return) call of this function, we need to
		//decrease .subtreeKeyCount; 
			bool res = true;
		//2: if key found in internal node
			if(this->testIfKeyFound(pos, key)){
				//we always have left and right children of key splitter
				assert(nodes[pos] != NULL && nodes[pos + 1] != NULL);
				//we always have to free internal key
				assert(needFree);

				if(internalKeyDelete(pos, 0, FD_BY_RIGHT_SIDE)) return res;
				if(internalKeyDelete(pos, 1, FD_BY_LEFT_SIDE)) return res;
				
				//keyCounts left < t && right < t:
				Parent::mergeChildrenToLeft(pos);
				res = nodes[pos]->deleteKey(key, treeModifier - 1, needFree);
				if(res) subtreeKeyCounts[pos]--;
				return res;
			}
		//3: pivot/merge
			//key is not found yet and we don't need free it? FAIL. bad assert
			//assert(needFree && node->nodes[pos]->keyCount >= MIN_KEY_COUNT);

			pos = balanceNode(pos);

			assert(pos < nodesCount);
			res = nodes[pos]->deleteKey(key, nodes[pos]->getItemOrNext(key), needFree);
			if(res) subtreeKeyCounts[pos]--;
			return res;

		}


		virtual ~InternalNode(){
			for(int i = 0; i < nodesCount; i++) delete nodes[i];
		}

	}; //InternalNode
	
	static ElementPtr fallDown(BasicNodePtr node, FallDownBehave how){
		if(how != FD_BY_RIGHT_SIDE && how != FD_BY_LEFT_SIDE) return NULL;
		if(NULL == node) return NULL;
		while(!node->isLeaf()){
			InternalNode &t = *static_cast<InternalNodePtr>(node);
		    node = t.nodes[how == FD_BY_RIGHT_SIDE ? t.nodesCount - 1 : 0];
		    assert(node != NULL);
		}
		return node->items[how == FD_BY_RIGHT_SIDE ? node->keyCount - 1 : 0];
	}
	
	class EKeyNotFound{}; //exception
	static KeyPos& searchKey(BasicNodePtr node, Key key){
		if(NULL == node)
			throw(EKeyNotFound());
		int found = 0;

		while(!found){
		    int pos = node->getItemOrNext(key);
		    if(pos < 0) throw(EKeyNotFound());
		    assert(node->items[pos] != NULL);
		    if(pos < node->keyCount && node->items[pos]->key == key)
		        return *new KeyPos(node, pos);
		    found = found || node->isLeaf();
		    if(!found) node = static_cast<InternalNodePtr>(node)->nodes[pos];
		}
		throw(EKeyNotFound());
	}

	const Value& getValByKey(Key key){
		KeyPos& kp = searchKey(root, key);
		return kp.node->items[kp.pos]->value;
	}

	bool doInsertKey(ElementPtr item){
		assert(root != NULL && item != NULL);

		if(BasicNode::MAX_KEY_COUNT != root->keyCount)
			return root->alterKey(item);

		InternalNodePtr newRoot = new InternalNode();
		BasicNodePtr oldRoot = root;

		//newTreeNode also sets nodesCount to 1, so the following line fixes it
		//newRoot->nodesCount = 0; fixed in c++ port

		oldRoot->parent = newRoot;
		root = newRoot;
		newRoot->insertNode(0, oldRoot, oldRoot->getSubtreeKeyCount());
		newRoot->splitChild(0);
		return root->alterKey(item);
	}

	void insertKey(ElementPtr item){
		if(doInsertKey(item)) size++;
	}

public:
	BTreeMap(): size(0), root(new LeafNode()), prx(*this) {};
	~BTreeMap(){ delete root; }
	inline Size getSize(){ return size; }

	bool deleteKey(Key key){
		assert(root != NULL);
		if(!root->keyCount) return false;

		int pos = getItemOrNext(root, key);

		if(root->keyCount != 1 || root->isLeaf()) return root->deleteKey(key, pos, true);

		assert(root->nodes[0] != NULL && root->nodes[1] != NULL);

		BasicNodePtr tmp = root->nodes[0];
		bool res = root->deleteKey(key, pos, true);
		if(!root->keyCount){
			delete root;
			tmp->parent = NULL;
			root = tmp;
		}
		return res;
	}

	class Proxy{
		friend class BTreeMap;

		BTreeMap& tree;
		Key key;
		Proxy(BTreeMap &tree): tree(tree) {}
	public:
		inline const Value& operator=(const Value &val) const{
			ElementPtr e = new BTreeMap::Element(key, val);
			tree.insertKey(e);
			return val;
		}
		inline operator Value() const{ return tree.getValByKey(key); }
	};
	friend class Proxy;
private: 
	Proxy prx;
public:

	const Proxy& operator[](Key key){
		prx.key = key;
		return prx;
	}

	class Iterator;

	Iterator& getIterOnKey(const Key& key){
		KeyPos &kp = searchKey(root, key);
		return Iterator(*this, kp->node, kp->pos);
	}

	inline const Iterator first(){
		return Iterator(*this, 0);
	}

	inline const Iterator& last(){
		return Iterator(*this, size - 1);
	}

	class Iterator{
	public:
		enum State{
			IT_NORMAL,
			IT_BEFORE_FIRST,
			IT_PAST_REAR,
			IT_BAD,
		};
	private:
		State state;
	public:
		inline bool isPastRear() const{ return state == IT_PAST_REAR; };
		inline bool isBeforeFirst() const{ return state == IT_BEFORE_FIRST; }
		inline bool isNormal() const{ return state == IT_NORMAL; }
		inline bool isBroken() const{ return state == IT_BAD; } //always false now
	private:
		BTreeMap &tree;
		BasicNodePtr node;
		Size pos;
		inline void set(BasicNodePtr _node, Size _pos, State _state){
			node = _node;
			pos = _pos;
			state = _state;
		}
	public:
		Iterator(const BTreeMap &tree, const BasicNodePtr node, const Size &pos, const State state = IT_NORMAL)
			: tree(tree), node(node), pos(pos), state(state) {}

		Iterator(BTreeMap &tree, int pos) : tree(tree){
			setPosition(pos);
		}
	private:
		void shiftForward(unsigned int shift);
		void shiftBackward(unsigned int shift);

		void shiftPosition(int shift){
			
			if(state == IT_PAST_REAR && shift > 0 || state == IT_BEFORE_FIRST && shift < 0)
				return;
			if(state == IT_BEFORE_FIRST){
				setPosition(0);
				shift--;
			} else if(state == IT_PAST_REAR){
				setPosition(tree.size - 1);
				shift++;
			}
			
			if(shift > 0) shiftForward(shift);
			else shiftBackward(-shift);
		}

		void setPosition(int pos){
			if(pos < 0 || pos >= tree.size){
				set(NULL, 0, pos < 0 ? IT_BEFORE_FIRST : IT_PAST_REAR);
				return;
			}

			BasicNodePtr root = tree.root;
			set(root, 0, IT_NORMAL);
			shiftPosition(pos - (root->isLeaf()? 0 : ((InternalNode*)root)->subtreeKeyCounts[0]));
		}
	public:
		inline State getState() const{ return state; }
		inline bool canDeref const(){ 
			return state == IT_NORMAL && (assert(node != NULL), pos < node->keyCount);
		}

		inline Iterator& operator=(int pos){ setPosition(pos); return *this; }

		inline Iterator& operator++(){ shiftPosition(1); return *this; }
		inline Iterator& operator--(){ shiftPosition(-1); return *this; }
		inline Iterator& operator++(int){ return ++*this; }
		inline Iterator& operator--(int){ return --*this; }

		inline Iterator& operator+=(int shift){
			shiftPosition(shift);
			return *this;
		}

		inline Iterator& operator-=(int shift){
			shiftPosition(-shift);
			return *this;
		}

		inline Iterator& operator=(const Iterator &it){
			set(it.node, it.pos, it.state);
			return *this;
		}

		/*struct Element{
			const Key &key;
			Value &val;
			Element(const Key &key, const Value &val) : key(key), val(val){};
		};*/

		const Element& operator*() const{
			return *node->items[pos];
		}
		const Element* operator->() const{
			return node->items[pos];
		}

		
	}; //Iterator
	
	void fallDownDelete(FallDownBehave fb){
		ElementPtr item = fallDown(root, fb);
		if(NULL == item) return;
		deleteKey(item->key);
	}
};

#define BTREEMAP_TEMPLATE template<typename Key, typename Value, int treeModifier>
#define BTREEMAP BTreeMap<Key, Value, treeModifier>
#define INODE(n, pos) static_cast<InternalNodePtr>((n)->nodes[pos])
#define INSC(n) static_cast<InternalNodePtr>(n)

BTREEMAP_TEMPLATE
void BTREEMAP::Iterator::shiftForward(unsigned int shift){
	bool found = false;
	while(!found){
		if(!node->isLeaf()){
			InternalNodePtr n = INSC(node);
			for(pos++; pos < n->nodesCount && !found; pos++){
				if(shift <= n->subtreeKeyCounts[pos]){
					found = true;
					n = INODE(n, pos);
					pos = -1;
				}
				if(found) break;//get outta for

				shift -= n->subtreeKeyCounts[pos];
				shift--;//passing key
				
				if(!shift) found = pos < node->keyCount;
				if(found) break;//get outta for

			}
			if(shift && found){
				found = false;
				continue;
			}
			node = n;
		} else{//if isLeaf
			if(pos + shift <= node->keyCount){
				pos += shift;
				shift = 0;
				found = pos < node->keyCount;
			} else{
				shift -= node->keyCount - pos;
				pos = node->keyCount;
				assert(!found);
			}
		}
		if(!found){//climb up
			if(node->isRoot()){
				set(NULL, 0, IT_PAST_REAR);
				return;
			}
			assert(node->parent != NULL);
			InternalNodePtr n = INSC(node);
			while(INODE(n->parent, n->parent->nodesCount- 1) == n){
				n = n->parent;
				if(n->isRoot()){
					set(NULL, 0, IT_PAST_REAR);
					return;
				}
				assert(n->parent != NULL && n->parent->nodes != NULL);
			}
			node = n;
			pos = node->parent->getItemOrNext(node->items[0]->key);
			node = node->parent;
			found = !shift;
		}
	}
	state = IT_NORMAL;
}

BTREEMAP_TEMPLATE
void BTREEMAP::Iterator::shiftBackward(unsigned int shift){
	bool found = false;

	while(!found){
		if(node->isLeaf()){
			if(shift <= pos){
				pos -= shift;
				shift = 0;
				found = true;
			} else{
				shift -= pos;
				pos = 0;
			}
		} else{
			for(pos; pos >= 0 && !found; pos--){
				InternalNodePtr n = INSC(node);
				if(shift <= n->subtreeKeyCounts[pos]){
					found = true;
					n = INODE(n, pos);
					pos = n->keyCount;
				}
				if(found) break;//for

				shift -= n->subtreeKeyCounts[pos];
				shift--;
				if(!shift) found = pos >= 0;
				node = n;
			}
			
			if(shift && found){
				found = false;
				continue;
			}
		}
		if(!found){
			if(node->isRoot()){
				set(NULL, 0, IT_BEFORE_FIRST);
				return;
			}
			InternalNodePtr n = INSC(node);
			assert(n->parent != NULL && n->parent->nodes != NULL);
			while(n->parent->nodes[0] == n){
				n= n->parent;
				if(n->isRoot()){
					set(NULL, 0, IT_BEFORE_FIRST);
					return;
				}
				assert(node->parent != NULL && node->parent->nodes != NULL);
			}
			node = n;

			//now the node isn't first in parent's children. go lurk there
			pos = node->parent->getItemOrNext(node->items[0]->key) - 1;
			node = node->parent;
			shift--;
			found = !shift;
		}
	}
	set(node, pos, IT_NORMAL);
}

};

#endif //B_TREE_H

