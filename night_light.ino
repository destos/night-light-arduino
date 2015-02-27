#include <SPI.h>
#include <Tween.h>
#include <Adafruit_NeoPixel.h>
#include <vcnl4000.h>
#include <MySensor.h>
#include <Wire.h>

// TODO learn about sleep mode and just wake up every minute to log the light level and see if we need to turn on the Neopixels
// TODO: code that slowly changes the pixels when adjusting to brightness.

const uint16_t SinuCycle[] PROGMEM = {
    0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   2,   2,   2,
    3,   3,   4,   4,   4,   4,   5,   5,   6,   7,   7,   8,   8,   8,   9,  10,
   11,  11,  12,  13,  13,  15,  15,  17,  19,  20,  22,  24,  28,  31,  34,  38,
   43,  48,  54,  60,  66,  73,  83,  91,  99, 108, 118, 130, 141, 153, 165, 177,
  194, 208, 222, 238, 254, 275, 292, 311, 330, 350, 371, 398, 420, 443, 468, 493,
  519, 546, 566, 595, 617, 639, 662, 678, 694, 718, 726, 743, 751, 768, 777, 794,
  803, 821, 830, 839, 848, 867, 876, 885, 895, 904, 914, 924, 933, 943, 953, 963,
  963, 973, 983, 983, 993, 993,1003,1003,1014,1014,1014,1023,1023,1023,1023,1023,
 1023,1023,1023,1023,1023,1014,1014,1014,1003,1003, 993, 993, 983, 983, 973, 963,
  963, 953, 943, 933, 924, 914, 904, 895, 885, 876, 867, 848, 839, 830, 821, 803,
  794, 777, 768, 751, 743, 726, 718, 694, 678, 662, 639, 617, 595, 566, 546, 519,
  493, 468, 443, 420, 398, 371, 350, 330, 311, 292, 275, 254, 238, 222, 208, 194,
  177, 165, 153, 141, 130, 118, 108,  99,  91,  83,  73,  66,  60,  54,  48,  43,
   38,  34,  31,  28,  24,  22,  20,  19,  17,  15,  15,  13,  13,  12,  11,  11,
   10,   9,   8,   8,   8,   7,   7,   6,   5,   5,   4,   4,   4,   4,   3,   3,
    2,   2,   2,   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0
};

// Temperature sensor pins
#define TMP_GRND 5
#define TMP_V 6
#define TMP_READ 0
int temp_sensor = 0;

// Neopixel LED configuration
#define NEOPIN 3
#define NUMPIXELS 12

#define MIN_BRIGHTNESS 60
uint8_t target_brightness = MIN_BRIGHTNESS;
uint8_t current_brightness = target_brightness;

// settings LED indicator settings
#define INDICATOR_LED 13
#define FLASH_DELAY 80
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);
vcnl4000 sensor = vcnl4000();

// modes
// proximity sensor result when > to trigger mode change
#define PROXIMITY_TRIGGER 3000
int mode = 1;
#define TOTAL_MODES 6
// 1, rainbow wheel
// 2, max white

Tween_t brightness_chase;

// MySensor Constants
#define CHILD_ID_LIGHT 0
#define CHILD_ID_TEMP 1

MySensor gw;
MyMessage light_msg(CHILD_ID_LIGHT, V_LIGHT_LEVEL);
MyMessage temp_msg(CHILD_ID_TEMP, V_TEMP);

void setup() {
    // Setup MySensor settings
    gw.begin();
    gw.sendSketchInfo("Night Light", "0.1");
    // Register all sensors to gateway (they will be created as child devices)
    // Register light sensor
    gw.present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
    // Register temperature sensor
    gw.present(CHILD_ID_TEMP, S_TEMP);

    // setup vcnl
    sensor.setup();

    // start neopixel strip
    strip.begin();
    strip.show();

    pinMode(INDICATOR_LED, OUTPUT);

#ifdef TMP_GRND
    // use digital pins to power the temperature sensor
    pinMode(TMP_GRND, OUTPUT);
    digitalWrite(TMP_GRND, LOW);
    pinMode(TMP_V, OUTPUT);
    digitalWrite(TMP_V, HIGH);
#endif
    Tween_setType(&brightness_chase, TWEEN_EASEINOUTCUBIC);
}

// used to fire intermitent functions
long previous_time = 0;
long interval = 900;
// long ambient_interval = 5000;


long brightness_start_ms;

void ambient_adjustments() {
    int ambient = sensor.readAmbient();
    // TODO, log previous brightness log and if their
    // average is above a certain amount turn off NEOPixels

    gw.send(light_msg.set(ambient));
    // Serial.println(ambient);
    // When bright in the room raise the brightness,
    target_brightness = constrain(map(ambient, 40, 250, MIN_BRIGHTNESS, 255), MIN_BRIGHTNESS, 255);
    // let's move to another brightness

    brightness_start_ms = millis();
    Tween_line(&brightness_chase, (float) target_brightness, (float) 1000);
}

void check_temperature(){
    temp_sensor = analogRead(TMP_READ);
    float voltage  = 0;
    float celsius  = 0;
    voltage = ( temp_sensor * 5000 ) / 1024; // convert raw sensor value to millivolts
    voltage = voltage - 500;
    celsius = voltage / 10;
    Serial.println(celsius);
    gw.send(temp_msg.set(celsius, 2));
}

void flash_mode(int mode) {
    for(int i=0; i<mode; i++){
        delay(FLASH_DELAY);
        digitalWrite(INDICATOR_LED, HIGH);
        delay(FLASH_DELAY);
        digitalWrite(INDICATOR_LED, LOW);
    }
}

// things to check every second
void check_for_input() {
    uint16_t prox = sensor.readProximity();
    Serial.println(prox);
    if(prox > PROXIMITY_TRIGGER){
       mode++;
       if(mode > TOTAL_MODES){
            mode = 1;
       }
       Serial.print("mode changed to:");
       Serial.println(mode);
       flash_mode(mode);
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
    if(WheelPos < 85) {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}

// used for fade loops
// uint16_t j;

// void rainbowCycle() {
//     uint16_t i;
//     if(j>256){
//         j=0;
//     }else{
//         j++;
//     }
//     for(i=0; i< strip.numPixels(); i++) {
//         strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
//     }
// }

bool adjust_brigtness = true;

uint32_t white_color = strip.Color(255, 255, 255);

void loop() {
    // Process incoming messages (like config from server)
    gw.process();
    switch(mode) {
        case 1:
            adjust_brigtness = true;
            spinning_rainbow(10*1000/256);
            break;
        // full bright white
        case 2:
            adjust_brigtness = false;
            current_brightness = 255;
            for (int i=0; i < strip.numPixels(); i++) {
                strip.setPixelColor(i, white_color);
            }
        case 3:
            adjust_brigtness = true;
            solid_color_breathe(42, 25);
            break;
        case 4:
            adjust_brigtness = true;
            solid_color_breathe(116, 25);
            break;
        case 5:
            adjust_brigtness = true;
            solid_color_breathe(221, 25);
            break;
        case 6:
            adjust_brigtness = true;
            candle_flicker(random(50,200));
            break;
        default:
            adjust_brigtness = true;
            for(uint8_t n = 0; n<NUMPIXELS; n++) strip.setPixelColor(n,20,20,20);
            break;
        // fade between all colors
        // case 3: {
        //     adjust_brigtness = true

        // }
    }

    // birhgtness adjustments and input interupt
    long current_ms = millis();
    if(current_ms - previous_time > interval) {
        previous_time = current_ms;
        ambient_adjustments();
        check_for_input();
        check_temperature();
    }

    if(adjust_brigtness){
        // move to tween
        current_brightness = (int) Tween_tick(&brightness_chase, (current_ms - brightness_start_ms));
        // current_brightness = (current_brightness / 2) + (target_brightness / 2);
        Serial.println(current_brightness);
    }
    strip.setBrightness(current_brightness);

    Serial.println('spiny');
    strip.show();
    // delay(20);
    gw.sleep(20);
}

void spinning_rainbow(uint8_t framelength){
    static uint8_t cyclepos = 0;
    static uint8_t ditherlev = 0;
    static unsigned long int periodstart=millis();

    uint8_t pixelOffsets[] = {0, 64, 128, 192};
    uint8_t colorOffsets[] = {0, 85, 170};

    uint16_t r,g,b;

    for(uint8_t n = 0; n<NUMPIXELS; n++){
        r=pgm_read_word(&SinuCycle[(cyclepos+pixelOffsets[n]+colorOffsets[0])%256]);
        g=pgm_read_word(&SinuCycle[(cyclepos+pixelOffsets[n]+colorOffsets[1])%256]);
        b=pgm_read_word(&SinuCycle[(cyclepos+pixelOffsets[n]+colorOffsets[2])%256]);
        r=r/NUMPIXELS+((ditherlev<(r%NUMPIXELS))?1:0);
        g=g/NUMPIXELS+((ditherlev<(g%NUMPIXELS))?1:0);
        b=b/NUMPIXELS+((ditherlev<(b%NUMPIXELS))?1:0);
        if(r>255)r=255;
        if(g>255)g=255;
        if(b>255)b=255;
        strip.setPixelColor(n,(uint8_t)r,(uint8_t)g,(uint8_t)b);
    }

    if (ditherlev == 3) ditherlev = 0;
    else ditherlev++;

    if(millis() > periodstart+framelength){
        cyclepos++;
        periodstart=millis();
    }
}

void candle_flicker(uint8_t framelength){
    static int level = 1000;
    static int fuel = 1000;
    static int oxygen = random(3,7);
    static unsigned long int periodstart=millis();

    if(millis() > periodstart+framelength){
        oxygen = random(3,7);
        periodstart = millis();
    }

    level += (fuel*oxygen-level/2)/4000;
    level -= 1;
    fuel -= level/3000;
    fuel += 1;

    if(level<400)level=400;
    if(level>(127*120))level=(127*120);

    for(uint8_t n = 0; n<NUMPIXELS; n++){
        int bright = level;
//TODO: finish
        switch(n%3){
          case 2:
            break;
        }
        if(n==2) bright+=(oxygen-5)*200;
        else if(n==0) bright-=(oxygen-5)*200;
        else if(n==1) bright+=(abs(oxygen-5)*200 - 400);
        strip.setPixelColor(n,bright/120+3,bright/160,bright/400-((bright<800)?0:2));
    }
}

void solid_color_breathe(unsigned int hue, unsigned int framelength){
    static uint8_t cyclepos = 0;
    static uint8_t ditherlev = 0;
    static unsigned long int periodstart=millis();

    uint8_t pixelOffsets[] = {10, 60, 50, 0};

    unsigned long R,G,B;
    uint16_t r,g,b;
    R = pgm_read_word(&SinuCycle[(hue+128)%256]);
    G = pgm_read_word(&SinuCycle[(hue+42)%256]);
    B = pgm_read_word(&SinuCycle[(hue+212)%256]);

    for(uint8_t n = 0; n<NUMPIXELS; n++){
        r=pgm_read_word(&SinuCycle[(cyclepos+pixelOffsets[n])%256]);
        g=pgm_read_word(&SinuCycle[(cyclepos+pixelOffsets[n])%256]);
        b=pgm_read_word(&SinuCycle[(cyclepos+pixelOffsets[n])%256]);
        R=R*(683+(r+1)/3)/1024;//+((ditherlev<(r%4))?1:0);
        G=G*(683+(g+1)/3)/1024;//+((ditherlev<(g%4))?1:0);
        B=B*(683+(b+1)/3)/1024;//+((ditherlev<(b%4))?1:0);
        if(R>255)R=255;
        if(G>255)G=255;
        if(B>255)B=255;
        strip.setPixelColor(n,(uint8_t)R,(uint8_t)G,(uint8_t)B);
    }

    if (ditherlev == 3) ditherlev = 0;
    else ditherlev++;

    if(millis() > periodstart+framelength){
        cyclepos++;
        periodstart=millis();
    }
}
