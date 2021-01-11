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

## LRNetServer
LRNetServer is 