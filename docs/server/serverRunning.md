# Running the LRNet Server
## Preamble
Running LRNetServer on a stable, enterprise-grade Linux server OS
such as RHEL or CentOS is mildly challenging because it
is bleeding-edge software. One approach is to build all of the
updated libraries needed into ```/usr/local``` and then install
the LRNet software there. Right now, LRNetServer is not a service
or daemon, so authorized users have to start the server program
manually, perhaps over ssh. The rest of this guide is aimed at
users new to Linux and Linux servers.

## Environment Setup
Every new user should do these steps once to set up the proper
environment.
Check your ```~/.profile``` file with the following command.

```cat ~/.profile```

On some servers, that may return nothing as the default user
needs no special steps to occur upon login. We simply want
the server to load the ```.bashrc``` file upon login.
You can append a line that does that with

```echo "source ~/.bashrc" >> ~/.profile```

We can make a simple ```.bashrc``` with two lines.

```
source /etc/bashrc
export LD_LIBRARY_PATH=/usr/local/openssl/openssl_gcc9/lib:/usr/local/lib:$LD_LIBRARY_PATH
```

Check to make sure that you aren't clobbering anything already
there by using

```cat ~/.bashrc```

You can make that ```.bashrc``` with
```
printf "source /etc/bashrc \n\
export LD_LIBRARY_PATH=\
/usr/local/openssl/openssl_gcc9/lib:\
/usr/local/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc
```


This is the case for our test CentOS server, as we built and 
installed Qt 5.15 to /usr/local, needed gcc9 to build it, and
needed openSSL built with gcc9 to enable the TLS control
channel and encrypted audio.

To use your new ```.bashrc```, you may log out and log back
in. You may also use ```source ~/.bashrc``` to use your new
```.bashrc```.

## Starting and stopping the LRNet Server

We also recommend using GNU Screen to save server output
temporarily and to allow you to log out of the server without
LRNetServer halting. Create a new Screen session with
```screen -S whateverNameYouWant```
If your environment is set up well, simply use

```lrnetserver```

in that Screen environment to start the server. It'll handle the rest.
If it crashes, it is safe to re-issue the command. Detach
from the screen session using Ctrl-a d. (Ctrl-a, then press
d to detach). To resume, use ```screen -r``` or in the case
of multiple existing sessions,

```screen -r PIDOrNameYouChose```

. Screen sessions remain across logins, so it never hurts to try
to resume your previous Screen for the LRNet Server.

To stop the ```lrnetserver``` instance that you started, simply issue
Ctrl-c within the Screen session. The server will print some final closing
messages and then exit to the command prompt. If ```lrnetserver```
had crashed before, it is still safe to issue Ctrl-c within the Screen
session.
