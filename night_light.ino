
#include <Adafruit_NeoPixel.h>
#include "vcnl4000.h"
#include <Wire.h>
#include <Tween.h>

// TODO learn about sleep mode and just wake up every minute to log the light level and see if we need to turn on the Neopixels
// TODO: code that slowly changes the pixels when adjusting to brightness.

// Neopixel LED configuration
#define NEOPIN 2

#define MIN_BRIGHTNESS 60
uint8_t target_brightness = MIN_BRIGHTNESS;
uint8_t current_brightness = target_brightness;

// settings LED indicator settings
#define INDICATOR_LED 13
#define FLASH_DELAY 80
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, NEOPIN, NEO_GRB + NEO_KHZ800);
vcnl4000 sensor = vcnl4000();

// modes
// proximity sensor result when > to trigger mode change
#define PROXIMITY_TRIGGER 3000
int mode = 1;
#define TOTAL_MODES 2
// 1, rainbow wheel
// 2, max white

Tween_t brightness_chase;

void setup() {
    Serial.begin(9600);
    sensor.setup();
    strip.begin();
    strip.show();
    pinMode(INDICATOR_LED, OUTPUT);
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

    // Serial.println(ambient);
    // When bright in the room raise the brightness,
    target_brightness = constrain(map(ambient, 40, 250, MIN_BRIGHTNESS, 255), MIN_BRIGHTNESS, 255);
    // let's move to another brightness

    brightness_start_ms = millis();
    Tween_line(&brightness_chase, (float) target_brightness, (float) 1000);
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
uint16_t j;

void rainbowCycle() {
    uint16_t i;
    if(j>256){
        j=0;
    }else{
        j++;
    }
    for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
}

bool adjust_brigtness = true;

uint32_t white_color = strip.Color(255, 255, 255);

void loop() {
    switch(mode) {
        // rainbow
        case 1: {
            adjust_brigtness = true;
            rainbowCycle();
            break;
        }
        // full bright white
        case 2: {
            adjust_brigtness = false;
            current_brightness = 255;
            for (int i=0; i < strip.numPixels(); i++) {
                strip.setPixelColor(i, white_color);
            }
        }
    }

    // birhgtness adjustments and input interupt
    long current_ms = millis();
    if(current_ms - previous_time > interval) {
        previous_time = current_ms;
        ambient_adjustments();
        check_for_input();
    }

    if(adjust_brigtness){
        // move to tween
        current_brightness = (int) Tween_tick(&brightness_chase, (current_ms - brightness_start_ms));
        // current_brightness = (current_brightness / 2) + (target_brightness / 2);
        Serial.println(current_brightness);
    }
    strip.setBrightness(current_brightness);

    strip.show();
    delay(20);
}
