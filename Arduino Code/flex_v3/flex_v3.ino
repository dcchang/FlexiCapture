//This sketch records output voltage changes as flex sensor bends and device is worn during daily activity.
//Output voltage increases as flex sensor resistance increases (as it bends more) because flex sensor is R2 in voltage divider.
//Sketch also records timestamps and time since sketch was first uploaded to board for every data point

//Outputs: resting.txt --> resting voltage value
//         raw data file(s) --> timestamps, running time, and voltage for all data

//libraries
#include <ArduinoSTL.h>         //for using vectors
#include <SPI.h>                //SD card
#include <SD.h>                 //SD card
#include <Wire.h>               //for timestamps 
#include "RTClib.h"             //for timestamps


#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

RTC_DS1307 rtc;  //to set up the RTC board

////declare files
File resting_voltage_file;
File raw_data_file;

//declare variables

//Moving average variables for voltage reading
// Define the number of samples to keep track of. The higher the number, the
// more the readings will be smoothed, but the slower the output will respond to
// the input. Using a constant rather than a normal variable lets us use this
// value to determine the size of the readings array.
const int numReadings = 10;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

const byte chipSelect = 10;  //to set up microSD card shield
const int FLEX_PIN = A1; // Pin connected to voltage divider output
const byte LED = 7;  //digital pin connected to LED light

const float VCC = 3.7; // Input voltage from battery

//Variables for getting voltage values
int resting_sum;
int resting_flexADC;
int resting_flexADC_average;
int flexADC;
float resting_flexV;

//time variables
unsigned long start_time;
unsigned long end_time;

//Time variables
int h;
int mi;
int s;

//data structure for storing information that will be written to SD card
struct Data {

  String time_string;  // timestamp
  unsigned long timer;  //timer variables which keeps track of total time since sketch is firsts uploaded to board
  float flexV;

};

Data vector_item; //vector items follow data structure of struct Data

std::vector<Data> data_vector; //creates vector to store information that will be stored on SD card

void setup() {

  // Open serial communications and wait for port to open:

  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif

  cs_7_5.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1, for setup of capacitive sensor

  //set up Serial Monitor/plotter
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  //
  pinMode(FLEX_PIN, INPUT);   //set analog pin to read in voltage divider ouptut
  pinMode(10, OUTPUT);  //set pin for SD card

  //initialize SD card
  if (!SD.begin(SPI_HALF_SPEED,10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  
  pinMode(LED, OUTPUT);   //set output pin for LED light

  //Set timestamps

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  //following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  raw_data_file = SD.open("raw_data.txt", FILE_WRITE);

  //raw data file contains timestamps, timer values and voltage
  raw_data_file.println("ts\tt\tV");  //create header for raw data file, file is tab separated

  //set resting_sum to 0
  resting_sum = 0;

  //Get the average resting voltage value by reading in the voltage 5000 times
  for (int i=0; i<5000; i++) {
    resting_flexADC = analogRead(FLEX_PIN);
    resting_sum += resting_flexADC;
  }
  resting_flexADC_average = resting_sum/5000;
  resting_flexV = resting_flexADC_average * VCC/ 1023.0;  //Calculate voltage at resting position
  
  resting_voltage_file = SD.open("resting.txt", FILE_WRITE);
  resting_voltage_file.println(resting_flexV); //Stores the resting voltage value in resting.txt file
  
  Serial.println("Resting voltage: " + String(resting_flexV));

  // initialize all the readings to resting_flexADC_average
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings_v[thisReading] = resting_flexADC_average;
    total += readings_v[thisReading];
  }

  raw_data_file.close();
  resting_voltage_file.close();
  
}

void loop() {

  //read in battery level

  float batterylevel = analogRead(A3);
  batterylevel = batterylevel * VCC/1023;
  Serial.println("BatteryVoltage = " + String(batterylevel));

  //if batterylevel is below 3.3, which is lower limit of battery, the LED will shine continousuly to let user know device needs to be charged
  if ((batterylevel) < 3.3) {
    digitalWrite(LED,HIGH);
  }
  else {
    digitalWrite(LED,LOW);
  }

  //Calculate moving average 
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
  
  //Calculate voltage
  vector_item.flexV = average * VCC / 1023.0; //calculate voltage
  Serial.println(vector_item.flexV);
  end_time = millis();

  //get updated timer value by subtracted end time from start time
  vector_item.timer = end_time - start_time;

  //get current date and time
  DateTime now = rtc.now();
  h = now.hour();
  mi = now.minute();
  s = now.second();
  vector_item.time_string = String(h) + ':' + String(mi) + ':' + String(s);
  
  data_vector.push_back({vector_item.time_string, vector_item.timer,vector_item.flexV});  //store timestamps, timer values and voltage values as one item in vector

  Serial.println(vector_item.flexV);

  //if vector contains 100 items, then store that data onto SD card and clear data from vector
  //This is needed to clear memory on TinyZero board, as board has limited memory
  //also considers fact that if vector has too many items before writing, more data will be lost during process of writing to SD card
  if (data_vector.size() >= 100) {
    raw_data_file = SD.open(data_filename, FILE_WRITE);

    //write data in vector to txt file as tab separated values
    for (int i=0; i< data_vector.size(); i++) {
      raw_data_file.println(String(data_vector[i].time_string) + "\t" + String(data_vector[i].timer) + "\t" + String(data_vector[i].flexV));
    }
    data_vector.clear();  //clear contents of vector
    raw_data_file.close();
  } 
  delay(25); //measure data every 25 milliseconds - use this to set sampling rate
}
