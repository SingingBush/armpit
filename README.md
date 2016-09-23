ARMPIT
======

ARM Processor Imitation Technology

[![Build Status](https://travis-ci.org/SingingBush/armpit.png)](https://travis-ci.org/SingingBush/armpit)

ARMPIT is a command line tool for simulating ARM processor instructions, started by Samael Bate (singingbush) in 2013.

The initial requirement of the application is to allocate memory from the host system to be used as virtual memory for this soft-cpu simulation. 
The virtual memory is then filled with binary values that represent instructions in ARM Assembly Language. So for example if the ARM program I 
want to simulate moves a value of 2 into Register 1 (done using 'MOV R1, #2'), then the binary representation of that instruction will be in 
the virtual memory as 11100011 10100000 00010000 00000010.

The ARM Assembly instructions that are to be supported are based on the ARMv2 processor, although this is a slimmed down version.

A datasheet is included with details about the instructions and how to convert them to and from their binary form.
