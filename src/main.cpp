#include <Arduino.h>

#include "midi_controller.h"
#include "packet_handler.h"
#include "timer_manager.h"
#include "utils.h"

// user pointer and initialize after Serial to prevent Serial breaky breaky
MidiController* midiController = nullptr;
TimerManager* timerManager = nullptr;
PacketHandler* packetHandler = nullptr;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(COIL_PIN, OUTPUT);
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
    packetHandler = new PacketHandler();
    Serial.begin(115200);
    timerManager = new TimerManager();
    midiController = new MidiController();
    while (Serial.available())
        Serial.read();
}

void loop() {
    // let the packet handler handle all communication
    packetHandler->update();
}
