PROGRAM = ../test.elf

CXX = g++

OBJS = \
	test.o

CPPFLAGS = -I../include

CXXFLAGS = -Wall
CXXFLAGS += -Wno-deprecated-declarations
CXXFLAGS += -O3
CXXFLAGS += -fpermissive

LIBS  = -L..
LIBS += -lsrmcrypt
LIBS += -lssl
LIBS += -lcrypto

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(PROGRAM) $(OBJS)

.PHONY: all clean
