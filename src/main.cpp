#include <Arduino.h>

#include "midi_controller.h"
#include "packet_handler.h"
#include "timer_manager.h"
#include "utils.h"

PacketHandler* packet_handler = nullptr;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(COIL_PIN, OUTPUT);
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
    // ensure the I/O is setup before initializing the packet handler
    packet_handler = new PacketHandler();
}

void loop() {
    // let the packet handler handle all communication
    packet_handler->update();
}
