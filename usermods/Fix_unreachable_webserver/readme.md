# Fix unreachable Webserver

This modification performs a ping request to the local IP address every 60 seconds. By this procedure the web server remains accessible in some problematic WLAN environments.

The modification works with static or DHCP IP address configuration 

_Story:_

Unfortunately, with all ESP projects where a web server or other network services are running, I have the problem that after some time the web server is no longer accessible.  Now I found out that the connection is at least reestablished when a ping request is executed by the device.

With this modification, in the worst case, the network functions are not available for 60 seconds until the next ping request.

## Installation 

Copy and replace the file `usermod.cpp` in wled00 directory.


