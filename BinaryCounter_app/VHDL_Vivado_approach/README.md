# FPGA

## Binary Counter - VHDL approach

In this approach of binary counter an application is created and tested in Xilinx Vivado. After testing via behavioural simulation application is also tested on physical board (in my case Zed Board).

The rightmost diode constantly blinks wiht freq = 50 Hz (due to GIF optimalization etc it may not be that visible) and with every button press it cuts down its frequency by half. At the end of the gif everything is reset with the Reset button (middle one).

![ezgif com-resize](https://github.com/user-attachments/assets/b69e0e9a-8795-47b0-9949-77a7e79c72b6)


* main.vhd - contains VHDL source code used to develop main functionality of the device.
* test.vhd - test file which contains example code for behavioural testing of the device's main functionality (LED display counter, frequency of LED blinking, state machine, debouncing of button, edge detection, etc..)

Below is depictured simulation result:
![Test](https://github.com/user-attachments/assets/27910d86-55b3-4ac6-bdf1-f31b87503fd0)

And a close-up to debouncing stage:
![debouncing](https://github.com/user-attachments/assets/46973359-0ccd-422e-8f50-17955254394f)

