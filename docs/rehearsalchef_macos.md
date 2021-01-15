# How to install and launch on macOS
1. Download the .dmg file.
2. Drag and drop RehearsalChef to whatever directory you want: probably /Applications or /Users/<your username>/Applications.
3. Double click the app to run it. It is signed and notarized, but your Mac must be set up to allow apps from the App Store *and* Identified Developers.

## How to mix and hear people as Chef
Talkback is not implemented yet. Right now, you need to connect as both a Chef and a Member with two separate instances of RehearsalChef. Right now the only way to do that is on the command line. If you installed the app bundle to /Applications, use this command:
```/Applications/rehearsalchef.app/Contents/MacOS/rehearsalchef &```
Otherwise, replace /Applications with where you installed RehearsalChef.

This build required modification to Jack2 such that libjack and libjackserver comply with hardened runtime restrictions. All source code and build scripts are published so that you may rebuild from source on *your* computer
The modified jack2 sources (for complying with hardened runtime) are at the following link on macos-lrnet. [https://github.com/alexcoy257/jack2](https://github.com/alexcoy257/jack2). The exact sources for lrnet are added as submodules to this repository.
If a GitHub download is not suitable, please contact Alex Coy directly to arrange delivery of an alternative format for sources and binaries.

Instructions for how to package the modified jack2 libraries and other dependencies will come shortly.
