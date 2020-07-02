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
        serverLog(LL_ERROR, "leftRotate rbt_tree null");
        return;
    }
    if (!node_x)
    {
        serverLog(LL_ERROR, "leftRotate node_x null");
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
        serverLog(LL_ERROR, "rightRotate rbt_tree null");
        return;
    }
    if (!node_x)
    {
        serverLog(LL_ERROR, "rightRotate node_x null");
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

void reorient(RBTTree *rbt_tree, RBTNode *node_x, int k)
{
    node_x->color = RED;
    node_x->left->color = node_x->right->color = BLACK;

    if(node_x->p->color == RED){
        node_x->p->p->color = RED;//此时x, p, x.p.p都为红
        if(node_x->p->key < node_x->p->p->key){
            if(k > node_x->p->key){
                node_x->color = BLACK;
                leftRotate(rbt_tree, node_x->p);
                rightRotate(rbt_tree, node_x->p);
            }
            else {
                node_x->color = BLACK;
                rightRotate(rbt_tree, node_x->p->p);
            }
        }
    }
    else // 父亲的顶点为 黑色
    {
        if(k < node_x->p->key){
            
            node_x->color = BLACK;
            rightRotate(rbt_tree,node_x->p);
            leftRotate(rbt_tree,node_x->p);
        }else{
            node_x->p->color = BLACK;
            leftRotate(rbt_tree, node_x->p->p);
        }
    }
    rbt_tree->root->color = BLACK;
}

RBTTree *insert(RBTTree *rbt_tree, int k)
{
    RBTNode *node_x = NULL, *node_p = NULL;
    node_x = rbt_tree->root;
    node_p = node_x;
    
    // 找到相关的点
    while (node_x != rbt_tree->nil)
    {
        // 
        if (node_x != rbt_tree->nil)
        {
            if(node_x->left->color == RED && node_x->right->color == RED)
            {
                // 两兄弟都是红色
                reorient(rbt_tree, node_x, k);
            }

            node_p = node_x;
            if (k < node_x->key)
            {
                node_x = node_x->left;
            }
            else if (k > node_x->key)
            {
                node_x = node_x->right;
            }
            else {
                serverLog(LL_ERROR, "insert key has");
                return rbt_tree;
            }
        }
    } // while
    serverLog(LL_NOTICE, "123");
    node_x = (RBTNode *)malloc(sizeof(RBTNode));
    assert( node_x != NULL);
    node_x->key = k;
    node_x->color = RED;
    node_x->left = node_x->right = rbt_tree->nil;
    node_x->p = node_p;

    //让x的父亲指向x
    if(rbt_tree->root == rbt_tree->nil)
    {    
        rbt_tree->root = node_x;
    } 
    else if(k < node_p->key)
    {
        node_p->left = node_x;
    }    
    else
    {   
        node_p->right = node_x;
    }

    reorient(rbt_tree, node_x, k);
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

    rbt_tree = insert(rbt_tree, 11);
}

