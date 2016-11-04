CPMemu
======

> Raspberry Pi is a trademark of the Raspberry Pi Foundation.

> Linux is a trademark of Linus Torvalds.

> If you read this file in an editor you should switch line wrapping on.

Overview
--------

CPMemu is a CP/M 2.2 emulator running on the Raspberry Pi and on other Linux systems. On the Raspberry Pi CPMemu can be used as a bare metal solution based on the Circle environment.

Currently CPMemu does not implement a specific set of terminal control sequences. Instead it sends all characters unchanged to the console.

Installation
------------

See the files *INSTALL* (bare metal) or *INSTALL.linux* (Linux including Raspbian).

Tools
-----

*cpmdisk* is a tool program running on Linux which manipulates CP/M disk image files. It reads, writes and deletes files from/to CP/M disk images, lists the contents of a disk image or creates a new disk image file. The possible command line arguments will be displayed if *cpmdisk* is called without an argument.

*hex2bin* converts .hex files, created by the MAC macro assembler, into binary files.

External Sources
----------------

CPMemu is making use of the following external sources:

* [z80emu Z80 CPU emulator](https://github.com/anotherlin/z80emu/) by Lin Ke-Fong
* [CP/M 2.2 original source](http://www.cpm.z80.de/download/cpm2-plm.zip) on ["The Unofficial CP/M Web site"](http://www.cpm.z80.de/)
* [MAC macro assembler](http://www.cpm.z80.de/download/mac-b.zip) on ["The Unofficial CP/M Web site"](http://www.cpm.z80.de/)

CP/M programs are not included. You can get them from CP/M archives on the Internet.
