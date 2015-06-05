#include "radius_server.h"
using std::map;
using std::string;
using std::vector;
using radius::packets::Packet;

namespace radius{

RadiusServer::RadiusServer(const map<string,string> &userPassMap,const string &secret):
    userPassMap(userPassMap),secret(secret) {}

vector<const Packet> RadiusServer::recvPacket(const Packet &packet){
    vector<const Packet> packetsToSend;


    std::transform(pendingPackets.begin(),
            pendingPackets.end(),
            std::back_inserter(packetsToSend),
            [](const PendingPacket&p){return p.packet;});

    return packetsToSend;
}

void RadiusServer::incrementCounters(){

}

}
