#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>

#include "main.h"

const uint8_t TOGGLE_BUTTON_PIN = 4;
const uint8_t INDICATOR_LED = 5;
const uint8_t MOTOR_PIN = 3;
const uint8_t EEPROM_ADDRESS = 0;
const uint8_t RX = 6; // Connect to TX pin on receiver
const uint8_t TX = 7; // Connect to RX pin on receiver
const unsigned long DEBOUNCE_DELAY = 20;

Servo lockMotor;
byte opened = 1; // Global state of the lock

void setup()
{
    pinMode(TOGGLE_BUTTON_PIN, INPUT);
    pinMode(INDICATOR_LED, OUTPUT);

    lockMotor.attach(MOTOR_PIN);

    // Initally set state if EEPROM has never been written to
    if (EEPROM.read(EEPROM_ADDRESS) == 255)
    { 
        EEPROM.write(EEPROM_ADDRESS, opened);
    }

    opened = EEPROM.read(EEPROM_ADDRESS); // Load state from memory
    move_motor();
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
    move_motor();

    // React to toggle button being pressed
    static byte state = opened;
    static byte readState = 0;
    static byte previousState = 0;
    static long previousDebounceTime = 0;

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
}

void move_motor(void)
{
    uint8_t targetAngle = 90 * opened;
    lockMotor.write(targetAngle);
    return;
}

void set_state(byte value)
{
    if (value > 1 || value < 0) 
    {
        return;
    }

    opened = value;
    EEPROM.write(EEPROM_ADDRESS, opened);
    return;
}

// Toggles state and saves to EEPROM
void toggle_state(void)
{
    set_state((opened + 1) % 2);
    return;
}