#!/bin/bash

rmmod passwd_gen_module.ko
make
insmod passwd_gen_module.ko
cat /dev/Password_Generator  
dmesg
make clean
