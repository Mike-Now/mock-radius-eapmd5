#ifndef RADIUS_SERVER_H_GSDHRZVP
#define RADIUS_SERVER_H_GSDHRZVP

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <random>
/* #include <climits> */
#include <algorithm>
#include <functional>
#include "packets/packet.h"
#include "packets/radius_packet.h"
#include "packets/eap_packet.h"
#include "logging.h"
#include "typedefs.h"
#include "spdlog/spdlog.h"
#include "auth_common.h"
#include "packets/utils.h"

namespace radius {

enum AuthMode { EAP_MD5 };

class RadiusServer {

    typedef std::map<std::string, std::string> UserPassMap;
    typedef std::shared_ptr<spdlog::logger> Logger;
    typedef std::unique_ptr<packets::Packet> PacketPtr;
    typedef std::unique_ptr<packets::RadiusPacket> RadiusPacketPtr;

    const int PENDING_LIMIT = 5;
    // pending EAP-Request with a counter
    struct PendingPacket {
        int counter = 0;
        packets::Packet packet;
        PendingPacket(const packets::Packet &p) : packet(p) {}
    };

    struct AuthRequestId {
        std::string userName;
        sockaddr_in nasAddr;
        byte msgId;

        AuthRequestId(const std::string &u, const sockaddr_in &addr, byte i)
            : userName(u), nasAddr(addr), msgId(i) {}
    };

    struct AuthRequestIdCompare {
        bool operator()(const AuthRequestId &lhs,
                        const AuthRequestId &rhs) const {
            return lhs.userName < rhs.userName &&
                   lhs.nasAddr.sin_addr.s_addr < rhs.nasAddr.sin_addr.s_addr &&
                   lhs.nasAddr.sin_port < lhs.nasAddr.sin_port &&
                   lhs.msgId < lhs.msgId;
        }
    };

    struct AuthData {
        std::vector<byte> challenge;
        AuthData(){}
        AuthData(const std::vector<byte> &v) : challenge(v) {}
    };

    std::map<AuthRequestId, AuthData, AuthRequestIdCompare> authProcMap;

    // list of pending EAP-Requests
    std::vector<PendingPacket> pendingPackets;

    const UserPassMap userPassMap;
    const std::string secret;
    Logger logger;

    void updatePending();
    const std::vector<packets::Packet>
    addPendingPackets(std::vector<packets::Packet> packetsTosend);
    AuthMode authMode;

    RadiusPacketPtr recvEapId(packets::RadiusPacket &radiusPacket,
                              packets::EapIdentity &eapIden,
                              const sockaddr_in &inAddr,
                              byte eapId);
    
    RadiusPacketPtr recvEapMd5Chal(packets::RadiusPacket &radiusPacket,
            packets::EapMd5Challenge &eapMd5, 
            std::array<byte,16> &refHash);
    void persistChallenge(const std::string &userName,
                          const sockaddr_in &inAddr, byte msgId,
                          std::vector<byte> &challenge);

  public:
    /**
     * @param userPassMap user credentials login x password
     * @param secret the secret shared with client (NAS)
     **/
    RadiusServer(const UserPassMap &userPassMap, const std::string &secret,
                 const Logger &logger, AuthMode authMode);

    const std::vector<packets::Packet>
    processPacket(const packets::Packet &packet);
};
}
#endif /* end of include guard: RADIUS_SERVER_H_GSDHRZVP */
