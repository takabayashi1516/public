PROGRAM = ../libmysql_util.so

OBJS  = libmysql_util.o

CXX = g++

VPATH = ../source: ../include

CFLAGS  = -Wall
CFLAGS += -shared
CFLAGS += -O3
CFLAGS += -fPIC
CFLAGS += -I../include/
CFLAGS += -L/usr/lib64/mysql/

LIBS    = -lmysqlclient

HEADERS  = libmysql_util.h

.SUFFIXES: .cpp .o

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) -o $(PROGRAM) $^ $(CFLAGS) $(LIBS)

.cpp.o:
	$(CXX) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)

$(OBJS): $(HEADERS)
