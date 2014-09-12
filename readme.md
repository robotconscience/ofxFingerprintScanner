ofxFingerprintScanner
=====================
* openFrameworks wrapper for Sparkfun's [Fingerprint Scanner](https://www.sparkfun.com/products/11792)
* Stands on the shoulders of giants:
    * [Sparkfun's Arduino library](https://github.com/sparkfun/Fingerprint_Scanner-TTL) 
    * some serial niceness referenced from [Patricio's Thermal Printer addon](https://github.com/patriciogonzalezvivo/ofxThermalPrinter)

In-Progress!
==========

Currently working
==========
* Turn the LED on and off (wow!)
* Read if a finger is on the device
* Get an ofImage of your fingerprint ($)
    * Recommend the fastest baud rate for that, otherwise you must be very, very patient
* Everything else is accessible from the underlying library!
    * (Except the high-res fingerprint capture, comin' soon)
* Code is pretty ugly at the moment, be warned!