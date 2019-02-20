#ifndef ARTNET_H
#define ARTNET_H

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// UDP specific
#define ART_NET_PORT 6454
// Opcodes
#define ART_DMX 0x5000
// Buffers
#define MAX_BUFFER_ARTNET 530
// Packet
#define ART_NET_ID "Art-Net\0"
#define ART_DMX_START 18

class Artnet
{
public:
  Artnet();

  void begin(byte mac[], byte ip[]);
  uint16_t read();

  inline void setArtDmxCallback(void (*fptr)(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress remoteIP))
  {
    artDmxCallback = fptr;
  }

private:
  EthernetUDP Udp;

  void (*artDmxCallback)(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress remoteIP);
};

#endif
