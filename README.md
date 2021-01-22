# gambal

[![Build Status](https://travis-ci.com/ashtum/gambal.svg?branch=master)](https://travis-ci.com/ashtum/gambal)

**gambal** is a tiny transparent graphical monitor, it provides an overview of network, CPU and memory usages.
It's light-weight and uses libX11 for GUI and should work on (almost) all distros.

![screenshot](https://user-images.githubusercontent.com/11743154/105464742-42815c80-5ca7-11eb-881e-067637981c79.png)

## Build

```shell
sudo apt-get install build-essential cmake libx11-dev
git clone https://github.com/ashtum/gambal.git
cd gambal
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
