# LiveRehearsal Suite
This is confusing, but the project at alexcoy257/cuws-gui was kind-of scrapped
in favor of this project becoming the new LiveRehearsal. The documentation
is actually among the code and the docs folder.

# Server Dependencies
This software is somewhat bleeding-edge and requires the building of a fair
amount of material. However, here is a list of dependencies that we tried
to log. Note that on CentOS 8, the default version of gcc is too low to support
building Qt 5.15. So, you will have to set up devtoolset-9 from scl and compile
your libraries and the server program with gcc 9 from that collection.

| Dependency | Build on CentOS 8 | Build on Debian Buster | Remarks |
| --         |--                 | --                     | --      |
| Qt 5.15    | Yes               | Yes, with default GCC  | Could use Qt Creator's Qt. The default Qt5 is too old on both distros|
| Qt 5.15 MySQL driver | Yes     | Yes, with default GCC  | Qt Creator's Qt does not ship with the Qt MySQL driver, but it is easily buildable from source |
| JACK       | Yes               | Maybe                  | On our Buster test systems, we built the latest version of JACK with DBUS support so that it could capture and release the sound card. On a headless, soundcardless server, you could get away with Debian's libjack-jackd2-dev. |
| Libcrypto  | Yes               | Not necessary          | Built openssl's libcrypto with gcc9's C compiler |
| OSCPack    | Yes               | Yes                    | Can use as pre-compiled library or build from source every time |
| [JackTrip Fork, EncryptedAudio Branch](github.com/alexcoy257/jacktrip) | Yes | Yes | |
| Faust      | Optional               | Optional          | We include a Faust-generated ChannelStrip.h, but you may use Faust to generate your own from the DSP file.|
| [JackServerTest](github.com/alexcoy257/jackservertest) | Yes | Yes | |

## QMake
We have tried to leverage QMake for building from .pro files. On lrnet,
alexcoy257/jacktrip, and jackservertest, you can set the jack directory,
libssl directory, lrnet_deps directory (where jacktrip and jackservertest
are, by default /usr/local), and install directory. The exact arguments
will appear below at a later time.

# Client Dependencies
On MacOS, you just need the app bundle. On Windows, we recommend installing
an ASIO driver if your interface supports Steinberg's ASIO standard. On
Linux, you will have to build the app yourself since we do not have a package
set up for any distribution.
