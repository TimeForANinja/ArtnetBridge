#ifndef ArtnetBridge_H
#define ArtnetBridge_H

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// UDP specific
#define ART_NET_PORT 6454
// Opcodes
#define ART_POLL 0x2000
#define ART_POLL_REPLY 0x2100
#define ART_DMX 0x5000
// Buffers
#define MAX_BUFFER_ARTNET 530
#define MAX_DMX 512
// Packet
#define ART_NET_ID "Art-Net\0"
#define ART_DMX_START 18
#define ART_HEAD_LENGTH 12
#define ART_META_LENGTH 6

struct artnet_reply_s {
  uint8_t  id[8];
  uint16_t opCode;
  uint8_t  ip[4];
  uint16_t port;
  uint8_t  verH;
  uint8_t  ver;
  uint8_t  subH;
  uint8_t  sub;
  uint8_t  oemH;
  uint8_t  oem;
  uint8_t  ubea;
  uint8_t  status;
  uint8_t  etsaman[2];
  uint8_t  shortname[18];
  uint8_t  longname[64];
  uint8_t  nodereport[64];
  uint8_t  numbportsH;
  uint8_t  numbports;
  uint8_t  porttypes[4]; //max of 4 ports per node
  uint8_t  goodinput[4];
  uint8_t  goodoutput[4];
  uint8_t  swin[4];
  uint8_t  swout[4];
  uint8_t  swvideo;
  uint8_t  swmacro;
  uint8_t  swremote;
  uint8_t  sp1;
  uint8_t  sp2;
  uint8_t  sp3;
  uint8_t  style;
  uint8_t  mac[6];
  uint8_t  bindip[4];
  uint8_t  bindindex;
  uint8_t  status2;
  uint8_t  filler[26];
} __attribute__((packed));

class ArtnetBridge {
public:
  ArtnetBridge();

  void setup(byte mac[6], byte ip[4], byte bc[4]);
  void read();
  void addRedirect(uint16_t universe, uint8_t port);

private:
  EthernetUDP Udp;
  struct artnet_reply_s ArtPollReply;

  // settings
  IPAddress broadcast;

  // inside read
  uint16_t packetSize;
  uint8_t artnetHead[ART_HEAD_LENGTH];
  uint8_t artnetMeta[ART_META_LENGTH];
  uint16_t opcode;
  uint16_t incomingUniverse;
  IPAddress local_ip;

  // outputting to dmx
  bool dmxStarted;
  void dmxBegin();
  void dmxEnd();
};

#endif
