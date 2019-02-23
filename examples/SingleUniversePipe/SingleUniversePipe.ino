#include <ArtnetBridge.h>

byte ip[] = {10,0,0,10};
byte mac[] = {0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC};
byte broadcast[] = {10, 0, 0, 255};

ArtnetBridge bridge;
void setup() {
  // start listsening
  bridge.setup(mac, ip, broadcast);
  // map universe 1 to pin 3
  bridge.addRedirect(0, 3);
}

void loop() {
  bridge.read();
}
