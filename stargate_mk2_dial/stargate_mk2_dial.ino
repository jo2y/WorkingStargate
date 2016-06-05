/* 
Arduino dialling and calibration script for Stargate Mk2 by Glitch
http://www.glitch.org.uk

This script is released as Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
http://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

/*
Stepper motor for chevron
Create the motor shield object with the default I2C address
Connect a stepper motor with 200 steps per revolution (1.8 degree)
to motor port #1 (M1 and M2)
*/
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *SMChevron = AFMS.getStepper(200, 1);

/*
Stepper  motor for symbol ring
Connect a stepper motor with 200 steps per revolution (1.8 degree)
to motor port #1 (M1 and M2)
*/
Adafruit_StepperMotor *SMGate = AFMS.getStepper(200, 2);

/*
Populate Chevrons[] array with output pins according to each chevron in dialling order.
Note: pins 0 & 1 cannot be used while the Arduino is connected to a PC
For this reason the pins defined by default start from 2.
*/
//int Chevrons[] = {7,8,9,3,4,5,10,2,6};
int Chevrons[] = {5,6,7,10,2,3,8,11,4};

//Chevron_Locked should be set to the last chevron in the dialling sequence
int Chevron_Locked = Chevrons[8];

//Ramp_Lights is the output pin wired to the ramp lights (this assumes all lights are connected to a single pin).
int Ramp_Lights = 12;

//Calibrate_LED is the output pin wired to the calibration led.
//int Calibrate_LED = 11;
int Calibrate_LED = 9;

//LDR is the analogue input pin wired to the light dependant resistor.
int LDR = 0;

//Set Cal to 0 for calibration and then dialling.
//Set Cal to 3 to bypass calibration and jump to dialling.
int Cal = 0;

//The calibration point is set 3% higher than the average. Adjust this number if you experience problems with calibration.
int  Calibration_Percent = 3;

//How long should the locked chevron be lit before continuing dialling. Default is 1500 (1.5 seconds).
int ChevronLocked = 1500;

//How many steps the stepper motor must turn per symbol.
//This value is overritten by the calibrate function.
//If you bypass calibration this value must be set accordingly.
float Step_Per_Symbol = 30.77;

//How many steps to move the stepper motor for the chevron.
int Chevron_Step = 10;

//Sample Startgate addresses. un-comment the address you want.
//Abydos
int Address[] = {27,7,15,32,12,30,1};

//Othala (Asgard homeworld)
//int Address[] = {11,27,23,16,33,3,9,1};

//Destiny
//int Address[] = {6,17,21,31,35,24,5,11,1};

//Other variable definitions. No need to change these.
float LDR_avg = 0.0;
float LDR_cal = 0.0;
int Dialling = 0;
float Step_increment = 0;
int Address_Length = 0;

void setup() {
  Serial.begin(9600);  // set up Serial library at 9600 bps
  Serial.println("Working Stargate Mk2");
  AFMS.begin();  // create with the default frequency 1.6KHz
  
  SMGate->setSpeed(10);  // 10 rpm 
  SMChevron->setSpeed(10);  // 10 rpm 
  for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron ++){
    pinMode(Chevrons[tmp_chevron], OUTPUT);
  }
  pinMode(Calibrate_LED, OUTPUT);
  pinMode(LDR,INPUT);
  pinMode(Ramp_Lights, OUTPUT);
  Address_Length = sizeof (Address) / sizeof(int);

  if (Cal == 3){
    Dialling = 1;
  }else{
    Cal = 0;
    Dialling = 0;
  }
}

void loop() {
  if (Cal < 3){
    calibrate();
  }else if (Dialling <= Address_Length){
    dial(Dialling);
    if (Dialling == Address_Length){
      SMChevron->step(Chevron_Step, FORWARD, SINGLE);
      SMChevron->release();
      Serial.println("Wormhole Established");
      for (int tmp_chevron1 = 0; tmp_chevron1 < 9; tmp_chevron1 ++){
        digitalWrite(Chevrons[tmp_chevron1], HIGH);
      }
      digitalWrite(Ramp_Lights, HIGH);
      delay(10000);
      for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron ++){
        digitalWrite(Chevrons[tmp_chevron], LOW);
      }
      digitalWrite(Ramp_Lights, LOW);
      Serial.println("Wormhole Disengaged");
    }
    Dialling ++;
  }
}

void dial(int Chevron) {
  Serial.print("Chevron ");
  Serial.print(Chevron);
  Serial.println(" Encoded");
  int Steps_Turn = 0;
  if (Chevron == 1) {
    Steps_Turn = round(Step_Per_Symbol * (Address[(Chevron - 1)] - 1));
    SMGate->step(Steps_Turn, BACKWARD, DOUBLE);
  }else{
    Steps_Turn = Address[(Chevron - 1)] - Address[(Chevron - 2)];
    if ((Chevron % 2) == 0) {
      if  (Steps_Turn < 0) {
        Steps_Turn = Steps_Turn * -1;
      }else{
        Steps_Turn = 39 - Steps_Turn;
      }
      Steps_Turn = round(Step_Per_Symbol * Steps_Turn);
      SMGate->step(Steps_Turn, FORWARD, DOUBLE);
    }else{
      if  (Steps_Turn < 0) {
        Steps_Turn = 39 + Steps_Turn;
      }
      Steps_Turn = round(Step_Per_Symbol * Steps_Turn);
      SMGate->step(Steps_Turn, BACKWARD, DOUBLE);
    }
  } 
  SMGate->release();
  if ((Dialling == 9) && (Address[(Chevron - 1)] != 1)){
    for (int tmp_fail_safe_gate = 0; tmp_fail_safe_gate <= 100; tmp_fail_safe_gate ++){
      int tmp_fail_safe_chevron = random(7);
      digitalWrite(Chevrons[tmp_fail_safe_chevron], LOW);
      SMGate->step(5, BACKWARD, DOUBLE);
      SMGate->release();
      digitalWrite(Chevrons[tmp_fail_safe_chevron], HIGH);
    }
    for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron ++){
      digitalWrite(Chevrons[tmp_chevron], LOW);
    }
    Dialling = 10;
  }else{
    Serial.print("Chevron ");
    Serial.print(Chevron);
    Serial.println(" Locked");
    digitalWrite(Chevron_Locked, HIGH);
    SMChevron->step(Chevron_Step, BACKWARD, SINGLE);
    SMChevron->release();
  }
  if (Dialling < Address_Length){
    delay(ChevronLocked);
    SMChevron->step(Chevron_Step, FORWARD, SINGLE);
    SMChevron->release();
    digitalWrite(Chevron_Locked, LOW);
    delay(20);
    if ((Address_Length < 8) || (Dialling < 4)){
      digitalWrite(Chevrons[(Chevron - 1)], HIGH);
    }else if ((Address_Length == 8) && ((Dialling >= 4) && (Dialling <= Address_Length))){
      if (Dialling == 4){
        digitalWrite(Chevrons[(6)], HIGH);
      }else if (Dialling == 5){
        digitalWrite(Chevrons[(3)], HIGH);
      }else if (Dialling == 6){
        digitalWrite(Chevrons[(4)], HIGH);
      }else if (Dialling == 7){
        digitalWrite(Chevrons[(5)], HIGH);
      }
    }else if ((Address_Length == 9) && ((Dialling >= 4) && (Dialling <= Address_Length))){
      if (Dialling == 4){
        digitalWrite(Chevrons[(6)], HIGH);
      }else if (Dialling == 5){
        digitalWrite(Chevrons[(7)], HIGH);
      }else if (Dialling == 6){
        digitalWrite(Chevrons[(3)], HIGH);
      }else if (Dialling == 7){
        digitalWrite(Chevrons[(4)], HIGH);
      }else if (Dialling == 8){
        digitalWrite(Chevrons[(5)], HIGH);
      }
    }
  }
}

void calibrate() {
  if (Cal == 0){
    digitalWrite(Calibrate_LED, HIGH);
    Serial.print("Reading 1 : ");
    int LDR_1 = analogRead(LDR);
    delay(2000);
    LDR_1 = analogRead(LDR);
    Serial.println(LDR_1);
    SMGate->step(10, BACKWARD, DOUBLE);
    Serial.print("Reading 2 : ");
    int LDR_2 = analogRead(LDR);
    Serial.println(LDR_2);
    SMGate->step(10, BACKWARD, DOUBLE);
    Serial.print("Reading 3 : ");
    int LDR_3 = analogRead(LDR);
    Serial.println(LDR_3);
    SMGate->step(10, BACKWARD, DOUBLE);
    Serial.print("Reading 4 : ");
    int LDR_4 = analogRead(LDR);
    Serial.println(LDR_4);
    SMGate->step(10, BACKWARD, DOUBLE);
    Serial.print("Reading 5 : ");
    int LDR_5 = analogRead(LDR);
    Serial.println(LDR_5);
    SMGate->step(10, BACKWARD, DOUBLE);
    Serial.print("Reading 6 : ");
    int LDR_6 = analogRead(LDR);
    Serial.println(LDR_6);
    SMGate->step(10, BACKWARD, DOUBLE);
    Serial.print("Reading 7 : ");
    int LDR_7 = analogRead(LDR);
    Serial.println(LDR_7);
    SMGate->step(10, BACKWARD, DOUBLE);
    Serial.print("Reading 8 : ");
    int LDR_8 = analogRead(LDR);
    Serial.println(LDR_8);
    SMGate->step(10, BACKWARD, DOUBLE);
    Serial.print("Reading 9 : ");
    int LDR_9 = analogRead(LDR);
    Serial.println(LDR_9);
    SMGate->step(10, BACKWARD, DOUBLE);
    Serial.print("Reading 10 : ");
    int LDR_10 = analogRead(LDR);
    Serial.println(LDR_10);
    LDR_avg = (LDR_1 + LDR_2 + LDR_3 + LDR_4 + LDR_5 + LDR_6 + LDR_7 + LDR_8 + LDR_9 + LDR_10) / 10;
    
    //Calculate average ldr value and set calibration point 3% higher
    LDR_cal = LDR_avg + ((LDR_avg / 100) * Calibration_Percent);
    
    Serial.print("LDR Average : ");
    Serial.println(LDR_avg);
    Serial.print("LDR Calibrate Point : ");
    Serial.println(LDR_cal);
    SMGate->release();
    Cal = 1;
  }else if (Cal == 1){
    digitalWrite(Calibrate_LED, HIGH);
    SMGate->step(1, FORWARD, MICROSTEP);
    SMGate->release();
    int tmp_cal = analogRead(LDR);
    if(tmp_cal > LDR_cal){
      Serial.print("LDR Calibrated : ");
      Serial.println(tmp_cal);
      Cal = 2;
      for (int tmp_chevron1 = 0; tmp_chevron1 < 9; tmp_chevron1 ++){
        digitalWrite(Chevrons[tmp_chevron1], HIGH);
      }
      delay(1000);
      for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron ++){
        digitalWrite(Chevrons[tmp_chevron], LOW);
      }
    }
  }else{
    digitalWrite(Calibrate_LED, HIGH);
    SMGate->step(1, FORWARD, MICROSTEP);
    SMGate->release();
    Step_increment ++;
    int tmp_cal = analogRead(LDR);
    if((tmp_cal > LDR_cal) && (Step_increment > 30)){
      // 39 symbols
      Step_Per_Symbol = Step_increment / 39;
      Serial.print("Steps Calculate : ");
      Serial.println(Step_Per_Symbol);
      Serial.print("Total Steps : ");
      Serial.println(Step_increment);
      digitalWrite(Calibrate_LED, LOW);
      Cal = 3;
      Dialling = 1;
      for (int tmp_chevron1 = 0; tmp_chevron1 < 9; tmp_chevron1 ++){
        digitalWrite(Chevrons[tmp_chevron1], HIGH);
      }
      delay(1000);
      for (int tmp_chevron = 0; tmp_chevron < 9; tmp_chevron ++){
        digitalWrite(Chevrons[tmp_chevron], LOW);
      }
      delay(4000);
    }else if (Step_increment == 133){
      digitalWrite(Chevrons[0], HIGH);
    }else if (Step_increment == 266){
      digitalWrite(Chevrons[1], HIGH);
    }else if (Step_increment == 399){
      digitalWrite(Chevrons[2], HIGH);
    }else if (Step_increment == 532){
      digitalWrite(Chevrons[6], HIGH);
    }else if (Step_increment == 665){
      digitalWrite(Chevrons[7], HIGH);
    }else if (Step_increment == 798){
      digitalWrite(Chevrons[3], HIGH);
    }else if (Step_increment == 931){
      digitalWrite(Chevrons[4], HIGH);
    }else if (Step_increment == 1064){
      digitalWrite(Chevrons[5], HIGH);
    }  
  }
}
