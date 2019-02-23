#ifndef ArtnetBridgeStorage_H
#define ArtnetBridgeStorage_H

#include <EthernetUdp.h>

#define MAX_UNIVERSES 1
#define MAX_DMX 512

class ArtnetBridgeStorage {
public:
  ArtnetBridgeStorage();

  void newUniverse(uint16_t universe, uint8_t pin);
  uint8_t * getDMXByUniverse(uint16_t universe);
  uint8_t * getDMXByIndex(uint8_t index);
  void readUniverse(uint16_t universe, EthernetUDP udp);
  bool hasUniverse(uint16_t universe);
  void removeUniverse(uint16_t universe);
  uint8_t getSize();
  uint8_t getPinByUniverse(uint16_t universe);
  uint8_t getPinByIndex(uint8_t index);
  volatile uint8_t * getPortByIndex(uint8_t index);
  uint8_t getBitByIndex(uint8_t index);

private:
  uint8_t savedUniverses;
  uint16_t universes[MAX_UNIVERSES];
  uint8_t outputPin[MAX_UNIVERSES];
  volatile uint8_t * outputPort[MAX_UNIVERSES];
  uint8_t dmxBit[MAX_UNIVERSES];
  uint8_t dmxData[MAX_UNIVERSES][MAX_DMX];

  uint8_t getIndexFromUniverse(uint16_t universe);
};

#endif
