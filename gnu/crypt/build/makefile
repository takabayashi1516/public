PROGRAM = ../libsrmcrypt.so

SRCS = $(wildcard ../source/*.cpp)
#OBJS = $(patsubst ../source/%.cpp,%.o,$(SRCS))
OBJS = $(SRCS:.cpp=.o)
#DEPS = $(OBJS:.o=.d)
DEPS = $(OBJS:.o=.d)

CXX = g++

CFLAGS  = -Wall
CFLAGS += -Wno-deprecated-declarations
CFLAGS += -shared
CFLAGS += -O3
CFLAGS += -fPIC
CFLAGS += -fpermissive
CFLAGS += -I../include
CFLAGS += -MMD -MP

LIBS  = -lssl
LIBS += -lcrypto

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CXX) -o $@ $^ $(LIBS) $(CFLAGS)

%.o: ../source/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	$(RM) $(PROGRAM) $(OBJS) $(DEPS)

.PHONY: all clean
