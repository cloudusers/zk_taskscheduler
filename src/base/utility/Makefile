PWD=$(shell echo `pwd`)

CXX := g++
CXXFLAGS := -g -Wall -O2 -static -D_FILE_OFFSET_BITS=64

LDFLAGS :=-fPIC -shared

INCLUDES :=-I./ \
			-I$(PWD)/../ \
			-I$(PWD)/../../ \
			-I$(PWD)/../../../thirdparty_not_used_if_not_recompile/include/ \
			-I$(PWD)/../../../thirdparty_not_used_if_not_recompile/include/zookeeper/

LIBS :=-lpthread \
	   -lssl \
	   -lrt


SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp, %.o, $(SRC))


############################################
all: default libtask_utility.a

default: 
	mkdir -p lib_boost lib_log lib_zk
	(cd lib_boost; ar x $(PWD)/../../../thirdparty_not_used_if_not_recompile/libs/libboost_thread.a)
	(cd lib_boost; ar x $(PWD)/../../../thirdparty_not_used_if_not_recompile/libs/libboost_system.a)
	(cd lib_boost; ar x $(PWD)/../../../thirdparty_not_used_if_not_recompile/libs/libboost_date_time.a)
	(cd lib_log; ar x $(PWD)/../../../thirdparty_not_used_if_not_recompile/libs/liblog4cplus.a)
	(cd lib_zk; ar x $(PWD)/../../../thirdparty_not_used_if_not_recompile/libs/libzookeeper_mt.a)
	(cd lib_zk; ar x $(PWD)/../../../thirdparty_not_used_if_not_recompile/libs/libzoolock.a)


libtask_utility.a: $(OBJ) $(LIBS)
	ar rc $@ $^ lib_boost/*.o lib_log/*.o lib_zk/*.o
	#mv $@ ../../../libs
	rm -rf lib_*



%.o:%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(INCLUDES)


.PHONY: clean
clean:
	rm -rf *.o
