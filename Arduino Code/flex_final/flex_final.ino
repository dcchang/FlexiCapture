//output voltage increases as flex sensor resistance increases (as it bends more)

#include <ArduinoSTL.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
//#include "CapacitiveSensor.h"

//CapacitiveSensor   cs_3_9 = CapacitiveSensor(7,5);        // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired


#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

RTC_DS1307 rtc;

////declare files
File resting_voltage_file;
File raw_data_file;

String data_filename;
//declare variables

//Moving average variables for voltage reading
// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
const int numReadings = 10;

int readings_v[numReadings];      // the readings from the analog input
int readIndex_v = 0;              // the index of the current reading
int total_v = 0;                  // the running total
int average_v = 0;                // the average

//moving average variables for capacitive readings
//int readings_c[numReadings];      // the readings from the analog input
//int readIndex_c = 0;              // the index of the current reading
//int total_c = 0;                  // the running total
//int average_c = 0;                // the 
//Voltage values
int resting_sum;
int resting_flexADC;
int resting_flexADC_average;
int flexADC;
float resting_flexV;

const byte chipSelect = 10;
const int FLEX_PIN = A1; // Pin connected to voltage divider output
const byte LED = 7;

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 3.7; // Measured voltage of Ardunio 5V line

//time variables
unsigned long start_time;
unsigned long end_time;

//Date and time variables

int m;
int d;
int y;
int h;
int mi;
int s;

//capacitive variable
//long total1;

//data structure for storing information that will be written to SD card
struct Data {

  String time_string;
  unsigned long current_time;
  float flexV;

};

Data vector_item; 

std::vector<Data> data_vector; 

byte x = 0;
byte counter = 0;

//do this everytime you put the device back on, so turn off and back on device every time you take it off
//Create resting voltage file and new data file every time you turn device back on

void restingCal(int count) {
  DateTime now = rtc.now();
  m = now.month();
  d = now.day();
  data_filename = "raw_data/" + String(m) + String(d) + "r_" + String(count) + ".txt";

  raw_data_file = SD.open(data_filename, FILE_WRITE);

  raw_data_file.println("ts\tt\tV");

  //While getting resting value, turn LED light on to indicate to user that calibration is occurring
  digitalWrite(LED, HIGH); 
  Serial.println("Getting voltage");

  //set resting_sum and total1 to 0
  resting_sum = 0;
//  total1 = 0;
  for (int i=0; i<5000; i++) {
    
    resting_flexADC = analogRead(FLEX_PIN);
    resting_sum += resting_flexADC;
//    total1 +=  cs_3_9.capacitiveSensor(30);
//    Serial.println(String(total1));
  }
  Serial.println("Process done");
  digitalWrite(LED, LOW);
  resting_flexADC_average = resting_sum/5000;
//  total1 = total1/5000;
  
  resting_flexV = resting_flexADC_average * VCC/ 1023.0;  //Calculate voltage at resting position
  
  resting_voltage_file = SD.open("resting.txt", FILE_WRITE);
  resting_voltage_file.println(resting_flexV); //Stores the resting voltage value onto a txt file
  
  Serial.println("Resting voltage: " + String(resting_flexV));

  //set total_v and total_c to 0 to initialize moving average properly
  total_v = 0;
//  total_c = 0;
  // initialize all the readings to resting_flexADC:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings_v[thisReading] = resting_flexADC_average;
    total_v += readings_v[thisReading];
  }

//  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
//    readings_c[thisReading] = total1;
//    total_c += readings_c[thisReading];
//  }
  
  raw_data_file.close();
  resting_voltage_file.close();
}

void setup() {

  // Open serial communications and wait for port to open:

  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif

//  cs_3_9.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(FLEX_PIN, INPUT);
  pinMode(10, OUTPUT);  //set pin for SD card

  //initialize SD card
  if (!SD.begin(SPI_HALF_SPEED,10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  pinMode(LED, OUTPUT);

  //Set time

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  //following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  SD.mkdir("raw_data");
  //get resting voltage value and setup raw data file
  Serial.println("Initial resting volage calibration!");
  restingCal(counter);

  //start time for time logging
  start_time = millis();
  
}

void loop() {

  //read in battery level

  float batterylevel = analogRead(A3);
  Serial.println("BatteryVoltage = ");
  Serial.println(batterylevel*VCC/1023.0);
  if ((batterylevel*VCC/1023.0) < 3.3) {
    digitalWrite(LED,HIGH);
  }
  else {
    digitalWrite(LED,LOW);
  }
//  
  //moving average for capacitive sensor
//  total_c = total_c - readings_c[readIndex_c];
//  // read from the sensor:
//  readings_c[readIndex_c] = cs_3_9.capacitiveSensor(30);
//  // add the reading to the total:
//  total_c = total_c + readings_c[readIndex_c];
//  // advance to the next position in the array:
//  readIndex_c = readIndex_c + 1;
//  
//  // if we're at the end of the array...
//  if (readIndex_c >= numReadings) {
//    // ...wrap around to the beginning:
//    readIndex_c = 0;
//  }
//  
//  // calculate the average:
//  average_c = total_c / numReadings;
//  Serial.println("Cap sensor: " + String(average_c));
//
////  //if value is less than some threshold, then device is not being worn and stop logging data
//  if (average_c < 2500) {
//    Serial.println("OFF HAND");
//    x = 1;
//  }
//
////  if value is greater than some threshold and was just put on the wrist, recalibrate for resting voltage value
//  else if (average_c >= 2500 && x == 1) {
//    Serial.print("ON HAND?");
//
//    //some checks to make sure person is trying to put device back on arm
//    int tally = 0;
//    int cap_check = 0;
//    for (int i=0; i< 5000; i++) {
//      cap_check = cs_3_9.capacitiveSensor(30);
//      Serial.println("Cap Check: " + String(cap_check));
//      if (cap_check > 3000) {
//        tally++;
//        Serial.println("Tally: " + String(tally));
//      }
//    }
//    if (tally > 2000) {
//       x = 0;
//      counter++;
//      Serial.println("RECALIBRATION!!!");
//      delay(60000); //give the user 1 minute to put on the device
//  //    Blink to notify that calibration is about to start
//      digitalWrite(LED, HIGH);
//      delay(500);
//      digitalWrite(LED, LOW);
//      delay(500);
//      digitalWrite(LED, HIGH);
//      delay(500);
//      digitalWrite(LED, LOW);
//      delay(500);
//      restingCal(counter);
//    }
//    else {
//      for (int thisReading = 0; thisReading < numReadings; thisReading++) {
//        readings_c[thisReading] = 0;
//        total_c = 0;
//      }
//    }
//  }
//  else {
 
    //Calculate moving average 
    total_v = total_v - readings_v[readIndex_v];
    // read from the sensor:
    readings_v[readIndex_v] = analogRead(FLEX_PIN);
    // add the reading to the total:
    total_v = total_v + readings_v[readIndex_v];
    // advance to the next position in the array:
    readIndex_v = readIndex_v + 1;
  
    // if we're at the end of the array...
    if (readIndex_v >= numReadings) {
      // ...wrap around to the beginning:
      readIndex_v = 0;
    }
  
    // calculate the average:
    average_v = total_v / numReadings;
    
    //Calculate voltage
    vector_item.flexV = average_v * VCC / 1023.0; //calculate voltage
    Serial.println(vector_item.flexV);
    end_time = millis();
  
    vector_item.current_time = end_time - start_time;
  
    //get current date and time
    DateTime now = rtc.now();
    h = now.hour();
    mi = now.minute();
    s = now.second();
    vector_item.time_string = String(h) + ':' + String(mi) + ':' + String(s);
//    Serial.println(vector_item.time_string);
  
    data_vector.push_back({vector_item.time_string, vector_item.current_time,vector_item.flexV});
//    Serial.println("Time: " + String(vector_item.current_time) + "\t" + "Voltage Value: " + String(vector_item.flexV));  
//    Serial.println(vector_item.flexV);
    if (data_vector.size() >= 100) {
      raw_data_file = SD.open(data_filename, FILE_WRITE);
//      digitalWrite(LED, HIGH);
      for (int i=0; i< data_vector.size(); i++) {
        raw_data_file.println(String(data_vector[i].time_string) + "\t" + String(data_vector[i].current_time) + "\t" + String(data_vector[i].flexV));
//        Serial.println("Writing to SD Card");
      }
//      digitalWrite(LED, LOW);
      raw_data_file.flush();
      data_vector.clear();
      raw_data_file.close();
    } 
    
//  }
  delay(25); //measure data at intervals of 25 milliseconds
}
