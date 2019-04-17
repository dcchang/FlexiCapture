#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos;    // variable to store the servo position

const int FLEX_PIN = A1; // Pin connected to voltage divider output

const float VCC = 5.0; // Measured voltage of Ardunio 5V line
const float R_DIV = 9.8; // Measured resistance of 3.3k resistor

int total;
int average=0;
int flexADC;
float flexV;
void setup() {
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
  pinMode(FLEX_PIN, INPUT);
  delay(5000);
}

void loop() {
  int angle = 0;
  for (pos = 5; pos <= 131; pos += 7) { // goes from 0 degrees to 180 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'   
    total = 0;
    for (int i=0; i < 1500; i++) {
      flexADC = analogRead(FLEX_PIN);
//      Serial.println(flexADC);
      total = total + flexADC;
      Serial.println(total); 
    }
    Serial.println(total);
    average = total/1500;
    flexV = average*VCC/1023.0;
    Serial.println("Angle" + String(angle) + "\t" + String(flexV));
//    flexADC = analogRead(FLEX_PIN);
    
//    Serial.println("Angle" + String(angle) + "\t" + String(flexV));
    angle += 5;
    delay(2000);
  }

  for (pos = 131; pos >= 5; pos -= 7) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(200);                       // waits 15ms for the servo to reach the position
  }
//  delay(7000);
}
