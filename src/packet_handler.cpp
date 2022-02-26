#include "packet_handler.h"

PacketHandler::PacketHandler()
    : midiController(nullptr),
      timerManager(nullptr),
      buffer{0},
      messageIndex(0),
      sequenceNumber(0),
      packetWaiting(false) {
  // initialize communication and all submodules
  Serial.begin(115200);
  timerManager = new TimerManager();
  midiController = new MidiController();
  // clear the Serial buffer
  while (Serial.available()) Serial.read();
}

PacketHandler::~PacketHandler() {
  // nothing to do here
}

void PacketHandler::Update() {
  // if we're waiting for a packet to be handled, retry the packet until OK/NOK
  if (packetWaiting) {
    HandlePacket();
    return;
  }
  // if we're not waiting for a packet to be handled, monitor serial connection
  if (!Serial.available()) return;
  uint8_t c = Serial.read();
  switch (messageIndex) {
    case LOC_STX:
      // if we're pointing to the first buffer index, simply wait for STX
      if (c != STX) return;
      break;
    case LOC_SEQ:
      if (c != sequenceNumber) {
        // sequence number mismatch, send back NACK and expected sequence number
        SendReply(c, sequenceNumber);
        messageIndex = LOC_STX;
        return;
      }
      break;
    case LOC_TYPE:
      break;  // nothing to do here
    case LOC_LEN:
      break;  // nothing to do here
    default:
      // if we're pointing to the ETX location, we can handle the message
      if (messageIndex == (ETX_OFFSET + buffer[LOC_LEN])) {
        if (c != ETX) {
          // invalid message received, as ETX is not in the correct location
          SendReply(messageIndex, sequenceNumber);
          messageIndex = LOC_STX;
          return;
        }
        // add the ETX to the buffer and handle it
        buffer[messageIndex] = c;
        HandlePacket();
        messageIndex = LOC_STX;
        return;
      }
      break;
  }
  buffer[messageIndex] = c;
  messageIndex++;
}

void PacketHandler::SendReply(uint8_t result, uint8_t sequenceNum) {
  uint8_t reply[] = {STX, result, sequenceNum, ETX};
  Serial.write(reply, sizeof(reply));
  if (result != ACK) {
    Serial.write(buffer, messageIndex + 1);
    uint8_t nullarr[] = {0x00};
    Serial.write(nullarr, sizeof(nullarr));
  }
}

void PacketHandler::HandlePacket() {
  uint8_t* data = &buffer[LOC_DATA];
  uint8_t len = buffer[LOC_LEN];
  switch (buffer[LOC_TYPE]) {
    case TYPE_MIDI_START:
      HandleResult(midiController->StartStream(data, len));
      break;
    case TYPE_MIDI_MSG:
      HandleResult(midiController->HandleMessage(data, len));
      break;
    case TYPE_MIDI_END:
      HandleResult(midiController->EndStream(data, len));
      break;
    default:
      break;  // invalid packet received!
  }
}

void PacketHandler::HandleResult(PHC::PACKET_HANDLE_RESULT result) {
  switch (result) {
    case PHC::PACKET_HANDLE_RESULT::RESULT_OK:
      SendReply(ACK, sequenceNumber);
      break;
    case PHC::PACKET_HANDLE_RESULT::RESULT_NOK:
      SendReply(103, sequenceNumber);
      break;
    case PHC::PACKET_HANDLE_RESULT::RESULT_WAIT:
      packetWaiting = true;
      return;
  }
  sequenceNumber++;
  packetWaiting = false;
  memset(buffer, 0, sizeof(buffer));
}