PROGRAM=..\test.exe

OPENSSL_HOME=C:\work\dl-tools\OpenSSL\OpenSSL-Win64

CPPFLAGS=/I"..\include" /I"$(OPENSSL_HOME)\include"

CFLAGS=/nologo /O2 /EHsc /MD

LDFLAGS=/NOLOGO

LIBPATH=/LIBPATH:"$(OPENSSL_HOME)\lib\VC\x64\MD"

LIBS=..\libsrmcrypt.lib

OBJS=test.obj

all: $(PROGRAM)

test.obj: test.cpp
	cl $(CPPFLAGS) $(CFLAGS) /c test.cpp

$(PROGRAM): $(OBJS)
	link $(LDFLAGS) /OUT:$(PROGRAM) $(LIBPATH) $(OBJS) $(LIBS)

clean:
	del /Q *.obj ..\*.exp ..\*.exe 2>NUL

.PHONY: clean
