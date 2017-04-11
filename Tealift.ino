/* Tealift - by Fabi - forgot my domain - April 2017 */
#include <FastLED.h>
#include <Servo.h>

Servo myservo;

const int BUTTON_PIN = 7;
const int SERVO_PIN = 5;
const int LED_PIN = LED_BUILTIN;

enum {STATE_INITIAL, STATE_LISTENING, STATE_BREWING, STATE_DRIPPING};
int stateActive = 0;

const int SERVO_INITIAL_POS = 50;
const int SERVO_SUNK_IN_POS = 400;
const int SERVO_DRIPPING_POS = 90;

int servoTargetPos = 0;
int servoActualPos = -1; // well, let´s just assume we don't know our starting position

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  myservo.attach(SERVO_PIN);
  setServoTargetPos(SERVO_INITIAL_POS);
}

void loop() {
  checkButton();
  checkTimer();
  displayInformation();
  moveArmToTargetPos();
}

void setServoTargetPos(int newTargetPos) {
//  TODO: validation and stuff
  servoTargetPos = newTargetPos;
}

void moveArmToTargetPos() {
  unsigned long throttleTime = 25;
  static unsigned long lastTimeMoved = 0;
  unsigned long deltaTime = millis() - lastTimeMoved;

  if (deltaTime >= throttleTime) {
    int deltaPos = servoTargetPos - servoActualPos;
    int posStep = deltaPos / 25;
    moveArmByDeg(posStep);
    lastTimeMoved = millis();
  }
}

void moveArmByDeg(int degree){
    int newPos = servoActualPos + degree;
    moveArmInPosition(newPos);
}

void moveArmInPosition(int newPos) {  
  if (newPos < SERVO_INITIAL_POS){
    newPos = SERVO_INITIAL_POS;
  }
  if (newPos > SERVO_SUNK_IN_POS) {
    newPos = SERVO_SUNK_IN_POS;
  }

  if (servoActualPos != newPos){ 
     myservo.write(newPos);
     servoActualPos = newPos; 
  }
}

void checkButton(){
  static int lastButtonState = LOW;
  static unsigned long lastTimeButtonChanged = 0;
  static unsigned long debounceTime = 50;
  int buttonState = digitalRead(BUTTON_PIN);

  // Debouncing to filter mechanical flickers
  if (buttonState != lastButtonState) {
    lastTimeButtonChanged = millis();
  }
  else if ((lastTimeButtonChanged + debounceTime) < millis()) {
    if (buttonState == HIGH) {
      digitalWrite(LED_PIN, HIGH);
      setServoTargetPos(SERVO_SUNK_IN_POS);
    } else {
      digitalWrite(LED_PIN, LOW);
      setServoTargetPos(SERVO_INITIAL_POS);
    }
  }
  lastButtonState = buttonState;
}


void setTimer() {
  
}
void checkTimer() {
  
}
void displayInformation() {
  // use stateActive to display information
}

