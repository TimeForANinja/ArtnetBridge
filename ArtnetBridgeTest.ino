#include <Artnet.h>
#include <DmxSimple.h>

#define UNIVERSE 0

Artnet artnet;
byte ip[] = {10,0,0,10};
byte mac[] = {0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC};
byte broadcast[] = {10, 0, 0, 255};

void setup() {
  Serial.begin(250000);
  artnet.begin(mac, ip);
//  artnet.setBroadcast(broadcast);
  artnet.setArtDmxCallback(onDmxFrame);

  DmxSimple.usePin(3);
  DmxSimple.maxChannel(32);
}

void loop() {
  artnet.read();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data, IPAddress remoteIP) {
  Serial.print("Universe: ");
  Serial.print(universe);
  Serial.print(" items: ");
  Serial.println(length);
  Serial.print("data: ");
  if(universe != UNIVERSE) return;
  for (int i = 0 ; i < length ; i++) {
    Serial.print(data[i]);
    Serial.print(" ");
    DmxSimple.write(i+1, data[i]);
  }
  Serial.println("");
}
