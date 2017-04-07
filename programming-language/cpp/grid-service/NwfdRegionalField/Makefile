###################################################################### 
#  @ Makefile for NwfdRegionalField
#  @Date      : 2015-07-09
#  @Author    : zhangl
#  @Dependent : null
#  @Target    : ./bin/libNwfdRegionalField.so
######################################################################
DIR_BIN  = ./bin
NWFD_RELEASE = /samba/nwfd-release

# source
LIB_SRC = DataProcesser.cpp DataProInterface.cpp NwfdRegionalField.cpp

#target
TARGET     = libNwfdRegionalField.so
BIN_TARGET = ${DIR_BIN}/${TARGET}

# For Test
TEST_SRC   = test.cpp
TEST_EXE   = ${DIR_BIN}/test

CC     = gcc
CXX    = g++
CFLAGS = -g -Wall -fPIC
DEFS   = -D_DEBUG

all: NwfdRegionalField

NwfdRegionalField:${LIB_SRC}
	$(CXX) -shared $(CFLAGS)  $^ -o ${BIN_TARGET}

test:${TEST_SRC}
	$(CXX) $(TEST_SRC) -ldl -o $(TEST_EXE)

.PHONY:clean copy
clean:
	@rm -vf $(BIN_TARGET) $(TEST_EXE)
copy:
	@cp -vf $(BIN_TARGET) $(NWFD_RELEASE)/.
