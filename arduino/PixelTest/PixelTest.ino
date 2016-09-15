// A simple sketch to test the Neopixels while installing them.

#include <Adafruit_NeoPixel.h>
#define NUMPIXELS 9
#define PIXELPIN 2
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELPIN, NEO_RGB + NEO_KHZ800);
uint32_t blue = pixels.Color(0, 0, 128);
uint32_t red = pixels.Color(0, 128, 0);
uint32_t green = pixels.Color(128, 0, 0);
uint32_t off = pixels.Color(0, 0, 0);
uint32_t colors[] = {blue, red, green};

void setup() {
  Serial.begin(9600);
  pixels.begin();
}

void loop() {
  for (int i = 0; i < NUMPIXELS; i++) {
    Serial.print("Blinking: ");
    Serial.println(i);
    for (int j = 0; j < 3; j++){
      Serial.println("on");
      pixels.setPixelColor(i, colors[j]);
      pixels.show();
      delay(1000);
      Serial.println("off");
      pixels.setPixelColor(i, off);
      pixels.show();
      delay(1000);
    }
  }
  delay(2000);
}
