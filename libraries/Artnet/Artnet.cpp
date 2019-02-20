#include <Artnet.h>

Artnet::Artnet() {}

void Artnet::begin(byte mac[], byte ip[]) {
  Ethernet.begin(mac,ip);
  Udp.begin(ART_NET_PORT);
}

uint16_t Artnet::read() {
  uint16_t packetSize = Udp.parsePacket();

  if (packetSize <= MAX_BUFFER_ARTNET && packetSize > 0) {
    uint8_t artnetPacket[MAX_BUFFER_ARTNET];
      Udp.read(artnetPacket, MAX_BUFFER_ARTNET);

      // Check that packetID is "Art-Net" else ignore
      for (byte i = 0 ; i < 8 ; i++) {
        if (artnetPacket[i] != ART_NET_ID[i]) return 0;
      }

      uint16_t opcode = artnetPacket[8] | artnetPacket[9] << 8;

      if (opcode == ART_DMX) {
        uint8_t sequence = artnetPacket[12];
        uint16_t incomingUniverse = artnetPacket[14] | artnetPacket[15] << 8;
        uint16_t dmxDataLength = artnetPacket[17] | artnetPacket[16] << 8;

        if (artDmxCallback) (*artDmxCallback)(incomingUniverse, dmxDataLength, sequence, artnetPacket + ART_DMX_START, Udp.remoteIP());
        return ART_DMX;
      }
  }
  return 0;
}
