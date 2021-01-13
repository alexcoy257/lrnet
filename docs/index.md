# LRNet
A free and open source project that builds upon JACK
and JackTrip to provide an enterprise-level method of
hosting remote, real-time, networked musical rehearsal
and performance. LRNet's initial developers are Alex Coy
and James Parker, both of which started development during
their senior undergraduate years at Cornell University.

LRNet has several components that attempt to follow a
model-view-controller style. Additionally, the code leverages
the cross-platform portability of Qt so that it builds and
runs on Windows, macOS, and Linux with relative ease. Support for embedded Linux such as Raspbian on the Raspberry Pi is
under development as well.

## LRNetClient
The heart of a client connection is with an OSC connection to a 
server running the LRNet Server program. The LRNetClient library
provides a quick way to get up and running with the functions
that the server expects. We use
[oscpack](http://www.rossbencina.com/code/oscpack) to
implement the LRNet OSC space. Find out more here (link coming).

## RehearsalChef
RehearsalChef is a GUI for LRNetClient Qt Widgets. It uses our
LRNetClient library to connect under the hood.

## PiRehearsalChef
PiRehearsalChef will be a GUI tailored for embedded Linux 
systems as the Raspberry Pi. Details will appear at some point.
There may be four to five control approaches that have very
little to do with LRNet and JackTrip.
* Support a Hifiberry ADC/DAC+ Pro HAT with Bluetooth or
DSI display control.
* Support a pure CLI (but using some visual like ncurses) 
ssh-based approach with any audio interface that does not use 
mdns (this is likely more compatible with enterprise-level 
networks that do not support multicast packets or .local DNS 
resolution).
* Support an ssh-based approach that uses mdns (like the
JackTrip Virtual Studio devices). At no point will the devices
attempt to connect to a server that you do not approve of.
* Support a USB audio interface with Bluetooth, DSI Display, or
PiTFT HAT control.
* At no point will we ever support X11 forwarding for a GUI.
* Extend RehearsalChef to allow control over a device running
PiRehearsalChef over a local or wide area network.

## LRNetServer
LRNetServer is a hub server that responds to OSC commands in
a particular way. The most salient feature that it allows for
hardened security of both the control and audio feeds.
This may make it an attractive choice for institutions,
contrasting with the JackTrip Virtual Studio, Aloha by Elk,
Sonobus, and SoundJack. LRNetServer should also be easy enough
for a competent sysadmin to set up securely on common server
hardware. We will try to make a starter Docker image that is
easily deployable to servers made in the last decade; we aren't
interested at all in hosting public servers ourselves.

Here is a very basic [guide](server/serverRunning.md) for sysadmins to introduce
server-starters to enough Linux to run LRNetServer successfully.

## LRNetVideo
We have plans to develop multiple video solutions that are
GPL-compatible. However, they will not appear soon whatsoever.

## LRNetNdiForwarder
This application will be available under a license that
allows you to link with the Newtek NDI SDK and incorporate the 
terms which protect Newtek's intellectual property. It simply 
forwards NDI video to an LRNetServer so that LRNetVideo can take
it from there. Feel free to develop other proprietary-to-LRNet
converters.

## Licensing
All of the LRNet audio code is available under the GPLv2, as we
include a recent copy of libjackserver. Parallel licenses for 
non-JACK LRNet configurations will come as we find setups that
work.