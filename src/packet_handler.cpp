#include "packet_handler.h"

#include "utils.h"

PacketHandler::PacketHandler()
    : midiController(nullptr), timerManager(nullptr), buffer{0}, messageIndex(0), sequenceNumber(0),
      packetWaiting(false) {
    // initialize communication and all submodules
    Serial.begin(115200);
    timerManager = new TimerManager();
    midiController = new MidiController();
    // clear the Serial buffer
    while (Serial.available())
        Serial.read();
}

PacketHandler::~PacketHandler() {
    // nothing to do here
}

void PacketHandler::update() {
    // if we're waiting for a packet to be handled, retry the packet until OK/NOK
    if (packetWaiting) {
        handlePacket();
        return;
    }
    // if we're not waiting for a packet to be handled, monitor serial connection
    if (!Serial.available())
        return;
    uint8_t c = Serial.read();
    switch (messageIndex) {
    case LOC_STX:
        // if we're pointing to the first buffer index, simply wait for STX
        if (c != STX)
            return;
        break;
    case LOC_SEQ:
        if (c != sequenceNumber) {
            // sequence number mismatch, send back NACK and expected sequence number
            sendReply(NACK, sequenceNumber);
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
                sendReply(NACK, sequenceNumber);
                messageIndex = LOC_STX;
                return;
            }
            // add the ETX to the buffer and handle it
            buffer[messageIndex] = c;
            handlePacket();
            messageIndex = LOC_STX;
            return;
        }
        break;
    }
    buffer[messageIndex] = c;
    messageIndex++;
}

void PacketHandler::sendReply(uint8_t result, uint8_t sequenceNum) {
    uint8_t reply[] = {STX, result, sequenceNum, ETX};
    Serial.write(reply, sizeof(reply));
    if (result != ACK) {
        Serial.write(buffer, messageIndex + 1);
        uint8_t nullarr[] = {0x00};
        Serial.write(nullarr, sizeof(nullarr));
    }
}

void PacketHandler::handlePacket() {
    uint8_t* data = &buffer[LOC_DATA];
    uint8_t len = buffer[LOC_LEN];
    debugprint("handle message: ");
    debugprint(buffer[LOC_TYPE]);
    switch (buffer[LOC_TYPE]) {
    case TYPE_MIDI_START:
        handleResult(midiController->startStream(data, len));
        break;
    case TYPE_MIDI_MSG:
        handleResult(midiController->handleMessage(data, len));
        break;
    case TYPE_MIDI_END:
        handleResult(midiController->endStream(data, len));
        break;
    case TYPE_RESET:
        handleResult(performReset());
        break;
    default:
        break;  // invalid packet received!
    }
}

void PacketHandler::handleResult(PHC::PACKET_HANDLE_RESULT result) {
    switch (result) {
    case PHC::PACKET_HANDLE_RESULT::RESULT_OK:
        sendReply(ACK, sequenceNumber);
        break;
    case PHC::PACKET_HANDLE_RESULT::RESULT_NOK:
        sendReply(NACK, sequenceNumber);
        break;
    case PHC::PACKET_HANDLE_RESULT::RESULT_WAIT:
        packetWaiting = true;
        return;
    }
    sequenceNumber++;
    packetWaiting = false;
    memset(buffer, 0, sizeof(buffer));
}

PHC::PACKET_HANDLE_RESULT PacketHandler::performReset() {
    using PHR = PHC::PACKET_HANDLE_RESULT;
    bool result = true;
    result &= midiController->performReset() == PHR::RESULT_OK;
    result &= timerManager->performReset() == PHR::RESULT_OK;
    return result ? PHR::RESULT_OK : PHR::RESULT_NOK;
}