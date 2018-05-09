#!/bin/bash

if [ "$(id -u)" != "0" ];
then
        echo "This script must be run as root" 1>&2
	exit -1
fi

if [ $# -lt 1 ];
then
	echo "specify an  ip address "
        echo "use script like: $0 [ip of remote machine]"
	exit -1
fi

while [ 1 ]
do
	echo -n "cmd>"
	read cmd_line
	hping3 --icmp $1 -e "$cmd_line" -c 1
done

