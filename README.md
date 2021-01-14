# ashmon

[![Build Status](https://travis-ci.org/ashtum/ashmon.svg?branch=master)](https://travis-ci.org/ashtum/ashmon)

**ashmon** is a light-weight graphical bandwidth monitor for linux, it uses libX11 for GUI and doesn't depend on any extra library.

![screenshot](https://user-images.githubusercontent.com/11743154/104591753-60850680-5682-11eb-9820-144f223d0126.png)

## Build

```shell
sudo apt-get install libx11-dev
git clone https://github.com/ashtum/ashmon.git
cd ashmon
mkdir build
cd build
cmake ..
sudo make install
```

If you want it start automatically at system boot time:

```shell
cd ashmon
./autostart.sh
```

## Gui operations

> **mouseover** shows selected network interface, change by click on `<` and `>` buttons.  
> **click and drag** moves window.  
> **mouse whele up/down** changes window opacity.  
> **right click** exits application.  
