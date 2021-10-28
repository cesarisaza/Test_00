//#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "DHT.h"
#include <EEPROM.h>

#define EEPROM_SIZE 44

// Test GuiHub

// Second change....

// Red por defecto 
//const char* ssid = "TECSIOT";     
//const char* password = "TELEMATICA"; 


const char* ssid = "TELEMATICA";     
const char* password = "ZUHR4B8TE1F0"; 

//const char* ssid = "Ceferinos";     
//const char* password = "C3F3R1N05"; 


const char* mqtt_server = "mqtt.cloud.kaaiot.com";

const String TOKEN = "siot02";
const String APP_VERSION = "c3vkn9befgnu0bisrn10-v1";

const unsigned long fiveSeconds = 1 * 5 * 1000UL;
static unsigned long lastPublish = 0 - fiveSeconds;
const int ledPin = 5;
const int BuzPin = 14;
WiFiClient espClient;
PubSubClient client(espClient);
#define DHTTYPE DHT11   // DHT 11
// MQ XX SENSOR IN GPIO 34
#define SENSOR  34
#define DELAY 500
              
// Initialize DHT sensor.
uint8_t DHTPin = 4; 
DHT dht(DHTPin, DHTTYPE);    

// DHT Sensor GPIO 4
float Temperature;
float Humidity;
int sensorValue = 0; 
int sensorValueR = 0;
int Conta_Bip = 0;
int alarma = 0;
int val = 0;
int Flag_alarma = 0;
int Bit_Interval = 180; 
int Setpoint; 
int Setpoint_Read;
char Set[2];
char Set2[1];
char Set3[1];
char L_Ssidr[1];
int Max_Con = 20;
int Bitacora = 0;
int Bitacora_Max = 180; // cada 15 min. un reporte 
int i=0;
int j;
int k;
int L=0;
int L_Ssid=0;
int L_Pass=0;
int randomNumber;
char Eprom_Ssid[20];
char Eprom_Password[20];
char ssid_d[20];     
char password_d[20]; 

void setup() {
  Serial.begin(115200);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(DHTPin, INPUT);
  dht.begin();  
  pinMode (ledPin, OUTPUT);
  pinMode (BuzPin, OUTPUT);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  EEPROM.begin(EEPROM_SIZE);
}

void loop() {
               
            Setpoint_Read = EEPROM.read(0);
            //Serial.print("EEProm Setpoint:");
            //Serial.println(Setpoint_Read);

            L_Ssid = EEPROM.read(42);
            //Serial.print("EEProm L_Ssid");
            //Serial.println(L_Ssid);
            
            
            L_Pass = EEPROM.read(43);
            //Serial.print("EEProm L_Pass");
            //Serial.println(L_Pass);
            
            for (j=1;j<=L_Ssid;j++){
                Eprom_Ssid[j-1]= EEPROM.read(j);
            }
            //Serial.print("EEProm Ssid");
            //Serial.println(Eprom_Ssid);  
            
            for (int i = 0; i <= L_Ssid; i++) {
               ssid_d[i] = Eprom_Ssid[i];
            }
            //Serial.print("EEProm Ssid");
            //Serial.println(ssid_d);  

            for (k=1;k<=L_Pass;k++){
              //Serial.print(k);
                Eprom_Password[k-1]= EEPROM.read(k+L_Ssid);
            }
            //Serial.print("EEProm Pass");
            //Serial.println(Eprom_Password);  
            
            
            //password_d = Eprom_Password;
            for (int i = 0; i <= L_Pass; i++) {
               password_d[i] = Eprom_Password[i];
            }

            //Serial.print("EEProm password");
            //Serial.println(password_d);  
  
  setup_wifi();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastPublish >= fiveSeconds) {
            lastPublish += fiveSeconds;
            DynamicJsonDocument telemetry(1023);
            telemetry.createNestedObject();
                    
            sensorValueR = analogRead(SENSOR);
            Serial.print("Sensor leido "); Serial.println(sensorValueR); 

            randomNumber = random(1,10);                                 
            sensorValue = (sensorValueR*0.54)+250;
            sensorValue = sensorValue+randomNumber;
            Serial.print("Ajustado PPM "); Serial.println(sensorValue);
          
              if (sensorValue < 200) {
                  sensorValue = 200;
              } 
          
              if (sensorValue > 2100) {
                  sensorValue = 2100;
              } 
              
              Serial.print("PPM "); Serial.println(sensorValue);
              telemetry[0]["PPM"] = sensorValue;
          
              if (sensorValue > 1000) {
                  Flag_alarma = 1;
              }   
              if (Flag_alarma == 1) {
                 for (val = 0; val < 10 ; val++) {
                  tone(BuzPin,4186, 200);
                  delay(100);
                 }
                 if (sensorValue < 700) {
                  Flag_alarma = 0;
                 }
              }
                 
              Humidity = dht.readHumidity(); // Gets the values of the humidity  
              telemetry[0]["Humidity"] = Humidity;
          
              Temperature = dht.readTemperature(); // Gets the values of the temperature
              telemetry[0]["Temperature"] = Temperature-6;

              if (Bitacora == Bitacora_Max) {
                Serial.print("Bitacora en 1 ");
                telemetry[0]["PPMB"] = sensorValue;
                telemetry[0]["HumedadB"] = Humidity;
                telemetry[0]["TemperaturaB"] = Temperature-6;
                String topic = "kp1/" + APP_VERSION + "/dcx/" + TOKEN + "/json";
                client.publish(topic.c_str(), telemetry.as<String>().c_str());
                Bitacora = 0;
              }
              Bitacora++;
      
              String topic = "kp1/" + APP_VERSION + "/dcx/" + TOKEN + "/json";
              client.publish(topic.c_str(), telemetry.as<String>().c_str());
              //Serial.println("Published on topic: " + topic);
          
           
            if (Conta_Bip == Bit_Interval) {
                  tone(BuzPin,4186, 200);
                  //Serial.println("Tono ON");
                  Conta_Bip=0;
                 }
             Conta_Bip++;
            }


}

void callback(char* topic, byte* payload, unsigned int length) {
//Serial.printf("\nHandling command message on topic: %s\n", topic);
String myString = String(topic);
char Letter_Test = myString[48];
Serial.println(myString);
if (Letter_Test=='N'){
  Serial.println("Recibido ON");
  digitalWrite (ledPin, LOW);
  delay(500);
  for (val = 0; val < 400 ; val++) {
        tone(BuzPin,4186, 200);
        delay(100);
       }
  }

if (Letter_Test=='F'){
  //Serial.println("Recibido OFF");
  digitalWrite (ledPin, HIGH);
  delay(500);
  Flag_alarma = 0;
}

char Letter_Test2 = myString[47];
if (Letter_Test2=='S'){
  //Serial.println("Recibido Setpoint");
  Set[0] = myString[48];
  Set[1] = myString[49];
  Set[2] = myString[50];
  Setpoint = atof(Set);
  Serial.println(Setpoint);
  EEPROM.write(0,Setpoint);   
  EEPROM.commit();
}

if (Letter_Test2=='L'){
  
  Serial.println("Recibida Longitud SSID");
  Set2[0] = 0;
  Set2[1] = 0;
  Set2[0] = myString[48];
  Set2[1] = myString[49];
  L_Ssid = atof(Set2);
  EEPROM.write(42,L_Ssid);   
  EEPROM.commit();
  delay(20);
  //Serial.printf(Set2);
  Serial.println(L_Ssid);
}

if (Letter_Test2=='I'){
  Serial.println("Recibido SSID");
  
  for (j=0;j<L_Ssid;j++){
  Eprom_Ssid[j]= myString[48+j];
  EEPROM.write(j+1,Eprom_Ssid[j]);   
  EEPROM.commit();
  delay(20);
  }
  Serial.print(Eprom_Ssid);
 }

if (Letter_Test2=='X'){
  Serial.println("Recibida Longitud PASSWORD");
  Set3[0] = 0;
  Set3[1] = 0;
  Set3[0] = myString[48];
  Set3[1] = myString[49];
  L_Pass = atof(Set3);
  EEPROM.write(43,L_Pass);   
  EEPROM.commit();
  delay(20);

  Serial.println(L_Pass);
}


if (Letter_Test2=='P'){
  Serial.println("Recibido Password");
  
  for (j=0;j<L_Pass;j++){
  Eprom_Password[j]= myString[48+j];
  EEPROM.write((j+1+L_Ssid),Eprom_Password[j]);   
  EEPROM.commit();
  delay(20);
  }
   Serial.println(Eprom_Password);
 }

memset(Eprom_Password, 0, sizeof(Eprom_Password));
memset(Eprom_Ssid, 0, sizeof(Eprom_Ssid));

DynamicJsonDocument doc(1023);
deserializeJson(doc, payload, length);
JsonVariant json_var = doc.as<JsonVariant>();

DynamicJsonDocument commandResponse(1023);
  for (int i = 0; i < json_var.size(); i++) {
    unsigned int command_id = json_var[i]["id"].as<unsigned int>();
    commandResponse.createNestedObject();
    commandResponse[i]["id"] = command_id;
    commandResponse[i]["statusCode"] = 200;
    commandResponse[i]["payload"] = "done";
  }

  String responseTopic = "kp1/" + APP_VERSION + "/cex/" + TOKEN + "/result/SWITCH";
  client.publish(responseTopic.c_str(), commandResponse.as<String>().c_str());
  //Serial.println("Published response to SWITCH command on topic: " + responseTopic);
}

void setup_wifi() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.println();
    Serial.printf("Connecting to [%s]", ssid);
    WiFi.begin(ssid, password);
    connectWiFi();
  }
}

void connectWiFi() {
  //while (WiFi.status() != WL_CONNECTED) {
  while (i<=Max_Con) {    
    delay(500);
    Serial.print(".");
    i+=1;
  }

  if (WiFi.status() != WL_CONNECTED) {  
    delay(200);
    Serial.println();
    Serial.printf("Connecting to [%s]", ssid_d);
    WiFi.begin(ssid_d, password_d);
    connectWiFi2();
  }
else {
  Serial.println();
  Serial.printf("WiFi connected to [%s]", ssid);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
}

void connectWiFi2() {
 
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println();
  Serial.printf("WiFi Default Connected to [%s]", ssid_d);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    char *client_id = "client-id-123ab";
    if (client.connect(client_id)) {
      Serial.println("Connected to WiFi");
      subscribeToCommand();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      Serial.printf("Connecting to [%s]", ssid_d);
      WiFi.begin(ssid_d, password_d);
      connectWiFi2();
    }
  }
}
void subscribeToCommand() {
  String topic = "kp1/" + APP_VERSION + "/cex/" + TOKEN + "/command/SWITCH/status";
  client.subscribe(topic.c_str());
  Serial.println("Subscribed on topic: " + topic);
}
