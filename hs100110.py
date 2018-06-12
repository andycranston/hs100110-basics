#! /usr/bin/python3
#
#  @(!--#) @(#) hs100110.lua, version 003, 31-may-2018
#
#  TP-Link HS100 Smart WiFi Plug controller (Lua version)
#
#  Send encoded JSON requests via TCP/IP to turn a
#  TP-Link HS100 and HS110 Smart WiFi plug on and off
#
#  Product details:
#
#      http://uk.tp-link.com/products/list-5258.html
#
#  Based on encoded JSON strings from:
#
#      https://community.openhab.org/t/tp-link-hs100-smart-plug-wifi/8591/6
#
#  This page has comprehensive and interesting reverse engineering details:
#
#      https://www.softscheck.com/en/reverse-engineering-tp-link-hs110/
#

########################################################################

#
# imports
#

import sys
import os
import socket

########################################################################

JSON_ON    = '{"system":{"set_relay_state":{"state":1}}}'
JSON_OFF   = '{"system":{"set_relay_state":{"state":0}}}'
JSON_QUERY = '{"system":{"get_sysinfo":{}}}'

MAX_PACKET_LENGTH = 65000

########################################################################

def plain2cipher(barray):
    key = 171
    result = bytearray(len(barray) + 4)
    i = 4
    for b in barray:
        a = key ^ b
        key = a
        result[i] = a
        i += 1
    return result

########################################################################

def cipher2plain(barray):
    key = 171
    result = bytearray(len(barray))
    i = 0
    for b in barray:
        a = key ^ b
        key = b
        result[i] = a
        i += 1
    return result

########################################################################

def showpacket(bytes):
    bpr = 10              # bpr is Bytes Per Row
    numbytes = len(bytes)

    if numbytes == 0:
        print("<empty frame>")
    else:
        i = 0
        while i < numbytes:
            if (i % bpr) == 0:
                print("{:04d} :".format(i), sep='', end='')

            print(" {:02X}".format(bytes[i]), sep='', end='')

            if ((i + 1) % bpr) == 0:
                print()

            i = i + 1

    if (numbytes % bpr) != 0:
        print()

    return

########################################################################

#
# Main
#

progname = os.path.basename(sys.argv[0])

verbose = 0
hostip = "-"
portnum = 9999
jsonstring = JSON_QUERY

argc = len(sys.argv)
i = 1
while i < argc:
    arg = sys.argv[i]
    i += 1

    if arg == '-h':
        if i == argc:
            print("{}: expecting host IP after -h option".format(progname), file=sys.stderr)
            sys.exit(1)

        hostip = sys.argv[i]
        i += 1
    elif arg == '-p':
        if i == argc:
            print("{}: expecting port number after -p option".format(progname), file=sys.stderr)
            sys.exit(1)

        portnum = sys.argv[i]
        i += 1
    elif arg == '-j':
        if i == argc:
            print("{}: expecting JSON string after -j option".format(progname), file=sys.stderr)
            sys.exit(1)

        jsonstring = sys.argv[i]
        i += 1
    elif arg == '-v':
        verbose += 1
    elif arg == '-on':
        jsonstring = JSON_ON
    elif arg == '-off':
        jsonstring = JSON_OFF
    elif arg == '-query':
        jsonstring = JSON_QUERY
    else:
        print("{}: invalid command line option/argument \"{}\"".format(progname, arg), file=sys.stderr)
        sys.exit(1)

if hostip == "-":
    print("{}: must specify a hostname/ip with the -h option".format(progname), file=sys.stderr)
    sys.exit(1)

if verbose >= 1:
    print("Host IP.......:", hostip)
    print("Port number...:", portnum)
    print("JSON string...:", jsonstring)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

s.connect((hostip, portnum))

s.send(plain2cipher(bytearray(jsonstring, 'utf-8')))

plugdata = s.recv(MAX_PACKET_LENGTH)

s.close()

print(cipher2plain(plugdata[4:]).decode('utf-8'))

sys.exit(0)

########################################################################
