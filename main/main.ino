#include <Servo.h>

//encoder help from
//bildr article: http://bildr.org/2012/08/rotary-encoder-arduino/

// I-O PINS
int encoderPin1 = 2;
int encoderPin2 = 3;
int encoderSwitchPin = 4; //push button switch

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

// TIMER VALUES
int workTimeMinutes = 0;
int breakTimeMinutes = 0;
bool workOrBreakSetup = false;

void setup() {
  Serial.begin (9600);

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
}

void loop() {
  Serial.print("Work Time: ");
  Serial.print(workTimeMinutes);
  Serial.println(" minutes");
  if(ButtonPressed()){
    SetBreakTime();
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
  changeTime(workOrBreakSetup);
}

void SetBreakTime(){
  workOrBreakSetup = true;
  ResetEncoderValues();
  while(true){
      Serial.print("Break Time: ");
      Serial.print(breakTimeMinutes);
      Serial.println(" minutes");
      if(ButtonPressed()){
        StartWorking();
      }
  }


}

void changeTime(bool workOrBreak){
  if (!workOrBreak){
    //change the work time
    if (encoderValue > lastEncoded){
      workTimeMinutes += 5;
    }
    else {
      workTimeMinutes -= 5;
      if (workTimeMinutes < 0) workTimeMinutes = 0;
    }
  }
  else {
    //change the break time
        if (encoderValue > lastEncoded){
      breakTimeMinutes += 5;
    }
    else {
      breakTimeMinutes -= 5;
      if (breakTimeMinutes < 0) workTimeMinutes = 0;
    }
  }
}

void StartWorking(){
  while(true){
    Serial.println("Go To Work!");
    delay(1000);
  }
}

void ResetEncoderValues(){
  lastEncoded = 0;
  encoderValue = 0;
  lastencoderValue = 0;
  lastMSB = 0;
  lastLSB = 0;
}

bool ButtonPressed(){
    // read the state of the switch into a local variable:
  int reading = digitalRead(encoderSwitchPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        return true;
      }
      else {
        return false;
      }
    }
  }
  return false;
}
