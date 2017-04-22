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
#define SERVO_INITIAL_POS   0.0
#define SERVO_DRIPPING_POS 90.0
#define SERVO_SUNK_IN_POS 168.0
#define SERVO_MIN_POS SERVO_INITIAL_POS
#define SERVO_MAX_POS SERVO_SUNK_IN_POS
#define SERVO_SPEED 10
Servo myservo;
bool servoMoving = false;
float servoTargetPos = SERVO_INITIAL_POS;
float servoActualPos = -1; // well, let´s just assume we don't know our starting position

//PIXEL GLOBALS
#define PIXEL_FPS  60
#define PIXEL_NUM 7
CRGB leds[PIXEL_NUM];

void setup() {
//  TIMER
  resetTimer();
  
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
  checkTeaTimer();
  EVERY_N_MILLISECONDS( SERVO_SPEED ) { moveArmToTargetPos(); }
  EVERY_N_MILLISECONDS( PIXEL_FPS )   { displayInformation(); }
}



/*
 * STATES
 */
void setStateInitial(){
  stateActive = STATE_INITIAL;
  setServoTargetPos(SERVO_INITIAL_POS);
}

void setStateSetting(){
  stateActive = STATE_SETTING;
  resetTimer();
  setTeaTimer();
  setServoTargetPos(SERVO_INITIAL_POS);
}

void setStateBrewing(){
  stateActive = STATE_BREWING;
  setServoTargetPos(SERVO_SUNK_IN_POS);
  activateTeaTimer();
}

void setStateDripping(){
  stateActive = STATE_DRIPPING;
  setServoTargetPos(SERVO_DRIPPING_POS);
}

void onButtonPressShort(){
  switch(stateActive){
  
    case STATE_INITIAL:
      // nothing
      break;

    case STATE_SETTING:
      setTeaTimer();
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
  resetTimer();
  setStateDripping();
}


/*
 * TIMER
 */
void setTeaTimer() {
  teaTime += ONE_MINUTE;
  if (teaTime > (7*ONE_MINUTE)){ teaTime = ONE_MINUTE; }
}

void activateTeaTimer(){
  timeTimerWasSet = millis();
}

void checkTeaTimer() {
  if (timeTimerWasSet < 0){
    return;
  }

  teaTimeLeft = (timeTimerWasSet + teaTime) - millis();
  if (teaTimeLeft <= 0){
    onTimerEnd();
  }
}

void resetTimer(){
 timeTimerWasSet = -1;
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
      posStep = delta > 0 ? 1 : -1;
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
        pixelAnimationNoop();
        break;
    }
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
}

void pixelAnimationNoop(){
  fadeToBlackBy(leds, PIXEL_NUM, 10);
}

void pixelAnimationShowTime(){
  uint8_t brightness = 0;
  for( int i = 0; i < PIXEL_NUM; i++) {
    if (i < (teaTime/ONE_MINUTE) ){
      brightness = 255;
    } else {
      brightness = 0;
    }
    leds[i] = CHSV(0, 0, brightness);
  }
}

void pixelAnimationBrewing(){
  // random colored speckles that blink in and fade smoothly
  unsigned long teaTimeLeftMinutes = (teaTimeLeft / ONE_MINUTE) + 1;
  fadeToBlackBy(leds, PIXEL_NUM, teaTimeLeftMinutes);
  int pos = random16(PIXEL_NUM);
  leds[pos] += CHSV( 30, 0, 255);
}

void pixelAnimationWarning(){
  uint8_t brightness = 0;
  uint8_t BeatsPerMinute = 120;
  uint8_t beat = beatsin8(BeatsPerMinute, 0, 100);
  
  //fadeToBlackBy( leds, NUM_LEDS, 10);
  for( int i = 0; i < PIXEL_NUM; i++) { //9948
    if (beat > 70){
      brightness = 255;
    } else {
      brightness = 0;
    }
    leds[i] = CHSV(0, 0, brightness);
  }
}

