/******************************************************************************
Flex_Sensor_Example.ino
Example sketch for SparkFun's flex sensors
  (https://www.sparkfun.com/products/10264)
Jim Lindblom @ SparkFun Electronics
April 28, 2016

Create a voltage divider circuit combining a flex sensor with a 47k resistor.
- The resistor should connect from A0 to GND.
- The flex sensor should connect from A0 to 3.3V
As the resistance of the flex sensor increases (meaning it's being bent), the
voltage at A0 should decrease.

Development environment specifics:
Arduino 1.6.7
******************************************************************************/
const int FLEX_PIN = A1; // Pin connected to voltage divider output

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 5; // Measured voltage of Ardunio 5V line
const float R_DIV = 9.8; // Measured resistance of 3.3k resistor

// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
const float STRAIGHT_RESISTANCE = 85.6; // resistance when straight
const float BEND_RESISTANCE = 407.0; // resistance at 90 deg

const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average



void setup() 
{
  Serial.begin(9600);
  pinMode(FLEX_PIN, INPUT);
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

void loop() 
{
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(FLEX_PIN);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
  float flexV = average * VCC/1023.0;
  Serial.println(flexV);
//  delay(1);        // delay in between reads for stability
//  float flexR = R_DIV * (VCC / flexV - 1.0);
//  Serial.println("Resistance: " + String(flexR) + " kohms");
//
//  // Use the calculated resistance to estimate the sensor's
//  // bend angle:
//  float angle = map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE,
//                   0, 90.0);
//  Serial.println("Bend: " + String(angle) + " degrees");
//  Serial.println();

  delay(25);
}
