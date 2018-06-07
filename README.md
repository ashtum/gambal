# ashmon
[![Build Status](https://travis-ci.org/ashtum/ashmon.svg?branch=master)](https://travis-ci.org/ashtum/ashmon)

A light , free and open source graphical network traffic monitor in linux that directly use xlib for draw graphs so work fine on any linux distributions and don't need extra gui library.

![ashmon_shot](https://cloud.githubusercontent.com/assets/11743154/6994226/55522172-db26-11e4-9648-7f3d6f932142.png)


## Build
install xlib development headers : ``` sudo apt-get install libx11-dev ``` or ``` sudo yum install libX11-devel ```<br/>go to project folder and run make : ``` make ```

## Usage
Install ashmon in /usr/bin folder : `sudo sh install.sh`<br/>Run ashmon : `ashmon "network-interface"` replace "network-interface" with that network interface you want monitored , like : `ashmon eth0` or `ashmon wlan0` or `ashmon ppp0`<br/>If you want ashmon start automatically on system startup just run autostart.sh script at the same path : `sudo sh autostart.sh`

## Gui operations:
> **click** fade out window for 5 seconds.

> **click and drag** move window everywhere you want.

> **mouse whele up/down** change window opacity.

> **right click** exit application.

## Changelogs
v1.8
>Fixing exposure freeze bug.

v1.7
>Spliting project in different files.
>Changing x11 configs and draw functions.
>Using a precise timer for 1 second interrupts.

v1.6
>Fix memory leak.

v1.5
>Fix dev_name bug in issue #1 (thanks to @nutpantz).

v1.4
>Save last time used network-interface name in config file.

v1.3
>Added autostart bash script.

v1.2 :
>Added Total Download/Upload bytes.

v1.1 :
>Edited ashmon_config path to ~/home.

>Added install.sh script for easy setup.

## Copyright
> *Copyright (c) 2015 Mohammad Nejati released under the GPL v2.0*
