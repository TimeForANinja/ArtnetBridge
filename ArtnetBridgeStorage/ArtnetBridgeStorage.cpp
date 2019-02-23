#include "./ArtnetBridgeStorage/ArtnetBridgeStorage.h"

ArtnetBridgeStorage::ArtnetBridgeStorage() {}

void ArtnetBridgeStorage::newUniverse(uint16_t universe, uint8_t pin) {
  universes[savedUniverses] = universe;
  for(uint16_t a = 0 ; a < MAX_DMX ; a++) {
    dmxData[savedUniverses][a] = 0;
  }
  outputPin[savedUniverses] = pin;
  outputPort[savedUniverses] = portOutputRegister(digitalPinToPort(pin));
  dmxBit[savedUniverses] = digitalPinToBitMask(pin);
  pinMode(pin, OUTPUT);
  savedUniverses++;
}

uint8_t * ArtnetBridgeStorage::getDMXByUniverse(uint16_t universe) {
  return dmxData[getIndexFromUniverse(universe)];
}

uint8_t * ArtnetBridgeStorage::getDMXByIndex(uint8_t index) {
  return dmxData[index];
}

void ArtnetBridgeStorage::readUniverse(uint16_t universe, EthernetUDP *Udp) {
  Udp->read(dmxData[getIndexFromUniverse(universe)], MAX_DMX);
}

bool ArtnetBridgeStorage::hasUniverse(uint16_t universe) {
  for(uint8_t a = 0 ; a < savedUniverses ; a++) {
    if(universes[a] == universe) return true;
  }
  return false;
}

void ArtnetBridgeStorage::removeUniverse(uint16_t universe) {
  uint8_t index = getIndexFromUniverse(universe);
  while(index+1 < savedUniverses) {
    universes[index] = universes[index+1];
    outputPort[index] = outputPort[index+1];
    outputPin[index] = outputPin[index+1];
    dmxBit[index] = dmxBit[index+1];
    memcpy(dmxData[index], dmxData[index+1], MAX_DMX);
    index++;
  }
  savedUniverses--;
}

uint8_t ArtnetBridgeStorage::getSize() {
  return savedUniverses;
}

uint8_t ArtnetBridgeStorage::getPinByUniverse(uint16_t universe) {
  return outputPin[getIndexFromUniverse(universe)];
}

uint8_t ArtnetBridgeStorage::getPinByIndex(uint8_t index) {
  return outputPin[index];
}

volatile uint8_t * ArtnetBridgeStorage::getPortByIndex(uint8_t index) {
  return outputPort[index];
}

uint8_t ArtnetBridgeStorage::getBitByIndex(uint8_t index) {
  return dmxBit[index];
}

// UTIL:
uint8_t ArtnetBridgeStorage::getIndexFromUniverse(uint16_t universe) {
  for(uint8_t a = 0 ; a < savedUniverses ; a++) {
    if(universes[a] == universe) return a;
  }
  return 0;
}
