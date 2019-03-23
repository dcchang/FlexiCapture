//stores voltage at 0 and 90 degrees and also gets 5 degree voltage difference

#include <SPI.h>
#include <SD.h>

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

File threshold_file;  

const float VCC = 3.7;
const float r1 = 9.75;  //resistance in kohms

int total;
int flexADC;
int average_0;
int average_90;
float voltage_0;
float voltage_90;

const int chipSelect = 10;
const int FLEX_PIN = A1;
const int LED = 8;

void setup() {

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

  threshold_file = SD.open("init.txt",FILE_WRITE);
  
//  pinMode(LED, OUTPUT);
  // put your setup code here, to run once:
   //Get voltage value at 0 degrees 

//  digitalWrite(LED, HIGH);
//  delay(500);
//  digitalWrite(LED, LOW);
//  delay(500);
//  digitalWrite(LED, HIGH);
//  delay(500);
//  digitalWrite(LED, LOW);
//  delay(500);
  Serial.println("Place flex sensor completely flat at 0 degrees. Getting voltage value at 0 degrees");
  delay(5000);
  
  Serial.println("Beginning calibration...");
  total = 0;
  for (int i=0; i<5000; i++) {
    flexADC = analogRead(FLEX_PIN);
    total += flexADC;
//    digitalWrite(LED, HIGH);
    delay(1);
  }
//  digitalWrite(LED, LOW);
  average_0 = total/5000;
  voltage_0 = average_0 * VCC / 1023.0; //calculate voltage
  Serial.println("Flat Voltage: " + String(voltage_0));
  threshold_file.println(voltage_0);
  //Get voltage value at 90 degrees
  
//  digitalWrite(LED, HIGH);
//  delay(500);
//  digitalWrite(LED, LOW);
//  delay(500);
//  digitalWrite(LED, HIGH);
//  delay(500);
//  digitalWrite(LED, LOW);
//  delay(500);
  Serial.println("Bend flex sensor at exactly 90 degrees. Getting resistance value at 90 degrees");
  delay(5000);

  
  Serial.println("Beginning calibration...");
  total = 0;
  for (int i=0; i<5000; i++) {
    flexADC = analogRead(FLEX_PIN);
    total += flexADC;
//    digitalWrite(LED, HIGH);
    delay(1);
  }
//  digitalWrite(LED, LOW);
  average_90 = total/5000;
  voltage_90 = average_90 * VCC / 1023.0; //calculate voltage
  Serial.println("Bend Voltage: " + String(voltage_90));
  threshold_file.println(voltage_90);
//
//  float resistance_0 = (r1*voltage_0)/(VCC-voltage_0);
//  Serial.println(resistance_0);
//  float resistance_90 = (r1*voltage_90)/(VCC-voltage_90);
//  Serial.println(resistance_90);
  //Resistance value at 5 degrees should be larger than resistance value at 0 degrees
  threshold_file.close();
  
}

void loop() {
  // put your main code here, to run repeatedly:

  //start blinking once calibration is complete
//  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(500);                       // wait for a second
//  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
//  delay(500);                       // wait for a second
}
