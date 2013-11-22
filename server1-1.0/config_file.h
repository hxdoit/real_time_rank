#ifndef _DAREN_CONFIG_FILE_H 
#define _DAREN_CONFIG_FILE_H 

#include <stdint.h>
#include <stdio.h>
#include <syslog.h>

#define    MAX_PATH_LEN         (512)
#define    MAX_FILE_NAME_LEN    (128)

 

/* 将配置文件中的配置项解析后存放到全局数组中 */

int parse_config_file(char *path_to_config_file);  

 

/* 从全局数组中读取一个个配置项。例如  get_config_var("var_name1");  */
char * get_config_var(char *var_name);  
#endif
