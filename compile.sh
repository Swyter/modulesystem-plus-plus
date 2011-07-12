CFLAGS=$(python3-config --includes)
LDFLAGS=$(python3-config --ldflags)
g++ -std=c++0x -O2 -Wall MS++/cMS.cpp MS++/StringUtils.cpp MS++/ModuleSystem.cpp MS++/CPyObject.cpp -o MS++-linux $CFLAGS $LDFLAGS 
chmod 755 MS++-linux