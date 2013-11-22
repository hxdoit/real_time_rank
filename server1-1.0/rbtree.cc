#include "rbtree.h"

static void rb_erase(node *, struct rb_root *);
static void __rb_erase_color(node *, node *, struct rb_root *);
static void rb_insert_color(node *, struct rb_root *);

static struct rb_root root;
 

static u_int compare(compare_unit left, compare_unit right)
{	
        //先比较积分再比较插入时间
	if (left.score < right.score)
	{
		return -1;
	}
	else if (left.score > right.score)
	{
		return 1;
	}
	else if (left.time < right.time) 
	{
		return 1;
	}
	else if (left.time > right.time)
	{ 
		return -1;
	}
	return 0;

  
}
//当插入或删除时，更新child_num字段
//插入时返回插入位置的父节点
//删除时，返回需要删除的节点
static node* rb_update_child_num(compare_unit c_unit, u_int purpose, struct rb_root *root)
{
	node *cur, *parent;
	parent= NULL;
        cur=root->rb_node;
	//节点的比较策略：先比较积分数再比较插入时间
	while(cur)
	{
                //当查找的目的是为了插入和删除时，需要更新子节点数目
		if (purpose == PURPOSE_INSERT)
		{
			cur -> child_num ++; 
		}
		else if (purpose == PURPOSE_DELETE)
		{
			cur -> child_num --;
		}
                
                //当是插入时，需要知道最终插入位置在父节点的左侧还是右侧
                int result = compare(c_unit, cur->c_unit);
		if (result<0)
		{
			parent=cur; 
			cur=node1(cur->rb_left); 
		}
		else if (result>0)
		{
			parent=cur; 
			cur=node1(cur->rb_right);
		}
		else 
		{
			// 命中
			return cur;
		}
	} 
	// 未命中
	return parent;
}

// 查找一个节点是否存在
static node* rb_just_find(compare_unit c_unit, struct rb_root *root)
{
	node *cur=root->rb_node;
	//节点的比较策略：先比较积分数再比较插入时间
	while(cur)
	{
		int result = compare(c_unit, cur->c_unit);
		if(result<0)
			cur=node1(cur->rb_left); 
		else if(result>0)
			cur=node1(cur->rb_right);
                else
                        return cur;
	} 
	// 未命中
	return NULL;
}

static void __rb_rotate_left(node *n, struct rb_root *root)
{
	node *right = node1(n->rb_right);

	if ((n->rb_right = right->rb_left))
		node1(right->rb_left)->rb_parent = id(n);
	right->rb_left = id(n);

	if ((right->rb_parent = n->rb_parent))
	{
		if (id(n) == node1(n->rb_parent)->rb_left)
			node1(n->rb_parent)->rb_left = id(right);
		else
			node1(n->rb_parent)->rb_right = id(right);
	}
	else
		root->rb_node = right;
	n->rb_parent = id(right);
	//以下代码为新加，更新child_num字段
	right -> child_num = n->child_num;
        n -> child_num=0;
	if (n -> rb_left)
		n -> child_num += node1(n -> rb_left) -> child_num; 
	if (n -> rb_right)
		n -> child_num += node1(n -> rb_right)->child_num; 
	n -> child_num += 1;
}

static void __rb_rotate_right(node *n, struct rb_root *root)
{
	node *left = node1(n->rb_left);

	if ((n->rb_left = left->rb_right))
		node1(left->rb_right)->rb_parent = id(n);
	left->rb_right = id(n);

	if ((left->rb_parent = n->rb_parent))
	{
		if (id(n) == node1(n->rb_parent)->rb_right)
			node1(n->rb_parent)->rb_right = id(left);
		else
			node1(n->rb_parent)->rb_left = id(left);
	}
	else
		root->rb_node = left;
	n->rb_parent = id(left);
	//以下代码为新加，更新child_num字段
	left -> child_num = n->child_num;
        n -> child_num=0;
	if (n -> rb_left)
		n -> child_num += node1(n -> rb_left) -> child_num; 
	if (n -> rb_right)
		n -> child_num += node1(n -> rb_right)->child_num; 
	n -> child_num += 1;
}

/********************华丽分隔*************************************/

struct rb_root * get_root(void)
{
	return &root;
}

// 插入成功返回0，失败返回值大于0
u_int rb_insert(extra_info e_info, compare_unit c_unit, struct rb_root *root, u_int* node_id)
{
        node* target;
	if( target=rb_just_find(c_unit, root))
	{
		//树中已经有这个节点  
                *node_id=id(target);
		fprintf(stderr, "the node being inserted already exists\n"); 
		return ERROR_REPEAT_INSERT; 
	} 
	//创建一个节点 
	target = access_rubish_node(NULL, ACCESS_GET);
        if(!target)
	    target = create_node();
        *node_id=id(target);
        target -> e_info= e_info;
	target -> rb_color=RB_RED;
	target  -> rb_parent = 0;
	target  -> rb_left= 0;
	target->rb_right=0;
	target ->c_unit= c_unit;
	target -> child_num= 1;
	if (root->rb_node == 0)
	{
		root->rb_node=target;
		rb_insert_color(target,root);
		return 0;
	}
	node* parent= rb_update_child_num(c_unit, PURPOSE_INSERT, root);
	u_int direction = 0;
	if (compare(c_unit, parent->c_unit)<0)
	{
		// 节点将被插入左侧
		target -> rb_parent = id(parent);
		parent -> rb_left = id(target); 
	}
	else
        {
		// 节点将被插入右侧
		target -> rb_parent = id(parent);
		parent -> rb_right= id(target); 
	}
	rb_insert_color(target,root);
        return 0;
}

//删除成功返回0，失败返回值大于0
u_int rb_delete(compare_unit c_unit, struct rb_root *root)
{

	if(! rb_just_find(c_unit, root))
	{
		//树中没有这个节点  
		fprintf(stderr, "the node being deleted doesn't exist\n"); 
		return ERROR_DELETE_NULL; 
	} 
	node* n= rb_update_child_num(c_unit, PURPOSE_DELETE, root);
	rb_erase(n, root);
        return 0;
}





//获取某个节点的排名
//返回名次，如果失败返回0
u_int rb_get_rank(compare_unit c_unit, struct rb_root *root)
{
	node* n= rb_just_find(c_unit, root);
	if (!n)
	{
		// 没有找到要查找的节点 
		fprintf(stderr, "the node being get rank doesn't exist\n"); 
		return 0;
	}
	int rank = 0;
	if (n -> rb_left)
		rank += node1(n -> rb_left) -> child_num;
	rank += 1;
	node* parent;
	while(parent=node1(n->rb_parent))
	{
		if (parent -> rb_right== id(n))
		{
			// 如果当前节点是父节点的右孩子
			if (parent -> rb_left)
				rank += node1(parent -> rb_left)->child_num; 
			rank += 1;
		}
		n = parent;
	}
	return rank;
}

//在确认节点存在情况下,获取某个节点的排名
//返回名次，如果失败返回0
u_int rb_get_rank_by_id(u_int node_id, struct rb_root *root)
{

        node* n=node1(node_id);
	int rank = 0;
	if (n -> rb_left)
		rank += node1(n -> rb_left) -> child_num;
	rank += 1;
	node* parent;
	while(parent=node1(n->rb_parent))
	{
		if (parent -> rb_right== id(n))
		{
			// 如果当前节点是父节点的右孩子
			if (parent -> rb_left)
				rank += node1(parent -> rb_left)->child_num; 
			rank += 1;
		}
		n = parent;
	}
	return rank;
}

void visit_tree(node* n)
{
    //    printf("%d %d %d %d %d\n",  n -> score,n->child_num,n->rb_left,n->rb_right,n->rb_color);
    if (n -> rb_left)
       visit_tree(node1(n->rb_left));  
    if (n -> rb_right)
       visit_tree(node1(n->rb_right));  
}

node* access_rubish_node(node*p,int mode)
{
  static node*n = NULL; 
  if (mode == ACCESS_GET)
  {
     node*temp=n;
     n = NULL;
     return temp;
  }
  else 
       n = p;
  return NULL;
}

/********************华丽分隔*************************************/

static void rb_insert_color(node *n, struct rb_root *root)
{
	node *parent, *gparent;

	while ((parent = node1(n->rb_parent)) && parent->rb_color == RB_RED)
	{
		gparent = node1(parent->rb_parent);

		if (parent == node1(gparent->rb_left))
		{
			{
				register node *uncle = node1(gparent->rb_right);
				if (uncle && uncle->rb_color == RB_RED)
				{
					uncle->rb_color = RB_BLACK;
					parent->rb_color = RB_BLACK;
					gparent->rb_color = RB_RED;
					n = gparent;
					continue;
				}
			}

			if (parent->rb_right == id(n))
			{
				register node *tmp;
				__rb_rotate_left(parent, root);
				tmp = parent;
				parent = n;
				n = tmp;
			}

			parent->rb_color = RB_BLACK;
			gparent->rb_color = RB_RED;
			__rb_rotate_right(gparent, root);
		} else {
			{
				register node *uncle = node1(gparent->rb_left);
				if (uncle && uncle->rb_color == RB_RED)
				{
					uncle->rb_color = RB_BLACK;
					parent->rb_color = RB_BLACK;
					gparent->rb_color = RB_RED;
					n = gparent;
					continue;
				}
			}

			if (parent->rb_left == id(n))
			{
				register node *tmp;
				__rb_rotate_right(parent, root);
				tmp = parent;
				parent = n;
				n = tmp;
			}

			parent->rb_color = RB_BLACK;
			gparent->rb_color = RB_RED;
			__rb_rotate_left(gparent, root);
		}
	}

	root->rb_node->rb_color = RB_BLACK;
}

static void __rb_erase_color(node *n, node *parent,
		struct rb_root *root)
{
	node *other;

	while ((!n || n->rb_color == RB_BLACK) && n != root->rb_node)
	{
		if (parent->rb_left == id(n))
		{
			other = node1(parent->rb_right);
			if (other->rb_color == RB_RED)
			{
				other->rb_color = RB_BLACK;
				parent->rb_color = RB_RED;
				__rb_rotate_left(parent, root);
				other = node1(parent->rb_right);
			}
			if ((!other->rb_left ||
						node1(other->rb_left)->rb_color == RB_BLACK)
					&& (!other->rb_right ||
						node1(other->rb_right)->rb_color == RB_BLACK))
			{
				other->rb_color = RB_RED;
				n = parent;
				parent = node1(n->rb_parent);
			}
			else
			{
				if (!other->rb_right ||
						node1(other->rb_right)->rb_color == RB_BLACK)
				{
					register node *o_left;
					if ((o_left = node1(other->rb_left)))
						o_left->rb_color = RB_BLACK;
					other->rb_color = RB_RED;
					__rb_rotate_right(other, root);
					other = node1(parent->rb_right);
				}
				other->rb_color = parent->rb_color;
				parent->rb_color = RB_BLACK;
				if (other->rb_right)
					node1(other->rb_right)->rb_color = RB_BLACK;
				__rb_rotate_left(parent, root);
				n = root->rb_node;
				break;
			}
		}
		else
		{
			other = node1(parent->rb_left);
			if (other->rb_color == RB_RED)
			{
				other->rb_color = RB_BLACK;
				parent->rb_color = RB_RED;
				__rb_rotate_right(parent, root);
				other = node1(parent->rb_left);
			}
			if ((!other->rb_left ||
						node1(other->rb_left)->rb_color == RB_BLACK)
					&& (!other->rb_right ||
						node1(other->rb_right)->rb_color == RB_BLACK))
			{
				other->rb_color = RB_RED;
				n = parent;
				parent = node1(n->rb_parent);
			}
			else
			{
				if (!other->rb_left ||
						node1(other->rb_left)->rb_color == RB_BLACK)
				{
					register node *o_right;
					if ((o_right = node1(other->rb_right)))
						o_right->rb_color = RB_BLACK;
					other->rb_color = RB_RED;
					__rb_rotate_left(other, root);
					other = node1(parent->rb_left);
				}
				other->rb_color = parent->rb_color;
				parent->rb_color = RB_BLACK;
				if (other->rb_left)
					node1(other->rb_left)->rb_color = RB_BLACK;
				__rb_rotate_right(parent, root);
				n = root->rb_node;
				break;
			}
		}
	}
	if (n)
		n->rb_color = RB_BLACK;
}

static void rb_erase(node *n, struct rb_root *root)
{
	node *child, *parent;
	int color;

	if (!n->rb_left)
		child = node1(n->rb_right);
	else if (!n->rb_right)
		child = node1(n->rb_left);
	else
	{
		node *old = n, *left;

		n = node1(n->rb_right);
                n -> child_num--;
		while ((left = node1(n->rb_left)))
                 {
			n = left;
                       n->child_num--;
                        
                 }
		child = node1(n->rb_right);
		parent = node1(n->rb_parent);
		color = n->rb_color;

		if (child)
			child->rb_parent = id(parent);
		if (parent)
		{
			if (parent->rb_left == id(n))
				parent->rb_left = id(child);
			else
				parent->rb_right = id(child);
		}
		else
			root->rb_node = child;

		if (n->rb_parent == id(old))
			parent = n;
		n->rb_parent = old->rb_parent;
		n->rb_color = old->rb_color;
		n->rb_right = old->rb_right;
		n->rb_left = old->rb_left;
		n->child_num= old->child_num;
                access_rubish_node(old, ACCESS_SET);
		if (old->rb_parent)
		{
			if (node1(old->rb_parent)->rb_left == id(old))
				node1(old->rb_parent)->rb_left = id(n);
			else
				node1(old->rb_parent)->rb_right = id(n);
		} else
			root->rb_node = n;

		node1(old->rb_left)->rb_parent = id(n);
		if (old->rb_right)
			node1(old->rb_right)->rb_parent = id(n);
		goto color;
	}

        access_rubish_node(n, ACCESS_SET);
	parent = node1(n->rb_parent);
	color = n->rb_color;

	if (child)
		child->rb_parent = id(parent);
	if (parent)
	{
		if (parent->rb_left == id(n))
			parent->rb_left = id(child);
		else
			parent->rb_right = id(child);
	}
	else
		root->rb_node = child;

color:
	if (color == RB_BLACK)
		__rb_erase_color(child, parent, root);
}


