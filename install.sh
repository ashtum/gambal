#!/bin/bash
if ! [ $(id -u) = 0 ]; then
   echo "This script must be run as root."
   exit 1
fi
/bin/cp ashmon /usr/bin
echo "ashmon installed successfuly."
