#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <EEPROM.h>

#include "main.h"

#define TOGGLE_BUTTON  8
#define MOTOR_PIN 7
#define DEBUG_LED 6
#define EEPROM_ADDRESS 0
#define READER_RESET 9 // Reset pin for card reader
#define READER_SS 10
#define DEBOUNCE_DELAY 20

Servo lockMotor;
MFRC522 reader(READER_SS, READER_RESET);
uint8_t opened = 1; // Global state of the lock

void setup() 
{
    ADCSRA = 0;
    SPI.begin();
    reader.PCD_Init();
    delay(4);
    reader.PCD_SetAntennaGain(reader.RxGain_max);
    pinMode(TOGGLE_BUTTON, INPUT);
    pinMode(DEBUG_LED, OUTPUT);
    lockMotor.attach(MOTOR_PIN);

    ADCSRA = 0; // Disable ADC

    // Initally set state if EEPROM has never been written to
    if (EEPROM.read(EEPROM_ADDRESS) == 255)
    { 
        EEPROM.write(EEPROM_ADDRESS, opened);
    }

    opened = EEPROM.read(EEPROM_ADDRESS); // Load state from memory
    set_state(opened);
}

void loop()
{
    if (check_rfid() == 0)
    {
        uint32_t key;
        for (uint8_t i = 0; i < 4; i++)
        {
            key <<= 8;
            key |= reader.uid.uidByte[i];
        }
        reader.PICC_HaltA();
        if ((key ^ 0x533780FC) == 0x0)
        {
            toggle_state();
        }
    }

    // React to toggle button being pressed
    static uint8_t state = opened;
    static uint8_t readState = 0;
    static uint8_t previousState = 0;
    static long previousDebounceTime = 0;

    // Debouncing code
    readState = digitalRead(TOGGLE_BUTTON);
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

// Check if rfid card is nearby
uint8_t check_rfid(void)
{
    if (!reader.PICC_IsNewCardPresent())
    {
        return 1;
    }

    if (!reader.PICC_ReadCardSerial())
    {
        return 1;
    }
    return 0;
}

void move_motor(void)
{
    uint8_t targetAngle = 90 * opened;
    lockMotor.write(targetAngle);
    return;
}

void set_state(uint8_t value)
{
    if (value > 1 || value < 0) 
    {
        return;
    }

    opened = value;
    EEPROM.write(EEPROM_ADDRESS, opened);
    move_motor();
    digitalWrite(DEBUG_LED, !opened);
    return;
}

void toggle_state(void)
{
    set_state((opened + 1) % 2);
    return;
}