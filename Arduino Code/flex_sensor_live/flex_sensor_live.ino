//This is an old version of the TinyZero code. This code is useful for seeing bend angles, angular velocity, and event data in real time
//But this version is more memory intensive and not good for long term data logging.
//Much of code here, such as calculating bend angles and identifying events, was moved to post processing script to save memory
//Reduces memory usage reduces amount of raw data lost during data logging and storing process.

//Script stores all data in raw data file
//Script also stores data that meets conditions as events in separate event file
//Script calculates bend angle and angular velocity in real time

//script gets bend angle based on changes in voltage 

//Outputs: resting.txt --> resting voltage value
//         raw_data.txt --> running time, voltages, angles, angular velocities
//         events.txt --> running time, voltages, angles, angular velocities when voltage is greater than 5 degree threshold

//Libraries
#include <ArduinoSTL.h>  //to use vectors
#include <SPI.h>  //for storing data to SD card
#include <SD.h>   //for storing data to SD Card

//used for displaying results to Serial Monitor when using a TinyZero
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

////declare files to be created on SD Card
File resting_voltage_file;  //Contains resting voltage values
File raw_data_file;  //Contains raw data
File event_file;  //Isolates data that constitutes an event from raw data

//declare variables

//Moving average variables
// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total;                      // the running total
int average = 0;                // the average

const int chipSelect = 10;  //to set up microSD card shield
const int FLEX_PIN = A1; // Pin connected to voltage divider output

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 3.7; // Measured voltage of Ardunio 3.7V line
const float R_DIV = 9.75; // Measured resistance of 10k resistor

//Variables for calculating volage
int resting_flexADC;
int resting_flexADC_average;
int flexADC;
int flexADC_average;
float resting_flexV;
float voltage_5;

//Variables for calculating resistance
float flexR;
float flexR_0;
float flexR_90;
float resistance_5;

//time variables
unsigned long start_time;
unsigned long end_time;
unsigned long finish;

//data structure for storing information that will be written to SD card
struct Data {

  unsigned long current_time;
  float flexV, angle, velocity;

};

Data vector_item; 

std::vector<Data> data_vector;  //Store all values in this vector
std::vector<Data> event_vector;  //Only store values that correspond to events in this vector

//switch for determining whether just in while loop or not
int x = 0;

void setup() {
  //Battery Monitor
  pinMode(7,OUTPUT);

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
  raw_data_file.println("Time (ms)\tVoltage (V)\tBend Angle (degrees)\tAngular Velocity (degrees/sec)");  //header for raw data file

  event_file = SD.open("events.txt", FILE_WRITE);
  event_file.println("Time (ms)\tVoltage (V)\tBend Angle (degrees)\tAngular Velocity (degrees/sec)");     //header for events file

  //get resting voltage value by reading in value 3000 times and getting average
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

  //Get the voltage and resistance values at 0 degrees
  Serial.println("Place flex sensor completely flat at 0 degrees. Getting resistance value at 0 degrees in 5 seconds.");
  delay(5000);

  //Get average value
  Serial.println("Beginning calibration...");
  total = 0;
  for (int i=0; i<5000; i++) {
    flexADC = analogRead(FLEX_PIN);
    total = total + flexADC;
    delay(1);
  }
  flexADC_average = total/5000;
  vector_item.flexV = flexADC_average * VCC / 1023.0; //calculate voltage
  flexR_0 = R_DIV*(vector_item.flexV/(VCC-vector_item.flexV));  //calculate resistance
  Serial.println("Flat Resistance: " + String(flexR_0));

  //Get the voltage and resistance values at 90 degrees. This and values at 0 degrees are used to get bend angles
  Serial.println("Bend flex sensor at exactly 90 degrees. Getting resistance value at 90 degrees in 5 seconds.");
  delay(5000);

  //Get average value
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
  flexR_90 = R_DIV*(vector_item.flexV/(VCC-vector_item.flexV));  //calculate resistance
  Serial.println("Bend Resistance: " + String(flexR_90));
  
  // initialize all the readings to resting_flexADC
  total = 0;
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = resting_flexADC_average;
    total += readings[thisReading];
  }
  
  start_time = millis();  //Begin start time for calculating current time
  data_vector.push_back({0,0,0,0});  //initialize vector with 0 values. Needed to calculate angular velocity.

  resistance_5 = map(5.0,0,90,flexR_0,flexR_90);  //Use this to determine resistance value that corresponds to 5 degrees based on 0 and 90 degree resistance values
  voltage_5 = VCC *(resistance_1)/(R_DIV+resistance_1);  //Uses resistance value to determine voltage value that corresponds to 5 degrees
  Serial.println(voltage_5);
}

void loop() {

  //read in battery level
  float batterylevel = analogRead(A3);
  Serial.println("BatteryVoltage = ");
  Serial.println(batterylevel*VCC/1023);
  if ((batterylevel*VCC/1023) < 3.3) {
    digitalWrite(7,HIGH);
  }
  else {
    digitalWrite(7,LOW);
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
  
  ///Calculate moving average for voltage reading
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

  // Use the calculated resistance to estimate the sensor's bend angle
  vector_item.angle = map(flexR, flexR_0, flexR_90, 0, 90.0);
  Serial.println("Bend: " + String(vector_item.angle) + " degrees");
  Serial.println();

  end_time = millis();

  //get updated time value by subtracted end time from start time
  vector_item.current_time = end_time - start_time;

  //calculate instantaneous angular velocity
  vector_item.velocity = (vector_item.angle-data_vector[data_vector.size()-1].angle)/(vector_item.current_time-data_vector[data_vector.size()-1].current_time);
  Serial.println("Angular Velocity: " + String(vector_item.velocity)); 
  
  data_vector.push_back({vector_item.current_time,vector_item.flexV,vector_item.angle,vector_item.velocity}); //store time, voltage, angle and angular velocity in vector
  
  raw_data_file = SD.open("raw_data.txt", FILE_WRITE);
  event_file = SD.open("events.txt", FILE_WRITE);
 
  //everytime flex sensor voltage is greater than 5 degree threshold, also record data in separate event text file
  int counter = 0;
  while (vector_item.flexV > voltage_5) {
    counter +=1;

    //if not previously in while loop, then assign x = 1 and reset moving average to most recent average value
    if (x != 1) {
      x = 1;
      total = 0;
      // initialize all the readings to average:
      for (int thisReading = 0; thisReading < numReadings; thisReading++) {
        readings[thisReading] = average;
        total += readings[thisReading];
      }
    }   
    ///Calculate moving average for voltage
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
  
    // Use the calculated resistance to estimate the sensor's bend angle:
    vector_item.angle = map(flexR, flexR_0, flexR_90, 0, 90.0);
    Serial.println("Bend: " + String(vector_item.angle) + " degrees");
    Serial.println();

    finish = millis();
    
    vector_item.current_time = finish - start_time;  //Get updated current time when in while loop
    
    vector_item.velocity = (vector_item.angle-data_vector[data_vector.size()-1].angle)/(vector_item.current_time-data_vector[data_vector.size()-1].current_time);
   
    
    //Add values to vectors
    data_vector.push_back({vector_item.current_time,vector_item.flexV,vector_item.angle,vector_item.velocity}); 
    event_vector.push_back({vector_item.current_time,vector_item.flexV,vector_item.angle,vector_item.velocity}); 

    //Write data to SD card files every 100 items and then clear contents of vectors to prevent memory overload on Arduino board
    if (counter == 100) {
      for (int i=0; i<data_vector.size(); i++) {
        raw_data_file.println(String(data_vector[i].current_time) + "\t" + String(data_vector[i].flexV) + "\t" + String(data_vector[i].angle) + "\t" + String(data_vector[i].velocity));
        counter += 1;
        Serial.println("Writing data to SD Card" + String(counter));
      }
      raw_data_file.flush();
      //write data to event file as well
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

//    //if number of values in vector is greater than 100, then write data to SD card and clear vector
  if (data_vector.size() >= 10) {
    for (int i=0; i< data_vector.size(); i++) {
      raw_data_file.println(String(data_vector[i].current_time) + "\t" + String(data_vector[i].flexV) + "\t" + String(data_vector[i].angle) + "\t" + String(data_vector[i].velocity));
      Serial.println("Writing to SD Card");
    }
    raw_data_file.flush();  //pushes data to SD card file
    data_vector.clear();
  }
  if (event_vector.size() >= 1) {
    for (int i=0; i< event_vector.size(); i++) {
      event_file.println(String(event_vector[i].current_time) + "\t" + String(event_vector[i].flexV) + "\t" + String(event_vector[i].angle) + "\t" + String(event_vector[i].velocity));
      Serial.println("OUTSIDE LOOP. Writing event to SD Card");
    }
    event_file.println();
    event_file.flush();  //pushes data to SD card file
    event_vector.clear();
    
  }

  raw_data_file.close();
  event_file.close();
  delay(25); //measure data at intervals of 25 milliseconds
}
