/* Tealift - by Fabi - forgot my domain - April 2017 */
#include <FastLED.h>
#include <Servo.h>

#define BUTTON_PIN 7
#define SERVO_PIN 5
#define PIXEL_PIN 9
#define LED_PIN LED_BUILTIN

enum {STATE_INITIAL, STATE_LISTENING, STATE_BREWING, STATE_DRIPPING};
int stateActive = 0;

//BUTTON GLOBALS
#define BUTTON_LONG_PRESS_TIME 800

//SERVO GLOBALS
#define SERVO_INITIAL_POS 0.0   //equals min value
#define SERVO_SUNK_IN_POS 170.0 //equals max value
#define SERVO_DRIPPING_POS 90.0
#define SERVO_SPEED 10
Servo myservo;
float servoTargetPos = SERVO_INITIAL_POS;
float servoActualPos = -1; // well, let´s just assume we don't know our starting position

//PIXEL GLOBALS
#define PIXEL_FPS  60
#define PIXEL_NUM 7
CRGB leds[PIXEL_NUM]; //number of leds inside

void setup() {
//  BUTTON
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

//  SERVO
  myservo.attach(SERVO_PIN);
  setServoTargetPos(SERVO_INITIAL_POS);

//  PIXEL
  pinMode(PIXEL_PIN, OUTPUT);
  digitalWrite(PIXEL_PIN, LOW);
  FastLED.addLeds<WS2812,PIXEL_PIN,GRB>(leds, PIXEL_NUM).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(127);
}

void loop() {
  checkButton();
  checkTimer();
  EVERY_N_MILLISECONDS( PIXEL_FPS ) { displayInformation(); }
  EVERY_N_MILLISECONDS( SERVO_SPEED ) { moveArmToTargetPos(); }
}

void setServoTargetPos(float newTargetPos) {
//  TODO: validation and stuff
  servoTargetPos = newTargetPos;
}

void moveArmToTargetPos() {
    float deltaPos = servoTargetPos - servoActualPos;
    int posStep = deltaPos > 0 ? 1 : -1;
    moveArmByDeg(posStep);
}

void moveArmByDeg(float degree){
    float newPos = servoActualPos + degree;
    moveArmInPosition(newPos);
}

void moveArmInPosition(float newPos) {  
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
  static unsigned long debounceTime = 25;
  
  int buttonState = digitalRead(BUTTON_PIN);
  static bool waitForRelease = false;
  static bool buttonLongPress = false;

  // Debouncing to filter mechanical flickers
  if (buttonState != lastButtonState) {
    lastTimeButtonChanged = millis();
  }
  
  else if ((lastTimeButtonChanged + debounceTime) < millis()) {
    digitalWrite(LED_PIN, buttonState);

    // The button is pressed
    if (buttonState == HIGH) {
      // if we have no long press, wait for release, else ignore the high state
      waitForRelease = !buttonLongPress;
      
      if ( ((lastTimeButtonChanged + BUTTON_LONG_PRESS_TIME) < millis()) && !buttonLongPress){
        waitForRelease = false;
        buttonLongPress = true;
        onButtonPressLong();
      }

    // The button is released
    } else {
      buttonLongPress = false;
      if (waitForRelease){
        waitForRelease = false;
        onButtonPressShort();
      }
    }
  }
    
  lastButtonState = buttonState;
}

void onButtonPressShort(){
  stateActive = STATE_BREWING;
  setServoTargetPos(SERVO_SUNK_IN_POS);
}

void onButtonPressLong(){
  stateActive = STATE_INITIAL;
  setServoTargetPos(SERVO_INITIAL_POS);
}


void setTimer() {
  
}
void checkTimer() {
  
}
void displayInformation() {
  static uint8_t gHue = 0; // rotating "base color" used by many of the patterns
//  gHue++;

  switch(stateActive){
    case STATE_INITIAL:
      gHue = 0;
      break;
    
    case STATE_BREWING:
      gHue = 31;
      break;
  }
  
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, PIXEL_NUM, gHue, 7);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
}

