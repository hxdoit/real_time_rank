#ifndef _OBJECT_RBTREE_RBTREE_H
#define _OBJECT_RBTREE_RBTREE_H
#include "node.h"
#include <stdio.h> 
#include "array.h"

#define DIRECTION_LEFT 1
#define DIRECTION_RIGHT 2 

#define PURPOSE_FIND 1 
#define PURPOSE_INSERT 2 
#define PURPOSE_DELETE 3 

#define ERROR_REPEAT_INSERT 1 
#define ERROR_DELETE_NULL   2 

#define ACCESS_GET 1 
#define ACCESS_SET 2 

struct rb_root
{
	node* rb_node;
};

extern struct rb_root* get_root(void);
extern u_int rb_insert(extra_info, compare_unit, struct rb_root *, u_int*);
extern u_int rb_delete(compare_unit, struct rb_root *);

extern u_int rb_get_rank(compare_unit, struct rb_root *);
extern u_int rb_get_rank_by_id(u_int, struct rb_root *);

extern void visit_tree(node*);
extern node* access_rubish_node(node*, int);


#endif	
