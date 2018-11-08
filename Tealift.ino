/* Tealift - by Fabi - forgot my domain - April 2017 */
#include <FastLED.h>
#include <Servo.h>

#define BUTTON_PIN 7
#define SERVO_PIN 5
#define PIXEL_PIN 9
#define LED_PIN LED_BUILTIN

#define ONE_MINUTE 60000

//STATE GLOBALS
enum {STATE_INITIAL, STATE_SETTING, STATE_BREWING, STATE_DRIPPING};
int stateActive = 0;

//TIMER GLOBALS
static unsigned long teaTime = 0;
static unsigned long teaTimeLeft = 0;
static unsigned long timeTimerWasSet = 0;

//BUTTON GLOBALS
#define BUTTON_LONG_PRESS_TIME 800

//SERVO GLOBALS
#define SERVO_MAX_SPEED 0.7
#define SERVO_MIN_SPEED 0.2
#define SERVO_UPDATE_SPEED 10

#define SERVO_INITIAL_POS  5.0
#define SERVO_SUNK_IN_POS 130.0
#define SERVO_DRIPPING_POS 65.0
#define SERVO_MIN_POS SERVO_INITIAL_POS
#define SERVO_MAX_POS SERVO_SUNK_IN_POS

Servo myservo;
bool servoMoving = false;
float servoTargetPos = SERVO_INITIAL_POS;
float servoActualPos = -1; // well, let´s just assume we don't know our starting position

//PIXEL GLOBALS
#define PIXEL_FPS  60
#define PIXEL_NUM 7
#define PIXEL_H_STD 20
#define PIXEL_S_STD 255
#define PIXEL_B_STD 255
CRGB leds[PIXEL_NUM];
static bool pixelIsDirty = false;

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

  //STATE
  setStateInitial();
}

void loop() {
  checkButton();
  checkTeaTimer();
  EVERY_N_MILLISECONDS( SERVO_UPDATE_SPEED ) { moveArmToTargetPos(); }
  EVERY_N_MILLISECONDS( 1000/PIXEL_FPS )   { displayInformation(); }
}



/*
 * STATES
 */
void setStateInitial(){
  stateActive = STATE_INITIAL;
  setServoTargetPos(SERVO_INITIAL_POS);
  resetTimer();
}

void setStateSetting(){
  stateActive = STATE_SETTING;
  setServoTargetPos(SERVO_INITIAL_POS);
  setTeaTime();
}

void setStateBrewing(){
  stateActive = STATE_BREWING;
  setServoTargetPos(SERVO_SUNK_IN_POS);
  activateTeaTimer();
}

void setStateDripping(){
  stateActive = STATE_DRIPPING;
  setServoTargetPos(SERVO_DRIPPING_POS);
  resetTimer();
}

void onButtonPressShort(){
  switch(stateActive){
  
    case STATE_INITIAL:
      setStateSetting();
      break;

    case STATE_SETTING:
      setTeaTime();
      break;

    case STATE_BREWING:
      // nothing
      break;

    case STATE_DRIPPING:
      setStateInitial();
      break;
  }
}

void onButtonPressLong(){
  switch(stateActive){

    case STATE_INITIAL:
      setStateSetting();
      break;

    case STATE_SETTING:
      setStateBrewing();
      break;

    case STATE_BREWING:
      setStateDripping();
      break;

    case STATE_DRIPPING:
      setStateInitial();
      break;
  }
}

void onTimerEnd(){
  setStateDripping();
}


/*
 * TIMER
 */
void setTeaTime() {
  teaTime += ONE_MINUTE;
  if (teaTime > (7*ONE_MINUTE)){ teaTime = ONE_MINUTE; }
}

void activateTeaTimer(){
  timeTimerWasSet = millis();
}

void checkTeaTimer() {
  if (timeTimerWasSet <= 0){
    return;
  }

  teaTimeLeft = (timeTimerWasSet + teaTime) - millis();
  if (teaTimeLeft <= 0){
    onTimerEnd();
  }
}

void resetTimer(){
 timeTimerWasSet = 0;
 teaTimeLeft = 0;
 teaTime = 0;
}



/*
 * BUTTON
 */
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



/*
 * SERVO
 */

float validateServosPos(float pos){
  if (pos < SERVO_MIN_POS){
    pos = SERVO_MIN_POS;
  }
  if (pos > SERVO_MAX_POS) {
    pos = SERVO_MAX_POS;
  }
  return pos;
}

 void setServoTargetPos(float newTargetPos) {
  newTargetPos = validateServosPos(newTargetPos);
  servoTargetPos = newTargetPos;
}

void moveArmToTargetPos() {
    float delta = servoTargetPos - servoActualPos;
    float posStep = 0;
    
    if (delta != 0){
      posStep = delta > 0 ? SERVO_MAX_SPEED : -SERVO_MAX_SPEED;
    }
    
    //slow down on the sinking end
    float sinkingZone = SERVO_SUNK_IN_POS - SERVO_DRIPPING_POS;
    bool isProbablyTouchingWater = servoActualPos > SERVO_DRIPPING_POS;
    if (isProbablyTouchingWater && delta > 0){
      posStep = posStep * (delta/(sinkingZone*2));
    }

    // do not get too slow...
    if (abs(posStep) > 0 && abs(posStep) < SERVO_MIN_SPEED){
      posStep = posStep > 0 ? SERVO_MIN_SPEED : -SERVO_MIN_SPEED;;
    }

    // do not get too fast either
    if (abs(posStep) > SERVO_MAX_SPEED){
      posStep = posStep > 0 ? SERVO_MAX_SPEED : -SERVO_MAX_SPEED;
    }

    // do this last step in whatever speed necessary
    if (abs(delta) < SERVO_MIN_SPEED){
      posStep = delta;
    }
        
    moveArmByDeg(posStep);
}

void moveArmByDeg(float degree){
    float newPos = servoActualPos + degree;
    moveArmInPosition(newPos);
}

void moveArmInPosition(float newPos) {
  newPos = validateServosPos(newPos);
  if (servoActualPos != newPos){ 
     myservo.write(newPos);
     servoMoving = true;
  }
  else {
    servoMoving = false;
  }
  
  servoActualPos = newPos;
}



/*
 * PIXEL
 */
void displayInformation() {
  static uint8_t gHue = 0; // rotating "base color" used by many of the patterns

  if (servoMoving){
    pixelAnimationWarning();
  }
  else {
    
    switch(stateActive){

      case STATE_INITIAL:
        pixelAnimationNoop();
        break;
  
      case STATE_SETTING:
        pixelAnimationShowTime();
        break;
  
      case STATE_BREWING:
        pixelAnimationBrewing();
        break;
  
      case STATE_DRIPPING:
        pixelAnimationMiniBlink();
        break;
    }
  }

  if (pixelIsDirty) {
    // send the 'leds' array out to the actual LED strip
    FastLED.show();  
    pixelIsDirty = false;
  }
  
}

void pixelAnimationNoop(){
  static unsigned long frameNum = 0;
  if (frameNum%PIXEL_FPS == 0){
    for( int i = 0; i < PIXEL_NUM; i++) {
      leds[i] = CHSV(PIXEL_H_STD, PIXEL_S_STD, 0);
    }
    pixelIsDirty = true;
  }
  frameNum++;
}

void pixelAnimationBlackout(bool instant){
  fadeToBlackBy(leds, PIXEL_NUM, PIXEL_FPS);
  if (instant){
    for( int i = 0; i < PIXEL_NUM; i++) {
      leds[i] = CHSV(PIXEL_H_STD, PIXEL_S_STD, 0);
    }
  }
  pixelIsDirty = true;
}

void pixelAnimationMiniBlink(){
  static unsigned long frameNum = 0;
  static unsigned long stepNum = 0;
  uint8_t brightness = 0;
  
  if (frameNum % (PIXEL_FPS/2) == 0){
    
    if (stepNum % 4 == 0){
      brightness = PIXEL_B_STD;
    } else {
      brightness = 0;
    }
    
    leds[0] = CHSV(PIXEL_H_STD, PIXEL_S_STD, brightness);
    
    for( int i = 1; i < PIXEL_NUM; i++) {
      leds[i] = CHSV(PIXEL_H_STD, PIXEL_S_STD, PIXEL_B_STD);
    }
    
    pixelIsDirty = true;
    stepNum++;
  }
  frameNum++;
}

void pixelAnimationFlash(){
  static unsigned long frameNum = 0;
  fadeToBlackBy(leds, PIXEL_NUM, PIXEL_FPS);
  if (frameNum%PIXEL_FPS == 0){
    for( int i = 0; i < PIXEL_NUM; i++) {
      leds[i] = CHSV(PIXEL_H_STD, PIXEL_S_STD, PIXEL_B_STD);
    }
    pixelIsDirty = true;
  }
  frameNum++;
}

void pixelAnimationPulse(){
  static unsigned long frameNum = 0;
  const int totalFrameCount = PIXEL_FPS;
  float splitPos = 0.2;
  float splitFrame = totalFrameCount * splitPos;
  uint8_t brightness = (frameNum/splitFrame) * 255;
  
  if (frameNum > splitFrame){
    brightness = (1 - ((frameNum-splitFrame)/(totalFrameCount-splitFrame))) * PIXEL_B_STD;
  }
  
  for( int i = 0; i < PIXEL_NUM; i++) {
    leds[i] = CHSV(PIXEL_H_STD, PIXEL_S_STD, brightness);
  }
  frameNum = (frameNum % totalFrameCount)+ 1;

  pixelIsDirty = true;
}

void pixelAnimationShowTime(){
  static unsigned long frameNum = 0;
  uint8_t brightness = 0;
  if (frameNum % (PIXEL_FPS/10) == 0){
    for( int i = 0; i < PIXEL_NUM; i++) {
      if (i < (teaTime/ONE_MINUTE) ){
        brightness = PIXEL_B_STD;
      } else {
        brightness = 0;
      }
      leds[i] = CHSV(PIXEL_H_STD, PIXEL_S_STD, brightness);
    }
    pixelIsDirty = true;
  }
  frameNum++;
}

void pixelAnimationBrewing(){
  unsigned long teaTimeLeftMinutes = (teaTimeLeft / ONE_MINUTE) + 1;
  static unsigned long frameNum = 0;
  uint8_t brightness = 0;

  if (frameNum % (int)(PIXEL_FPS/10) == 0) {
    fadeToBlackBy(leds, PIXEL_NUM, (int)(PIXEL_B_STD/5)); //last argument tells how much to dim
    pixelIsDirty = true;
  }

  if (frameNum % PIXEL_FPS == 0) {
    for (int i = 0; i < PIXEL_NUM; i++) {
      if (i < (teaTimeLeftMinutes) ){
        brightness = PIXEL_B_STD;
      } else {
        brightness = 0;
      }
      leds[(i+frameNum)%PIXEL_NUM] = CHSV(PIXEL_H_STD, PIXEL_S_STD, brightness);
    }
    pixelIsDirty = true;
  }

  frameNum++;
}

void pixelAnimationWarning(){
  uint8_t brightness = 0;
  static unsigned long frameNum = 0;
  static unsigned long stepNum = 0;

  if (frameNum % (PIXEL_FPS/10) == 0) {
    leds[0] = CHSV(PIXEL_H_STD, PIXEL_S_STD, PIXEL_B_STD);

    for (int i = 0; i < PIXEL_NUM-1; i++) {
      
      if ( (i+stepNum)%3 == 0){
        brightness = PIXEL_B_STD;
      }
      else {
        brightness = 0;
      }
      
      leds[i+1] = CHSV(PIXEL_H_STD, PIXEL_S_STD, brightness);
    }
    stepNum++;

    pixelIsDirty = true;
  }
  
  frameNum++;
}

