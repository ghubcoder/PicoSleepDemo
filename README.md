

Raspberry Pico - low power sleep demo
==========

This is a simple demo to show how you can place the pico into a low power sleep mode, which according to the docs will pull around 1.3mA. This should be useful to allow minimal power consumption and periodic waking to perform some work.

This is based off the code provided [here](https://github.com/raspberrypi/pico-playground/blob/master/sleep/hello_sleep/hello_sleep.c)

Layout as follows:

```
~/pico/pico-sdk/
~/pico/pico-extras/
~/pico/sleepdemo/
```

Create a new build dir inside this repo:

```
mkdir ~/pico/sleepdemo/build
```

Change into the directory and run the following:

```
export PICO_SDK_PATH=../../pico-sdk
export PICO_EXTRAS_PATH=../../pico-extras
cmake ..
make
```

This will create the `main.elf` file below. Alternatively drag the `main.uf2` file onto the pico via usb. 

To deploy this using SWD and monitor using minicom, run the following once you have wired everything up as per the pico docs. 

Start following in a terminal:
```
openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg
```

Also start minicom to listen for output in a new terminal:
```
minicom -b 115200 -o -D /dev/serial0
```

Enter gdb and load file:
```
gdb-multiarch main.elf
target remote localhost:3333
load
monitor reset init
continue
```

In the minicom terminal window you should see debug output and the led should blink on and off each minute. 

