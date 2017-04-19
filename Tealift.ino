/* Tealift - by Fabi - forgot my domain - April 2017 */
#include <FastLED.h>
#include <Servo.h>

#define BUTTON_PIN 7
#define SERVO_PIN 5
#define PIXEL_PIN 9
#define LED_PIN LED_BUILTIN

enum {STATE_INITIAL, STATE_LISTENING, STATE_BREWING, STATE_DRIPPING};
int stateActive = 0;

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
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

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
  FastLED.setBrightness(255);
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
  unsigned long throttleTime = SERVO_SPEED;
  static unsigned long lastTimeMoved = 0;
  unsigned long deltaTime = millis() - lastTimeMoved;

  if (deltaTime >= throttleTime) {
    float deltaPos = servoTargetPos - servoActualPos;
    int posStep = deltaPos > 0 ? 1 : -1;
    moveArmByDeg(posStep);
    lastTimeMoved = millis();
  }
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
  static unsigned long debounceTime = 100;
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
  // read stateActive to display information

  gHue++;
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, PIXEL_NUM, gHue, 7);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
}

