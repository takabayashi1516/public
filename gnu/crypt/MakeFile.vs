PROGRAM=libsrmcrypt.dll

OPENSSL_HOME=C:\work\dl-tools\OpenSSL\OpenSSL-Win64

CPPFLAGS=/I. /I"$(OPENSSL_HOME)\include"

CFLAGS=/nologo /O2 /EHsc /MD /DSRMCRYPT_EXPORTS
LDFLAGS=/DLL /NOLOGO

LIBPATH=/LIBPATH:"$(OPENSSL_HOME)\lib\VC\x64\MD"

LIBS=libssl.lib libcrypto.lib

OBJS=crypt.obj

IMPLIB=libsrmcrypt.lib

all: $(PROGRAM)

crypt.obj: crypt.cpp crypt.h
	cl $(CPPFLAGS) $(CFLAGS) /c crypt.cpp

$(PROGRAM): $(OBJS)
	link $(LDFLAGS) /OUT:$(PROGRAM) /IMPLIB:$(IMPLIB) $(LIBPATH) $(OBJS) $(LIBS)

clean:
	del /Q *.obj *.exp *.lib *.dll 2>NUL
