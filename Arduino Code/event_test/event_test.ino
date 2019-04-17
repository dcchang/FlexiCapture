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
  myservo.write(0);
  delay(2000);
}

void loop() {
//  //10 degrees fast - 300 ms total
  int count = 0;
  for (int i=0; i <5; i++) {
    count++;
//    start = millis();
//    myservo.write(17);
//    delay(3000);
//    myservo.write(5);
    for (pos = 5; pos <= 17; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        Serial.println();
        Serial.println("10 degrees fast start " + String(count));
        start_thresh = millis();
        
      }
      delay(250);                       // waits 15ms for the servo to reach the position
     
    }
//    delay(3000);
    for (pos = 17; pos >= 5; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        end_thresh = millis();
        Serial.println();
        Serial.println("10 degrees fast end " + String(count));
      }
      delay(250);                       // waits 15ms for the servo to reach the position
      
    }
//    finish = millis();
    duration = end_thresh - start_thresh;
    Serial.println("Event time\t" + String(duration));
//    delay(3000);
  }
  delay(2000);
   //10 degrees slow - 2 sec total
//  count = 0;
//  for (int i=0; i <5; i++) {
//    count++;
//    Serial.println();
//    Serial.println("10 degrees slow start " + String(count));
//    start = millis();
//    mywrite(17);
//    delay(1000);
//    mywrite(5);
////    for (pos = 5; pos <= 17; pos += 1) { // goes from 0 degrees to 180 degrees
////    // in steps of 1 degree
////    myservo.write(pos);              // tell servo to go to position in variable 'pos'
////    delay(100);                       // waits 15ms for the servo to reach the position
////    }
////    for (pos = 17; pos >= 5; pos -= 1) { // goes from 180 degrees to 0 degrees
////      myservo.write(pos);              // tell servo to go to position in variable 'pos'
////      delay(100);                       // waits 15ms for the servo to reach the position
////    }
//    Serial.println();
//    Serial.println("10 degrees slow end " + String(count));
//    finish = millis();
//    duration = finish - start;
//    Serial.println("Event time\t" + String(duration));
//    delay(1000);
//  }
//  delay(5000);
  //8 degrees fast - 300 ms total
  count = 0;
  for (int i=0; i <5; i++) {
    count++;
//    start = millis();
//    myservo.write(15);
//    delay(3000);
//    myservo.write(5);
    for (pos = 5; pos <= 15; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        Serial.println();
        Serial.println("8 degrees fast start " + String(count));
        start_thresh = millis();
      }
      delay(250);                       // waits 15ms for the servo to reach the position
      
    }
//    delay(2000);
    for (pos = 15; pos >= 5; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        end_thresh = millis();
        Serial.println();
        Serial.println("8 degrees fast end " + String(count));
        
      }
      delay(250);                       // waits 15ms for the servo to reach the position
    }
//    finish = millis();
    duration = end_thresh - start_thresh;
    Serial.println("Event time\t" + String(duration));
//    delay(3000);
  }
  delay(2000);
  //8 degrees slow - 2 sec total
//  count = 0;
//  for (int i=0; i <5; i++) {
//    count++;
//    Serial.println();
//    Serial.println("8 degrees slow start " + String(count));
//    start = millis();
//    for (pos = 5; pos <= 15; pos += 1) { // goes from 0 degrees to 180 degrees
//      // in steps of 1 degree
//      myservo.write(pos);              // tell servo to go to position in variable 'pos'
//      delay(100);                       // waits 15ms for the servo to reach the position
//    }
//    for (pos = 15; pos >= 5; pos -= 1) { // goes from 180 degrees to 0 degrees
//      myservo.write(pos);              // tell servo to go to position in variable 'pos'
//      delay(100);                       // waits 15ms for the servo to reach the position
//    }
//    Serial.println();
//    Serial.println("8 degrees slow end " + String(count));
//    finish = millis();
//    duration = finish - start;
//    Serial.println("Event time\t" + String(duration));
//    delay(1000);
//  }
//  delay(5000);
  //6 degrees fast - 300 ms total
  count = 0;
  for (int i=0; i <5; i++) {
    count++;
//    start = millis();
//    myservo.write(12);
//    delay(3000);
//    myservo.write(5);
    for (pos = 5; pos <= 12; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        Serial.println();
        Serial.println("6 degrees fast start " + String(count));
        start_thresh = millis();
      }
      delay(250);                       // waits 15ms for the servo to reach the position
    }
//    delay(3000);
    for (pos = 12; pos >= 5; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        end_thresh = millis();
        Serial.println();
        Serial.println("6 degrees fast end " + String(count));
      }
      delay(250);                       // waits 15ms for the servo to reach the position
    }
    finish = millis();
    duration = end_thresh - start_thresh;
    Serial.println("Event time\t" + String(duration));
//    delay(3000);
  }
  delay(2000);
  //6 degrees slow - 2 sec total
//  count = 0;
//  for (int i=0; i <5; i++) {
//    count++;
//    Serial.println();
//    Serial.println("6 degrees slow start " + String(count));
//    start = millis();
//    for (pos = 5; pos <= 12; pos += 1) { // goes from 0 degrees to 180 degrees
//      // in steps of 1 degree
//      myservo.write(pos);              // tell servo to go to position in variable 'pos'
//      delay(100);                       // waits 15ms for the servo to reach the position
//    }
//    for (pos = 12; pos >= 5; pos -= 1) { // goes from 180 degrees to 0 degrees
//      myservo.write(pos);              // tell servo to go to position in variable 'pos'
//      delay(100);                       // waits 15ms for the servo to reach the position
//    }
//    Serial.println();
//    Serial.println("6 degrees slow end " + String(count));
//    finish = millis();
//    duration = finish - start;
//    Serial.println("Event time\t" + String(duration));
//    delay(1000);
//  }
//  delay(5000);  
}
