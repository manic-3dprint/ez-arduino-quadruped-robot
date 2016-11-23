/*
   This program is for this robot http://www.thingiverse.com/thing:1912377   
*/
#include <Arduino.h>
#include "Oscillator.hpp"

//comment out below line if you don't use HC_SR04 sensor
#define __HC_SR04__

#ifdef __HC_SR04__
#define HC_SR04_TRIGGER_PIN 10
#define HC_SR04_ECHO_PIN 11
#define MIN_DISTANCE 10
#define MAX_DISTANCE MIN_DISTANCE + 10
#endif

#define MODE_PIN  12
#define SERVO_START_PIN 2
#define TIMEOUT 5000

#define FORWARD 'f'
#define LEFT 'l'
#define STAND 's'
#define RIGHT 'r'
#define BACKWARD 'b'
#define GO 'g'
#define RIGHT_FRONT 'c'
#define RIGHT_BACK 'e'
#define LEFT_FRONT 'd'
#define LEFT_BACK 'h'

// servo ID
#define FRONT_RIGHT_0 0
#define FRONT_RIGHT_1 1
#define BACK_RIGHT_0  2
#define BACK_RIGHT_1  3
#define BACK_LEFT_0   4
#define BACK_LEFT_1   5
#define FRONT_LEFT_0  6
#define FRONT_LEFT_1  7

typedef struct {
  int index;
  int degree;
  int delay;
} angle_t;


angle_t *forward = (angle_t []) {
  {FRONT_RIGHT_0, 140, 0},
  {FRONT_RIGHT_1, 60, 0},
  {BACK_RIGHT_0, 40, 0},
  {BACK_RIGHT_1, 90, 0},
  {BACK_LEFT_0, 150, 0},
  {BACK_LEFT_1, 60, 0},
  {FRONT_LEFT_0, 55, 0},
  {FRONT_LEFT_1, 90, 200},
  {BACK_LEFT_0, 80, 0},
  {FRONT_LEFT_0, 100, 200},
  {FRONT_RIGHT_1, 90, 0},
  {BACK_RIGHT_1, 120, 0},
  {BACK_LEFT_1, 110, 0},
  {FRONT_LEFT_1, 120, 200},
  {FRONT_RIGHT_0, 80, 0},
  {BACK_RIGHT_0, 100, 0},
  {BACK_RIGHT_1, 90, 0},
  {BACK_LEFT_0, 125, 0},
  {FRONT_LEFT_0, 55, 200},
  { -1, -1, -1}
};

angle_t *backward = (angle_t []) {
  {BACK_LEFT_0, 140, 0},
  {BACK_LEFT_1, 60, 0},
  {FRONT_LEFT_0, 40, 0},
  {FRONT_LEFT_1, 90, 0},
  {FRONT_RIGHT_0, 150, 0},
  {FRONT_RIGHT_1, 60, 0},
  {BACK_RIGHT_0, 55, 0},
  {BACK_RIGHT_1, 90, 200},
  {FRONT_RIGHT_0, 80, 0},
  {BACK_RIGHT_0, 100, 200},
  {BACK_LEFT_1, 90, 0},
  {FRONT_LEFT_1, 120, 0},
  {FRONT_RIGHT_1, 110, 0},
  {BACK_RIGHT_1, 120, 200},
  {BACK_LEFT_0, 80, 0},
  {FRONT_LEFT_0, 100, 0},
  {FRONT_LEFT_1, 90, 0},
  {FRONT_RIGHT_0, 125, 0},
  {BACK_RIGHT_0, 55, 200},
  { -1, -1, -1}
};

#define LEG_UP 50
#define LEG_DN 90
#define LEG_CLOSE 50  // move inward
#define LEG_OPEN 150  // move outward
#define LEG_TURN 125
#define INV(x) (180-x)

angle_t *right = (angle_t []) {
  {FRONT_RIGHT_1, LEG_UP, 0},
  {BACK_LEFT_1,   LEG_UP, 100},
  {FRONT_RIGHT_0, LEG_CLOSE, 0},
  {BACK_LEFT_0,   LEG_CLOSE, 100},
  //
  {FRONT_RIGHT_1, LEG_DN, 0},
  {BACK_LEFT_1,   LEG_DN, 100},
  {BACK_RIGHT_1, LEG_OPEN, 0},
  {FRONT_LEFT_1, LEG_OPEN, 100},
  //
  {FRONT_RIGHT_0, LEG_TURN,  0},
  {BACK_LEFT_0,   LEG_TURN,  100},
  {BACK_RIGHT_1,  LEG_DN,   0},
  {FRONT_LEFT_1,  LEG_DN,   100},
  { -1, -1, -1}
};

angle_t *left = (angle_t []) {
  {FRONT_LEFT_1, INV(LEG_UP), 0},
  {BACK_RIGHT_1, INV(LEG_UP), 100},
  {FRONT_LEFT_0, INV(LEG_CLOSE), 0},
  {BACK_RIGHT_0, INV(LEG_CLOSE), 100},
  //
  {FRONT_LEFT_1, INV(LEG_DN), 0},
  {BACK_RIGHT_1, INV(LEG_DN), 100},
  {BACK_LEFT_1, INV(LEG_OPEN), 0},
  {FRONT_RIGHT_1, INV(LEG_OPEN), 100},
  //
  {FRONT_LEFT_0, INV(LEG_TURN),  0},
  {BACK_RIGHT_0, INV(LEG_TURN),  100},
  {BACK_LEFT_1,  INV(LEG_DN),   0},
  {FRONT_RIGHT_1, INV(LEG_DN),   100},
  { -1, -1, -1}
};



angle_t *stand = (angle_t []) {
  {FRONT_RIGHT_0, 125, 0},
  {FRONT_RIGHT_1, 90, 0},
  {BACK_RIGHT_0, 55, 0},
  {BACK_RIGHT_1, 90, 0},
  {BACK_LEFT_0, 125, 0},
  {BACK_LEFT_1, 90, 0},
  {FRONT_LEFT_0, 55, 0},
  {FRONT_LEFT_1, 90, 150},
  { -1, -1, -1}
};


char movements[] = {FORWARD, LEFT, STAND, RIGHT, BACKWARD};
Oscillator servos[8];
unsigned long cur_time, prev_time;
char action;
char prev_action;
boolean auto_mode = true;

void locomotion(angle_t angles[]);

void setup() {
  int i;
  for (i = 0; i < 8; i++) {
    servos[i].attach(i + SERVO_START_PIN);
  }
  Serial.begin(115200);

#ifdef __HC_SR04__
  pinMode(HC_SR04_TRIGGER_PIN, OUTPUT);
  pinMode(HC_SR04_ECHO_PIN, INPUT);
#endif
  //
  pinMode(MODE_PIN, INPUT);
  auto_mode = digitalRead(MODE_PIN);
  if (auto_mode) {
    analogWrite(A0, 0);
    prev_action = action = FORWARD;
  } else {
    prev_action = action = STAND;
    analogWrite(A0, 128);
  }
  //
  locomotion(stand);
  randomSeed(analogRead(A7));
  cur_time = prev_time = millis();
  delay(3000);
}

void loop() {
  //locomotion(forward);return;
  if (auto_mode) {
    cur_time = millis();
    if (cur_time - prev_time >= TIMEOUT) {
      prev_time = cur_time;
      do {
        action = movements[(int)random(0, 5)];
      } while (action == prev_action || action == STAND);
      //Serial.println(action);
    }
#ifdef __HC_SR04__
    action = detect_obstacle(action);
#endif
  } else {
    if (Serial.available () > 0) {
      action = Serial.read ();
    }
  }

  if (1) {
    long receivedNumber = 0;
    boolean negative = false;
    byte channel = 0;

    switch (action) {
      case '>':
        //        if (negative)
        //          do(-receivedNumber, channel);
        //        else
        //          do(receivedNumber, channel);
        break;
      case  '<':  // fall through to start a new number
        receivedNumber = 0;
        negative = false;
        channel = 0;
        break;
      case GO:
#ifdef __HC_SR04__
        action = detect_obstacle(action);
        if (action == BACKWARD) {
          goto __backward;
        } else if (action == STAND) {
          goto __stand;
        }
#endif
      case FORWARD:
        if (action != prev_action) {
          prev_action = action;
          locomotion(stand);
        }
        locomotion(forward);
        break;

      case BACKWARD: // Go BACK
__backward:
        if (action != prev_action) {
          prev_action = action;
          locomotion(stand);
        }
        locomotion(backward);
        break;
      case RIGHT:
        if (action != prev_action) {
          prev_action = action;
          locomotion(stand);
        }
        locomotion(right);
        break;
      case RIGHT_FRONT:
        locomotion(stand);
        locomotion(right);
        locomotion(stand);
        locomotion(forward);
        locomotion(stand);
        break;
      case RIGHT_BACK:
        locomotion(stand);
        locomotion(right);
        locomotion(stand);
        locomotion(backward);
        locomotion(stand);
        break;
      case LEFT:
        if (action != prev_action) {
          prev_action = action;
          locomotion(stand);
        }
        locomotion(left);
        break;
      case LEFT_FRONT:
        locomotion(stand);
        locomotion(left);
        locomotion(stand);
        locomotion(forward);
        locomotion(stand);
        break;
      case LEFT_BACK:
        locomotion(stand);
        locomotion(left);
        locomotion(stand);
        locomotion(backward);
        locomotion(stand);
        break;
      case STAND:
__stand:
        locomotion(stand);
        break;
      case 'x':
        channel = 1;
        break;
      case 'y':
        channel = 2;
        break;
      case '0' ... '9':
        receivedNumber *= 10;
        receivedNumber += action - '0';
        break;
      case '-':
        negative = true;
        break;
    } // end of switch
  }
}



void locomotion(angle_t angles[]) {
  int i = 0;
  while (1) {
    if (angles[i].degree == -1 ||
        angles[i].index == -1 ||
        angles[i].delay == -1) {
      break;
    }
    servos[angles[i].index].SetPosition(angles[i].degree);
    if (angles[i].delay > 0) delay(angles[i].delay);
    i++;
  }
  //delay(1000);
}


#ifdef __HC_SR04__
char detect_obstacle(char action) {
  char i = action;
  long cm = distance_measure();
  if (cm < MIN_DISTANCE) {
    i = STAND;
  } else if (cm >= MIN_DISTANCE && cm <= MAX_DISTANCE) {
    i = BACKWARD;
  }
  return i;
}

long distance_measure()
{
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:
  long duration, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:

  digitalWrite(HC_SR04_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(HC_SR04_TRIGGER_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(HC_SR04_TRIGGER_PIN, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  duration = pulseIn(HC_SR04_ECHO_PIN, HIGH);

  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  cm =  duration / 29 / 2;
  //Serial.print(cm);
  //Serial.println("cm");
  // delay(100);
  return cm;
}
#endif

