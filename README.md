# ashmon

[![Build Status](https://travis-ci.org/ashtum/ashmon.svg?branch=master)](https://travis-ci.org/ashtum/ashmon)

A light and transparent graphical network traffic monitor for linux.  
ashmon directly uses xlib for its GUI that means it can works fine on almost any linux distribution and doesn't need any extra library.

![screenshot](https://user-images.githubusercontent.com/11743154/92367349-89612200-f10b-11ea-92eb-0f4577185ce2.png)

## Build

To build ashmon you need install xlib development package with : ``` sudo apt-get install libx11-dev ``` or ``` sudo yum install libX11-devel ``` 
Then go to project folder and build project with ``` make ``` command.

## Usage

Install ashmon in /usr/bin folder : `sudo sh install.sh`  
Run ashmon : `ashmon network-interface-name` replace "network-interface-name" with your network interface that you want to monitor, like : `ashmon eth0` or `ashmon wlan0`  
If you want ashmon start automatically on system startup : `sudo sh autostart.sh`

## Gui operations

> **click** fades out window for 5 seconds.  
> **click and drag** moves window everywhere you want.  
> **mouse whele up/down** changes window opacity.  
> **right click** exits application.  
