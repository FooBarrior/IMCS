#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "linear_sequence_assoc.h"

/*TODO implement subtree key counters*/

#define t 2
//B-tree nodes consisNts of:
//at least 1 and at most (2 * t - 1) keys in root
//at least (t - 1) and at most (2 * t - 1) keys in other nodes
#define MIN_KEY_COUNT (t - 1)
#define MAX_KEY_COUNT (2 * t - 1)
//MAX_CHILD_COUNT MUST BE always (MAX_KEY_COUNT + 1)
#define MAX_CHILD_COUNT (MAX_KEY_COUNT + 1)

typedef struct {
    LSQ_IntegerIndexT key;
    LSQ_BaseTypeT data;
} TreeItem, *TreeItemRef;

typedef struct TreeNode{
    //the number of children is always 0 in leaves, and (keyCount + 1) otherwise
    size_t keyCount, childCount;
    TreeItemRef items[MAX_KEY_COUNT];
    
    //if there are n items in node, then there are (n + 1) children there;
    //thinking parent could be useful
    struct TreeNode **nodes, *parent;
    
    //subtreeKeyCount is helpful to iterative operations
    //to leave disk data flow possible, we store here counts for each child node
    size_t *subtreeKeyCount;
    
    //i am also ensured that it will be always leaf, if it is sptlitted from leaf, or it is initial node
} TreeNode, *TreeNodeRef;

typedef struct {
    LSQ_IntegerIndexT size;
    TreeNodeRef root;
    int height;
    //seems to be optional, but would be declared, AH FUCK ILL DELETE IT >[} *TODO*
    TreeNodeRef first, last;
} Tree, *TreeRef;

//key position structure for key-searching functions
typedef struct {
    TreeNodeRef node;
    int pos;
} KeyPos, *KeyPosRef;

//structure-initializing functions
static TreeNodeRef newTreeNodeRef(int keyCount, TreeNodeRef parent, bool isLeaf);
static KeyPosRef newKeyPos(TreeNodeRef node, int pos);

#define UNDEF_KEY_POS (newKeyPos(NULL, 0))

//key searching functions
static int getItemOrNext(TreeNodeRef node, LSQ_IntegerIndexT key);
static KeyPosRef SearchKey(TreeNodeRef node, LSQ_IntegerIndexT key);

//determine whether the node is leaf or not
static int isLeaf(TreeNodeRef node);

//functions, inserting/deleting key directly from node
static void addKey(TreeNodeRef node, int pos, TreeItemRef x);
static void addNode(TreeNodeRef node, TreeNodeRef newNode, int pos);
static void excludeKey(TreeNodeRef node, int pos, bool needFree);
static void excludeNode(TreeNodeRef node, int pos, bool needFree);

//main key insertion algorithm block
static void pourElements(TreeNodeRef a, TreeNodeRef b, int start);
static bool splitChild(TreeNodeRef node, int x);
static bool insertKey(TreeNodeRef node, TreeItemRef key);
static bool insertKeyInTree(TreeRef tree, TreeItemRef item);

//main deletion algorithm block
static void mergeToLeft(TreeNodeRef parent, int lPos);
static bool deleteKey(TreeNodeRef node, LSQ_IntegerIndexT key, int pos, bool needFree);
static bool deleteKeyFromTree(TreeRef tree, LSQ_IntegerIndexT key);

TreeNodeRef newTreeNodeRef(int keyCount, TreeNodeRef parent, bool isLeaf){
    int i = 0;
    TreeNodeRef res = malloc(sizeof(TreeNode));
    if(NULL == res) return NULL;
    
    if(!isLeaf){

        res->children = malloc(sizeof(TreeNode)*MAX_CHILD_COUNT);
        res->subtreeKeyCount = malloc(sizeof(size_t)*MAX_CHILD_COUNT);
        if(NULL == res->children || NULL == res->subtreeKeyCount){
            free(res->children);
            free(res->subtreeKeyCount);
            free(res);
            return NULL;
        }
        
    } else
        res->children = res->subtreeKeyCount = NULL;

    res->keyCount = keyCount;
    res->childCount = isLeaf ? 0 : keyCount + 1;
    res->parent = parent;
    
    for(i = res->childCount; i < MAX_CHILD_COUNT; i++){
        res->subtreeKeyCount 0;
        res->nodes[i] = NULL;
    }
    return res;
}

KeyPosRef newKeyPos(TreeNodeRef node, int pos){
    KeyPosRef res = malloc(sizeof(KeyPos));
    if(NULL == res) return NULL;
    res->node = node;
    res->pos = pos;
    return res;
}

int getItemOrNext(TreeNodeRef node, LSQ_IntegerIndexT key){
    int pos = 0;
    assert(node != NULL);
    for(pos = 0; pos < node->keyCount && node->items[pos]->key < key; pos++);
    return pos;
}

KeyPosRef SearchKey(TreeNodeRef node, LSQ_IntegerIndexT key){
    if(NULL == node) return UNDEF_KEY_POS;
    int found = 0;
    
    while(!found){
        int pos = getItemOrNext(node, key);
        if(pos < 0) return UNDEF_KEY_POS;
        assert(node->items[pos] != NULL);
        if(pos < node->keyCount && node->items[pos]->key == key)
            return newKeyPos(node, key);
        found = found || isLeaf(node);
        if(!found) node = node->nodes[pos];
    }
}

int isLeaf(TreeNodeRef node){
//me don't allocate memory for .nodes if it's leaf
    return node != NULL && NULL == node->nodes;
}

void addKey(TreeNodeRef node, int pos, TreeItemRef x){
    assert(node != NULL && pos >= 0 && pos <= node->keyCount && node->items != NULL);
    assert(node->keyCount < MAX_KEY_COUNT);
    memmove(
        node->items + pos + 1,
        node->items + pos,
        sizeof(TreeNodeRef) * (node->keyCount - pos)
    );
    node->items[pos] = x;
    node->keyCount++;
}

void addNode(TreeNodeRef node, TreeNodeRef newNode, int pos){
    assert(node != NULL && pos >= 0 && pos <= node->childCount && node->items != NULL);
    assert(node->childCount < MAX_CHILD_COUNT);
    memmove(
        node->nodes + pos + 1,
        node->nodes + pos,
        sizeof(TreeNodeRef) * (node->childCount - pos)
    );
    node->nodes[pos] = newNode;
    node->childCount++;
}

void excludeKey(TreeNodeRef node, int pos, int needFree){
    assert(node != NULL && pos >= 0 && pos < node->keyCount && node->items != NULL);
    //assert(node->keyCount > MIN_KEY_COUNT); root???
    if(needFree) free(node->items[pos]);
    memmove(
        node->items + pos,
        node->items + pos + 1,
        sizeof(TreeNodeRef) * (node->keyCount - pos - 1)
    );
    node->items[--node->keyCount] = NULL;
}

void excludeNode(TreeNodeRef node, int pos, int needFree){
    assert(node != NULL && pos >= 0 && pos < node->childCount && node->nodes != NULL);
    assert(node->childCount > t);
    if(needFree) free(node->nodes[pos]);
    memmove(
        node->nodes + pos,
        node->nodes + pos + 1,
        sizeof(TreeNodeRef) * (node->childCount - pos - 1)
    );
    node->nodes[--node->childCount] = NULL;
}

void pourElements(TreeNodeRef a, TreeNodeRef b, int start){
    assert(a->keyCount + b->keyCount - start <= MAX_KEY_COUNT && a->childCount == a->keyCount + 1);
    memcpy(a->items + a->keyCount, b->items + start, (start + 1) * sizeof(TreeItemRef));
    
    if(!isLeaf(a))
        memcpy(a->nodes + a->childCount, b->nodes + start + 1, (start + 1) * sizeof(TreeNodeRef));
    
    int i = 0;
    for(i = start; i < b->keyCount; i++) b->items[i] = NULL, b->nodes[i + 1] = NULL;

    a->childCount = (a->keyCount += b->keyCount - start);
    b->childCount = (b->keyCount = start) + 1;
}

bool splitChild(TreeNodeRef node, int x){
    assert(node != NULL && MAX_KEY_COUNT != node->keyCount);
    assert(x >= 0 && x <= node->keyCount && node->keyCount >= 0);
    TreeNodeRef child = node->nodes[x], newNode = newTreeNodeRef(0, node);
    if(NULL == newNode) return false;
    assert(child != NULL);
    pourElements(newNode, child, MIN_KEY_COUNT);
    //insert middle child's key in node
    addKey(node, x, child->items[t]); // это мне не нра
    addNode(node, newNode, x + 1);
    return true;
}

bool insertKey(TreeNodeRef node, TreeItemRef key){
    assert(node != NULL && key != NULL);
    int pos = getItemOrNext(node, key->key);
    if(isLeaf(node)){
        addKey(node, pos, key);
        return true;
    }
    
    if(node->nodes[pos]->keyCount == MAX_KEY_COUNT){
        if(!splitChild(node, pos)) return false;
        //after we splitted, a chance, that a key
        //added in node is less then a new key, is apperaring
        if(node->items[pos]->key < key->key) pos++;
        assert(node->items[pos]->key < key->key);
    }
    return insertKey(node->nodes[pos], key);
}

bool insertKeyInTree(TreeRef tree, TreeItemRef item){
    if(NULL == tree) return false;
    assert(tree->root != NULL && item != NULL);
//    tree->size++;
    if(MAX_KEY_COUNT != tree->root->keyCount) return insertKey(tree->root, item); // may be assert?
    TreeNodeRef newRoot = newTreeNodeRef(0, NULL), oldRoot = tree->root;
    oldRoot->parent = newRoot;
    tree->root = newRoot;
    splitChild(newRoot, 0);
    return insertKey(tree->root, item);
}

//each of x and y has less then t keys and keys of y migrate to x, splitten by mediane
void mergeToLeft(TreeNodeRef parent, int lPos){
    assert(parent != NULL);
    assert(lPos >= 0 && lPos < parent->keyCount && parent->keyCount <= MAX_KEY_COUNT);
    TreeNodeRef x = parent->nodes[lPos], y = parent->nodes[lPos + 1];
    assert(x->keyCount < t && y->keyCount < t);
    x->items[x->keyCount++] = parent->items[lPos];
    x->nodes[x->childCount++] = y->nodes[0];
    pourElements(x, y, 0);
    excludeKey(parent, lPos, 0);
    excludeNode(parent, lPos + 1, 1);
}

bool deleteKey(TreeNodeRef node, LSQ_IntegerIndexT key, int pos, int needFree){
//Kormen, 2a-2b
    bool res = false;
    if(NULL == node) return false;
    
    bool internalDelete(TreeNodeRef child, int childKeyPos){
        if(child->keyCount < t) return false;
        if(needFree) free(node->items[pos]);
        node->items[pos] = child->items[childKeyPos];
        res = deleteKey(child, child->items[childKeyPos]->key, childKeyPos, 0);
        return true;
    }
//1
    if(isLeaf(node)){
        if(node->items[pos]->key != key) return false;
        excludeKey(node, pos, true);//!!!
        return true;
    }
//2
//if key found in internal node
    if(node->items[pos]->key == key){
        //we always have left and right children of key splitter
        assert(node->nodes[pos] != NULL);
        if(internalDelete(node->nodes[pos], node->nodes[pos]->keyCount - 1)) return res;
        assert(node->nodes[pos + 1] != NULL);
        if(internalDelete(node->nodes[pos + 1], 0)) return res;
        mergeToLeft(node, pos); //keyCounts left < t && right < t
    }
//3
    if(node->items[pos]->key != key){
//key is not found yet and we don't need free it? FAIL
        assert(needFree && node->nodes[pos]->keyCount >= MIN_KEY_COUNT);
        if(node->nodes[pos]->keyCount == MIN_KEY_COUNT){
            TreeNodeRef mc = node->nodes[pos], lc = node->nodes[pos - 1], rc = node->nodes[pos + 1];
            int lkp = pos - 1, rkp = pos; /*  /'|'\  */
            assert(lc != NULL || rc != NULL);
            if(lc != NULL && lc->keyCount > MIN_KEY_COUNT){
                addKey(mc, 0, node->items[lkp]);
                node->items[lkp] = lc->items[lc->keyCount - 1];
                addNode(mc, lc->nodes[lc->childCount - 1], 0);
                excludeKey(lc, lc->keyCount - 1, 0);
                excludeNode(lc, lc->childCount - 1, 0);
            } else if(rc != NULL && rc->keyCount > MIN_KEY_COUNT){
                addKey(mc, mc->keyCount - 1, node->items[rkp]);
                node->items[rkp] = rc->items[0];
                excludeKey(rc, 0, 0);
                addNode(mc, rc->nodes[0], mc->childCount - 1);
                excludeNode(rc, 0, 0);
            } else{
                if(lc != NULL) mergeToLeft(node, lkp);
                else if(rc != NULL) mergeToLeft(node, rkp);
                else assert(1);
            }
        }
        return deleteKey(node->nodes[pos], key, getItemOrNext(node, key), 1);
    }

} //deleteKey

bool deleteKeyFromTree(TreeRef tree, LSQ_IntegerIndexT key){
    assert(tree != NULL && tree->root != NULL);
    TreeNodeRef rt = tree->root;
    if(!rt->keyCount) return false;

    int pos = getItemOrNext(rt, key);
    if(1 == rt->keyCount){
        assert(rt->items[0] != NULL == rt->items[1] != NULL); //well, yep, either root is also leaf, or not, nor none of that
        
        TreeNodeRef tmp = rt->nodes[0];
        bool res = deleteKey(rt, key, pos, 1);
        if(!rt->keyCount){
            free(rt);
            tree->root = tmp;
        }
        return res;
        
        /*if(rt->items[0] == NULL){
            if(rt->items[0]->key != key) return 0;
            excludeKey(rt, 0, 1);
            free(rt);
            tree->root = NULL;
            return 1;
        }
        if(rt->items[0] < t && rt->items[1] < t){
            tree->root = rt->items[0];
            mergeToLeft(rt, 0);
            free(rt);
            pos = getItemOrNext(tree->root, key);
            return deleteKey(tree->root, key, pos, 1);
        }
         */
    }
    return deleteKey(rt, key, pos, 1);
}

void destroySubTree(TreeNodeRef node){
    int i = 0;
    for(i = 0; i < node->keyCount; i++) if(node->items[i] != NULL) free(node->items[i]);
    for(i = 0; i < node->childCount; i++){
        if(NULL == node->nodes[i]) continue;
        destroySubTree(node->nodes[i]);
        free(node->nodes[i]);
    }
}

LSQ_HandleT LSQ_CreateSequence(void){
    TreeRef tree = malloc(sizeof(Tree));
    tree->first = tree->last = NULL;
    tree->size = 0;
    tree->root = newTreeNodeRef(0, NULL);
    return (LSQ_HandleT)tree;
}

void LSQ_DestroySequence(LSQ_HandleT handle){
    if(NULL == handle) return;
    TreeRef tree = (TreeRef)handle;
    destroySubTree(tree->root);
    free(tree->root);
    free(tree);
}

LSQ_IntegerIndexT LSQ_GetSize(LSQ_HandleT handle){
    return ((NULL == handle) ? 0 : ((TreeRef)handle)->size);
}

int LSQ_IsIteratorDereferencable(LSQ_IteratorT iterator){
    
}

