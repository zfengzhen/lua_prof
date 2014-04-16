#!/bin/sh
g++ -fPIC -shared -I/usr/local/include -L./usr/local/lib -llua -o lua_prof.so lua_prof.cpp
