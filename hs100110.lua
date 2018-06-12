#! /usr/local/bin/lua

--
--  @(!--#) @(#) hs100110.lua, version 003, 31-may-2018
--
--  TP-Link HS100 Smart WiFi Plug controller (Lua version)
--
--  Send encoded JSON requests via TCP/IP to turn a
--  TP-Link HS100 and HS110 Smart WiFi plug on and off
--
--  Product details:
--
--      http://uk.tp-link.com/products/list-5258.html
--
--  Based on encoded JSON strings from:
--
--      https://community.openhab.org/t/tp-link-hs100-smart-plug-wifi/8591/6
--
--  This page has comprehensive and interesting reverse engineering details:
--
--      https://www.softscheck.com/en/reverse-engineering-tp-link-hs110/
--

-----------------------------------------------------------------

--
--  Constants
--

JSON_ON    = "{\"system\":{\"set_relay_state\":{\"state\":1}}}"
JSON_OFF   = "{\"system\":{\"set_relay_state\":{\"state\":0}}}"
JSON_QUERY = "{\"system\":{\"get_sysinfo\":null}}"

-----------------------------------------------------------------

function plain2cipher(bytes)
  b = ""

  key = 171

  for i = 1, #bytes do
    c = bytes:byte(i)

    keyedbyte = key ~ c

    key = keyedbyte

    b = b .. string.char(keyedbyte)
  end

  return b
end

-----------------------------------------------------------------

function cipher2plain(bytes)
  b = ""

  key = 171

  for i = 1, #bytes do
    c = bytes:byte(i)

    keyedbyte = key ~ c

    key = c

    b = b .. string.char(keyedbyte)
  end

  return b
end

-----------------------------------------------------------------

function showpacket(title, packet)
  print(title .. ":")

  local bpr = 10       -- "bpr" means bytes per row of output

  lenpacket = #packet

  for i = 1, lenpacket do
    if (((i - 1) % bpr) == 0) then
      line = string.format("%04d:", i)
    end

    c = packet:byte(i)

    line = line .. string.format(" %02X", c)

    if ( (i == lenpacket) or ((i % bpr) == 0) ) then
      print(line)
    end
  end

  return
end

-----------------------------------------------------------------

--
--  Main
--

local socket = require("socket")
local verbose = 0
local hostip = "-"
local portnum = 9999
local jsonstring = JSON_QUERY

local argc = #arg

if (argc > 0) then
  local i = 1

  while i <= argc do
    if arg[i] == "-h" then
      if i == argc then
        print("ERROR: expecting host IP after -h option")
        os.exit()
      else
        i = i + 1
        hostip = arg[i]
      end
    elseif arg[i] == "-p" then
      if i == argc then
        print("ERROR: expecting port number after -p option")
        os.exit()
      else
        i = i + 1
        portnum = arg[i]
      end
    elseif arg[i] == "-j" then
      if i == argc then
        print("ERROR: expecting JSON string after -j option")
        os.exit()
      else
        i = i + 1
        jsonstring = arg[i]
      end
    elseif arg[i] == "-v" then
      verbose = verbose + 1
    elseif arg[i] == "-on" then
      jsonstring = JSON_ON
    elseif arg[i] == "-off" then
      jsonstring = JSON_OFF
    elseif arg[i] == "-query" then
      jsonstring = JSON_QUERY
    else
      print("ERROR: invalid command line option/argument \"" .. arg[i] .. "\"")
      os.exit()
    end

    i = i + 1
  end
end

if hostip == "-" then
  print("ERROR: must specify a hostname/ip with the -h option")
  os.exit()
end

if verbose >= 1 then
  print("Host IP.......: " .. hostip)
  print("Port number...: " .. portnum)
  print("JSON string...: " .. jsonstring)
end

local s = socket.tcp()

if verbose >= 1 then
  print("Connecting to " .. hostip .. " on port " .. portnum)
end

local err, errmsg = s:connect(hostip, portnum)

if err == nil then
  print("ERROR: connect failed: " .. errmsg)
  os.exit()
else
  if verbose >= 1 then
    print("Connected OK")
  end
end

json = plain2cipher(jsonstring)

lenjson = #json

packet = ""
packet = packet .. string.char((lenjson & 0xFF000000) >> 24)
packet = packet .. string.char((lenjson & 0x00FF0000) >> 16)
packet = packet .. string.char((lenjson & 0x0000FF00) >> 8)
packet = packet .. string.char((lenjson & 0x000000FF) >> 0)
packet = packet .. json

if verbose >= 2 then
  showpacket("Encoded", packet)
end

s:send(packet)

resplenbytes = s:receive(4)

if verbose >= 2 then
  showpacket("Response length", resplenbytes)
end

resplen = 0

resplen = resplen + (resplenbytes:byte(1) << 24)
resplen = resplen + (resplenbytes:byte(2) << 16)
resplen = resplen + (resplenbytes:byte(3) << 8)
resplen = resplen + (resplenbytes:byte(4) << 0)

if verbose >= 2 then
  print("Response length is: " .. resplen)
end

response = s:receive(resplen)

if verbose >= 2 then
  showpacket("Encryped response", response)
end

response = cipher2plain(response)

if verbose >= 2 then
  showpacket("Plain response", response)
end

print(response)

if verbose >= 1 then
  print("Closing socket")
end

s:close()

if verbose >= 1 then
  print("Closed")
end

os.exit()
