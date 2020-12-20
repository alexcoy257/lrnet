#include <QApplication>
#include "lrnetserver.h"
#include <getopt.h>

int gVerboseFlag = 0;
QString mCertFile = "";
QString mKeyFile = "";

enum LongOptIDS {
  OPT_AUTHCERT = 1001,
  OPT_AUTHKEY
};

void printUsage(){
  std::cout << "Help not implemented \n";
}

void parseInput(int argc, char** argv)
{
    // Always use decimal point for floating point numbers
    setlocale( LC_NUMERIC, "C" );
    // If no command arguments are given, print instructions
    if(argc == 1) {
        std::exit(0);
    }

    // Usage example at:
    // http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
    // options descriptor
    //----------------------------------------------------------------------------
    static struct option longopts[] = {
        { "certfile", required_argument, NULL, OPT_AUTHCERT }, // Certificate for server authentication
        { "keyfile", required_argument, NULL, OPT_AUTHKEY }, // Private key for server authentication
        { "help", no_argument, NULL, 'h' }, // Print Help
        { NULL, 0, NULL, 0 }
    };

    // Parse Command Line Arguments
    //----------------------------------------------------------------------------
    /// \todo Specify mandatory arguments
    int ch;
    while ((ch = getopt_long(argc, argv,
                             "h", longopts, NULL)) != -1)
        switch (ch) {

        case 'h': // HElp
            //-------------------------------------------------------
            std::cout << "Help not implemented \n";
            std::exit(0);
            break;
        case OPT_AUTHCERT:
          mCertFile = optarg;
          break;
        case OPT_AUTHKEY:
          mKeyFile = optarg;
          break;
        case ':': {
          printUsage();
          printf("*** Missing option argument *** see above for usage\n\n");
          break; }
        case '?': {
          printUsage();
          printf("*** Unknown, missing, or ambiguous option argument *** see above for usage\n\n");
          std::exit(1);
          break; }
        default: {
            //-------------------------------------------------------
            printUsage();
            printf("*** Unrecognized option -%c *** see above for usage\n",ch);
            std::exit(1);
            break; }
        }

    // Warn user if undefined options where entered
    //----------------------------------------------------------------------------
    if (optind < argc) {
      if (strcmp(argv[optind],"help")!=0) {
        std::cout << "...separator..." << std::endl;
        std::cout << "*** Unexpected command-line argument(s): ";
        for( ; optind < argc; optind++) {
          std::cout << argv[optind] << " ";
        }
        std::cout << std::endl << "...separator..." << std::endl;
      }
      printUsage();
      std::exit(1);
    }

}


int main(int argc, char** argv){

  QApplication app(argc, argv, false);
      parseInput(argc, argv);
      LRNetServer * server = new LRNetServer();
      server->setCertFile(mCertFile);
      server->setKeyFile(mKeyFile);
      server->start();
      return app.exec();
}
