//sketch used for basic flex sensor testing while flex sensor is connected to Arduino Uno via breadboard.
//Moving average is used to give more accurate voltage readings

const int FLEX_PIN = A1; // Pin connected to voltage divider output

const float VCC = 5; // Measured voltage of Ardunio 5V line
const float R_DIV = 9.8; // Measured resistance of 10k resistor (R1 in voltage divider)

//variables for moving average of voltage value
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

float flexV;

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
  //calculate moving average for voltage values
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
 
  flexV = average * VCC/1023.0;  //Convert voltage value into volts
  Serial.println(flexV);
  delay(25);
}
