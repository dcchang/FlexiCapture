//script gets bend angle based on changes in voltage 

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
File resting_voltage_file;
File raw_data_file;
File event_file;

//declare variables

// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total;                  // the running total
int average = 0;                // the average

//Voltage values
int resting_flexADC;
int resting_flexADC_average;
int flexADC;
int flexADC_average;
float resting_flexV;
float voltage_1;

const int chipSelect = 10;
const int FLEX_PIN = A1; // Pin connected to voltage divider output

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 3.7; // Measured voltage of Ardunio 5V line
const float R_DIV = 9750.0; // Measured resistance of 3.3k resistor

// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
//const float STRAIGHT_RESISTANCE = 23000; // resistance when straight
//const float BEND_RESISTANCE = 44800.0; // resistance at 90 deg
float flexR;
float flexR_0;
float flexR_90;
float resistance_1;

//time variables
unsigned long start_time;
unsigned long end_time;
unsigned long finish;

struct Data {

  unsigned long current_time;
  float flexV, angle, velocity;

};

Data vector_item; 

std::vector<Data> data_vector; 
std::vector<Data> event_vector; 
//switch for determining whether just in while loop or not
int x = 0;

void setup() {
  //Battery Monitor
  pinMode(8,OUTPUT);
//  pinMode(7,OUTPUT);
//  pinMode(12,OUTPUT);
//
//  digitalWrite(8,HIGH);
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(10, OUTPUT);
  pinMode(FLEX_PIN, INPUT);
  
  if (!SD.begin(SPI_HALF_SPEED,10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  raw_data_file = SD.open("raw_data.txt", FILE_WRITE);
  raw_data_file.println("Time (ms)\tVoltage (V)\tBend Angle (degrees)\tAngular Velocity (degrees/sec)");

  event_file = SD.open("events.txt", FILE_WRITE);
  event_file.println("Time (ms)\tVoltage (V)\tBend Angle (degrees)\tAngular Velocity (degrees/sec)");

  total = 0;
  for (int i=0; i<3000; i++) {
    resting_flexADC = analogRead(FLEX_PIN);
    total = total + resting_flexADC;
    delay(1);
  }
  resting_flexADC_average = total/3000;
  
  resting_flexV = resting_flexADC_average * VCC/ 1023.0;  //Calculate voltage at resting position
  
  resting_voltage_file = SD.open("resting.txt", FILE_WRITE);
  resting_voltage_file.println(resting_flexV); //Stores the resting voltage value onto a txt file
  
  raw_data_file.close();
  event_file.close();
  resting_voltage_file.close();
  
  Serial.println("Resting voltage: " + String(resting_flexV));

  Serial.println("Place flex sensor completely flat at 0 degrees. Getting resistance value at 0 degrees");
  delay(5000);
  
  Serial.println("Beginning calibration...");
  total = 0;
  for (int i=0; i<5000; i++) {
    flexADC = analogRead(FLEX_PIN);
    total = total + flexADC;
    delay(1);
  }
  flexADC_average = total/5000;
  vector_item.flexV = flexADC_average * VCC / 1023.0; //calculate voltage
  flexR_0 = R_DIV*(vector_item.flexV/(VCC-vector_item.flexV));
  Serial.println("Flat Resistance: " + String(flexR_0));
  Serial.println("Flat Voltage: " + String(vector_item.flexV));

  Serial.println("Bend flex sensor at exactly 90 degrees. Getting resistance value at 90 degrees");
  delay(5000);

  Serial.println("Beginning calibration...");
  total = 0;
  for (int i=0; i<5000; i++) {
    flexADC = analogRead(FLEX_PIN);
    vector_item.flexV = flexADC * VCC / 1023.0; //calculate voltage
    total = total + flexADC;
    delay(1);
  }
  average = total/5000;
  vector_item.flexV = average * VCC / 1023.0; //calculate voltage
  flexR_90 = R_DIV*(vector_item.flexV/(VCC-vector_item.flexV));
  Serial.println("Bend Resistance: " + String(flexR_90));
  
  // initialize all the readings to resting_flexADC:
  total = 0;
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = resting_flexADC_average;
    total += readings[thisReading];
  }
  
  start_time = millis();
  data_vector.push_back({0,0,0,0});

  resistance_1 = map(5.0,0,90,flexR_0,flexR_90);
  voltage_1 = VCC *(resistance_1)/(R_DIV+resistance_1);
  Serial.println(voltage_1);
}

void loop() {
  
  float batterylevel = analogRead(A3);
//  Serial.println("BatteryVoltage = ");
//  Serial.println(batterylevel*VCC/1023);
  if ((batterylevel*VCC/1023) < VCC) {
    digitalWrite(8,LOW);
//    digitalWrite(7,HIGH);
  }
  else {
    digitalWrite(8,HIGH);
//    digitalWrite(7,LOW);
  }
 
  
  // initialize all the readings to resting_flexADC if the sketch was previously in the while loop:
  if (x == 1) {
    x = 0;
    total = 0;
    for (int thisReading = 0; thisReading < numReadings; thisReading++) {
      readings[thisReading] = resting_flexADC_average;
      total += readings[thisReading];
    }
  }
  ///Calculate moving average 
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

  //Calculate voltage and resistance
  vector_item.flexV = average * VCC / 1023.0; //calculate voltage
  Serial.println("OUTSIDE WHILE LOOP");
  Serial.println("Voltage Value! " + String(vector_item.flexV));
  
  flexR = R_DIV*(vector_item.flexV/(VCC-vector_item.flexV));
  Serial.println("Resistance: " + String(flexR) + " ohms");

  // Use the calculated resistance to estimate the sensor's
  // bend angle:
  vector_item.angle = map(flexR, flexR_0, flexR_90, 0, 90.0);
  Serial.println("Bend: " + String(vector_item.angle) + " degrees");
  Serial.println();

  end_time = millis();

  vector_item.current_time = end_time - start_time;

  vector_item.velocity = (vector_item.angle-data_vector[data_vector.size()-1].angle)/(vector_item.current_time-data_vector[data_vector.size()-1].current_time);
  Serial.println("Angular Velocity: " + String(vector_item.velocity));

 
  
  data_vector.push_back({vector_item.current_time,vector_item.flexV,vector_item.angle,vector_item.velocity});
  
  float range_low = resting_flexV - 0.15;
  float range_high = resting_flexV + 0.15;
//  float range_low = resting_flexV - voltage_1;
//  float range_high = resting_flexV + voltage_1;
  
  raw_data_file = SD.open("raw_data.txt", FILE_WRITE);
  event_file = SD.open("events.txt", FILE_WRITE);
  //everytime flex sensor voltage is outside of range of resting voltage, then record time this takes place for with corresponding voltages and store values on SD card as single instance
  int counter = 0;
  while (vector_item.flexV <= range_low || vector_item.flexV >= range_high) {
    counter +=1;
    if (x != 1) {
      x = 1;
      total = 0;
      // initialize all the readings to average:
      for (int thisReading = 0; thisReading < numReadings; thisReading++) {
        readings[thisReading] = average;
        total += readings[thisReading];
      }
    }   
    ///Calculate moving average 
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

    //Calculate voltage and resistance
    vector_item.flexV = average * VCC / 1023.0; //calculate voltage
    Serial.println("INSIDE WHILE LOOP");
    Serial.println("VOLTAGE " + String(vector_item.flexV));
    flexR = R_DIV*(vector_item.flexV/(VCC-vector_item.flexV));
    Serial.println("Resistance: " + String(flexR) + " ohms");
  
    // Use the calculated resistance to estimate the sensor's
    // bend angle:
    vector_item.angle = map(flexR, flexR_0, flexR_90, 0, 90.0);
    Serial.println("Bend: " + String(vector_item.angle) + " degrees");
    Serial.println();

    finish = millis();
    
    vector_item.current_time = finish - start_time;
    
    vector_item.velocity = (vector_item.angle-data_vector[data_vector.size()-1].angle)/(vector_item.current_time-data_vector[data_vector.size()-1].current_time);
   
    
    //Add values to vectors
    data_vector.push_back({vector_item.current_time,vector_item.flexV,vector_item.angle,vector_item.velocity}); 
    event_vector.push_back({vector_item.current_time,vector_item.flexV,vector_item.angle,vector_item.velocity}); 
    if (counter == 100) {
      for (int i=0; i<data_vector.size(); i++) {
        raw_data_file.println(String(data_vector[i].current_time) + "\t" + String(data_vector[i].flexV) + "\t" + String(data_vector[i].angle) + "\t" + String(data_vector[i].velocity));
        counter += 1;
        Serial.println("Writing data to SD Card" + String(counter));
      }
      raw_data_file.flush();
      for (int i=0; i<data_vector.size(); i++) {
        event_file.println(String(event_vector[i].current_time) + "\t" + String(event_vector[i].flexV) + "\t" + String(event_vector[i].angle)+ "\t" + String(event_vector[i].velocity));
        counter += 1;
        Serial.println("Writing event to SD Card" + String(counter));
      }
      event_file.flush();
      data_vector.clear();
      event_vector.clear();
      counter = 0;
    }
   
    delay(25);
  }

//    //write values to sd card if memory reaches certain capacity (download Memoryfree library maybe)
//    //if number of values in array >= some number (2400), then empty/overwrite vector
  if (data_vector.size() >= 10) {
    for (int i=0; i< data_vector.size(); i++) {
      raw_data_file.println(String(data_vector[i].current_time) + "\t" + String(data_vector[i].flexV) + "\t" + String(data_vector[i].angle) + "\t" + String(data_vector[i].velocity));
      Serial.println("Writing to SD Card");
    }
    raw_data_file.flush();
    data_vector.clear();
  }
  if (event_vector.size() >= 1) {
    for (int i=0; i< event_vector.size(); i++) {
      event_file.println(String(event_vector[i].current_time) + "\t" + String(event_vector[i].flexV) + "\t" + String(event_vector[i].angle) + "\t" + String(event_vector[i].velocity));
      Serial.println("OUTSIDE LOOP. Writing event to SD Card");
    }
    event_file.println();
    event_file.flush();
    event_vector.clear();
    
  }

  raw_data_file.close();
  event_file.close();
  delay(25); //measure data at intervals of 25 milliseconds
}
