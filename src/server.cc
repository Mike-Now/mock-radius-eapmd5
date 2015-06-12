#include <iostream>
#include "server_loop.h"
#include "tclap/CmdLine.h"
#include "logging.h"
#include "csv_reader.h"
#include "radius_server.h"

const std::string LOGGER_NAME = "server";

int main(int argc, char **argv) {
    using namespace TCLAP;
    using namespace std;
    try {

        CmdLine cmd("RADIUS Server with EAP-MD5", ' ');

        ValueArg<string> logpathArg("l", "log",
                                    "The path where log file shall be written",
                                    false, "server.log", "string");
        cmd.add(logpathArg);

        ValueArg<string> dbArg("d", "database",
                               "The path to the plain text file with user data",
                               false, "users.txt", "string");
        cmd.add(dbArg);

        ValueArg<string> secretArg("s", "secret", "The secret shared with NAS",
                                   true, "", "string");
        cmd.add(secretArg);

        ValueArg<int> portArg("p", "port", "Binded port", false, 8080,
                              "number");
        cmd.add(portArg);

        ValueArg<string> ipArg("a", "address", "Binded IP address", false,
                               "inany", "IP");

        cmd.add(ipArg);

        cmd.parse(argc, argv);
using namespace radius;
using namespace radius::packets;

        int port = portArg.getValue();
        string ip = ipArg.getValue();
        string secret = secretArg.getValue();
        string logpath = logpathArg.getValue();
        radius::initLogger(logpath, LOGGER_NAME);

        auto logger = spdlog::get(LOGGER_NAME);

        string dbpath = dbArg.getValue();

        radius::startServer(ip.c_str(), port);
        // temporary server loop
		RadiusServer radServer(readCsvFile(dbpath),secret,logger);
        /*while (1) {
            radius::sendData(radius::receiveData());
        }*/
		Packet iPacket = radius::receiveData();
		printf("%d", iPacket.bytes[0]);
		radServer.recvPacket(iPacket);
		
        radius::stopServer();
    } catch (CmdLineParseException &ce) {
        cerr << "error: " << ce.error() << ce.argId() << endl;
    }
}
