
$ LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:.; export LD_LIBRARY_PATH


--- mongo ---
https://www.trifields.jp/how-to-install-mongodb-on-ubuntu-2751

$ sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv 0C49F3730359A14518585931BC711F9BA15703C6

$ echo "deb [ arch=amd64,arm64 ] http://repo.mongodb.org/apt/ubuntu xenial/mongodb-org/3.4 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-3.4.list


$ sudo apt-get update
$ sudo apt-get install mongodb-org


$ sudo service mongod start
$ sudo service mongod stop
$ sudo service mongod restart


--- mongo c driver ---
http://mongoc.org/libmongoc/current/installing.html

  - Install libmongoc with a Package Manager
      $ sudo apt-get install libmongoc-1.0-0
  - Install libbson with a Package Manager
      $ sudo apt-get install libbson-1.0

  - Building on Unix
      $ sudo apt-get install cmake libssl-dev libsasl2-dev

  - Building from git
      $ git clone https://github.com/mongodb/mongo-c-driver.git
      $ cd mongo-c-driver
      $ git checkout 1.12.0
      $ mkdir cmake-build
      $ cd cmake-build
      $ cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ..
      $ make
      $ sudo make install


