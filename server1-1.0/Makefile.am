AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=server1
server1_SOURCES=server.cc array.h array.cc config_file.h config_file.cc rbtree.h rbtree.cc thread_pool.h thread_pool.cc workers.h workers.cc hash_map.h hash_map.cc tool.h tool.cc rbsignal.h rbsignal.cc 


#以下配置为学习研究使用，项目不需要

#静态库文件配置
lib_LIBRARIES=librbsignal.a
librbsignal_a_SOURCES=rbsignal.cc
rbsignalincludedir=$(includedir)/rbsignal
rbsignalinclude_HEADERS=$(top_srcdir)/rbsignal.h

#打包时需要同时打包的文件
EXTRA_DIST=$(top_srcdir)/test.conf

#配置文件配置
server1dir=$(prefix)/conf
server1_DATA=test.conf
