#!/usr/bin/perl -w
#This works on Debian Linux
#I have not tested it on a Windows system running Perl, but it should work if all dependencies are met.
#Also, FIREWALL.  <--- I guarantee that will be the issue.

use IO::Socket;

### Create UDP Listen Socket
my $udpsocket = new IO::Socket::INET (
  LocalPort => '54321',
  Proto => 'udp',
  );
  die "Could not create socket: $!\n" unless $udpsocket;

### Data Manipulation and Display
while(1) {
    $udpsocket->recv(my $data,512);
    print $data;
}
