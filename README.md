OmniFunken
==========
OmniFunken aims to be a general purpose media render daemon. Currently it only supports Apple AirPlay.


Features
--------
* Apple AirPlay
* RS232 device control (planned)


Build Requirements
------------------
Required:
* Qt 5.2
* OpenSSL
* Avahi (w/ Apple Bonjour compatibility layer)
* libao
* ALSA

Optionally:
* SDL (planned)


Get started
-----------
```
qmake
make
./omnifunken -n "MyPropagatedAirplayName"
```


Command line syntax
-------------------
For command line options run `omnifunken -h`.


TODO
----
* RS232 device control
* ALSA audio output
* init scripts (write pidfile)
* audio device selection
* timing/synchronisation


Thanks to
---------
Thanks to James Laird for his [shairport](https://github.com/abrasive/shairport) project on which this project is heavily based.


