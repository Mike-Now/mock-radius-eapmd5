#include "auth_common.h"

using radius::packets::RadiusPacket;
using radius::packets::RadiusAVP;
using radius::packets::EapData;
using radius::packets::UserName;
using radius::packets::EapPacket;
using radius::packets::EapIdentity;
using radius::packets::EapMd5Challenge;
using radius::packets::MessageAuthenticator;


namespace radius {

namespace {
// array of 0s
const std::array<byte, 16> nullAuth{};
const std::string EMPTY_STR = "";
typedef std::uniform_int_distribution<unsigned int> IntGenerator;
typedef std::independent_bits_engine<std::mt19937, 4, unsigned int>
    UniBiIntGenerator;
std::mt19937 seedGen(std::random_device{}());
UniBiIntGenerator genBytes(seedGen);

const std::array<byte, 16> EMPTY_MD5{};
}

bool checkMessageAuthenticator(const RadiusPacket &packet,
                               const std::string &secret,
                               const std::array<byte, 16> &authenticator) {
    std::vector<std::unique_ptr<RadiusAVP>> avpList = packet.getAVPList();
    MessageAuthenticator *ma;
    std::for_each(avpList.begin(), avpList.end(),
                  [&](const std::unique_ptr<RadiusAVP> &avp) {
                      if (avp->getType() == RadiusAVP::MESSAGE_AUTHENTICATOR) {
                          RadiusAVP *ap = const_cast<RadiusAVP *>(avp.get());
                          ma = static_cast<MessageAuthenticator *>(ap);
                      }
                  });

    if (packet.getCode() == RadiusPacket::ACCESS_REQUEST) {
        return calcMessageAuthenticatorChecksum(
                   packet, secret, packet.getAuthenticator()) == ma->getMd5();
    }
    return calcMessageAuthenticatorChecksum(packet, secret, authenticator) ==
           ma->getMd5();
}

bool checkAuthenticator(const RadiusPacket &packet,
                        const std::array<byte, 16> &authenticator) {
    if (packet.getCode() == RadiusPacket::ACCESS_REQUEST) {
        return true;
    }
    if (calcAuthenticatorChecksum(packet, authenticator) !=
        packet.getAuthenticator()) {
        return false;
    }
    return true;
}

void calcAndSetAuth(packets::RadiusPacket &packet,
                    const std::array<byte, 16> &authenticator) {
    if (packet.getCode() == RadiusPacket::ACCESS_REQUEST) {
        return;
    }
    packet.setAuthenticator(authenticator);
    packet.setAuthenticator(calcAuthenticatorChecksum(packet));
}
void calcAndSetAuth(packets::RadiusPacket &packet) {
    calcAndSetAuth(packet, packet.getAuthenticator());
}

void calcAndSetMsgAuth(packets::RadiusPacket &packet, const std::string &secret,
                       const std::array<byte, 16> &authenticator) {
    packet.setAuthenticator(authenticator);
    std::unique_ptr<MessageAuthenticator> maPtr =
        findMessageAuthenticator(packet);
    if (maPtr.get() == nullptr) {
        maPtr.reset(new MessageAuthenticator());
        packet.addAVP(static_cast<const MessageAuthenticator &>(*maPtr));
    }

    MessageAuthenticator nMa;
    nMa.setMd5(calcMessageAuthenticatorChecksum(packet, secret, authenticator));
    packet.replaceAVP(*maPtr, nMa);
}

void calcAndSetMsgAuth(packets::RadiusPacket &packet,
                       const std::string &secret) {
    calcAndSetMsgAuth(packet, secret, packet.getAuthenticator());
}

std::array<byte, 16>
calcAuthenticatorChecksum(const packets::RadiusPacket &packet) {
    return calcAuthenticatorChecksum(packet, packet.getAuthenticator());
}

std::array<byte, 16>
calcAuthenticatorChecksum(const packets::RadiusPacket &packet,
                          const std::array<byte, 16> &authenticator) {
    RadiusPacket refPacket(packet);
    refPacket.setAuthenticator(authenticator);
    std::array<byte, 16> md5 = md5Bin(refPacket.getBuffer());
    return md5;
}

std::array<byte, 16>
calcMessageAuthenticatorChecksum(const packets::RadiusPacket &packet,
                                 const std::string &secret) {
    return calcMessageAuthenticatorChecksum(packet, secret,
                                            packet.getAuthenticator());
}
std::array<byte, 16>
calcMessageAuthenticatorChecksum(const packets::RadiusPacket &packet,
                                 const std::string &secret,
                                 const std::array<byte, 16> &authenticator) {

    RadiusPacket refPacket = packet;

    if (packet.getCode() != RadiusPacket::ACCESS_REQUEST) {
        refPacket.setAuthenticator(authenticator);
    }

    std::unique_ptr<MessageAuthenticator> maPtr =
        findMessageAuthenticator(packet);

    MessageAuthenticator emptyMa;
    emptyMa.setMd5(nullAuth);

    refPacket.replaceAVP(*maPtr, emptyMa);

    return md5HmacBin(refPacket.getBuffer(), secret);
}

std::unique_ptr<packets::MessageAuthenticator>
findMessageAuthenticator(const packets::RadiusPacket &packet) {
    std::vector<std::unique_ptr<RadiusAVP>> avpList = packet.getAVPList();
    MessageAuthenticator *ma = nullptr;
    std::for_each(avpList.begin(), avpList.end(),
                  [&](const std::unique_ptr<RadiusAVP> &avp) {
                      if (avp->getType() == RadiusAVP::MESSAGE_AUTHENTICATOR) {
                          RadiusAVP *ap = const_cast<RadiusAVP *>(avp.get());
                          ma = static_cast<MessageAuthenticator *>(ap);
                      }
                  });
    if (ma == nullptr) {
        return std::unique_ptr<MessageAuthenticator>(nullptr);
    }
    return std::unique_ptr<MessageAuthenticator>(new MessageAuthenticator(*ma));
}

bool checkIntegrity(const packets::RadiusPacket &packet,
                    const std::string &secret,
                    const std::array<byte, 16> &authenticator) {

    return checkAuthenticator(packet, authenticator) &&
           checkMessageAuthenticator(packet, secret, authenticator);
}

bool isRequest(const RadiusPacket &packet) {
    if (packet.getCode() == RadiusPacket::ACCESS_REQUEST) {
        return true;
    }
    return false;
}

bool isValid(const RadiusPacket &packet) {
    std::vector<std::unique_ptr<RadiusAVP>> avpList = packet.getAVPList();
    int messageAuthenticatorC = 0;
    int eapMessageC = 0;
    int userNameC = 0;
    for (const auto &avpPtr : avpList) {
        byte type = avpPtr->getType();
        switch(type){
            case RadiusAVP::MESSAGE_AUTHENTICATOR:
                messageAuthenticatorC++;
                break;
            case RadiusAVP::EAP_MESSAGE:
                eapMessageC++;
                break;
            case RadiusAVP::USER_NAME:
                userNameC++;
                break;
        }
    }

    return messageAuthenticatorC == 1 && eapMessageC > 0 && userNameC==1;
}

std::string getUserName(const RadiusPacket &packet){
    std::vector<std::unique_ptr<RadiusAVP>> avpList = packet.getAVPList();
    UserName*un=nullptr;
    std::for_each(avpList.begin(), avpList.end(),
                  [&](const std::unique_ptr<RadiusAVP> &avp) {
                      if (avp->getType() == RadiusAVP::USER_NAME) {
                          RadiusAVP *ap = const_cast<RadiusAVP *>(avp.get());
                          un = static_cast<UserName *>(ap);
                      }
                  });
    if(un==nullptr){
        return EMPTY_STR;
    }
    return un->getString();
}

std::vector<byte> generateRandomBytes(unsigned int min, unsigned int max) {
    IntGenerator genLen{min, max};
    unsigned int len = genLen(seedGen);
    std::vector<byte> randInts((int)std::ceil((double)len / 4));
    std::generate(randInts.begin(), randInts.end(), [&] { return genBytes(); });

    byte *castInts = static_cast<byte *>(&randInts[0]);
    std::vector<byte> bytes(&castInts[0], &castInts[len]);
    return bytes;
}

std::array<byte, 16> generateRandom16() {
    std::array<byte, 16> arr;
    std::vector<byte> vec = generateRandomBytes(4, 4);
    std::copy(vec.begin(), vec.end(), arr.begin());
    return arr;
}
std::array<byte, 16> calcChalVal(byte ident, std::vector<byte> chal,
                                 const std::string &secret) {
    std::vector<byte> buffer;
    buffer.push_back(ident);
    buffer.insert(buffer.end(), secret.begin(), secret.end());
    buffer.insert(buffer.end(), chal.begin(), chal.end());
    return md5Bin(buffer);
}
std::array<byte, 16> calcChalVal(const packets::EapPacket &packet,
                                 const std::string &secret) {
    std::unique_ptr<EapData> eapData = packet.getData();
    EapMd5Challenge *md5Chal = dynamic_cast<EapMd5Challenge *>(eapData.get());
    return calcChalVal(packet.getIdentifier(),md5Chal->getValue(),secret);
}

EapPacket makeIdentity(const std::string id) {
    EapPacket packet;
    EapIdentity identity;
    identity.setIdentity(id);
    packet.setData(static_cast<EapData &>(identity));
    return packet;
}

EapPacket makeChallengeResp(const std::array<byte, 16> chalResp) {
    EapPacket packet;
    EapMd5Challenge eapChal;
    std::vector<byte> chalRespVec(chalResp.begin(), chalResp.end());
    eapChal.setValue(chalRespVec);
    packet.setData(static_cast<EapData &>(eapChal));
    return packet;
}
}
