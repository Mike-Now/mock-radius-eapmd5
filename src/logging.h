#include "packets/radius_packet.h"
#include "packets/eap_packet.h"
#include "spdlog/spdlog.h"
#include <string>
#include <sstream>
#include <vector>
namespace radius {
void initLogger(const std::string &logPath, const std::string &logName);
std::string packet2LogBytes(const packets::RadiusPacket &packet);
}
