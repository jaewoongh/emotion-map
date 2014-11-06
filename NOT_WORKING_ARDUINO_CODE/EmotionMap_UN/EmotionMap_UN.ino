#include <Bridge.h>
#include <HttpClient.h>
#include <Process.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>

// Pins for pulse sensor
#define pulsePin A0
#define blinkPin 13
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

// Run data sending part only once in a while
long sendTimer = 0;
long sendInterval = 5000;

// Software serial for GPS data
SoftwareSerial mySerial(8, 7);

String nmeaChar = "";
String nmeaString = "";

// Update rate
#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
#define PMTK_SET_NMEA_UPDATE_5HZ  "$PMTK220,200*2C"
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F"

// turn on only the second sentence (GPRMC)
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"
// turn on GPRMC and GGA
#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
// turn on ALL THE DATA
#define PMTK_SET_NMEA_OUTPUT_ALLDATA "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
// turn off output
#define PMTK_SET_NMEA_OUTPUT_OFF "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"

#define PMTK_Q_RELEASE "$PMTK605*31"


void setup() {
  // Serial
  Serial.begin(9600);
//  while (!Serial);
  
  // Start GPS
  mySerial.begin(9600);
  mySerial.println(PMTK_Q_RELEASE);
  mySerial.println(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  mySerial.println(PMTK_SET_NMEA_UPDATE_1HZ);

  // Set up interrupts for pulse sensor
  interruptSetup();

  // Start bridge
  Serial.println("Starting bridge..");
  Bridge.begin();
  
  Serial.println("Start!");
  delay(2000);
}

void loop() {
  // Get sensor data
  if (QS == true) {
    Serial.print("BPM: ");
    Serial.println(BPM);
    QS = false;
  }
  
  // Get GPS data
  if (mySerial.available()) {
    char c = mySerial.read();
    if (c == '$') {
      nmeaString = nmeaChar;
      nmeaChar = "$";
      Serial.println(nmeaString);
    } else if (c != '\n') {
      nmeaChar += c;
    }
  }

  if (millis() - sendTimer > sendInterval) {
    // Check wifi status
    Process wifiCheck;
    int wifiResult = 0;
    wifiCheck.runShellCommand("/usr/bin/pretty-wifi-info.lua | grep Signal");
    while (wifiCheck.running());    // Do nothing until the process finishes
    if (wifiCheck.available()) {
      wifiResult = wifiCheck.parseInt();
    }
    Serial.print("Wifi strength: ");
    Serial.println(wifiResult);
    
    if (BPM != 0 && nmeaString.indexOf("$GPRMC") == 0) {
  
      // Make request string out of data
      String request = "http://104.236.54.44:4444/at/" + String(nmeaString) + "/feeling/" + String(BPM);
    
      // Save or upload data
      if (wifiResult < 10) {
        // Save data on linux side
        Serial.print("Save data: ");
        Serial.println(request);
        Process writeFile;
        writeFile.runShellCommand("echo " + request + " >> /tmp/emotion_data.txt");
        while (writeFile.running());  // Do nothing until the process finishes
      } else {
        // Directly send GET request to the server
        // Check if there're data not been sent yet
        Serial.println("Load data");
        Process readFile;
        readFile.runShellCommand("cat /tmp/emotion_data.txt");
        while (readFile.running());   // Do nothing until the process finishes
        String data = "";
        while (readFile.available()) {
          char inByte = (char)readFile.read();
          if (inByte == '\n') {
            makeGetRequest(data);
            Serial.print("Sent: ");
            Serial.println(data);
            data = "";
          } else {
            data += inByte;
          }
        }
        Serial.println(data);
        
        // Remove the file
        Process removeFile;
        removeFile.runShellCommand("rm /tmp/emotion_data.txt");
        while (removeFile.running());  // Wait until the process finishes
        Serial.println("Removing data");
        while (removeFile.available()) {
          Serial.print(removeFile.read());
        }
        
        // Send the latest data
        Serial.print("Send latest request: ");
        Serial.println(request);
        makeGetRequest(request);
      }
      
    }
    
    sendTimer = millis();
    nmeaString = "";
    BPM = 0;
    
    Serial.println("");
  }
  
  delay(20);
}

void makeGetRequest(String req) {
    HttpClient client;
    client.get(req);
}
