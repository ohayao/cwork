#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include "bridge/bridge_main/log.h"

//红黑树
typedef enum ColorType {RED, BLACK} ColorType;
typedef struct RBTNode{
    int key;
    struct RBTNode *left;
    struct RBTNode *right;
    struct RBTNode *p;
    ColorType color;
}RBTNode;

typedef struct rbt_root_t{
    RBTNode* root;
    RBTNode* nil; // 是一个根的标志, 最顶端
}RBTTree;

// 提供接口
// rbt_root_t* rbt_init(void)

// initRBT()
RBTTree *initRBT()
{
    RBTTree *rbt_tree = NULL;
    rbt_tree = (RBTTree *)malloc(sizeof(RBTTree));
    if (!rbt_tree)
    {
        return NULL;
    }
    rbt_tree->nil = (RBTNode *)malloc(sizeof(RBTNode));
    assert(NULL != rbt_tree->nil);
    rbt_tree->nil->color = BLACK;
    rbt_tree->nil->left = rbt_tree->nil->right = NULL;
    rbt_tree->nil->p = NULL;

    rbt_tree->root = rbt_tree->nil;
    return rbt_tree;
}

// 红黑树旋转
// 两种旋转
// 左旋转 和 右旋转
void leftRotate(RBTTree *rbt_tree, RBTNode *node_x)
{
    if (!rbt_tree)
    {
        serverLog(LL_ERROR, "rbt_tree null");
        return;
    }
    if (!node_x)
    {
        serverLog(LL_ERROR, "node_x null");
        return;
    }

    // x 和 y 进行一个换位
    RBTNode *node_y = node_x->right;
    node_x->right = node_y->left;

    // 换位之后, 需要保持父亲信息
    if (node_x->right != rbt_tree->nil)
    {
        node_x->right->p = node_x;
    }

    node_y->p = node_x->p;
    // 如果以前的根 是 x, 那么根就易主
    if (node_x->p == rbt_tree->nil) {
        rbt_tree->root = node_y;
    }
    // 要查看Y 到底是比 p 大还是小
    // 如果 Y 的 key 小
    // 如果
    else if (node_y->key < node_y->p->key ) {
        node_y->p->left = node_y;
    }
    // 
    else
    {
        node_y->p->right = node_y;
    }
    node_y->left = node_x;
    node_x->p = node_y;
    return;
}

// node_x 是在最顶上的
void rightRotate(RBTTree *rbt_tree, RBTNode *node_x)
{
    if (!rbt_tree)
    {
        serverLog(LL_ERROR, "rbt_tree null");
        return;
    }
    if (!node_x)
    {
        serverLog(LL_ERROR, "node_x null");
        return;
    }
    RBTNode *node_y = node_x->left;
    node_x->left = node_y->right;

    // 换位后, 保持父亲信息
    // nil 是真的树根结点
    // 如果 
    if (node_x->left != rbt_tree->nil)
    {
        node_x->left->p = node_x;
    }
    node_y->p = node_x->p;
    if (node_y->p == rbt_tree->nil)
    {
        rbt_tree->root = node_y;
    }
    else if (node_y->key < node_y->p->key)
    {
        node_y->p->left = node_y;
    }
    else
    {
        node_y->p->right = node_y;
    }
    
    node_y->right = node_x;
    node_x->p = node_y;
}

RBTTree *insert(RBTTree *rbt_tree, int k)
{
    RBTNode *x, *p;
}

int main()
{
    RBTTree *rbt_tree = initRBT();
    if (!rbt_tree)
    {
        serverLog(LL_ERROR, "initRBT wrong");
        return 1;
    }
    serverLog(LL_NOTICE, "initRBT success");

}

