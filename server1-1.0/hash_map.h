#ifndef _OBJECT_HASH_MAP_H
#define _OBJECT_HASH_MAP_H
#include <tr1/unordered_map>
// document: http://www.cplusplus.com/reference/unordered_map/unordered_map/erase/
using namespace std;

typedef unsigned long KEY;
typedef unsigned int VALUE;

typedef std::tr1::unordered_map<KEY, VALUE> hash_map;

 void set_element(KEY, VALUE);
 int del_element(KEY);
 VALUE get_element(KEY);

#endif
