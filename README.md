PinoutResolver
==============

This tool provide an easy way to resolve pinout and peripherals usage in a micro-controller unit, according to a request file, and a device pinout description file.

This tools is written with Qt4 (for easy to use XML library).


Usage
-----

### Device Description file

This is an XML file where available peripherals are listed (Timers, ADC, ...), and all usage with theirs corresponding pinouts (ex: PWM with Timer 1 on pins PA1 and PA2)


### Request file

This is an XML file describing what is wanted (ex: 2 PWM, 3 ADC channels and a RS232 port).
