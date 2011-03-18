#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "linear_sequence_assoc.h"


//MSVC port
#if __STDC_VERSION__ >= 199901L || __GNUC__ >= 3
#include <stdbool.h>
#else
#define bool int
#define true 1
#define false 0
#define inline __inline
#endif

//this identificator t is used by Cormen to define the relative value,
//that is, in turn, defining tree's upper and lower node bounds;
//though it's naming differs from other constants,
//this exception is included into the internal naming convention
#define t 200

//B-tree nodes consists of:
//at least 1 and at most (2 * t - 1) keys in root
//at least (t - 1) and at most (2 * t - 1) keys in other nodes
#define MIN_KEY_COUNT (t - 1)
#define MAX_KEY_COUNT (2 * t - 1)

//MAX_CHILD_COUNT MUST BE always (MAX_KEY_COUNT + 1)
#define MAX_CHILD_COUNT (MAX_KEY_COUNT + 1)
//MIN_CHILD_COUNT MUST BE always (MIN_KEY_COUNT + 1)
#define MIN_CHILD_COUNT (MIN_KEY_COUNT + 1)

//*_CHILD_COUNT constants are defined for non-leaf nodes
//MIN_*_COUNT constants are defined for non-root nodes


typedef struct {
    LSQ_IntegerIndexT key;
    LSQ_BaseTypeT data;
} TreeItem, *TreeItemPtr;

typedef struct TreeNode{
    //the number of children is always 0 in leaves, and (keyCount + 1) otherwise
    //this could be controllen by function isTreeConditionFulfilled
    size_t keyCount, childCount;
    TreeItemPtr items[MAX_KEY_COUNT];

    //parent could be useful for some good job
    struct TreeNode **nodes, *parent;

    //subtreeKeyCount is helpful to iterative operations
    //to leave disk data flow possible, we store here counts for each child node
    size_t *subtreeKeyCount;

    //i am also ensured, that it will be always leaf,
    //if it is sptlitted from leaf, or it is initial node
} TreeNode, *TreeNodePtr;

typedef struct {
    LSQ_IntegerIndexT size;
    TreeNodePtr root;
    size_t height;
} Tree, *TreePtr;

typedef enum {
    IT_NORMAL,
    IT_BEFORE_FIRST,
    IT_PAST_REAR,
    IT_BAD,
} TreeIteratorState;

//key position structure for key-searching functions and iterators
typedef struct {
    TreeNodePtr node;
    int pos;
} KeyPos, *KeyPosPtr;

typedef struct {
    TreePtr tree;
    TreeNodePtr node;
    int pos;
    TreeIteratorState state;
} TreeIterator, *TreeIteratorPtr;

//enumerable for determining of how does fallDown() falls down
typedef enum {
    FD_UNDETERMINED,
    FD_BY_LEFT_SIDE,
    FD_BY_RIGHT_SIDE,
} FallDownBehave;



//determine if node is valid b-tree node
static inline bool isTreeConditionFulfilled(TreeNodePtr node);
static inline bool isTreeNodeChildCountValid(TreeNodePtr node);

//structure-initializing functions
static TreeNodePtr newTreeNode(size_t keyCount, TreeNodePtr parent, bool leaf);
static KeyPosPtr newKeyPos(TreeNodePtr node, int pos);
static TreeIteratorPtr newTreeIterator(TreePtr tree, TreeNodePtr node, int pos, TreeIteratorState state);

//key searching functions
static TreeItemPtr fallDown(TreeNodePtr node, FallDownBehave how);
static int getItemOrNext(TreeNodePtr node, LSQ_IntegerIndexT key);
static inline bool isKeyFound(TreeNodePtr node, int pos, LSQ_IntegerIndexT key);
static KeyPosPtr SearchKey(TreeNodePtr node, LSQ_IntegerIndexT key);

/******************************************************************************/
//in following function block functions are heuristically determining
//some parameters from existing node data
//their implementation is not strictly defined and could change from build to build

//determine whether the node is leaf/root or not
static inline bool isLeaf(TreeNodePtr node);
static inline bool isRoot(TreeNodePtr node);

/******************************************************************************/

//compute total number of keys under the node
static size_t subforestKeyCount(TreeNodePtr node);

//functions, inserting/deleting key directly from node
static void addKey(TreeNodePtr node, int pos, TreeItemPtr x);
static void addNode(TreeNodePtr node, TreeNodePtr newNode, int subtreeKeyCount, int pos);
static void excludeKey(TreeNodePtr node, int pos, bool needFree);
static void excludeNode(TreeNodePtr node, int pos, bool needFree);

//main key insertion algorithm block
static void pourElements(TreeNodePtr a, TreeNodePtr b, int start);
static bool splitChild(TreeNodePtr node, int x);
static bool insertKey(TreeNodePtr node, TreeItemPtr key);
static bool insertKeyInTree(TreePtr tree, TreeItemPtr item);

//main deletion algorithm block
static void mergeToLeft(TreeNodePtr parent, int lPos);
static int balanceNode(TreeNodePtr node, int pos);
static bool internalKeyDelete(TreeNodePtr node, int pos, int shift, FallDownBehave fb);
static bool deleteKey(TreeNodePtr node, LSQ_IntegerIndexT key, int pos, bool needFree);
static bool deleteKeyFromTree(TreePtr tree, LSQ_IntegerIndexT key);
static inline void fallDownDelete(TreePtr tree, FallDownBehave fb);

//initialize given iterator to given values
static inline void setIterator(TreeIteratorPtr it, TreeNodePtr node, int pos, TreeIteratorState state);

#ifdef LSQ_DEBUG
#define INLINE_ON_RELEASE
#else
#define INLINE_ON_RELEASE inline
#endif
//tree debug info output
static INLINE_ON_RELEASE void dumpNode(TreeNodePtr node);
static INLINE_ON_RELEASE void dump(TreeNodePtr node);


/******************************************************************************/

//not sure if it'll be ever used
bool isTreeConditionFulfilled(TreeNodePtr node){
    return
        NULL == node || //huh, never mind
        (isRoot(node) || node->keyCount >= MIN_KEY_COUNT) &&
        node->keyCount <= MAX_KEY_COUNT &&
        isTreeNodeChildCountValid(node)
    ;
}

bool isTreeNodeChildCountValid(TreeNodePtr node){
    return
        NULL == node ||
        (isRoot(node) || node->childCount >= MIN_CHILD_COUNT) &&
            node->childCount <= MAX_CHILD_COUNT
        || isLeaf(node) && 0 == node->childCount;
}

TreeNodePtr newTreeNode(size_t keyCount, TreeNodePtr parent, bool leaf){
    int i = 0;
    TreeNodePtr res = malloc(sizeof(TreeNode));
    if(NULL == res) return NULL;
    size_t childCount = 0;

    if(!leaf){
        childCount = keyCount + 1;
        res->nodes = malloc(sizeof(TreeNodePtr) * MAX_CHILD_COUNT);
        res->subtreeKeyCount = malloc(sizeof(size_t) * MAX_CHILD_COUNT);
        if(NULL == res->nodes || NULL == res->subtreeKeyCount){
            free(res->nodes);
            free(res->subtreeKeyCount);
            free(res);
            return NULL;
        }

        for(i = childCount; i < MAX_CHILD_COUNT; i++){
            res->subtreeKeyCount[i] = 0;
            res->nodes[i] = NULL;
        }

    } else{
        res->nodes = NULL;
        res->subtreeKeyCount = NULL;
    }

    for(i = keyCount; i < MAX_KEY_COUNT; i++) res->items[i] = NULL;
    
    res->keyCount = keyCount;
    res->childCount = childCount;
    res->parent = parent;

    return res;
}

KeyPosPtr newKeyPos(TreeNodePtr node, int pos){
    KeyPosPtr res = malloc(sizeof(KeyPos));
    if(NULL == res) return NULL;
    res->node = node;
    res->pos = pos;
    return res;
}

TreeItemPtr fallDown(TreeNodePtr node, FallDownBehave how){
    if(how != FD_BY_RIGHT_SIDE && how != FD_BY_LEFT_SIDE) return NULL;
    if(NULL == node) return NULL;
    while(!isLeaf(node)){
        assert(node != NULL && node->nodes != NULL);
        node = node->nodes[how == FD_BY_RIGHT_SIDE ? node->childCount - 1 : 0];
    }
    return node->items[how == FD_BY_RIGHT_SIDE ? node->keyCount - 1 : 0];
}

int getItemOrNext(TreeNodePtr node, LSQ_IntegerIndexT key){
    int pos = 0;
    assert(node != NULL);
    for(pos = 0; pos < node->keyCount && node->items[pos]->key < key; pos++);
    return pos;
}

bool isKeyFound(TreeNodePtr node, int pos, LSQ_IntegerIndexT key){
    assert(node->keyCount >= pos);
    return pos < node->keyCount && node->items[pos]->key == key;
}

KeyPosPtr SearchKey(TreeNodePtr node, LSQ_IntegerIndexT key){
    if(NULL == node) return NULL;
    int found = 0;

    while(!found){
        int pos = getItemOrNext(node, key);
        if(pos < 0) return NULL;
        assert(node->items[pos] != NULL);
        if(pos < node->keyCount && node->items[pos]->key == key)
            return newKeyPos(node, pos);
        found = found || isLeaf(node);
        if(!found) node = node->nodes[pos];
    }
    return NULL;
}

bool isLeaf(TreeNodePtr node){
    //memory isn't allocating for .nodes if it's leaf
    return node != NULL && NULL == node->nodes;
}

bool isRoot(TreeNodePtr node){
    //node's parent is NULL <=> node is root
    return NULL == node->parent;
}

size_t subforestKeyCount(TreeNodePtr node){
    size_t res = 0, i = 0;
    if(!isLeaf(node))
        for(i = 0; i < node->childCount; i++) res += node->subtreeKeyCount[i];
    return res + node->keyCount;
}

void addKey(TreeNodePtr node, int pos, TreeItemPtr x){
    assert(node != NULL && pos >= 0 && pos <= node->keyCount && node->items != NULL);
    assert(node->keyCount < MAX_KEY_COUNT);
    memmove(
        node->items + pos + 1,
        node->items + pos,
        sizeof(TreeNodePtr) * (node->keyCount - pos)
    );
    node->items[pos] = x;
    node->keyCount++;
}

void addNode(TreeNodePtr node, TreeNodePtr newNode, int subtreeKeyCount, int pos){
    assert(isTreeNodeChildCountValid(node));
    if(isLeaf(node)) return;
    
    assert(node != NULL && pos >= 0 && pos <= node->childCount && node->items != NULL);
    assert(node->childCount < MAX_CHILD_COUNT);
    memmove(
        node->nodes + pos + 1,
        node->nodes + pos,
        sizeof(TreeNodePtr) * (node->childCount - pos)
    );
    memmove(
        node->subtreeKeyCount + pos + 1,
        node->subtreeKeyCount + pos,
        sizeof(size_t) * (node->childCount - pos)
    );

    newNode->parent = node;
    node->nodes[pos] = newNode;
    node->subtreeKeyCount[pos] = subtreeKeyCount;
    node->childCount++;
}

void excludeKey(TreeNodePtr node, int pos, bool needFree){
    assert(node != NULL && pos >= 0 && pos < node->keyCount && node->items != NULL);
    //assert(node->keyCount > MIN_KEY_COUNT); root???
    if(needFree) free(node->items[pos]);
    memmove(
        node->items + pos,
        node->items + pos + 1,
        sizeof(TreeNodePtr) * (node->keyCount - pos - 1)
    );
    node->items[--node->keyCount] = NULL;
}

void excludeNode(TreeNodePtr node, int pos, bool needFree){
    if(isLeaf(node)) return;
    
//    assert(isTreeNodeChildCountValid(node)); //why not?
    assert(node != NULL && pos >= 0 && pos < node->childCount && node->nodes != NULL);
    if(needFree) free(node->nodes[pos]);
    memmove(
        node->nodes + pos,
        node->nodes + pos + 1,
        sizeof(TreeNodePtr) * (node->childCount - pos - 1)
    );
    memmove(
        node->subtreeKeyCount + pos,
        node->subtreeKeyCount + pos + 1,
        sizeof(size_t) * (node->childCount - pos - 1)
    );
    
    node->childCount--;
    node->nodes[node->childCount] = NULL;
    node->subtreeKeyCount[node->childCount] = 0;
}

//a=>dest, b=>src, pour from start to end of b
//for children there is one more to drag, comparing with items
void pourElements(TreeNodePtr a, TreeNodePtr b, int start){
    size_t count = b->keyCount - start;
    int i = 0;
    assert(a->keyCount + count <= MAX_KEY_COUNT);
    //assert(isTreeConditionFulfilled(a)); /*unreliable*/
    //assert(isTreeNodeChildCountValid(a));/*unreliable too*/

    memcpy(a->items + a->keyCount, b->items + start, count * sizeof(TreeItemPtr));
    a->keyCount += count;

    for(i = start; i < b->keyCount; i++) b->items[i] = NULL;
    b->keyCount = start - 1;

    if(isLeaf(a)) return;

    count++;
    for(i = start; i < b->childCount; i++) b->nodes[i]->parent = a;
    memcpy(a->nodes + a->childCount, b->nodes + start, count * sizeof(TreeNodePtr));
    memcpy(a->subtreeKeyCount + a->childCount, b->subtreeKeyCount + start, count * sizeof(size_t));
    for(i = start; i < b->childCount; i++){
        b->nodes[i] = NULL;
        b->subtreeKeyCount[i] = 0;
    }
    
    a->childCount += count;
    b->childCount = start;
}

bool splitChild(TreeNodePtr node, int x){
    assert(node != NULL && MAX_KEY_COUNT != node->keyCount);
    assert(x >= 0 && x <= node->keyCount && node->keyCount >= 0);

    TreeNodePtr child = node->nodes[x];
    assert(child != NULL);

    TreeNodePtr newNode = newTreeNode(0, node, isLeaf(child));
    if(NULL == newNode) return false;
    newNode->childCount = 0;

    pourElements(newNode, child, t);
    //insert mediane key in node
    addKey(node, x, child->items[t - 1]);
    child->items[t - 1] = NULL;
    addNode(node, newNode, subforestKeyCount(newNode), x + 1);
    node->subtreeKeyCount[x] -= node->subtreeKeyCount[x + 1] + 1;

    return true;
}

bool insertKey(TreeNodePtr node, TreeItemPtr key){
    assert(node != NULL && key != NULL);
    int pos = getItemOrNext(node, key->key);
    bool keyFound = isKeyFound(node, pos, key->key);
    if(keyFound){
        node->items[pos]->data = key->data;
        return false;
    }
    
    if(isLeaf(node)){
        addKey(node, pos, key);
        return true;
    }

    if(node->nodes[pos]->keyCount == MAX_KEY_COUNT){
        if(!splitChild(node, pos)) return false;
        //after we splitted, a chance, that a key
        //added in node is less then a new key, is apperaring
        assert(node->items[pos] != NULL);
        if(node->items[pos]->key < key->key) pos++;
        assert(pos == node->keyCount || node->items[pos]->key > key->key);
    }
    bool res = insertKey(node->nodes[pos], key);
    if(res) node->subtreeKeyCount[pos]++;
    return res;
}

bool insertKeyInTree(TreePtr tree, TreeItemPtr item){
    if(NULL == tree) return false;

    assert(tree->root != NULL && item != NULL);

    if(MAX_KEY_COUNT != tree->root->keyCount)
        return insertKey(tree->root, item);

    TreeNodePtr newRoot = newTreeNode(0, NULL, false), oldRoot = tree->root;

    //newTreeNode also sets childCount to 1, so the following line fixes it
    newRoot->childCount = 0;

    oldRoot->parent = newRoot;
    tree->root = newRoot;
    addNode(newRoot, oldRoot, subforestKeyCount(oldRoot), 0);
    splitChild(newRoot, 0);
    return insertKey(tree->root, item);
}

//each of x and y has less then t keys and keys of y migrate to x, splitten by mediane
//it is, of course, used both on left and right merging
void mergeToLeft(TreeNodePtr parent, int lPos){
    assert(parent != NULL);
    if(isLeaf(parent)) return;
    assert(lPos >= 0 && lPos < parent->keyCount && parent->keyCount <= MAX_KEY_COUNT);
    TreeNodePtr x = parent->nodes[lPos], y = parent->nodes[lPos + 1];
    assert(x->keyCount < t && y->keyCount < t);

    x->items[x->keyCount++] = parent->items[lPos];

    pourElements(x, y, 0);

    excludeKey(parent, lPos, false);


    parent->subtreeKeyCount[lPos] += parent->subtreeKeyCount[lPos + 1] + 1;
    excludeNode(parent, lPos + 1, true); //and free it, yep
}

//do pivot when we haven't found objective key yet
int balanceNode(TreeNodePtr node, int pos){
    if(node->nodes[pos]->keyCount != MIN_KEY_COUNT) return pos;

    assert(node->nodes != NULL);
    TreeNodePtr mc = node->nodes[pos];

    TreeNodePtr lc = pos > 0 ? node->nodes[pos - 1] : NULL;

    TreeNodePtr rc = pos + 1 < node->childCount ? node->nodes[pos + 1] : NULL;

    int lkp = pos - 1, rkp = pos;
    assert(lc != NULL || rc != NULL);
    assert(isTreeConditionFulfilled(lc) && isTreeConditionFulfilled(rc));

    if(lc != NULL && lc->keyCount > MIN_KEY_COUNT){

        int lastKey = lc->keyCount - 1;
        addKey(mc, 0, node->items[lkp]);
        node->items[lkp] = lc->items[lastKey];

        excludeKey(lc, lastKey, false);
        node->subtreeKeyCount[pos - 1]--;
        node->subtreeKeyCount[pos]++;

        if(!isLeaf(lc)){
            int lastNode = lc->childCount - 1;
            size_t kc = lc->subtreeKeyCount[lastNode];
            node->subtreeKeyCount[pos - 1] -= kc;
            node->subtreeKeyCount[pos] += kc;

            addNode(mc, lc->nodes[lastNode], kc, 0);
            excludeNode(lc, lastNode, false);
        }
        
        return pos;
    }

    if(rc != NULL && rc->keyCount > MIN_KEY_COUNT){

        addKey(mc, mc->keyCount, node->items[rkp]);
        node->items[rkp] = rc->items[0];

        excludeKey(rc, 0, false);
        node->subtreeKeyCount[pos + 1]--;
        node->subtreeKeyCount[pos]++;

        if(!isLeaf(rc)){
            size_t kc = rc->subtreeKeyCount[0];
            node->subtreeKeyCount[pos + 1] -= kc;
            node->subtreeKeyCount[pos] += kc;

            addNode(mc, rc->nodes[0], kc, mc->childCount);
            excludeNode(rc, 0, false);
        }

        return pos;
    }

    if(lc != NULL){
        mergeToLeft(node, lkp);
        //this is the specific rule, because node is poured to left
        pos--;
    }
    else if(rc != NULL) mergeToLeft(node, rkp);
    else assert(false);
    
    return pos;
}

//Cormen: 2a-2b
bool internalKeyDelete(TreeNodePtr node, int pos, int shift, FallDownBehave fb){
    int childNodePos = pos + shift;
    TreeNodePtr child = node->nodes[childNodePos];
    if(childNodePos >= node->childCount || child->keyCount < t) return false;

    free(node->items[pos]);
    node->items[pos] = fallDown(child, fb);
    
    LSQ_IntegerIndexT key = node->items[pos]->key;
    assert(deleteKey(child, key, getItemOrNext(child, key), false));
    node->subtreeKeyCount[childNodePos]--;

    return true;
}

bool deleteKey(TreeNodePtr node, LSQ_IntegerIndexT key, int pos, bool needFree){
//ALWAYS, on successful (with true on return) call of this function, we need to
//decrease .subtreeKeyCount; how to do it pretty? Suppose, i'll wrap it in *TODO*, or just leave;
    bool res = true;
    if(NULL == node) return false;

//1
    if(isLeaf(node)){
        if(!isKeyFound(node, pos, key)) return false;
        excludeKey(node, pos, needFree);
        return true;
    }
//2
//if key found in internal node
    if(isKeyFound(node, pos, key)){
        //we always have left and right children of key splitter
        assert(node->nodes[pos] != NULL && node->nodes[pos + 1] != NULL);
        //we always have to free internal key
        assert(needFree);

        if(internalKeyDelete(node, pos, 0, FD_BY_RIGHT_SIDE)) return res;
        if(internalKeyDelete(node, pos, 1, FD_BY_LEFT_SIDE)) return res;
        
        //keyCounts left < t && right < t:
        mergeToLeft(node, pos);
        res = deleteKey(node->nodes[pos], key, t-1, needFree);
        if(res) node->subtreeKeyCount[pos]--;
        return res;
    }
//3: pivot/merge
    //key is not found yet and we don't need free it? FAIL. bad assert
    //assert(needFree && node->nodes[pos]->keyCount >= MIN_KEY_COUNT);

    pos = balanceNode(node, pos);

    assert(pos < node->childCount);
    res = deleteKey(node->nodes[pos], key, getItemOrNext(node->nodes[pos], key), needFree);
    if(res) node->subtreeKeyCount[pos]--;
    return res;

} //deleteKey

bool deleteKeyFromTree(TreePtr tree, LSQ_IntegerIndexT key){
    assert(tree != NULL && tree->root != NULL);
    TreeNodePtr rt = tree->root;
    if(!rt->keyCount) return false;

    int pos = getItemOrNext(rt, key);

    if(rt->keyCount != 1 || isLeaf(rt)) return deleteKey(rt, key, pos, true);

    assert(rt->nodes[0] != NULL && rt->nodes[1] != NULL);

    TreeNodePtr tmp = rt->nodes[0];
    bool res = deleteKey(rt, key, pos, true);
    if(!rt->keyCount){
        free(rt);
        tree->root = tmp;
        tmp->parent = NULL;
    }
    return res;
}

void destroySubTree(TreeNodePtr node){
    int i = 0;
    for(i = 0; i < node->keyCount; i++) if(node->items[i] != NULL) free(node->items[i]);
    for(i = 0; i < node->childCount; i++){
        if(NULL == node->nodes[i]) continue;
        destroySubTree(node->nodes[i]);
        free(node->nodes[i]);
    }
}

LSQ_HandleT LSQ_CreateSequence(){
    TreePtr tree = malloc(sizeof(Tree));
    tree->size = 0;
    tree->root = newTreeNode(0, NULL, true);
    return (LSQ_HandleT)tree;
}

void LSQ_DestroySequence(LSQ_HandleT handle){
    if(NULL == handle) return;
    TreePtr tree = handle;
    destroySubTree(tree->root);
    free(tree->root);
    free(tree);
}

LSQ_IntegerIndexT LSQ_GetSize(LSQ_HandleT handle){
    return ((NULL == handle) ? 0 : ((TreePtr)handle)->size);
}

int LSQ_IsIteratorDereferencable(LSQ_IteratorT iterator){
    TreeIteratorPtr it = iterator;
    return
        it != NULL && it->state == IT_NORMAL &&
        (assert(it->node != NULL), it->pos < it->node->keyCount);
}

int LSQ_IsIteratorPastRear(LSQ_IteratorT iterator){
    return iterator != NULL && ((TreeIteratorPtr)iterator)->state == IT_PAST_REAR;
}

int LSQ_IsIteratorBeforeFirst(LSQ_IteratorT iterator){
    return iterator != NULL && ((TreeIteratorPtr)iterator)->state == IT_BEFORE_FIRST;
}

void LSQ_AdvanceOneElement(LSQ_IteratorT iterator){
    LSQ_ShiftPosition(iterator, 1);
}

void LSQ_RewindOneElement(LSQ_IteratorT iterator){
    LSQ_ShiftPosition(iterator, -1);
}

static inline void shiftForward(TreeIteratorPtr it, unsigned int shift){
    TreeNodePtr node = it->node;
    int pos = it->pos;
    bool found = false;

    while(!found){
        if(!isLeaf(node)){
            for(pos++; pos < node->childCount && !found; pos++){
                if(shift <= node->subtreeKeyCount[pos]){
                    found = true;
                    node = node->nodes[pos];
                    pos = -1;
                }
                if(found) break;//get outta for

                shift -= node->subtreeKeyCount[pos];
                shift--;//passing key
                
                if(!shift) found = pos < node->keyCount;
                if(found) break;//get outta for

            }
            if(shift && found){
                found = false;
                continue;
            }
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
            if(isRoot(node)){
                setIterator(it, NULL, 0, IT_PAST_REAR);
                return;
            }
            assert(node->parent != NULL && node->parent->nodes != NULL);
            while(node->parent->nodes[node->parent->childCount - 1] == node){

                node = node->parent;
                if(isRoot(node)){
                    setIterator(it, NULL, 0, IT_PAST_REAR);
                    return;
                }
                assert(node->parent != NULL && node->parent->nodes != NULL);
            }
            pos = getItemOrNext(node->parent, node->items[0]->key);
            node = node->parent;
            found = !shift;
        }
    }
    setIterator(it, node, pos, IT_NORMAL);
}

static inline void shiftBackward(TreeIteratorPtr it, unsigned int shift){
    TreeNodePtr node = it->node;
    int pos = it->pos;
    bool found = false;

    while(!found){
        if(isLeaf(node)){
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
                if(shift <= node->subtreeKeyCount[pos]){
                    found = true;
                    node = node->nodes[pos];
                    pos = node->keyCount;
                }
                if(found) break;//for

                shift -= node->subtreeKeyCount[pos];
                shift--;;
                if(!shift) found = pos >= 0;
            }
            
            if(shift && found){
                found = false;
                continue;
            }
        }
        if(!found){
            if(isRoot(node)){
                setIterator(it, NULL, 0, IT_BEFORE_FIRST);
                return;
            }
            assert(node->parent != NULL && node->parent->nodes != NULL);
            while(node->parent->nodes[0] == node){

                node = node->parent;
                if(isRoot(node)){
                    setIterator(it, NULL, 0, IT_BEFORE_FIRST);
                    return;
                }
                assert(node->parent != NULL && node->parent->nodes != NULL);
            }
            
            //now the node isn't first in parent's children. go lurk there
            pos = getItemOrNext(node->parent, node->items[0]->key) - 1;
            node = node->parent;
            shift--;
            found = !shift;
        }
    }
    setIterator(it, node, pos, IT_NORMAL);
}

void LSQ_ShiftPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT shift){
    if(NULL == iterator || !shift) return;
    TreeIteratorPtr it = iterator;

    if(it->state == IT_PAST_REAR && shift > 0 || it->state == IT_BEFORE_FIRST && shift < 0)
        return;
    if(it->state == IT_BEFORE_FIRST){
        LSQ_SetPosition(it, 0);
        shift--;
    } else if(it->state == IT_PAST_REAR){
        LSQ_SetPosition(it, it->tree->size - 1);
        shift++;
    }
    
    if(shift > 0) shiftForward(it, shift);
    else shiftBackward(it, -shift);

}

void setIterator(TreeIteratorPtr it, TreeNodePtr node, int pos, TreeIteratorState state){
    if(NULL == it) return;
    it->node = node;
    it->pos = pos;
    it->state = state;
}

void LSQ_SetPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT pos){
    TreeIteratorPtr it = iterator;
    
    if(pos < 0 || pos >= it->tree->size){
        setIterator(it, NULL, 0, pos < 0 ? IT_BEFORE_FIRST : IT_PAST_REAR);
        return;
    }

    TreeNodePtr root = it->tree->root;
    setIterator(it, root, 0, IT_NORMAL);
    LSQ_ShiftPosition(it, pos - (isLeaf(root) ? 0 : root->subtreeKeyCount[0]));
}

TreeIteratorPtr newTreeIterator(TreePtr tree, TreeNodePtr node, int pos, TreeIteratorState state){
    TreeIteratorPtr it = malloc(sizeof(TreeIterator));
    it->tree = tree;
    setIterator(it, node, pos, state);
    return it;
}

LSQ_IteratorT LSQ_GetElementByIndex(LSQ_HandleT handle, LSQ_IntegerIndexT index){
    if(NULL == handle) return NULL;
    TreePtr tree = handle;
    KeyPosPtr keyPos = SearchKey(tree->root, index);
    if(NULL == keyPos) return newTreeIterator(tree, NULL, 0, IT_BAD);

    return newTreeIterator(tree, keyPos->node, keyPos->pos, IT_NORMAL);
}

LSQ_IteratorT LSQ_GetFrontElement(LSQ_HandleT handle){
    TreeIteratorPtr it = malloc(sizeof(TreeIterator));
    if(NULL == it) return NULL;
    it->tree = handle;
    LSQ_SetPosition(it, 0);
    
    return it;
}

LSQ_IteratorT LSQ_GetPastRearElement(LSQ_HandleT handle){
    return newTreeIterator(handle, NULL, 0, IT_PAST_REAR);
}

void LSQ_DestroyIterator(LSQ_IteratorT iterator){
    free(iterator);
}

LSQ_BaseTypeT* LSQ_DereferenceIterator(LSQ_IteratorT iterator){
    if(!LSQ_IsIteratorDereferencable(iterator)) return NULL;
    TreeIteratorPtr it = iterator;
    return &it->node->items[it->pos]->data;
}

LSQ_IntegerIndexT LSQ_GetIteratorKey(LSQ_IteratorT iterator){
    if(!LSQ_IsIteratorDereferencable(iterator)) return 0;
    TreeIteratorPtr it = iterator;
    return it->node->items[it->pos]->key;
}

void LSQ_InsertElement(LSQ_HandleT handle, LSQ_IntegerIndexT key, LSQ_BaseTypeT value){
    TreeItemPtr item = malloc(sizeof(TreeItem));
    if(NULL == item) return;

    item->key = key;
    item->data = value;
    if(insertKeyInTree(handle, item))
        ((TreePtr)handle)->size++;
}

void fallDownDelete(TreePtr tree, FallDownBehave fb){
    TreeItemPtr item = fallDown(tree->root, fb);
    if(NULL == item) return;
    LSQ_DeleteElement(tree, item->key);
}

void LSQ_DeleteFrontElement(LSQ_HandleT handle){
    fallDownDelete(handle, FD_BY_LEFT_SIDE);
}

void LSQ_DeleteRearElement(LSQ_HandleT handle){
    fallDownDelete(handle, FD_BY_RIGHT_SIDE);
}

void LSQ_DeleteElement(LSQ_HandleT handle, LSQ_IntegerIndexT key){
    if(deleteKeyFromTree(handle, key))
        ((TreePtr)handle)->size--;
}



#ifdef LSQ_DEBUG
#include <stdio.h>

#endif
void dumpNode(TreeNodePtr node){
#ifdef LSQ_DEBUG
#define LSQ_DUMP_KEYS
    int i = 0;
#ifndef LSQ_DUMP_KEYS
    if(node->subtreeKeyCount != NULL) for(i = 0; i < node->childCount; i++)
        print("%d\t", node->subtreeKeyCount[i]);
    else printf("%d\t", node->keyCount);
#else
    for(i = 0; i < node->keyCount; i++)
        printf("%d\t", node->items[i]->key);
#endif
    printf("|||\t");
#endif
}

#define Q_SIZE 10000
int Ql = 0, Qr = 0, Qsize = 0;
TreeNodePtr Qu[Q_SIZE];

inline static void push(TreeNodePtr node){
    Qu[Qr++] = node;
    Qr %= Q_SIZE;
    Qsize++;
}

inline static TreeNodePtr pop(){
    int pos = Ql++;
    Ql %= Q_SIZE;
    Qsize--;
    return Qu[pos];
}

void dump(TreeNodePtr node){
#ifdef LSQ_DEBUG

    int i = 0, size = 0;
    push(node);
    printf("|||\t");
    while(Ql != Qr){
        node = pop();
        dumpNode(node);
        size += node->childCount;
        for(i = 0; i < node->childCount; i++) push(node->nodes[i]);
        if(Qsize == size){
            size = 0;
            printf("\n");
            printf("|||\t");
        }
    }
    printf("\n");
    //getchar();
    
#endif
}

void LSQ_DumpSequence(LSQ_HandleT handle){
    dump(((TreePtr)handle)->root);
}


