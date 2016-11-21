#include "ESP8266WiFi.h"
#include <WiFiClientSecure.h>
#include <TheThingsNetwork.h>
#include <SoftwareSerial.h>
#include <Wire.h>  // Include Wire if you're using I2C
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library
#include <ArduinoJson.h>
#include "DHT.h"

SoftwareSerial softSerial(13, 15); // RX, TX

#define debugSerial Serial
#define loraSerial softSerial

#define PIN_RESET 255  //
#define DC_JUMPER 0  // I2C Addres: 0 - 0x3C, 1 - 0x3D

#define DHTPIN D4     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

TheThingsNetwork ttn;
MicroOLED oled(PIN_RESET, DC_JUMPER);  // I2C Example

/* Node 1 */
const byte devAddr[4] = { 0x00, 0x1A, 0x5F, 0xBA };
const byte appSKey[16] = { 0xAD, 0x50, 0xB9, 0xE8, 0x9D, 0x46, 0xA5, 0xC8, 0x97, 0x74, 0x74, 0x48, 0x3E, 0x00, 0xB6, 0x15 };
const byte nwkSKey[16] = { 0xA1, 0xC1, 0xF3, 0x7F, 0x92, 0xD5, 0x49, 0xF7, 0xC7, 0x92, 0xDE, 0x71, 0x86, 0x69, 0x3E, 0xB8 };

/* Node 2 
const byte devAddr[4] = { 0x7A, 0xFD, 0xAD, 0x24 };
const byte appSKey[16] = { 0xFB, 0x9F, 0xF5, 0x04, 0x47, 0xF8, 0x5C, 0x68, 0x2D, 0xDD, 0xD6, 0x51, 0x19, 0x68, 0x4E, 0x27 };
const byte nwkSKey[16] = { 0x27, 0x28, 0x73, 0xCB, 0x96, 0xE1, 0x88, 0x1F, 0x40, 0xA4, 0xCB, 0xAB, 0xB9, 0x31, 0xA4, 0xEB };
*/

/* Node 3 
const byte devAddr[4] = { 0xF2, 0x20, 0xAE, 0xAB };//{ 0x7A, 0xFD, 0xAD, 0x24 };
const byte appSKey[16] = { 0xA8, 0xBC, 0x92, 0x76, 0xCB, 0x43, 0x16, 0x6F, 0x38, 0xB4, 0xC5, 0x86, 0xE9, 0x56, 0xA8, 0xFC };
const byte nwkSKey[16] = { 0xDD, 0x76, 0xD0, 0x47, 0xB0, 0xB7, 0x83, 0x93, 0xAB, 0x73, 0x7D, 0xC1, 0x5B, 0xD4, 0xFD, 0x97 };
*/

int counter = 1;

void printOledCenter(String text, int font)
{
  int middleX = oled.getLCDWidth() / 2;
  int middleY = oled.getLCDHeight() / 2;
  
  oled.clear(PAGE);
  oled.setFontType(font);
  // Try to set the cursor in the middle of the screen
  oled.setCursor(middleX - (oled.getFontWidth() * (text.length()/2)+1),
                 middleY - (oled.getFontWidth() / 2));
  // Print the text:
  oled.print(text);
  oled.display();
  delay(1500);
  oled.clear(PAGE);
}

void printOledTop(String text, int font)
{  
  oled.clear(PAGE);
  oled.setFontType(font);
  // Try to set the cursor in the top of the screen
  oled.setCursor(0,0);
  // Print the text:
  oled.print(text);
  oled.display();
  delay(1500);
  oled.clear(PAGE);
}

String readDevice(String cmd){
  loraSerial.println(cmd);
  String lineRx = loraSerial.readStringUntil('\n');
  return lineRx.substring(0,lineRx.length()-1);
}

void ttnPost(String body){
  debugSerial.println("Sleeping for 2 seconds before starting sending out test packets.");
  delay(2000);
  //body = "Test";
  // Create a buffer with three bytes  
  //byte data[3] = { 0x01, 0x02, 0x03 };
  byte data[body.length()+1];
  body.getBytes(data, body.length()+1);
  // Send it to the network
  int result = ttn.sendBytes(data, sizeof(data));
  debugSerial.print("TTN Message Sent: ");
  debugSerial.println(counter);
  String msgString = "TTN Sent";
  msgString += "\nBytes:"+String(sizeof(data));
  msgString += "\nTries:"+String(counter);
  if (result < 0){
    msgString += "\nFailure";
  }
  else {
    debugSerial.print("Success but no downlink");
    msgString += "\nSuccess";
  }
  if (result > 0) {
    debugSerial.println("Received " + String(result) + " bytes");
    // Print the received bytes
    msgString += "\n";
    for (int i = 0; i < result; i++) {
      debugSerial.print(String(ttn.downlink[i]) + " ");
      msgString += String(ttn.downlink[i]);
    }
    debugSerial.println();
  }
}

String readTemp() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return(String(0));
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  return String(f);
}

void setup()
{
  oled.begin();
  oled.clear(ALL);
  oled.display();
  delay(1000);
  oled.clear(PAGE);

  debugSerial.begin(115200);
  loraSerial.begin(57600);//57600,9600
  
  //reset rn2483
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  delay(500);
  digitalWrite(12, HIGH);

  ttn.init(loraSerial, debugSerial);
  ttn.reset();
  ttn.personalize(devAddr, nwkSKey, appSKey);
  
  #ifdef DEBUG
  ttu.debugStatus();
  #endif
  Serial.println("Setup for The Things Network.");
  delay(100);

  dht.begin();
}

void loop()
{
  debugSerial.println();
//  debugSerial.println();
//  debugSerial.println("Device Information");
//  debugSerial.println();
//  ttn.showStatus();
//
  printOledCenter("Batt:",0);
  printOledCenter(readDevice("sys get vdd"),0);
//  printOledCenter("EUI:",0);
//  printOledCenter(readDevice("sys get hweui"),0);

  String tempF = readTemp();
  ttnPost(tempF);
  printOledCenter(tempF+" F", 0);
  debugSerial.println();
  counter++;
  delay(300000);
}
