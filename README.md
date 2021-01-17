# ashmon

[![Build Status](https://travis-ci.org/ashtum/ashmon.svg?branch=master)](https://travis-ci.org/ashtum/ashmon)

**ashmon** is a tiny transparent graphical monitor for linux, it shows Network, CPU and memory usages.  
It's light-weight and uses libX11 for GUI and doesn't depend on any extra library.

![screenshot](https://user-images.githubusercontent.com/11743154/105398916-28f4fc00-5c38-11eb-9ea9-14e3b5402b01.png)

## Build

```shell
sudo apt-get install build-essential cmake libx11-dev
git clone https://github.com/ashtum/ashmon.git
cd ashmon
mkdir build
cd build
cmake ..
sudo make install
sudo gtk-update-icon-cache /usr/share/icons/hicolor
```

## Gui operations

> **mouseover** Appears options, change network interface or style by click on `<` and `>` buttons.  
> **click and drag** moves window.  
> **mouse whele up/down** changes window opacity.  
> **right click** exits application.  
