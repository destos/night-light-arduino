/*
mag3110.cpp
libary for using the I2C  vcnl4000 light sensor

(c) Written by Jeroen Cappaert for NanoSatisfi, August 2012
*/

#include <Arduino.h>
#include "vcnl4000.h"
#include <Wire.h>

vcnl4000::vcnl4000() {}

void vcnl4000::setup(void) {
    // uint8_t rev = read8(VCNL4000_PRODUCTID);
    // if ((rev & 0xF0) == 0x10) {
    // }
    Wire.begin();

    // set to 20 * 10mA = 200mA
    write8(VCNL4000_IRLED, 20);
    // float ma = read8(VCNL4000_IRLED) * 10, DEC;

    write8(VCNL4000_PROXINITYADJUST, 0x81);

    // arrange for continuous conversion
    //write8(VCNL4000_AMBIENTPARAMETER, 0x89);
}

// get operating frequency
uint8_t vcnl4000::get_freq(void) {
    uint8_t freq = read8(VCNL4000_SIGNALFREQ);
    return freq;
    // if (freq == VCNL4000_3M125) Serial.println("3.125 MHz");
    // if (freq == VCNL4000_1M5625) Serial.println("1.5625 MHz");
    // if (freq == VCNL4000_781K25) Serial.println("781.25 KHz");
    // if (freq == VCNL4000_390K625) Serial.println("390.625 KHz");
}

// set frequenct
void vcnl4000::set_freq(int freq) {
    write8(VCNL4000_SIGNALFREQ, freq);
}

// configure sensor
// void vcnl4000::configAmb(void) {
//   writeByte(AMB_ADDR, AMBIENT_PARAMETER, 0x0F); // Single conversion mode, 128 averages
//   writeByte(AMB_ADDR, IR_CURRENT, 20);          // Set IR current to 200mA
//   writeByte(AMB_ADDR, PROXIMITY_FREQ, 2);       // 781.25 kHz
//   writeByte(AMB_ADDR, PROXIMITY_MOD, 0x81);     // 129, recommended by Vishay
// }

// read ambient light values
unsigned int vcnl4000::readAmbient(void)
{
    unsigned int data;
    uint8_t temp;

    //trigger the ambient light sensor
    temp = read8(VCNL4000_COMMAND);

    // command the sensor to perform ambient measure
    write8(VCNL4000_COMMAND, temp | 0x10);

    // wait for the proximity data ready bit to be set
    while(!(read8(VCNL4000_COMMAND) & 0x40));

    //read value from 2 bytes
    uint8_t amb_MSB = read8(VCNL4000_AMBIENTDATA_MSB);
    uint8_t amb_LSB = read8(VCNL4000_AMBIENTDATA_LSB);

    //combine the bytes
    data = ( amb_LSB | (amb_MSB << 8) );

    return data;
}

uint16_t vcnl4000::readProximity(void) {
    write8(VCNL4000_COMMAND, VCNL4000_MEASUREPROXIMITY);
    while (1) {
        uint8_t result = read8(VCNL4000_COMMAND);
        //Serial.print("Ready = 0x"); Serial.println(result, HEX);
        if (result & VCNL4000_PROXIMITYREADY) {
            return read16(VCNL4000_PROXIMITYDATA);
        }
        delay(1);
    }
}


// Read 1 byte from the VCNL4000 at 'address'
uint8_t vcnl4000::read8(uint8_t address)
{
    uint8_t data;

    Wire.beginTransmission(VCNL4000_ADDRESS);
    Wire.write(address);
    Wire.endTransmission();

    delayMicroseconds(170);  // delay required

    Wire.requestFrom(VCNL4000_ADDRESS, 1);
    while(!Wire.available());

    return Wire.read();
}


// Read 2 byte from the VCNL4000 at 'address'
uint16_t vcnl4000::read16(uint8_t address)
{
    uint16_t data;

    Wire.beginTransmission(VCNL4000_ADDRESS);
    Wire.write(address);
    Wire.endTransmission();

    Wire.requestFrom(VCNL4000_ADDRESS, 2);
    while(!Wire.available());
    data = Wire.read();
    data <<= 8;
    while(!Wire.available());
    data |= Wire.read();

    return data;
}

// write 1 byte
void vcnl4000::write8(uint8_t address, uint8_t data)
{
    Wire.beginTransmission(VCNL4000_ADDRESS);
    Wire.write(address);
    Wire.write(data);
    Wire.endTransmission();
}
