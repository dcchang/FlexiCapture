/*sketch stores voltages and logs time as flex sensor is bent.
Sketch considers event to be anytime voltage is different from resting voltage.
Sketch logs running timer and voltage values on raw data file only when event is occurring (so it doesn't log all data), 
and times of each event on separate event file.
Outputs: resting.txt --> resting voltage value
         raw_data.txt --> elapsed time and voltages for each event
         events.txt --> Time of each finger event
*/
#include <ArduinoSTL.h>
#include <SPI.h>
#include <SD.h>

//
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif
//
////declare files
File raw_data_file;
File finger_events_file;
File resting_voltage_file;

//declare variables

// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

//Voltage values
float resting_flexV;
float flexV;

const int chipSelect = 10;
const int FLEX_PIN = A1; // Pin connected to voltage divider output

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 3.7; // Measured voltage of Ardunio 5V line
const float R_DIV = 9750.0; // Measured resistance of 3.3k resistor


//time variables
unsigned long start;
unsigned long finish;
unsigned long elapsed_time;
float event_time;

std::vector<std::pair<unsigned long, float>> data_vector; //create a vector of pairs to store time and voltage

void setup() {
  //Battery Monitor
  pinMode(8,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(12,OUTPUT);

  digitalWrite(8,HIGH);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(FLEX_PIN, INPUT);
  
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
 
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  raw_data_file = SD.open("raw_data.txt", FILE_WRITE);
  raw_data_file.println("Time (ms)\tVoltage (V)");

  finger_events_file = SD.open("events.txt", FILE_WRITE);
  finger_events_file.println("Time (sec)");
  
//Use smoothing to get resting voltage value
    for (int i=0; i<1000; i++) {
    // subtract the last reading:
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

    delay(1);        // delay in between reads for stability
  }
  
  resting_flexV = average * VCC/ 1023.0;  //Calculate voltage at resting position
  
  resting_voltage_file = SD.open("resting.txt", FILE_WRITE);
  resting_voltage_file.println(resting_flexV); //Stores the resting voltage value onto a txt file
  
  raw_data_file.close();
  finger_events_file.close();
  resting_voltage_file.close();
  
  Serial.println("Resting voltage: " + String(resting_flexV));
}

void loop() {
  
  float batterylevel = analogRead(A3);
  SerialUSB.print("BatteryVoltage = ");
  SerialUSB.println(batterylevel*VCC/1023);
  if ((batterylevel*VCC/1023) < VCC) {
    digitalWrite(8,LOW);
    digitalWrite(7,HIGH);
  }
  else {
    digitalWrite(8,HIGH);
    digitalWrite(7,LOW);
  }
  
  // Read the ADC, and calculate voltage and resistance from it
  delay(15);

  ///Calculate moving average 
  for (int i=0; i<10; i++) {
    // subtract the last reading:
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

    delay(1);        // delay in between reads for stability
  }
  
  flexV = average * VCC / 1023.0; //calculate voltage
  Serial.println("Voltage Value! " + String(flexV));
  start = millis();  //start time for event
  
  raw_data_file = SD.open("raw_data.txt", FILE_WRITE);
  //everytime flex sensor voltage is not equal to resting voltage, then record time this takes place for with corresponding voltages and store values on SD card
  while (flexV != resting_flexV) {
        ///Calculate moving average 
    for (int i=0; i<10; i++) {
      // subtract the last reading:
      total = total - readings[readIndex];
      // read from the sensor:
    //   int flexADC = analogRead(FLEX_PIN); // Reads the voltage at pin A0 (Vout) and stores that in flexADC
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

      delay(1);        // delay in between reads for stability
    }

    flexV = average * VCC / 1023.0; //calculate voltage
    Serial.println("VOLTAGE " + String(flexV));
   
    finish = millis();  //end time

    elapsed_time = finish-start;  //Elapsed time of event. For every new event, this value starts over from 0
    
    //Create a vector that stores time and voltages together

    data_vector.push_back(std::make_pair(elapsed_time,flexV));
    Serial.println("Time: " + String(elapsed_time) + "  Voltage: " + String(flexV));
    Serial.println(data_vector.size());
    float flexR = R_DIV * (VCC / flexV - 1.0);
    Serial.println("Resistance: " + String(flexR) + " ohms");

    //write values to sd card if memory reaches certain capacity
    //if number of values in array >= some number (2400), then empty/overwrite vector
    if (data_vector.size() >= 2400) {
      for (const auto& p : data_vector) {
        Serial.println(String(p.first) + "\t" + String(p.second));
        raw_data_file.println(String(p.first) + "\t" + String(p.second));
        raw_data_file.flush();
      }
      data_vector.clear();
    }
    delay(15); //measure data at intervals of 25 milliseconds
  }
  
//Once you exit while loop, unload remaining contents of data vector to text file and add a newline for the next event
  
  if (data_vector.size() >= 1) {
    for (const auto& p : data_vector) {
      raw_data_file.println(String(p.first) + "\t" + String(p.second));
    }
    raw_data_file.println('\n');
    data_vector.clear();
    
  //record total time of finger movement
    finger_events_file = SD.open("events.txt", FILE_WRITE);
    event_time = data_vector.back().first/1000.0;
    finger_events_file.println(event_time,3);
    Serial.println("Time of event " + String(event_time));
    finger_events_file.close();
    
  } 
  raw_data_file.close();
}
