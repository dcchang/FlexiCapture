/*Final version of code for FlexiCapture device, a continuous data logging wearable device.
This sketch records output voltage changes as device is worn during daily activity.
Output voltage increases as flex sensor resistance increases (as it bends more) because flex sensor is R2 in voltage divider.
Sketch also records timestamps and time since sketch was first uploaded to board for every data point
Uses a capacitive sensor to determine when device is being worn and when to log data.

Outputs: resting.txt --> resting voltage values for each time calibration is done
         raw data file(s) --> timestamps, running time, and voltage for all data
*/

//libraries
#include <ArduinoSTL.h>         //for using vectors
#include <SPI.h>                //SD card
#include <SD.h>                 //SD card
#include <Wire.h>               //for timestamps 
#include "RTClib.h"             //for timestamps
#include "CapacitiveSensor.h"   //for capacitive sensor

CapacitiveSensor   cs_7_5 = CapacitiveSensor(7,5);        // 10M resistor between pins 7 & 5, pin 5 is sensor pin, add a wire and or foil if desired

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

RTC_DS1307 rtc;  //to set up the RTC board

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
int readings_c[numReadings];      // the readings from the analog input
int readIndex_c = 0;              // the index of the current reading
int total_c = 0;                  // the running total
int average_c = 0;                // the average

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

//Date and time variables

int m;
int d;
int h;
int mi;
int s;

//capacitance reading from capacitive sensor
long capacitance;

//data structure for storing information that will be written to SD card
struct Data {

  String time_string;  // timestamp
  unsigned long timer;  //timer variables which keeps track of total time since sketch is firsts uploaded to board
  float flexV;

};

Data vector_item; //vector items follow data structure of struct Data

std::vector<Data> data_vector; //creates vector to store information that will be stored on SD card

byte x = 0;  //switch used to determine whether device was previously on or off wrist

byte counter = 0;  //for numbering raw data files

//Calibration function to get new resting voltage value
//Does this everytime you put the device back on
//Stores new resting voltage file and creates new corresponding data file every time you put device back on
//keep track of count which increases by 1 every time function is called to distinguish between raw data files

void restingCal(int count) {
  //get current date (month and day) for filename
  DateTime now = rtc.now();
  m = now.month();
  d = now.day();

  //raw data files get stored in "raw_data" folder. Files follow naming scheme: (month)(date)(r_)(count).txt, no spaces
  //i.e. On April 1st, first file will be named 41r_0.txt, second file will be named 41r_1.txt
  data_filename = "raw_data/" + String(m) + String(d) + "r_" + String(count) + ".txt";   

  raw_data_file = SD.open(data_filename, FILE_WRITE);

  //raw data file contains timestamps, timer values and voltage
  raw_data_file.println("ts\tt\tV");  //create header for raw data file, file is tab separated

  //While getting resting value, turn LED light on to indicate to user that calibration is occurring
  digitalWrite(LED, HIGH); 
  Serial.println("Calibrating!");

  //set resting_sum and capacitance to 0
  resting_sum = 0;
  capacitance = 0;

  //Get the average resting voltage value by reading in the voltage 5000 times
  //also initialize capacitance reading so moving average values are accurate
  for (int i=0; i<5000; i++) {
    resting_flexADC = analogRead(FLEX_PIN);
    resting_sum += resting_flexADC;
    capacitance +=  cs_7_5.capacitiveSensor(30);
  }
  Serial.println("Calibration done!");
  digitalWrite(LED, LOW);
  resting_flexADC_average = resting_sum/5000;
  capacitance = capacitance/5000;
  
  resting_flexV = resting_flexADC_average * VCC/ 1023.0;  //Calculate voltage at resting position
  
  resting_voltage_file = SD.open("resting.txt", FILE_WRITE);
  resting_voltage_file.println(resting_flexV); //Stores the resting voltage value in resting.txt file
  
  Serial.println("Resting voltage: " + String(resting_flexV));

  //reset total_v and total_c to 0 to initialize moving average properly
  total_v = 0;
  total_c = 0;
  // initialize all the readings to resting_flexADC_averageg
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings_v[thisReading] = resting_flexADC_average;
    total_v += readings_v[thisReading];
  }

  // initialize all readings to capacitance
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings_c[thisReading] = capacitance;
    total_c += readings_c[thisReading];
  }
  
  raw_data_file.close();
  resting_voltage_file.close();
}

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
  
  SD.mkdir("raw_data");
  
  //get resting voltage value and setup raw data file by calling calibration function
  restingCal(counter);

  //start time for timer logging
  start_time = millis();
  
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

  //moving average for capacitive sensor
  total_c = total_c - readings_c[readIndex_c];
  // read from the sensor:
  readings_c[readIndex_c] = cs_7_5.capacitiveSensor(30);
  // add the reading to the total:
  total_c = total_c + readings_c[readIndex_c];
  // advance to the next position in the array:
  readIndex_c = readIndex_c + 1;
  
  // if we're at the end of the array...
  if (readIndex_c >= numReadings) {
    // ...wrap around to the beginning:
    readIndex_c = 0;
  }
  
  // calculate the average:
  average_c = total_c / numReadings;
  Serial.println("Cap sensor: " + String(average_c));

//  //if value is less than some threshold, then device is not being worn and stop logging data
  if (average_c < 6000) {
    Serial.println("OFF HAND");
    x = 1;
  }

  //  if value is greater than some threshold and was just put on the wrist, recalibrate for resting voltage value
  else if (average_c >= 6000 && x == 1) {
    Serial.print("Device on wrist?");

    //some checks to make sure person is trying to put device back on wrist
    int tally = 0;
    int cap_check = 0;
    for (int i=0; i< 5000; i++) {
      //check 5000 times to see if capacitance values are above threshold. Add a tally every time value is above threshold.
      cap_check = cs_7_5.capacitiveSensor(30);
      if (cap_check > 6000) {
        tally++;
        Serial.println("Tally: " + String(tally));
      }
    }
    //if tally value is greater than 50% of checks, then can safely assume user is putting device back on wrist
    //proceed with calibration
    if (tally > 2500) {
       x = 0;  //make x = 0 to indicate that device is on wrist and execute code that gets voltage values
      counter++;
      Serial.println("RECALIBRATION!!!");
      delay(60000); //give the user 1 minute to fully put on the device
      
      // Blink twice to notify user that calibration is about to start
      digitalWrite(LED, HIGH);
      delay(500);
      digitalWrite(LED, LOW);
      delay(500);
      digitalWrite(LED, HIGH);
      delay(500);
      digitalWrite(LED, LOW);
      delay(500);
      restingCal(counter);
    }
    //if tally value is less than 50% of checks, then user is likely not putting device back on wrist. Values above threshold
    //likely due to noise and not from wrist. Reset moving average and go back to main loop. 
    else {
      for (int thisReading = 0; thisReading < numReadings; thisReading++) {
        readings_c[thisReading] = 0;
        total_c = 0;
      }
    }
  }
  
  else {
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
