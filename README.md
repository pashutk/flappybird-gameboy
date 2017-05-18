# Flappybird-gameboy
Flappybird-like game for Nintendo Gameboy (Gameboy Original or Gameboy Color).

## Demo images
<img src="https://github.com/pashutk/flappybird-gameboy/raw/master/launchscreen.png" width="280" />
<img src="https://github.com/pashutk/flappybird-gameboy/raw/master/game.png" width="280" />
<img src="https://github.com/pashutk/flappybird-gameboy/raw/master/launchscreen.gif" width="280" />
<img src="https://github.com/pashutk/flappybird-gameboy/raw/master/game.gif" width="280" />

## Build ROM
To build Gameboy .gb ROM file use [GBDK library](http://gbdk.sourceforge.net/)
Clone project to `gbdk` folder and then compile code to ROM with command:
`PATH_TO_GBDK/bin/lcc -o game.gb game.c`

## Play online
You can use [this online emulator](https://github.com/taisel/GameBoy-Online) to launch rom in browser
