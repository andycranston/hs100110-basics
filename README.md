# TP-Link HS100/HS110 SmartPlug basic on/off scripts/programs

This repository provides three scripts/programs which can turn a TP-Link
HS100 or HS110 SmartPlug on or off.  They can also send any JSON formatted
string to the plug.

The scripts/programs are written in the following languages:

* C
* Python
* Lua

The Lua script requires the Lua sockets package to be installed.

The idea is to provide something that will run in a wider range of
environments that just a single script.

## Credits and Links

Lubomir Stroetmann and Tobias Esser from softScheck wrote this brilliant
article on the TP-Link HS110 plug here:

> [Reverse Engineering the TP-Link HS110]<https://www.softscheck.com/en/reverse-engineering-tp-link-hs110/>

They have their own Python script in this GitHub repository:

> <https://github.com/softScheck/tplink-smartplug>

Specifically the `tplink-smartplug.py` script:

> <https://github.com/softScheck/tplink-smartplug/blob/master/tplink-smartplug.py>

This link has really useful stuff on the wide range of JSON strings that can
be sent to the plug:

> <https://github.com/softScheck/tplink-smartplug/blob/master/tplink-smarthome-commands.txt>

You might prefer their script to any of mine :-]  But for the sake of software
diversity feel free to read on.

## Installing the C program

Copy the program `hs100110.c` file to a directory in your PATH and
compile with:

```
gcc -o hs100110 hs100110.c
```

The program can now be run as follows:

```
hs100110 -h 192.168.1.65 -on
hs100110 -h 192.168.1.65 -off
hs100110 -h 192.168.1.65 -query
```

to turn the plug on, off or display its current status respectively. Replace 192.168.1.65
with the IP address of one of your own SmartPlugs.

## Installing the Python script

Note the Python script is written using Python 3.

Copy the script `hs100110.py` to a handy directory and change to that directory.

The program can now be run as follows:

```
python hs100110.py -h 192.168.1.65 -on
python hs100110.py -h 192.168.1.65 -off
python hs100110.py -h 192.168.1.65 -query
```

to turn the plug on, off or display its current status respectively. Replace 192.168.1.65
with the IP address of one of your own SmartPlugs.

Depending on your Python environment you may have to type:

```
python3 hs100110.py -h 192.168.1.65 -on
python3 hs100110.py -h 192.168.1.65 -off
python3 hs100110.py -h 192.168.1.65 -query
```

instead.

## Installing the Lua script

Note the Lua script is written using Lua version 5.

You need the LuaSocket package installed as a prerequisite. See:

> [LuaSocket by Diego Nehab]<https://github.com/diegonehab/luasocket>

Copy the script `hs100110.lua` to a directory in your PATH, rename to just
`hs100110` and make it executable by all.  For example:

```
cp hs100110.lua $HOME/bin
cd $HOME/bin
mv hs100110.lua hs100110
chmod a+x hs100110
```

The program can now be run as follows:

```
hs100110 -h 192.168.1.65 -on
hs100110 -h 192.168.1.65 -off
hs100110 -h 192.168.1.65 -query
```

to turn the plug on, off or display its current status
respectively. Replace 192.168.1.65 with the IP address of one of your
own SmartPlugs.

## Command line arguments

The following command line arguments are recognised:

```
-h IPADDRESS
-on
-off
-query
-j "JSON string"
```

The -h option is followed by the IP address (or hostname) of the SmartPLug you want to control.

The -on option will turn the plug on.

The -off option will turn the plug off.

The -query option displays status information of a plug.  Typical output might look like:

```
{"system":{"get_sysinfo":{"err_code":0,"sw_ver":"1.2.5 Build 171213 Rel
.095335","hw_ver":"1.0","type":"IOT.SMARTPLUGSWITCH","model":"HS110(UK)
","mac":"50:C7:BF:5B:0D:56","deviceId":"80061F21F93C97CCF2136449B397E93
71878595B","hwId":"2448AB56FB7E126DE5CF876F84C6DEB5","fwId":"0000000000
0000000000000000000000","oemId":"90AEEA7AECBF1A879FCA3C104C58C4D8","ali
as":"IP: 86.133.159.133","dev_name":"Wi-Fi Smart Plug With Energy Monit
oring","icon_hash":"","relay_state":0,"on_time":0,"active_mode":"schedu
le","feature":"TIM:ENE","updating":0,"rssi":-59,"led_off":0,"latitude":
55.041531,"longitude":-6.951144}}}
```

The -j option is followed by a JSON string to the sent to the plug.  For example:

```
hs100110 -h 192.168.1.65 -j '{"system":{"set_dev_alias":{"alias":"Living Room Lamp"}}}'
```

will set the name of the SmartPlug to "Living Room Lamp".

## What next?

Once you have used one or more of these scripts to turn a plug on, off and query
it's status you might want to look at my other repository:

* hs100110-nextsteps

________________________________________________________________________
