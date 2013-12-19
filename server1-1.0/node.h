#ifndef _OBJECT_NODE_H
#define _OBJECT_NODE_H

typedef unsigned long u_long;
typedef unsigned int u_int;

//节点上的附加信息
typedef struct 
{
	u_long oid;    //对象id

}extra_info;

//比较单元，即用于排序的字段
typedef struct 
{
	u_int score;   //积分
	u_int time;    //插入时间

}compare_unit;

//一个节点
typedef struct rb_node
{
	u_int rb_parent;
	u_int rb_left;
	u_int rb_right;
	compare_unit c_unit;
	extra_info e_info;
	u_int child_num:31;
	u_int rb_color:1;
        #define RB_RED          0
        #define RB_BLACK        1
}node;

#endif
