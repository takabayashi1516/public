
■ビルド
$ pushd build
$ make
$ popd
  ※opensslバージョンにに合わせmakefileを修正
    CFLAGS += -D__OPENSSL_CAPSULE #←コメントアウトするか否か

■テストビルド
$ pushd test
$ make -f makefile_server #←サーバ側
$ popd

■実行
$ pushd test
$ LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:../; export LD_LIBRARY_PATH
$ ./test-server ${port} ${my_certificate} ${my_private_key} ${ca_file}

■その他
  EPOLLIN = 0x001,
  EPOLLPRI = 0x002,
  EPOLLOUT = 0x004,
  EPOLLRDNORM = 0x040,
  EPOLLRDBAND = 0x080,
  EPOLLWRNORM = 0x100,
  EPOLLWRBAND = 0x200,
  EPOLLMSG = 0x400,
  EPOLLERR = 0x008,
  EPOLLHUP = 0x010,
  EPOLLRDHUP = 0x2000,
  EPOLLWAKEUP = 1u << 29,
  EPOLLONESHOT = 1u << 30,
  EPOLLET = 1u << 31

