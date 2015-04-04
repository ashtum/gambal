# ashmon

[![Build Status](https://travis-ci.org/ashtum/ashmon.svg?branch=master)](https://travis-ci.org/ashtum/ashmon)

A low size , free and open source graphical application for network usage monitoring in linux that directly use xlib for draw graphs so work fine on any linux distributions and don't need extra gui library .

![ashmon_shot](https://cloud.githubusercontent.com/assets/11743154/6987887/5575b242-da61-11e4-9076-0d1dc9ca5f70.png)


## Download and Usage
Download [Latest Release](https://github.com/ashtum/ashmon/releases/latest) extract files , go to path and install ashmon by : `sudo sh install.sh`<br/>run ashmon by : `ashmon "network-interface"` replace "network-interface" with that network interface you want monitored , like : `ashmon eth0` or `ashmon wlan0` or `ashmon ppp0`

**gui operations :**
> **click** fade out window for 5 seconds .

> **click and drag** move window everywhere you want .

> **mouse whele up/down** change window opacity .

> **right click** exit application .


## How to compile
install xlib development headers : ``` sudo apt-get install libx11-dev ``` or ``` sudo yum install libX11-devel ```<br/>go to project folder and run make : ``` sudo make ```

## Changelogs
####v1.1 :
>Edited ashmon_config path to ~/home .

>Added install.sh script for easy setup.

## Copyright

> *Copyright (c) 2015 Mohammad Nejati released under the GPL v2.0*
