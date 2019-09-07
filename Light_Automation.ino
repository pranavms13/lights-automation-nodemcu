/**********************************************************************/
/*  Project         : Light Automaion Using NodeMCU / ESP8266 Module  */
/*  Author          : Pranav M S                                      */
/*  Email           : pranavms13@yahoo.com                            */
/*  Date Published  : 05-09-2019                                      */
/*  Tested at       : 05-09-2019 20:50                                */
/*  License         : GNU GENERAL PUBLIC LICENSE v3                   */
/*  File Version    : v1.0                                            */
/**********************************************************************/

#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


#define L1 D6  //Defining Pin for Light 1. Please use PWM Pin
#define L2 D5  //Defining Pin for Light 2.

#define WLAN_SSID1       "<SSID 1>"             // Your 1st SSID
#define WLAN_PASS1       "<Password 1>"        // Your 1st password

/********************** Failover Connection Setings **************************/

#define WLAN_SSID2       "<SSID 2>"             // Your 2nd SSID
#define WLAN_PASS2       "<Password 2>"        // Your 2nd password

//------------------------------------------------------------------------------
/**************************** Adafruit.io Setup ******************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "<adafruit-username>"            // Replace it with your username
#define AIO_KEY         "8dde5dde1a5a42133cf6e2ff3e9fbcd0"   // Replace with your Project Auth Key

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe Light1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/L1"); // Feed 1 Name
Adafruit_MQTT_Subscribe Light2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/L2"); // Feed 2 Name

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  
  /************************* Connecting to WiFi Access Point ****************************/
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID1);

  WiFi.begin(WLAN_SSID1, WLAN_PASS1);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW); //Built-in LED Stays ON until the Boot Process is Completed
  delay(1000);
  int tries=20;
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");  // Prints "......."
    tries--;
    if(tries=0){
      if(WiFi.status() != WL_CONNECTED){  //Connecting to Failover WiFi Access Point
        Serial.println(); Serial.println();
        Serial.print("Connecting to ");
        Serial.println(WLAN_SSID2);
        WiFi.begin(WLAN_SSID2, WLAN_PASS2);
        while(WiFi.status() != WL_CONNECTED) {
          delay(1000);
          Serial.print(".");  // Prints "......."
        }
      }
    }
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
//--------------------------------------------------------------------------------------------

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&Light1);
  mqtt.subscribe(&Light2);
}

void loop() {
 
  MQTT_connect();
  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &Light1) {
      int Light1_State = atoi((char *)Light1.lastread); //Read Light 1 State from Adafruit.io ( any value from 0 to 5 )
      Serial.print("Light 1, State : ");
      Serial.print(Light1_State);
      int st1=0;
      switch(Light1_State){
        case 0:
          st1=0;
          break;
        case 1:
          st1=920;
          break;
        case 2:
          st1=946;
          break;
        case 3:
          st1=971;
          break;
        case 4:
          st1=985;
          break;
        case 5:
          st1=997;
          break;
      }
      Serial.print(", AnalogWrite : ");
      Serial.println(st1);
      analogWrite(L1, st1);
    }
    if (subscription == &Light2) {
      int Light2_State = atoi((char *)Light2.lastread);  //Read Light 2 State from Adafruit.io ( 0 or 1 )
      Serial.print("Light 2, State : ");
      Serial.print(Light2_State);
      Serial.print(", Digital Write : ");
      Serial.println(Light2_State);
      analogWrite(L2, Light2_State);  
    }
  }
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 10;
  
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection...");
    pinMode(BUILTIN_LED, OUTPUT);
    digitalWrite(BUILTIN_LED, LOW); // Built-in LED remains on if the connection is not made
    mqtt.disconnect();
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
  if((mqtt.connect()==0)){
    digitalWrite(BUILTIN_LED, HIGH); //Turns off The Built-in LED is connection is made. Turning off of the LED Indicates Finished Boot
  }
}
