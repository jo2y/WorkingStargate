// A motor calibration tool.
// Will move the motor forward multiples of 100 steps.
// It will repeat each multiple twice while enabling the associated
// pixel. The first pixel will be 1 * 100, the second is 2 * 100, etc.
#define NUMPIXELS 9

// Gate motor
// The number of steps in a full rotation
#define GATEMOTORSTEPS 200.0
#define GATEGEARTEETH 13.0
#define GATERINGTEETH 39.0
#define GATEGEARRATIO ( GATERINGTEETH / GATEGEARTEETH )
// For M1/M2 set to 1, for M3/M4 set to 2.
#define GATEMOTORPORT 2
#define GATESPEED 50  // RPMs
#define NUMGLYPHS 39.0
#define STEPSPERGLYPH ( GATEMOTORSTEPS * GATEGEARRATIO / NUMGLYPHS)

// Pin mapping
#define PIXELPIN 2

#include <Adafruit_NeoPixel.h>
#include <Adafruit_MotorShield.h>
// Global objects
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELPIN, NEO_RGB + NEO_KHZ800);
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *gate = AFMS.getStepper(GATEMOTORSTEPS, GATEMOTORPORT);
uint32_t off = pixels.Color(0, 0, 0);
uint32_t on = pixels.Color(0, 0, 128);

void setup() {
  pixels.begin();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, off);
  }
  pixels.show();
  AFMS.begin();
  gate->setSpeed(GATESPEED);
}

void loop() {
  for (int i = 0; i < NUMPIXELS; i++){
    pixels.setPixelColor(i, on);
    pixels.show();
    gate->step((i + 1) * 100, FORWARD, SINGLE);
    delay(2000);
    gate->step((i + 1) * 100, FORWARD, SINGLE);
    delay(2000);
    pixels.setPixelColor(i, off);
    pixels.show();
  }
  delay(1000);
}
