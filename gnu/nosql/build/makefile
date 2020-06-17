PROGRAM = ../libmongo_util.so

OBJS  = libmongo_util.o

CXX = g++

VPATH = ../source: ../include

CFLAGS  = -Wall
CFLAGS += -shared
CFLAGS += -O3
CFLAGS += -fPIC
CFLAGS += -I/usr/local/include/libmongoc-1.0/
CFLAGS += -I/usr/local/include/libbson-1.0/
CFLAGS += -I../include/

LIBS    = -lmongoc-1.0
LIBS   += -lbson-1.0

HEADERS  = libmongo_util.h

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
