# FPGA

## Binary Counter

Binary counter is a simple application created on Zed Board using Zynq-7000 with two different approaches: 
* Using Vivado + Vitis to program PS and PL parts using C language
* Using Vivado to program PL part using VHDL language.

Binary Counter's app target is to count the amount of times button being pressed. With every press the binary counter increases its value and displays its result via LEDs. One diode will constantly blink with given frequency - starting from 50 Hz and lowering it by half with every button press down to 1.56 Hz (50, 25, 12.5 ... 1.56). After that, the frequency returns back to 50 Hz. Application should reset after pressing the RESET button.  

The aim of this application is to experiment (mostly in vitis) with different peripherals. That part will be explained in C_Vitis_approach folder.