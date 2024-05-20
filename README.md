SparkFun Simultaneous RFID Tag Reader Library
===========================================================

Changes
-------
Added filter support for both reading and writing. 
Added configurations for more specific reading adn writing operations.
Noteable benefits:
* Decrease overheating by setting the off-time in continuous mode
* Write to a specific tag using a filter
* Single multi-tag read, with or withour filter
* Can read all tag metadata, including embedded data
* Detailed documentation of the inner workings of the reader
I have not tested locking, using the passwords, making tags untraceable, and some other operations.
Use with caution!


![SparkFun Simultaneous RFID Tag Reader - NANO M6E](https://cdn.sparkfun.com//assets/parts/1/1/9/1/6/14066-06a.jpg)

[*SparkFun Simultaneous RFID Tag Reader - NANO M6E (SEN-14066)*](https://www.sparkfun.com/products/14066)

This is a powerful shield capable of reading multiple RFID tags at the same time. It can also write to tags. Works with UHF Gen2 RFID tags.

Thanks to:

* paulvha on forums for finding and recommending [fix for writeEPC bug](https://forum.sparkfun.com/viewtopic.php?f=118&t=49574&start=15)

Note - Most examples make use of [SoftwareSerial](https://docs.arduino.cc/learn/built-in-libraries/software-serial), which is not included in all platform cores. If you run into compilation errors due to this, you'll need to install an appropriate library for your platform (eg. [ESPSoftwareSerial](https://github.com/plerup/espsoftwareserial/) for ESP32), or replace `softSerial` in the examples with something that works for your platform (eg. `Serial2` for ESP32 or `Serial5` for teensy, see Advanced Example1).

Repository Contents
-------------------

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
* **keywords.txt** - Keywords from this library that will be highlighted in the Arduino IDE. 
* **library.properties** - General library properties for the Arduino package manager. 

Documentation
--------------

* **[Hookup Guide](https://learn.sparkfun.com/tutorials/simultaneous-rfid-tag-reader-hookup-guide)** - Hookup guide for the RFID shield.
* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - Basic information on how to install an Arduino library.
* **[Product Repository](https://github.com/sparkfun/Simultaneous_RFID_Tag_Reader)** - Main repository (including hardware files) for the RFID Shield.

License Information
-------------------

This product is _**open source**_! 

The **code** is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release anything derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
