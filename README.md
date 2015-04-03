# ashmon

[![Build Status](https://travis-ci.org/ashtum/ashmon.svg?branch=master)](https://travis-ci.org/ashtum/ashmon)

A low size , free and open source graphical application for network usage monitoring in linux that directly use xlib for draw graphs so work fine on any linux distributions and don't need extra gui library .

![ashmon_shot](https://cloud.githubusercontent.com/assets/11743154/6987887/5575b242-da61-11e4-9076-0d1dc9ca5f70.png)


## Download and Usage
Download [Latest Release](https://github.com/ashtum/ashmon/releases/latest) extract file , go to path and execute ashmon by : ```./ashmon "network-interface"``` replace "network-interface" with your network interfaces like : ```./ashmon eth0``` or ```./ashmon wlan0``` or ```./ashmon ppp0```

ashmon create a new "ashmon_config" file that save lasted window position and opacity .

**gui operations :**
> **click** fade out window for 5 seconds .

> **click and drag** move window everywhere you want .

> **mouse whele up/down** change window opacity .

> **right click** exit application .


## How to compile
install xlib development headers : ``` sudo apt-get install libx11-dev ``` or ``` sudo yum install libX11-devel ```

go to project folder and run make : ``` sudo make ```


## Copyright

> *Copyright (c) 2015 Mohammad Nejati released under the GPL v2.0*
