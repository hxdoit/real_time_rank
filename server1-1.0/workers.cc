#include "workers.h"
#include "rbtree.h"

pthread_rwlock_t work_lock = PTHREAD_RWLOCK_INITIALIZER;

// 服务初始化时插入节点
void init_insert_node(void *arg)
{
	ss_input* input = (ss_input*)arg;
	u_int re=0, node_id=0 ;

        compare_unit c_unit;
        c_unit.score=input->score;
        c_unit.time=input->time;
        extra_info e_info;
        e_info.oid=input->oid;

	re = rb_insert(e_info, c_unit, get_root(), &node_id);
	if(re == 0)
	{
		set_element(input->oid, node_id);
	}
}
// 插入节点
char* insert_node(void *arg)
{
	ss_input* input = (ss_input*)arg;
	u_int code=0,data=0,node_id=0 ;
	char *msg = NULL;

	node_id= get_element(input->oid);
	if(node_id)
	{
		code= 100001;
		msg = "repeat insert"; 
	}
	else 
	{
		compare_unit c_unit;
		c_unit.score=input->score;
		c_unit.time=input->time;
		extra_info e_info;
		e_info.oid=input->oid;
		rb_insert(e_info, c_unit, get_root(), &node_id);
		set_element(input->oid, node_id);
		code= 100000;
		msg = "ok"; 
	}
	data = rb_get_rank_by_id(node_id, get_root());
	return set_result(code,msg,data);
}

// 删除节点
char* delete_node(void *arg)
{
	ss_input* input = (ss_input*)arg;
	u_int code=100000,data=0, old_node_id=0 ;
	char *msg ="ok";
	old_node_id=get_element(input->oid); 
	if(old_node_id)
	{
		node* target=node1(old_node_id);
		compare_unit c_unit;
		c_unit.score=target->c_unit.score;
		c_unit.time=target->c_unit.time;
		rb_delete(c_unit, get_root());
                del_element(input->oid);
	}
	return set_result(code,msg,data);
}

// 更新节点
char* update_node(void *arg)
{
	ss_input* input = (ss_input*)arg;
	u_int re=0,code=0,data=0 ;
	char *msg="ok";

	u_int old_node_id=get_element(input->oid);
	compare_unit c_unit;
	if(old_node_id)
	{
                //先删掉老节点
                node* old_node=node1(old_node_id);
		c_unit.score=old_node->c_unit.score;
		c_unit.time=old_node->c_unit.time;
		rb_delete(c_unit, get_root());
	}
	//再重新插入
	c_unit.score=input->score;
	c_unit.time=input->time;
	extra_info e_info;
	e_info.oid=input->oid;
	u_int new_node_id;
	rb_insert(e_info, c_unit ,get_root(), &new_node_id);
	set_element(input->oid, new_node_id);

	//算出排名
	data = rb_get_rank_by_id(new_node_id, get_root());  
	return set_result(code,msg,data);
}

// 查询节点排名
char* query_node(void *arg)
{
	ss_input* input = (ss_input*)arg;
	u_int code=100000,data=0 ;
	char *msg="ok";

        u_int node_id = get_element(input->oid);
        if(node_id)
		data = rb_get_rank_by_id(node_id, get_root());  
	return set_result(code,msg,data);
}

void save_nodes_to_file_1(node*n, FILE*f)
{
	if(n)
	{
		char buf[17];
		*(u_long*)buf = n->e_info.oid;
		*(u_int*)(buf+8) = n->c_unit.score;
		*(u_int*)(buf+12) = n->c_unit.time;
		buf[16]='\n';
                fwrite(buf,1,17,f);
	}
        if(n->rb_left)
		save_nodes_to_file_1(node1(n->rb_left),f);
        if(n->rb_right)
		save_nodes_to_file_1(node1(n->rb_right),f);
}
//将树中所有节点落地 
char* save_nodes_to_file(void *arg)
{
	time_t timer;
	struct tm *timeinfo;
	timer = time(NULL);
	timeinfo= localtime(&timer);

        char file_name[100];
        char save_succ[100];
        memset(file_name,0,100);
        memset(save_succ,0,100);
	char* path = get_config_var("nodes_to_file_path");
	if(!path)path = "/tmp/";

	sprintf (file_name, "%s/nodes_to_file_%4d-%02d-%02d",path, 1900+timeinfo->tm_year, 1+timeinfo->tm_mon,timeinfo->tm_mday); 
        strcpy(save_succ, file_name);
        strcat(file_name, ".log");
        strcat(save_succ, ".succ");

	FILE* f;
	f=fopen(file_name, "w");
        save_nodes_to_file_1(get_root()->rb_node, f);
        fclose(f);

        //将数据dump完成后，写一个文件标示成功
	FILE* f_succ;
	f_succ=fopen(save_succ, "w");
        fclose(f_succ);
}

char* set_result(u_int code,char* msg,u_int data)
{
	char* result = (char*)malloc(100);
	memset(result,100,0); 
	sprintf(result,"{\"code\":%d,\"msg\":\"%s\",\"data\":%d}",code,msg,data);
	return result;
}

/***********************************************************************/

// 插入节点
char* insert_node_s(void *arg)
{
	pthread_rwlock_wrlock(&work_lock);
        char* r=insert_node(arg);
	pthread_rwlock_unlock(&work_lock);
	return r;
}

// 删除节点
char* delete_node_s(void *arg)
{
	pthread_rwlock_wrlock(&work_lock);
	char* r=delete_node(arg);	
	pthread_rwlock_unlock(&work_lock);
	return r;
}

// 更新节点
char* update_node_s(void *arg)
{
	pthread_rwlock_wrlock(&work_lock);
	char* r=update_node(arg);
	pthread_rwlock_unlock(&work_lock);
	return r;
}
// 查询节点排名
char* query_node_s(void *arg)
{
	pthread_rwlock_rdlock(&work_lock);
	char* r=query_node(arg);
	pthread_rwlock_unlock(&work_lock);
	return r;
}

