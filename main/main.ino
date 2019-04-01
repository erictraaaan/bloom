#include <SimpleTimer.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

/*
	BLOOM - WORK AND BREAK PERIOD MANAGER
	GROUP 8 - MECHENG 4B03
	SOURCE CODE DEVELOPED BY ERIC TRAN
*/

//LED DISPLAY
Adafruit_7segment matrix = Adafruit_7segment();

// I-O PINS
int encoderPin1 = 2;
int encoderPin2 = 3;
int encoderSwitchPin = 4; //push button switch

//RGD LED PINS
int redPin = 9;
int greenPin = 10;
int bluePin = 11;
bool blinkOn = true;
int blinkTimerID;

// ENCODER VALUES
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
long lastencoderValue = 0;
int lastMSB = 0;
int lastLSB = 0;

// DEBOUNCING
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
bool startUpDelay = true;

// TIMER VALUES
int workTimeMinutes = 0;
int breakTimeMinutes = 0;
bool workOrBreakSetup = false;
bool setupMode = true;
SimpleTimer timer;
int timerID;
int countdown = 0;
bool workTime = true;
int motorTimerID;

//SERVO MOTOR
Servo myServo;
int servoPin = 6;
const float totalRotationSeconds = 10;
const float rotationDivider = 20;
float rotationAmountPer = totalRotationSeconds/rotationDivider;
float rotationWaitPeriod;

void setup() {
  //DISPLAY SETUP
  #ifndef __AVR_ATtiny85__
  Serial.begin(9600);
  #endif
  matrix.begin(0x70);
  matrix.setBrightness(15);
  matrix.blinkRate(1);
  
  //ENCODER SETUP
  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);
  pinMode(encoderSwitchPin, INPUT);
  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on
  digitalWrite(encoderSwitchPin, HIGH); //turn pullup resistor on
  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);
  Serial.println("Welcome to Bloom.");
  Serial.println("Tell us how long you want to work: ");

  //RGB LED SETUP
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  setLedRed();

  //SERVO SETUP
  myServo.attach(servoPin);
  myServo.write(90);

  blinkTimerID = timer.setInterval(1000, blinkLED);
  
}

void loop() {

  timer.run();
  matrix.print(workTimeMinutes,DEC);
  matrix.writeDisplay();
  if (CheckButtonPress()){
    if (startUpDelay){
      startUpDelay = false;
    }
    else{
      SetBreakTime();
    }
  }
}

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit
  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number 
  int sum = (lastEncoded << 2) | encoded; //adding it to the previous encoded value 
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++; 
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --; 
  lastEncoded = encoded; //store this value for next time 
  if(setupMode) changeTime(workOrBreakSetup);
}

void SetBreakTime(){
  Serial.println("Tell us how long you want to break: ");
  workOrBreakSetup = true;
  ResetEncoderValues();
  setLedBlue();
  
  while(true){
    timer.run();
    matrix.print(breakTimeMinutes,DEC);
    matrix.writeDisplay();
    if(CheckButtonPress()){
      setupMode = false;
      WorkSetup();
    }
  }
}

void changeTime(bool workOrBreak){
  if (!workOrBreak){
    //change the work time
    if (encoderValue > lastEncoded && ( (encoderValue - lastEncoded) > 5 )){
      workTimeMinutes += 5;
      if (workTimeMinutes > 180) workTimeMinutes = 180;
    }
    else {
      if ( (lastEncoded - encoderValue) > 5){
              workTimeMinutes -= 5;
      if (workTimeMinutes < 0) workTimeMinutes = 0;
      }
    }
    Serial.print("Work Time: ");
    Serial.print(workTimeMinutes);
    Serial.println(" minutes");
  }
  else {
    //change the break time
        if (encoderValue > lastEncoded){
      breakTimeMinutes += 5;
      if (breakTimeMinutes > 180) breakTimeMinutes = 180;
    }
    else {
      breakTimeMinutes -= 5;
      if (breakTimeMinutes < 0) breakTimeMinutes = 0;
    }
    Serial.print("Break Time: ");
    Serial.print(breakTimeMinutes);
    Serial.println(" minutes");
  }
}

void WorkSetup(){
  matrix.blinkRate(0);
  timerID = timer.setInterval(1000, UpdateLCD);
  setLedGreen();
  SetMotorParameters();
  StartWorking();
}

void SetMotorParameters(){
  rotationWaitPeriod = workTimeMinutes*60/rotationDivider;
}

void StartWorking(){  
  Serial.println("Lets get to work!");
  countdown = workTimeMinutes*60;
  ShowCountdown();
  while(true){
    matrix.print(countdown/60 + 1,DEC);
    matrix.writeDisplay();
    timer.run();
  }
}

void ShowCountdown(){
  Serial.print(countdown/60);
  Serial.println(" minutes remaining.");
}

void ShowSeconds(){
  Serial.print(countdown);
  Serial.println(" seconds left.");
}

void StartBreak(){
  Serial.println("Time for a break!");
  setLedBlue();
  countdown = breakTimeMinutes*60;
  ShowCountdown();
  while(true){
    matrix.print(countdown/60 + 1,DEC);
    matrix.writeDisplay();    
    timer.run();
  }
}

void ResetEncoderValues(){
  lastEncoded = 0;
  encoderValue = 0;
  lastencoderValue = 0;
  lastMSB = 0;
  lastLSB = 0;
}

void UpdateLCD(){
  countdown--;
  ShowSeconds();
  if (countdown % 60 == 0){
      ShowCountdown();
  }
  
  if (countdown <= 0){
    if(workTime){
      workTime = !workTime;
      StartBreak();
    }
    else {
      workTime = !workTime;
      WorkSetup();
    }

  }
}

bool CheckButtonPress(){
  if(debounceButton(buttonState) == HIGH && buttonState == LOW){
    buttonState = HIGH;
    return true;
  }
  else if (debounceButton(buttonState) == LOW && buttonState == HIGH){
    buttonState = LOW;
    return false;
  }
  return false;
}

boolean debounceButton(boolean state){
  boolean stateNow = digitalRead(encoderSwitchPin);
  if (state != stateNow){
    delay(10);
    stateNow = digitalRead(encoderSwitchPin);
  }
  return stateNow;
}

void setLedRed(){
  setColor(255,0,0); 
}

void setLedBlue(){
  setColor(0,0,255);
}

void setLedGreen(){
  setColor(0,255,0);
}

void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}

void blinkLED(){
    Serial.println(" blink LED ");
  if (blinkOn){
    setColor(0,0,0);
  }
  else {
    setColor(113,238,184);
  }
  blinkOn = !blinkOn;
}
