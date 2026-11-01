make
make -f testmakefile
export LD_LIBRARY_PATH=.
./test

make clean
make -f testmakefile clean
