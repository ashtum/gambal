# Gambal

[![Build Status](https://travis-ci.com/ashtum/gambal.svg?branch=master)](https://travis-ci.com/ashtum/gambal)

**Gambal** is a tiny transparent window that provides an overview of network, CPU and memory usages.  
It's light-weight and uses libX11 for GUI and should work on all distros.

![demo](https://user-images.githubusercontent.com/11743154/127499775-e5237d27-da46-48da-a8ae-cf082ce0876c.gif)
> **mouse wheel up/down** changes transparency.  

## Installation

There are .deb and .rpm packages for different atchitectures in [latest release](https://github.com/ashtum/gambal/releases/latest) page.

## Building

If you are on Arch linux and xorg-fonts-misc package is not installed. install it and logout and login to your desktop.

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
