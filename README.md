# Simple Pong on Pico

This is a vary simplistic pong for the raspberrry pi pico, with the waveshare [Pico LCD 1.14](https://www.waveshare.com/wiki/Pico-LCD-1.14) kit.

## Setup

The setup requires 3 main steps:
- c cross compiler that supports targets for rp2040
- [pico sdk](https://github.com/raspberrypi/pico-sdk) for std implementation.
- Waveshare's Pico LCD sdk.
```sh
# installing the relevant compiler (on fedora for me)
# install this package, and every other that may be connected to it.
sudo dnf install arm-none-eabi

# in a seperate directory
# cloning pico sdk
git clone https://github.com/raspberrypi/pico-sdk.git
export PICO_SDK_PATH="$(pwd)/pico-sdk"

# move to the original directory
# make sure you have 7zip (7z), then download
mkdir pico_lcd && cd pico_lcd
wget https://files.waveshare.com/upload/0/06/Pico-LCD-1.14.zip
7z e Pico-LCD-1.14.zip
cd c
# copy the pong files
mkdir pong
cp ../../src/** pong/
# overwrite files for changing
# (if you know cmake and make, you can edit the files yourself)
cp ../../patches/* .
```

## Compilation

This part is quite straight forward:
```sh
# inside the 'c' directory:
cmake .
make
```

After the compilation, a file named 'main.uf2' will be created.  
This file should be placed as firmware for the Pico.  

## How to Play

In the default configuration the right player (p1) is controlled by a human with 'A' and 'B', while the the left player (p2) is controlled by the (very stupid) AI (random...).  
To change the control of the players you can define `P1_COMP` (p1 is AI) and/or `P2_PLAYER` (p2 is using the joysick).  
The game lasts until a player reaches 9 points, and then ends.  
To restart the game, restart the Pico.  
