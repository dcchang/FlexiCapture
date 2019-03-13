//output voltage increases as flex sensor resistance increases (as it bends more)

#include <ArduinoSTL.h>
#include <SPI.h>
#include <SD.h>

//
#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif
//

//Set date
//date = x

////declare files
File resting_voltage_file;
File raw_data_file;
//declare variables

//Moving average variables
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
int resting_flexADC;
int resting_flexADC_average;
int flexADC;
float resting_flexV;

const int chipSelect = 10;
const int FLEX_PIN = A1; // Pin connected to voltage divider output

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 3.7; // Measured voltage of Ardunio 5V line
//const float R_DIV = 9750.0; // Measured resistance of 3.3k resistor

// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
//const float STRAIGHT_RESISTANCE = 23000; // resistance when straight
//const float BEND_RESISTANCE = 44800.0; // resistance at 90 deg
float flexR;
float flexR_0;
float flexR_90;
float resistance_5;
float voltage_5;
float voltage_diff;

//time variables
unsigned long start_time;
unsigned long end_time;
//unsigned long finish;

//data structure for storing information that will be written to SD card
struct Data {

  unsigned long current_time;
  float flexV;

};

Data vector_item; 

std::vector<Data> data_vector; 
//switch for determining whether just in while loop or not
int x = 0;

//do this everytime you put the device back on, so turn off and back on device every time you take it off
//Create resting voltage file and new data file every time you turn device back on
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

  //get current time and date
  //date = getDate()
  //time = getTime()
  //File names change for each day
  //This way, data for each day can be in their own folder
  //When you do post processing, you can click one folder at a time 
  //cannot turn off board, instead make it sleep
  String data_filename = String(date) + String(Time) + "_rawdata.txt";
  raw_data_file = SD.open(filename, FILE_WRITE);
  
  raw_data_file.println("t\tV");
  
  for (int i=0; i<3000; i++) {
    resting_flexADC = analogRead(FLEX_PIN);
    total = total + resting_flexADC;
    delay(1);
  }
  resting_flexADC_average = total/3000;
  
  resting_flexV = resting_flexADC_average * VCC/ 1023.0;  //Calculate voltage at resting position
  
  resting_voltage_file = SD.open("resting.txt", FILE_WRITE);
  resting_voltage_file.println(resting_flexV); //Stores the resting voltage value onto a txt file
  
  Serial.println("Resting voltage: " + String(resting_flexV));
  
  // initialize all the readings to resting_flexADC:
  total = 0;
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = resting_flexADC_average;
    total += readings[thisReading];
  }
  start_time = millis();

  raw_data_file.close();
  resting_voltage_file.close();
  
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
//  
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

  //Calculate voltage
  vector_item.flexV = average * VCC / 1023.0; //calculate voltage
  
  end_time = millis();

  vector_item.current_time = end_time - start_time;
  
  data_vector.push_back({vector_item.current_time,vector_item.flexV});
  Serial.println("Time: " + String(vector_item.current_time) + "\t" + "Voltage Value: " + String(vector_item.flexV));  
 
  if (data_vector.size() >= 100) {
    raw_data_file = SD.open("raw_data.txt", FILE_WRITE);
    for (int i=0; i< data_vector.size(); i++) {
      raw_data_file.println(String(data_vector[i].current_time) + "\t" + String(data_vector[i].flexV));
      Serial.println("Writing to SD Card");
    }
    raw_data_file.flush();
    data_vector.clear();
    raw_data_file.close();
  } 
  delay(25); //measure data at intervals of 25 milliseconds
}
