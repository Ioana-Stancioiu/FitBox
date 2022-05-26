#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// sampling size
#define N 100
#define BUZZER 9
#define CALORIES_PER_STEP 0.03877
#define METERS_PER_STEP 0.5
#define SEC10 10000

Adafruit_ADXL345_Unified accel;

float xavg = 0, yavg = 0, zavg = 0;
float threshold = 0.5;
int steps = 0;
int stored_steps = 0;
bool flag = false;
bool reachedGoal = false;
float calories = 0;
float distance = 0;
String activityStatus = "Move!";
long stored_time;

void setup() {
  pinMode(BUZZER, OUTPUT);
  stored_time = millis();
  accel = Adafruit_ADXL345_Unified(1234);
  
  Serial.begin(9600);

  // check if ADXL345 is connected
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while(1);
  }

  accel.setRange(ADXL345_RANGE_4_G);

  readAvgAccel();
}

void loop() {
    countSteps();
    checkNumSteps();
    checkMovement();
    sendData();
}

// calculates average acceleration
// used for calibrating accelerometer
void readAvgAccel() {
  sensors_event_t event;  
  for (int i = 0; i < N; i++) {
    accel.getEvent(&event);
    xavg += event.acceleration.x;
    yavg += event.acceleration.y;
    zavg += event.acceleration.z;
  }
  
  xavg /= N;
  yavg /= N;
  zavg /= N;
}

void countSteps() {
   float acc1 = getAccel();
   delay(20);
   float acc2 = getAccel();

   float delta = acc2 - acc1;

    // if difference is bigger than threshold a step was taken
    if (delta > threshold && !flag) {
        steps++;
        
        reachedGoal = false;
        flag = true;
        
        if (activityStatus == "Move!" || 
          activityStatus == "Good job!") {
          activityStatus = "One more!";
        }
    } else if (delta < threshold && flag) {
        flag = false;
    }
}

// calculates acceleration magnitude
float getAccel() {
   sensors_event_t event;
   float xacc = 0, yacc = 0, zacc = 0;
   float acc = 0;
   
   for (int i = 0; i < N; i++) {
      accel.getEvent(&event);
      xacc += event.acceleration.x;
      yacc += event.acceleration.y;
      zacc += event.acceleration.z;
   }

   xacc /= N;
   yacc /= N;
   zacc /= N;

  acc = sqrt(((xacc - xavg) * (xacc - xavg)) +
             ((yacc - yavg) * (yacc - yavg)) +
             ((zacc - zavg) * (zacc - zavg)));
        
  return acc;
}

// checks user's activity
void checkMovement() {
  // if 10 seconds have passed without taking a
  // step, activate buzzer
  if (millis() - stored_time >= SEC10) {
       stored_time = millis();
       if (stored_steps == steps) {
         activityStatus = "Move!";
         tone(BUZZER, 1000, 500);
       } else {
         stored_steps = steps;
       }
   }
}

// checks user's progress
void checkNumSteps() {
  // congratulate user at intervals of 10 steps
  if (reachedGoal)
     return;
  
  if (steps && steps % 10 == 0) {
      reachedGoal = true;
      activityStatus = "Good job!";
      tone(BUZZER, 440, 500);
  }
}

// send data to bluetooth application
void sendData() {
    calories = CALORIES_PER_STEP * steps;
    distance = (METERS_PER_STEP * steps) / 100;
    Serial.print(steps);
    Serial.print(",");
    Serial.print(calories);
    Serial.print(",");
    Serial.print(distance);
    Serial.print(",");
    Serial.print(activityStatus);
    Serial.print(";");
}
