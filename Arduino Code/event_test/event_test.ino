//This sketch is the first version used for event testing. This sketch controls a servo which is attached to the flex sensor
//The servo moves to 10 degrees then back to 0 degrees. It does this again for 8 degrees and 6 degrees. The servo moves fast 
//and slow for all angles to compare flex sensor's ability to respond to different movement speeds.

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos=0;    // variable to store the servo position
long start;
long finish;
long duration;
long start_thresh;
long end_thresh;

void setup() {
  Serial.begin(9600);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo.write(5);  //set servo to 0 position to start, 5 = 0 degrees based on calibration
  delay(2000);  //wait 2 seconds before beginning
}

void loop() {
  //10 degrees fast - 390 ms total time for servo to move from 
  //0 to 10 and back to 0.

  count = 0;
  for (int i=0; i <5; i++) {
    count++;
    Serial.println();
    Serial.println("10 degrees fast start " + String(count));
    start = millis();  //start recording time of event
    
    for (pos = 5; pos <= 17; pos += 1) { // goes from 0 degrees to 10 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = 17; pos >= 5; pos -= 1) { // goes from 10 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    Serial.println();
    Serial.println("10 degrees fast end " + String(count));
    finish = millis();  //stop recording time of event
    //Duration is actual time of event. Can compare this measured time of event from flex sensor using SD card data
    duration = finish - start;  //Subtract the end time from the start time to get duration of servo movement
    Serial.println("Event time\t" + String(duration));
    delay(1000);  //wait one second in between trials
  }
  delay(5000);  //wait 5 seconds in between different bending movements

  //10 degrees slow - 2.6 sec total time
  count = 0;
  for (int i=0; i <5; i++) {
    count++;
    Serial.println();
    Serial.println("10 degrees slow start " + String(count));
    start = millis();
    for (pos = 5; pos <= 17; pos += 1) { // goes from 0 degrees to 10 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(100);                       // waits 100ms for the servo to reach the position
    }
    for (pos = 17; pos >= 5; pos -= 1) { // goes from 10 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(100);                       // waits 100ms for the servo to reach the position
    }
    Serial.println();
    Serial.println("10 degrees slow end " + String(count));
    finish = millis();
    duration = finish - start;
    Serial.println("Event time\t" + String(duration));
    delay(1000);
  }
  delay(5000);
  
  // 8 degrees fast - 390 ms total time
  count = 0;
  for (int i=0; i <5; i++) {
    count++;
    Serial.println();
    Serial.println("8 degrees fast start " + String(count));
    start = millis();
   
    for (pos = 5; pos <= 15; pos += 1) { // goes from 0 degrees to 8 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = 15; pos >= 5; pos -= 1) { // goes from 8 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    Serial.println();
    Serial.println("8 degrees fast end " + String(count));
    finish = millis();
    duration = finish - start;
    Serial.println("Event time\t" + String(duration));
    delay(1000);
  }
  delay(5000);

  //8 degrees slow - 2.6 sec total time
  count = 0;
  for (int i=0; i <5; i++) {
    count++;
    Serial.println();
    Serial.println("8 degrees slow start " + String(count));
    start = millis();
    for (pos = 5; pos <= 15; pos += 1) { // goes from 0 degrees to 8 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(100);                       // waits 100ms for the servo to reach the position
    }
    for (pos = 15; pos >= 5; pos -= 1) { // goes from 8 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(100);                       // waits 100ms for the servo to reach the position
    }
    Serial.println();
    Serial.println("8 degrees slow end " + String(count));
    finish = millis();
    duration = finish - start;
    Serial.println("Event time\t" + String(duration));
    delay(1000);
  }
  delay(5000);
  
  //6 degrees fast - 390 ms total time

  count = 0;
  for (int i=0; i <5; i++) {
    count++;
    Serial.println();
    Serial.println("6 degrees fast start " + String(count));
    start = millis();

    for (pos = 5; pos <= 12; pos += 1) { // goes from 0 degrees to 6 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = 12; pos >= 5; pos -= 1) { // goes from 6 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    Serial.println();
    Serial.println("6 degrees fast end " + String(count));
    finish = millis();
    duration = finish - start;
    Serial.println("Event time\t" + String(duration));
    delay(1000);
  }
  delay(5000);

  //6 degrees slow - 2.6 sec total time
  count = 0;
  for (int i=0; i <5; i++) {
    count++;
    Serial.println();
    Serial.println("6 degrees slow start " + String(count));
    start = millis();
    for (pos = 5; pos <= 12; pos += 1) { // goes from 0 degrees to 6 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(100);                       // waits 100ms for the servo to reach the position
    }
    for (pos = 12; pos >= 5; pos -= 1) { // goes from 6 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(100);                       // waits 100ms for the servo to reach the position
    }
    Serial.println();
    Serial.println("6 degrees slow end " + String(count));
    finish = millis();
    duration = finish - start;
    Serial.println("Event time\t" + String(duration));
    delay(1000);
  }
  delay(5000);  
}
