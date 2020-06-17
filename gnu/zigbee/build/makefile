PROGRAM = ../libzigbee_util.so

OBJS  = libzigbee_util.o

CXX = g++

VPATH = ../source: ../include

CFLAGS  = -Wall
CFLAGS += -shared
CFLAGS += -O3
CFLAGS += -fPIC
CFLAGS += -I../include/
CFLAGS += -I../../include/
CFLAGS += -L../../lib/

LIBS    = -ltls

HEADERS  = libzigbee_util.h

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
