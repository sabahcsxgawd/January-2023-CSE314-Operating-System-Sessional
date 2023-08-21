#!/usr/bin/bash
clear
rm ./a
g++ -pthread a.c -o a
./a