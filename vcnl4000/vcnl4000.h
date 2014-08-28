/*
mag3110.h
libary for using the I2C vcnl4000 light sensor

(c) Written by Jeroen Cappaert for NanoSatisfi, August 2012
*/

#ifndef vcnl4000_h
#define vcnl4000_h

#include <Arduino.h>

// the i2c address
#define VCNL4000_ADDRESS 0x13 // IR and Ambient Light 0x26 write, 0x27 read

// commands and constants
#define VCNL4000_COMMAND 0x80           // starts measurments, relays data ready info
#define VCNL4000_PRODUCTID 0x81         // product ID/revision ID, should read 0x11
#define VCNL4000_IRLED 0x83             // sets IR current in steps of 10mA 0-200mA
#define VCNL4000_AMBIENTPARAMETER 0x84  // Configures ambient light measures
#define VCNL4000_AMBIENTDATA_MSB 0x85   // high byte of ambient light measure
#define VCNL4000_AMBIENTDATA_LSB 0x86   // low byte of ambient light measure
#define VCNL4000_PROXIMITYDATA 0x87     // High byte of proximity measure
// #define PROXIMITY_RESULT_LSB 0x88    // low byte of proximity measure
#define VCNL4000_SIGNALFREQ 0x89        // Proximity IR test signal freq, 0-3
#define VCNL4000_PROXINITYADJUST 0x8A   // proximity modulator timing

#define VCNL4000_3M125 0
#define VCNL4000_1M5625 1
#define VCNL4000_781K25 2
#define VCNL4000_390K625 3

#define VCNL4000_MEASUREAMBIENT 0x10
#define VCNL4000_MEASUREPROXIMITY 0x08
#define VCNL4000_AMBIENTREADY 0x40
#define VCNL4000_PROXIMITYREADY 0x20


class vcnl4000
{
  public:
    vcnl4000();
    // ~vcnl4000();
    void setup(void);
    uint8_t get_freq(void);
    uint8_t set_freq(int freq);
    void write8(uint8_t address, uint8_t data);
    uint8_t read8(uint8_t address);
    uint16_t read16(uint8_t address);
    unsigned int readAmbient(void);
    uint16_t readProximity(void);

  private:

};

#endif
