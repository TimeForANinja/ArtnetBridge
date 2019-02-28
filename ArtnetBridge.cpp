#include <ArtnetBridge.h>
#include "./ArtnetBridgeStorage/ArtnetBridgeStorage.h"
#include "./ArtnetBridgeStorage/ArtnetBridgeStorage.cpp"

ArtnetBridge::ArtnetBridge() {}

ArtnetBridgeStorage storage;

void ArtnetBridge::setup(byte mac[6], byte ip[4], byte bc[4]) {
  // set variables
  broadcast = bc;

  // prefill the reply struct
  int8_t id[8];
  sprintf((char *)id, "Art-Net");
  memcpy(ArtPollReply.id, id, sizeof(ArtPollReply.id));

  ArtPollReply.opCode = ART_POLL_REPLY;
  ArtPollReply.port =  ART_NET_PORT;

  memset(ArtPollReply.goodinput,  0x08, 4);
  memset(ArtPollReply.goodoutput,  0x80, 4);
  memset(ArtPollReply.porttypes,  0xc0, 4);

  uint8_t shortname [18];
  uint8_t longname [64];
  sprintf((char *)shortname, "artnet arduino");
  sprintf((char *)longname, "Art-Net -> Arduino Bridge");
  memcpy(ArtPollReply.shortname, shortname, sizeof(shortname));
  memcpy(ArtPollReply.longname, longname, sizeof(longname));

  ArtPollReply.etsaman[0] = 0;
  ArtPollReply.etsaman[1] = 0;
  ArtPollReply.verH       = 1;
  ArtPollReply.ver        = 0;
  ArtPollReply.subH       = 0;
  ArtPollReply.sub        = 0;
  ArtPollReply.oemH       = 0;
  ArtPollReply.oem        = 0xFF;
  ArtPollReply.ubea       = 0;
  ArtPollReply.status     = 0xd2;
  ArtPollReply.swvideo    = 0;
  ArtPollReply.swmacro    = 0;
  ArtPollReply.swremote   = 0;
  ArtPollReply.style      = 0;

  ArtPollReply.numbportsH = 0;
  ArtPollReply.numbports  = 4;
  ArtPollReply.status2    = 0x08;

  uint8_t swin[4]  = {0x01, 0x02, 0x03, 0x04};
  uint8_t swout[4] = {0x01, 0x02, 0x03, 0x04};
  for(uint8_t i = 0; i < 4; i++) {
    ArtPollReply.swout[i] = swout[i];
    ArtPollReply.swin[i] = swin[i];
  }

  sprintf((char *)ArtPollReply.nodereport, "%i DMX output universes active.", ArtPollReply.numbports);

  // start listening
  Ethernet.begin(mac, ip);
  Udp.begin(ART_NET_PORT);
}

void ArtnetBridge::read() {
  packetSize = Udp.parsePacket();

  if (packetSize > MAX_BUFFER_ARTNET || packetSize <= 0) return;

  Udp.read(artnetHead, ART_HEAD_LENGTH);

  // Check that packetID is "Art-Net" else ignore
  for (uint8_t i = 0 ; i < 8 ; i++) {
    if (artnetHead[i] != ART_NET_ID[i]) return;
  }

  opcode = artnetHead[8] | artnetHead[9] << 8;

  if (opcode == ART_DMX) {
    Udp.read(artnetMeta, ART_META_LENGTH);

    incomingUniverse = artnetMeta[2] | artnetMeta[3] << 8;
    if (!storage.hasUniverse(incomingUniverse)) return;
    storage.readUniverse(incomingUniverse, &Udp);
  } else if (opcode == ART_POLL) {
    // fill the rest of the reply struct
    // then send it to the network's broadcast address
    local_ip = Ethernet.localIP();
    ArtPollReply.bindip[0] = local_ip[0];
    ArtPollReply.bindip[1] = local_ip[1];
    ArtPollReply.bindip[2] = local_ip[2];
    ArtPollReply.bindip[3] = local_ip[3];
    memcpy(ArtPollReply.ip, ArtPollReply.bindip, sizeof(ArtPollReply.ip));
    // send the packet to the broadcast address
    Udp.beginPacket(broadcast, ART_NET_PORT);
    Udp.write((uint8_t *)&ArtPollReply, sizeof(ArtPollReply));
    Udp.endPacket();
  }
}

void ArtnetBridge::addRedirect(uint16_t universe, uint8_t port) {
  bool restartRequired = dmxStarted;

  if (restartRequired) dmxEnd();
  storage.newUniverse(universe, port);
  if (restartRequired) dmxBegin();
}

// The modern chips (168, 328P, 1280) use identical mappings.
#define TIMER2_INTERRUPT_ENABLE() TIMSK2 |= _BV(TOIE2)
#define TIMER2_INTERRUPT_DISABLE() TIMSK2 &= ~_BV(TOIE2)
#define ISR_NAME TIMER2_OVF_vect
#define BITS_PER_TIMER_TICK (F_CPU / 31372)

void ArtnetBridge::dmxBegin() {
  dmxStarted = true;
  TIMER2_INTERRUPT_ENABLE();
}

void ArtnetBridge::dmxEnd() {
  TIMER2_INTERRUPT_DISABLE();
  dmxStarted = false;
}

// Transmit a complete DMX byte
// We have no serial port for DMX, so everything is timed using an exact
// number of instruction cycles.
void dmxSendByte(volatile uint8_t value, volatile uint8_t *dmxPort, uint8_t dmxBit) {
  uint8_t bitCount, delCount;
  __asm__ volatile (
    "cli\n"
    "ld __tmp_reg__,%a[dmxPort]\n"
    "and __tmp_reg__,%[outMask]\n"
    "st %a[dmxPort],__tmp_reg__\n"
    "ldi %[bitCount],11\n" // 11 bit intervals per transmitted byte
    "rjmp bitLoop%=\n"     // Delay 2 clock cycles.
  "bitLoop%=:\n"\
    "ldi %[delCount],%[delCountVal]\n"
  "delLoop%=:\n"
    "nop\n"
    "dec %[delCount]\n"
    "brne delLoop%=\n"
    "ld __tmp_reg__,%a[dmxPort]\n"
    "and __tmp_reg__,%[outMask]\n"
    "sec\n"
    "ror %[value]\n"
    "brcc sendzero%=\n"
    "or __tmp_reg__,%[outBit]\n"
  "sendzero%=:\n"
    "st %a[dmxPort],__tmp_reg__\n"
    "dec %[bitCount]\n"
    "brne bitLoop%=\n"
    "sei\n"
    :
      [bitCount] "=&d" (bitCount),
      [delCount] "=&d" (delCount)
    :
      [dmxPort] "e" (dmxPort),
      [outMask] "r" (~dmxBit),
      [outBit] "r" (dmxBit),
      [delCountVal] "M" (F_CPU/1000000-3),
      [value] "r" (value)
  );
}

uint8_t dmxState;
// DMX interrupt routine
// chunking the DMX data (where possible) to optimise time usage
ISR(ISR_NAME, ISR_NOBLOCK) {
  // Prevent this interrupt running recursively
  TIMER2_INTERRUPT_DISABLE();

  uint16_t bitsLeft = BITS_PER_TIMER_TICK; // DMX Bit periods per timer tick
  bitsLeft >>=2; // 25% CPU usage
  while (1) {
    if (dmxState == 0) {
      // Next thing to send is reset pulse and start code
      // which takes 35 bit periods
      uint8_t i;
      if (bitsLeft < 35) break;
      bitsLeft-=35;
      for(i = 0 ; i < storage.getSize() ; i++) {
        *storage.getPortByIndex(i) &= ~storage.getBitByIndex(i);
      }
      for (i = 0; i < 11; i++) delayMicroseconds(8);
      for(i = 0 ; i < storage.getSize() ; i++) {
        *storage.getPortByIndex(i) |= storage.getBitByIndex(i);
      }
      delayMicroseconds(12);
      for(i = 0 ; i < storage.getSize() ; i++) {
        dmxSendByte(0, storage.getPortByIndex(i), storage.getBitByIndex(i));
      }
    } else {
      // Now send a channel which takes 11 bit periods
      if (bitsLeft < 11) break;
      bitsLeft-=11;
      for(uint8_t i = 0 ; i < storage.getSize() ; i++) {
        dmxSendByte(storage.getDMXByIndex(i)[dmxState-1], storage.getPortByIndex(i), storage.getBitByIndex(i));
      }
    }
    // Successfully completed that stage - move state machine forward
    dmxState++;
    if (dmxState > MAX_DMX) {
      dmxState = 0; // Send next frame
      break;
    }
  }

  // Enable interrupts for the next transmission chunk
  TIMER2_INTERRUPT_ENABLE();
}
