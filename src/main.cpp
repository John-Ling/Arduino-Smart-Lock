#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>

#include "main.h"

const uint8_t TOGGLE_BUTTON_PIN = 4;
const uint8_t INDICATOR_LED = 5;
const uint8_t SERVO_PIN = 3;
const uint8_t EEPROM_ADDRESS = 0;
const unsigned long DEBOUNCE_DELAY = 20;

byte opened = 1; // Global state of the lock

void setup()
{
    pinMode(TOGGLE_BUTTON_PIN, INPUT);
    pinMode(INDICATOR_LED, OUTPUT);

    if (EEPROM.read(EEPROM_ADDRESS) == 255)
    { // Initally set state if EEPROM has never been written to
        EEPROM.write(EEPROM_ADDRESS, opened);
    }

    opened = EEPROM.read(EEPROM_ADDRESS); // Load opened state from memory
    Serial.begin(9600);
}

void loop()
{
    if (opened == 1)
    {
        digitalWrite(INDICATOR_LED, HIGH);
    }
    else
    {
        digitalWrite(INDICATOR_LED, LOW);
    }

    #pragma region Toggle Button Code
    byte state = opened;
    static byte readState = 0;
    static byte previousState = 0;
    static long previousDebounceTime = 0;
    // React to toggle button being pressed

    // Debouncing code
    readState = digitalRead(TOGGLE_BUTTON_PIN);
    if (readState != previousState)
    {
        previousDebounceTime = millis();
    }

    if ((millis() - previousDebounceTime) <= DEBOUNCE_DELAY)
    {
        previousState = readState;
        return;
    }

    if (readState == state)
    {
        previousState = readState;
        return;
    }

    // Actual code that changes state
    state = readState;
    if (readState == 0)
    {
        toggle_state();
    }

    previousState = readState;
    #pragma endregion
}

// Moves servo, updates variable opened and saves state to EEPROM
void toggle_state(void)
{
    opened = (opened + 1) % 2;
    EEPROM.write(EEPROM_ADDRESS, opened);
}
