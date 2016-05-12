PWD=$(shell echo `pwd`)

CXX := g++
CXXFLAGS := -g -Wall -O2  -D_FILE_OFFSET_BITS=64

LDFLAGS :=-L/usr/local/lib \
		-L/usr/lib/

INCLUDES :=-I./ \
			-I$(PWD)/../include/ \
			-I$(PWD)/../thirdparty_not_used_if_not_recompile/include/ \
			-I$(PWD)/../thirdparty_not_used_if_not_recompile/include/zookeeper/

LIBS :=-lpthread \
	   -lssl \
	   -lrt \
	   $(PWD)/../libs/libtask_sche.a


SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp, %.o, $(SRC))


############################################
all:task_scheduler
	$(MAKE) $^

task_scheduler:$(OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LIBS) $(RPATH)



%.o:%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(INCLUDES)


.PHONY: clean
clean:
	rm -rf *.o
