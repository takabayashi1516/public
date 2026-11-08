PROGRAM=..\libsrmcrypt.dll

OPENSSL_HOME=\work\dl-tools\OpenSSL\OpenSSL-Win64

CPPFLAGS=/I..\include /I"$(OPENSSL_HOME)\include"

CFLAGS=/nologo /O2 /EHsc /MD /DSRMCRYPT_EXPORTS

LDFLAGS=/DLL /NOLOGO

LIBPATH=/LIBPATH:"$(OPENSSL_HOME)\lib\VC\x64\MD"

LIBS=libssl.lib libcrypto.lib

SRCS=..\source\crypt.cpp \
     ..\source\DerivedCtrCipher.cpp

OBJS=$(SRCS:.cpp=.obj)

IMPLIB=..\libsrmcrypt.lib

all: $(PROGRAM)

.cpp.obj:
	cl $(CPPFLAGS) $(CFLAGS) /c $< /Fo$@

$(PROGRAM): $(OBJS)
	link $(LDFLAGS) /OUT:$(PROGRAM) /IMPLIB:$(IMPLIB) \
		$(LIBPATH) $(OBJS) $(LIBS)

clean:
	del /Q ..\source\*.obj ..\*.dll ..\*.exp 2>NUL
	del /Q ..\*.lib 2>NUL
