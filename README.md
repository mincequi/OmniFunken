OmniFunken
==========
OmniFunken aims to be a general purpose media render daemon.


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


## Get started
```
qmake
make
./omnifunken -n "MyPropagatedAirplayName"
```


Command line syntax
-------------------
For command line options run `omnifunken -h`.



