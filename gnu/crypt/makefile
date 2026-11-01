PROGRAM = ./libsrmcrypt.so

OBJS  = crypt.o

CXX = g++

VPATH = ./

CFLAGS  = -Wall
CFLAGS += -Wno-deprecated-declarations
CFLAGS += -shared
CFLAGS += -O3
CFLAGS += -fPIC
CFLAGS += -fpermissive
CFLAGS += -I./

LIBS    = -lssl
LIBS   += -lcrypto

HEADER  = crypt.h

.SUFFIXES: .cpp .o

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) -o $(PROGRAM) $^ $(LIBS) $(CFLAGS)

.cpp.o:
	$(CXX) $(CFLAGS) -c $<

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)

$(OBJS): $(HEADER)
