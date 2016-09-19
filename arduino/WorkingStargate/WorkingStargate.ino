// Configurable values
//
// LED and Pixel Related Settings
//
// How long should the locking chevron be engaged?
#define LOCK_DELAY 1500  // in milliseconds

//
// Arduino Pin Mappings
//
// This section defines which arduino pin connects to which electronic device.
// The data pin for the chain of NeoPixels around the gate.
#define GATE_PIXELS 5

// DHD buttons
#define DHD_OUTER A2  // Serial pin 4
#define DHD_INNER A3  // Serial pin 5
// The data pin for the chain of NeoPixels around the DHD.
#define DHD_PIXELS 8

// Calibration LDR and LED
#define CAL_LDR A1
#define CAL_LED 10

// These are the pins used for the music maker shield. If you're using the
// adafruit mp3 shield, leave as-is.
#define AUDIO_SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define AUDIO_SHIELD_CS 7      // VS1053 chip select pin (output)
#define AUDIO_SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define AUDIO_CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define AUDIO_DREQ 3           // VS1053 Data request, ideally an Interrupt pin
#define AUDIO_VOLUME A0
// MINVOL/MAXVOL should be (0, 254) with 0 being the loudest and 254 as silence.
#define MINVOL 90
#define MAXVOL 0

// Unusable pins.
// These pins have double duty. Annotating them so the dual role is not
// forgotten.
#define RX 0
#define TX 1
#define MOSI 11
#define MISO 12
#define SCK 13
#define SDA A4
#define SCL A5

//
// Stepper Motor Settings
//
// The Glyph motor moves the ring with the symbols and the Chevron motor causes
// the top chevron to move.
// If you are using adafruit stepper motors, 200.0 is likely the correct value.
// If your motor doesn't list the number of steps, look for a degrees per step
// value. Divide 360 / degrees to get the value for here. (Adafruit motors are
// 1.8 degrees per step.
#define GLYPH_MOTOR_STEPS 200.0
#define CHEVRON_MOTOR_STEPS 200.0
// The port for the adafruit motor shield. If the stepper motor is connected to
// M1/M2 set 1, for M3/M4 set 2.
#define GLYPH_MOTOR_PORT 1
#define CHEVRON_MOTOR_PORT 2
// How fast should the motors be moved? The values here are likely suitable.
#define GLYPH_MOTOR_SPEED 10  // RPMs
#define CHEVRON_MOTOR_SPEED 10  // RPMs

//
// Audio Settings
//
// The SOUND_* defines specify files on the SD card that should be played at
// certain times in a dialing sequence.
#define SOUND_CHEVRON_LOCK "CHEVRON.MP3"
#define SOUND_LONG_SPIN "ROLL.MP3"
#define SOUND_SHORT_SPIN "ROLL.MP3"
#define SOUND_WORMHOLE_CLOSING "CLOSE.MP3"
#define SOUND_WORMHOLE_OPENING "OPEN.MP3"
#define SOUND_WORMHOLE_OPEN "WORMHOLE.MP3"

//
// Debugging Settings
//
// Set DEBUG to 1 to enable Serial output, 0 to disable
#define DEBUG 1

#if DEBUG
#define debug(line) Serial.print(line)
#define debugln(line) Serial.println(line)
#else
#define debug(line)
#define debugln(line)
#endif

//
// STL Design Settings.
// Values related to the printed models. These would only change if the STL
// files were redesigned.
#define NUM_GLYPHS 39.0
#define GLYPH_GEAR_TEETH 13.0
#define GLYPH_RING_TEETH 78.0
#define GLYPH_GEAR_RATIO ( GLYPH_RING_TEETH / GLYPH_GEAR_TEETH )
#define GLYPH_RING_STEPS ( GLYPH_MOTOR_STEPS * GLYPH_GEAR_RATIO )
#define STEPS_PER_GLYPH ( GLYPH_RING_STEPS / NUM_GLYPHS)
#define NUM_PIXELS 9
#define TOP_PIXEL 4
#define LOCK_STEPS 10

// No user servicable values below here.
#include <SD.h>
#include <Adafruit_MCP23008.h>
#include <Adafruit_MotorShield.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_VS1053.h>

// Global objects
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(
    NUM_PIXELS, GATE_PIXELS, NEO_RGB + NEO_KHZ800);
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *SMGlyph = AFMS.getStepper(
    GLYPH_MOTOR_STEPS, GLYPH_MOTOR_PORT);
Adafruit_StepperMotor *SMChevron = AFMS.getStepper(
    CHEVRON_MOTOR_STEPS, CHEVRON_MOTOR_PORT);
Adafruit_VS1053_FilePlayer audio = Adafruit_VS1053_FilePlayer(
    AUDIO_SHIELD_RESET, AUDIO_SHIELD_CS, AUDIO_SHIELD_DCS,
    AUDIO_DREQ, AUDIO_CARDCS);
Adafruit_MCP23008 extra_pins = Adafruit_MCP23008();

// Colors
uint32_t off = pixels.Color(0, 0, 0);
uint32_t on = pixels.Color(0, 255, 0);

// The number of motor steps from the Earth glyph to each
// list of glyphs in clockwise order.
// http://rdanderson.com/stargate/glyphs/glyphs.htm
// Explicitly declare the array size to ensure NUM_GLYPHS matches contents.
float GLYPHS[(int) NUM_GLYPHS] = {
  0, // Earth
  1 * STEPS_PER_GLYPH, // Crater
  2 * STEPS_PER_GLYPH, // Virgo
  3 * STEPS_PER_GLYPH, // Bootes
  4 * STEPS_PER_GLYPH, // Centaurus
  5 * STEPS_PER_GLYPH, // Libra
  6 * STEPS_PER_GLYPH, // Serpens Caput
  7 * STEPS_PER_GLYPH, // Norma
  8 * STEPS_PER_GLYPH, // Scorpio
  9 * STEPS_PER_GLYPH, // Cra
  10 * STEPS_PER_GLYPH, // Scutum
  11 * STEPS_PER_GLYPH, // Sagittarius
  12 * STEPS_PER_GLYPH, // Aquila
  13 * STEPS_PER_GLYPH, // Mic
  14 * STEPS_PER_GLYPH, // Capricon
  15 * STEPS_PER_GLYPH, // Pisces Austrinus
  16 * STEPS_PER_GLYPH, // Equuleus
  17 * STEPS_PER_GLYPH, // Aquarius
  18 * STEPS_PER_GLYPH, // Pegasus
  19 * STEPS_PER_GLYPH, // Sculptor
  20 * STEPS_PER_GLYPH, // Pisces
  21 * STEPS_PER_GLYPH, // Andromeda
  22 * STEPS_PER_GLYPH, // Triangulum
  23 * STEPS_PER_GLYPH, // Aries
  24 * STEPS_PER_GLYPH, // Perseus
  25 * STEPS_PER_GLYPH, // Cetus
  26 * STEPS_PER_GLYPH, // Taurus
  27 * STEPS_PER_GLYPH, // Auriga
  28 * STEPS_PER_GLYPH, // Eridanus
  29 * STEPS_PER_GLYPH, // Orion
  30 * STEPS_PER_GLYPH, // Canis Minor
  31 * STEPS_PER_GLYPH, // Monoceros
  32 * STEPS_PER_GLYPH, // Gemini
  33 * STEPS_PER_GLYPH, // Hydra
  34 * STEPS_PER_GLYPH, // Lynx
  35 * STEPS_PER_GLYPH, // Cancer
  36 * STEPS_PER_GLYPH, // Sextans
  37 * STEPS_PER_GLYPH, // Leo Minor
  38 * STEPS_PER_GLYPH, // Leo
};

// List of addresses provided by
// http://www.rdanderson.com/stargate/glyphs/glyphs.htm
// Addresses with missing symbols were skipped.
int milky_addresses[][6] = {
  {27, 7, 15, 32, 12, 30}, // Abydos
  {20, 18, 11, 38, 10, 32}, // Apophis' Base
  {20, 2, 35, 8, 26, 15}, // Camelot
  {29, 3, 6, 9, 12, 16}, // Castiana
  {29, 18, 19, 20, 21, 22}, // Sahal
  {9, 2, 23, 15, 37, 20}, // Chulak
  {14, 21, 16, 11, 30, 7}, // Clava Thesara Infinitas
  {26, 20, 35, 32, 23, 4}, // Clava Thesara Infinitas (again?)
  {3, 32, 16, 8, 10, 12}, // Destroyers
  {28, 38, 35, 9, 15, 3}, // Edora
  {30, 37, 9, 7, 18, 16}, // Euronda
  {29, 8, 18, 22, 4, 25}, // Juna
  {6, 16, 8, 3, 26, 25}, // Kallana
  {26, 35, 6, 8, 23, 14}, // Kheb
  {18, 2, 30, 12, 26, 33}, // K'Tau
  {24, 12, 32, 7, 11, 34}, // Martin's Homeworld (destroyed)
  {38, 28, 15, 35, 3, 19}, // NID Off-world Base
  {28, 8, 16, 33, 13, 31}, // P2X-555
  {38, 9, 28, 15, 35, 3}, // P34-353J
  {19, 8, 4, 37, 26, 16}, // P3W-451 (Black hole planet)
  {9, 26, 34, 27, 17, 21}, // P3X-118
  {3, 28, 9, 35, 24, 15}, // P3X-562
  {25, 8, 18, 29, 4, 22}, // P9C-372
  {12, 36, 23, 18, 7, 27}, // PB5-926
  {35, 3, 31, 29, 5, 17}, // Praclarush Taonas
  {30, 19, 34, 9, 33, 18}, // Sangreal Planet
  {33, 28, 23, 26, 16, 21}, // Tartarus
  {6, 33, 27, 37, 11, 18}, // Tollan (destroyed)
  {4, 29, 8, 22, 18, 25}, // Tollana
  {3, 8, 2, 12, 19, 30}, // Vagon Brei
  {9, 18, 27, 15, 21, 36}, // Unnammed Planet from Unending
};

// Special gate addresses.
int earth[] = {28, 26, 5, 36, 11, 29};
int othana[] = {11, 27, 23, 16, 33, 3, 9};

// Pixel related functions

void initPixels() {
  pixels.begin();
  pixelsOff();
}

void lockChevron(int chevron) {
  pixels.setPixelColor(TOP_PIXEL, on);
  pixels.show();
  debugln(F("Lowering Top Chevron."));
  startSound(SOUND_CHEVRON_LOCK);
  
  SMChevron->step(LOCK_STEPS, BACKWARD, SINGLE);
  delay(LOCK_DELAY);
  waitSound();
  debugln(F("Raising Top Chevron."));
  SMChevron->step(LOCK_STEPS, FORWARD, SINGLE);
  pixels.setPixelColor(TOP_PIXEL, off);
  pixels.setPixelColor(chevron, on);
  pixels.show();
}

void pixelsOff() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, off);
  }
  pixels.show();
}

// Gate Movement functions

void initMotors() {
  AFMS.begin();
  SMGlyph->setSpeed(GLYPH_MOTOR_SPEED);
  SMChevron->setSpeed(CHEVRON_MOTOR_SPEED);
  debugln(F("Raising Top Chevron."));
  SMChevron->step(10, FORWARD, SINGLE);
}

void calibrateGlyphs() {
  debugln(F("Starting Glyph Calibration."));
  // Increase the motor speed to calibrate quickly.
  SMGlyph->setSpeed(max(GLYPH_MOTOR_SPEED, 60));
  pinMode(CAL_LDR, INPUT_PULLUP);
  pinMode(CAL_LED, OUTPUT);
  digitalWrite(CAL_LED, LOW);
  float setpoint = 1024.0;
  unsigned int readings = 0;
  for (int i = 0; i < 10; ++i) {
    readings += analogRead(CAL_LDR);
    SMGlyph->step(1, BACKWARD, MICROSTEP);
  }
  setpoint = (readings / 10.0) * 0.5;
  debug(F("Setpoint: "));
  debugln(setpoint);
  digitalWrite(CAL_LED, HIGH);
  while (analogRead(CAL_LDR) > setpoint) {
    SMGlyph->step(1, FORWARD, MICROSTEP);
  }
  digitalWrite(CAL_LED, LOW);
  SMGlyph->release();
  SMGlyph->setSpeed(GLYPH_MOTOR_SPEED);
  debugln(F("Finishing Calibration."));
}

void rotateGate(int glyph, int chevron) {
  // Assume the gate starts with Earth at the top.
  static float curr_glyph = GLYPHS[0];
  int direction = (chevron % 2) ? FORWARD : BACKWARD;
  int steps = 0;

  // Bounds checking before we access GLYPHS.
  glyph = constrain(glyph, 0, NUM_GLYPHS);

  int offset = GLYPHS[glyph];
  if (direction == FORWARD) {
    steps = offset - curr_glyph;
  } else if (direction == BACKWARD) {
    steps = curr_glyph - offset;
  }
  // Keep the rotation within a single rotation of the gate.
  while (steps < 0) {
    steps += GLYPH_RING_STEPS;
  }
  while (steps > GLYPH_RING_STEPS) {
    steps -= GLYPH_RING_STEPS;
  }

  debug(F("Moving "));
  debug(steps);
  debug(F(" steps "));
  debug((direction == FORWARD) ? F("Forward") : F("Backward"));
  debug(F(" to glyph "));
  debug(glyph);
  debug(F(" for chevron "));
  debugln(chevron);
  startSound(SOUND_SHORT_SPIN);
  // TODO(jo2y): Update rotate to take into account partial steps.
  SMGlyph->step((int) steps, direction, SINGLE);
  stopSound(3000);
  curr_glyph = GLYPHS[glyph];
  lockChevron(chevron);
}

void initAudio() {
  debugln(F("Starting Audio."));
  if (!audio.begin()) {
    debugln(F("Audio shield did not initialize."));
#if DEBUG
    audio.dumpRegs();
#endif
  }
  // Set volume multiple times to quickly narrow in on the set value.
  // This is to overcome the dampening that happens normally.
  setVolume(); setVolume(); setVolume(); setVolume(); setVolume();
  audio.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
}

void setVolume() {
  static int oldvol = int((MINVOL + MAXVOL) / 2);
  int vol = analogRead(AUDIO_VOLUME);
  // analogRead will return (0, 1023) and setVolume takes (0, 254), but values
  // larger than 90 seem too quiet to hear, so scale (0, 1024) to (0, 90).
  // 0 is full volume and 254 is silence.
  // 255 will activate analog shutdown mode.
  vol = map(vol, 0, 1023, MINVOL, MAXVOL);
  // Sometimes we get noise from the ADC, so constrain the jumps in volume
  // to something sane. If the volume truly was changed a lot, we will gradually
  // converge on the new setting.
  vol = constrain(vol, oldvol - 3, oldvol + 3);
  if (oldvol != vol) {
    if (abs(oldvol - vol) > 2) {
      // Avoid spamming serial if the volume is fluctuating by a small amount
      // due to electrical noise.
      debug(F("Changing volume to: "));
      debugln(vol);
    }
    audio.setVolume(vol, vol);
    oldvol = vol;
  }
}

void startSound(const char *file) {
  debug(F("Playing file: "));
  debugln(file);
  setVolume();
  if (!audio.startPlayingFile(file)) {
    debug(F("Could not open file: "));
    debugln(file);
  }
}

void waitSound() {
  debugln(F("Waiting for track to end."));
  while(audio.playingMusic) {
    setVolume();
    delay(100);
  }
}

void stopSound(int sleep) {
  debugln(F("Interrupting audio."));
  delay(sleep);
  audio.stopPlaying();
}

void initSD() {
  debugln(F("Initializing SD Card."));
  SD.begin(AUDIO_CARDCS);
}

void initExtraPins() {
  debugln(F("Initializing the MCP23008."));
  extra_pins.begin();
  for (int x = 0; x < 8; ++x) {
    extra_pins.pinMode(x, INPUT);
    extra_pins.pullUp(x, HIGH);
  }
}

void randomDialing() {
  static int chevron = 5;
  debugln(F("Starting dialing sequence."));
  debug(F("Locking in Chevron "));
  debugln(chevron);

  if (chevron == 5) {
    delay(3000);
    pixelsOff();
  }
  rotateGate(random(NUM_GLYPHS), chevron);
  if (chevron == 4) {
    // The last chevron was locked, so open the gate.
    startSound(SOUND_WORMHOLE_OPENING);
    waitSound();
    startSound(SOUND_WORMHOLE_OPEN);
    waitSound();
    pixelsOff();
    startSound(SOUND_WORMHOLE_CLOSING);
    waitSound();
  }
  chevron++;
  
  if (chevron > 7) {
    chevron = 1;
  }
}

void setup() {
#if DEBUG
  Serial.begin(9600);
#endif
  debugln(F("Working Stargate MKII."));
  debugln(F("Initializing."));

  // Ideally we would use an analog pin that is not connected to anything else
  // to ensure the value is real random noise. However, all the pins are
  // allocated to something. Using CAL_LDR + AUDIO_VOLUME means randomness will
  // will be based on ambient light conditions and the last set volume.
  int seed = analogRead(CAL_LDR) + analogRead(AUDIO_VOLUME);
  debug(F("Random seed: "));
  debugln(seed);
  randomSeed(seed);

  initExtraPins();
  initPixels();
  initMotors();
  initAudio();
  initSD();
  calibrateGlyphs();
  debugln(F("Initialization Complete."));
}

void loop() {
  randomDialing();
}
