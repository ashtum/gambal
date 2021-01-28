# Gambal

[![Build Status](https://travis-ci.com/ashtum/gambal.svg?branch=master)](https://travis-ci.com/ashtum/gambal)

**Gambal** is a tiny transparent window that provides an overview of network, CPU and memory usages.  
It's light-weight and uses libX11 for GUI and should work on all distros.

![screenshot](https://user-images.githubusercontent.com/11743154/106142151-5e44a100-6186-11eb-81fa-652f4a794d3e.gif)

## Installation

There are .deb and .rpm packages for different atchitectures in [latest release](https://github.com/ashtum/gambal/releases/latest) page.

## Building

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
