#include "hash_map.h"
static hash_map hm;

void set_element(KEY key, VALUE value)
{
	hm[key]=value; 
}
VALUE get_element(KEY key)
{
	hash_map::const_iterator got = hm.find (key);
	if ( got == hm.end() )
		return 0;
	else
		return got->second;
}

int del_element(KEY key)
{
	return 	hm.erase(key);
}
