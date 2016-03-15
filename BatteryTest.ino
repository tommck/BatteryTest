#include <ESP8266WiFi.h>
#include <WiFiClient.h>
 
const char* ssid     = "McKearneys";
const char* password = "";
const char* hostName = "hoopy-test"; // TODO: CHANGE FOR EACH DEVICE!
 
const char* host = "192.168.0.106";
const int httpPort = 8080;

const int numberOfMinutesToSleep = 1;
#define MICROSECONDS_PER_MINUTE 60*1000000  // 60 seconds * 1 million 

const int blueLedPin = 2;
int ledState = LOW;

const int numReadings = 5;
float temps[numReadings];
float hums[numReadings];
float batteryLevels[numReadings];

void blueLed(bool onOff) {
  if (onOff) 
  {      
    ledState = HIGH;
  }
  else
  {
    ledState = LOW;
  }
  digitalWrite(blueLedPin, ledState);
}

void toggleBlueLed() {
  blueLed(ledState == LOW);
}

void readBatteryLevel() {

  for (int i=0; i < numReadings; i++) {
    toggleBlueLed();
    
    int level = analogRead(A0);

    batteryLevels[i] = level;

    yield();
  }
}

String arrayToJson(float arr[]) 
{
  String result = F("[");
  for(int i=0; i < numReadings; i++) 
  {
    float val = arr[i];
    result += arr[i];
    if (i < numReadings-1) {
      result += F(",");
    }
  }
  result += F("]");

  return result;
}

void readAllAndReport() {
  // server is expecting temp/hum, just add dummy values
  for(int i=0; i < numReadings; ++i) {
    temps[i] = -1;
    hums[i] = -1;
  }
  
  readBatteryLevel();

  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);
  
  WiFi.printDiag(Serial);
  Serial.println("After Diag");

  WiFi.begin("McKearneys", "");

  Serial.println(F("After Begin"));
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
    yield();
  }
 
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  Serial.print(F("connecting to "));
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println(F("connection failed"));
    return;
  }
  
  // We now create a URI for the request
  String url = F("/stats");
  Serial.print(F("POSTING to URL: "));
  Serial.println(url);

  String messageBody = String(F("{\"temps\": ")) 
    + arrayToJson(temps) + F(", \"humidity\": ") 
    + arrayToJson(hums) + F(", \"battery\": ")
    + arrayToJson(batteryLevels) + F("}\r\n");

  Serial.println(messageBody);
        
  client.print(String(F("POST ")) + url + F(" HTTP/1.1\r\n") +
        F("Host: ") + hostName + F("\r\n") +
        F("Content-Type: application/json\r\n") +
        F("Content-Length: ") +
        String(messageBody.length()) + F("\r\n\r\n"));
  client.print(messageBody);

  // wait for the server to do its thing
  delay(500);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.println(line);
  }
  
  Serial.println(F("closing connection"));
}



void setup() {
  pinMode(blueLedPin, OUTPUT); // Blue LED pin
  Serial.begin(115200);
  delay(100);
}


void loop() {
  Serial.println("RUNNING");

  readAllAndReport();
  Serial.println("Delay...");
  delay(60000);
}



