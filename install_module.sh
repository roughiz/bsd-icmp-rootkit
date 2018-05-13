#!/bin/sh
# Simple script to create the kernel module with the Makefile

echo "Creating the kernet module : roughiz and creating the app __icmpshell"
gcc __icmpshell.c -o __icmpshell.o
cd module && make
