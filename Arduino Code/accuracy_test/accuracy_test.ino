//Sketch for conducting accuracy testing of flex sensor
//Getting voltage values from this sketch only relevant when flex sensor plugged into breadboard and not wired to TinyZero.

#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos=0;    // variable to store the servo position

//For testing flex sensor with flex sensor plugged into breadboard
const int FLEX_PIN = A1; // Pin connected to voltage divider output
const float VCC = 5.0; // Measured voltage of Ardunio 5V line
const float R_DIV = 9.8; // Measured resistance (kohms) of resistor (R1) in series with flex sensor
int flexADC;  //analog reading of input voltage from circuit
float flexV;  //voltage reading in volts

void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
  pinMode(FLEX_PIN, INPUT);  //assigns FLEX_PIN (A1) as input pin to read in voltage. 
  delay(5000);  
}

void loop() {
  int angle = 0;
  for (pos = 5; pos <= 131; pos += 7) { // goes from 0 degrees to 90 degrees (calibrated using 5 as 0 degrees and 131 as 90 degrees as servo was not perfectly accurate)
    myservo.write(pos);              // tell servo to go to position in variable 'pos'   
    
    //at every angle position, read in output voltage up to 1500 times and allow voltage reading to stabilize
    //only applicable when flex sensor plugged into bread board
    for (int i=0; i < 1500; i++) {
      flexADC = analogRead(FLEX_PIN);  //read in voltage
      delay(1);
    }
    //Calculate and display output voltage 
    flexV = flexADC*VCC/1023.0;  //convert analog value into typical voltage reading by multiplying flexADC by operating voltage and divide by 1023 units (determined by resolution)
    Serial.println("Angle" + String(angle) + "\t" + String(flexV));
    angle += 5;  //Increment angle by 5 degrees
    delay(2000);  //delay for 2 seconds in between angles
  }
  //return back to starting position (0 degrees)
  for (pos = 131; pos >= 5; pos -= 7) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(200);                       // waits 200ms for the servo to reach the position
  }
}
