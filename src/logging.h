#include "packets/radius_packet.h"
#include "packets/eap_packet.h"
#include <string>
#include <sstream>
namespace radius {
void initLoggers();
std::string packet2LogBytes(const packets::RadiusPacket &packet);
}
