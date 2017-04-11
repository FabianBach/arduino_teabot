/* Tealift - by Fabi - forgot my domain - April 2017 */

#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

#include <Servo.h>

Servo myservo;

const int BUTTON_PIN = 7;
const int SERVO_PIN = 9;
const int LED_PIN = 13;

const int STATE_INITIAL = 0;
const int STATE_LISTENING = 1;
const int STATE_BREWING = 2;
const int STATE_DRIPPING = 3;

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
  unsigned long throttleTime = 10;
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
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(LED_PIN, HIGH);
    setServoTargetPos(SERVO_SUNK_IN_POS);
  } else {
    // turn LED off:
    digitalWrite(LED_PIN, LOW);
    setServoTargetPos(SERVO_INITIAL_POS);
  }
}
void checkTimer() {
  
}
void displayInformation() {
  // use stateActive to display information
}

