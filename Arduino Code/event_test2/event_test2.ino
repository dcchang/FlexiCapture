/*This sketch is the second version used for event testing. This sketch controls a servo which is attached to the flex sensor
The servo moves to 10 degrees then back to 0 degrees. It does this again for 8 degrees and 6 degrees. The servo moves at the same 
speed for all angles.
*/

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos=0;    // variable to store the servo position
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

  //10 degrees - takes about 3 seconds to reach 10 degrees

  //Bend sensor to 10 degrees and back to 0 degrees 5 times
  int count = 0;
  for (int i=0; i <5; i++) {
    count++;
    for (pos = 5; pos <= 17; pos += 1) { // goes from 0 degrees to 10 degrees, pos values adjusted based on calibration
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      //Pos == 11 corresponds to when servo moves to 5 degrees. 5 degrees is the threshold used to identify finger events
      //When flex sensor bends to 5 degrees, start recording time of event.
      if (pos == 11) {
        Serial.println();
        Serial.println("10 degrees start " + String(count));
        start_thresh = millis();
        
      }
      delay(250);                       // waits 250ms for the servo to reach each position in the for loop - controls speed of servo
     
    }
    for (pos = 17; pos >= 5; pos -= 1) { // goes from 10 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      //When servo moves back to 5 degrees, stop recording time of event as sensor is moving below 5 degree threshold.
      if (pos == 11) {
        end_thresh = millis();
        Serial.println();
        Serial.println("10 degrees end " + String(count));
      }
      delay(250);                       // waits 250ms for the servo to reach the position
      
    }
    //Duration is actual time of event. Can compare this measured time of event from flex sensor using SD card data
    duration = end_thresh - start_thresh;  //Subtract the end time from the start time to get duration of time servo is in position above 5 degree threshold.
    Serial.println("Event time\t" + String(duration));
  }
  delay(2000);  //delay 2 seconds between 10 degree testing and 8 degree testing

 //8 degree testing - process is same as 10 degrees
  count = 0;
  for (int i=0; i <5; i++) {
    count++;
    for (pos = 5; pos <= 15; pos += 1) { // goes from 0 degrees to 8 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        Serial.println();
        Serial.println("8 degrees start " + String(count));
        start_thresh = millis();
      }
      delay(250);                       // waits 250ms for the servo to reach the position
      
    }
    for (pos = 15; pos >= 5; pos -= 1) { // goes from 8 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        end_thresh = millis();
        Serial.println();
        Serial.println("8 degrees end " + String(count));
        
      }
      delay(250);                       // waits 250ms for the servo to reach the position
    }
    duration = end_thresh - start_thresh;
    Serial.println("Event time\t" + String(duration));
  }
  delay(2000);
 
  //6 degrees test
  count = 0;
  for (int i=0; i <5; i++) {
    count++;
    for (pos = 5; pos <= 12; pos += 1) { // goes from 0 degrees to 6 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        Serial.println();
        Serial.println("6 degrees start " + String(count));
        start_thresh = millis();
      }
      delay(250);                       // waits 250ms for the servo to reach the position
    }

    for (pos = 12; pos >= 5; pos -= 1) { // goes from 6 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      if (pos == 11) {
        end_thresh = millis();
        Serial.println();
        Serial.println("6 degrees end " + String(count));
      }
      delay(250);                       // waits 250ms for the servo to reach the position
    }
    finish = millis();
    duration = end_thresh - start_thresh;
    Serial.println("Event time\t" + String(duration));

  }
  delay(2000);
}
