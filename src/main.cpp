#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PCF8574.h>
#include <FastLED.h>
#include <ThingSpeak.h>
#include "creds.h"
#include "secrets.h"

const int SENSOR_PIN = A0;
const int MOTOR_PIN = 7;
const int LED_PIN = D0;
const int LED_COUNT = 1;

const int BUTTON_PINS[] = {0,1,2,3,4};
const int BUTTON_COUNT = sizeof(BUTTON_PINS)/sizeof(BUTTON_PINS[0]);

const char* hostName = "pff-thingspeak";

PCF8574 pcf;
CRGB LED[1];
WiFiClient client;

unsigned long currentMillis = millis();
unsigned long lastMillis = millis();
unsigned long lastWriteMillis = millis();
unsigned long previousMillis = 0;
unsigned long call_interval = 15250;
unsigned long one_second = 1000;

int movementCount = 0;
int data[5] = {0, 0, 3, 1, 1};
int dataLen = sizeof(data)/sizeof(data[0]);
int fields[5] = {1, 2, 3, 4, 5};
int fieldsLen = sizeof(fields)/sizeof(fields[0]);
int FEED_PORTION, SNACK_PORTION, FEED_TRIGGER = 0, SNACK_TRIGGER = 0, portionCount = 0, PORTION_SIZE = 4;

#pragma region FUNCTIONS
void motorOn(){
    pcf.write(MOTOR_PIN, LOW);
}

void motorOff(){
  pcf.write(MOTOR_PIN, HIGH);
}

boolean switchOpen() {
  return (analogRead(SENSOR_PIN) >= 20 ? true : false);
}

boolean motorRunning() {
  return (pcf.read(MOTOR_PIN) == LOW ? true : false);
 }

void ledOff()
{
  LED[0] = CRGB::Black;
  FastLED.show();
  delay(30);
}

void ledBlue()
{
  LED[0] = CRGB::Blue;
  FastLED.show();
  delay(30);
}

void ledGreen()
{
  LED[0] = CRGB::Green;
  FastLED.show();
  delay(30);
}

void ledRed()
{
  LED[0] = CRGB::Red;
  FastLED.show();
  delay(30);
}

void ledYellow()
{
  LED[0] = CRGB::Yellow;
  FastLED.show();
  delay(30);
}

void ledPurple()
{
  LED[0] = CRGB::Purple;
  FastLED.show();
  delay(30);
}

void ledRedBlue()
{
  for(int j=0;j<=4;j++){
    LED[0] = CRGB::Red;
    FastLED.show();
    delay(75);
    LED[0] = CRGB::Blue;
    FastLED.show();
    delay(75);
  }
}

void ledRedGreen()
{
  for(int j=0;j<=4;j++){
    LED[0] = CRGB::Red;
    FastLED.show();
    delay(75);
    LED[0] = CRGB::Green;
    FastLED.show();
    delay(75);
  }
}

void advanceMotor() {
  if(!motorRunning()) motorOn();
  
  if(motorRunning()){
    ///Serial.printf("Motor is running...\n");

    // if motor is running, wait for the next fin to be detected
    Serial.printf("Looking for fin..");
    while(switchOpen() == true){      
      ledRedBlue();
      Serial.printf(".");
      delay(25);
    }

    Serial.printf("fin found!\nLooking for gap");
    // if the motor is running and the next fin is detected, run until the next gap
    while(switchOpen() == false) {
      ledRedGreen();
      Serial.printf(".");
      delay(25);
    }    

    Serial.printf("gap found!\n");

    motorOff();
    
    movementCount++;

    ledGreen();
  } 
}

int readThingSpeak()
{
  int statusCode = ThingSpeak.readMultipleFields(myChannelNumber, READ_API_KEY);
  
  int zeros = 0;
  
  if(statusCode == 200){
    for(int i=0;i<fieldsLen;i++){
      data[i] = ThingSpeak.getFieldAsInt(fields[i]);
      Serial.printf("Field %d value: %d\n", fields[i], data[i]);
      zeros = (data[i] != 0) ?: zeros++;
    }
    
    if(zeros<3) {
      FEED_TRIGGER = data[0];
      SNACK_TRIGGER = data[1];
      FEED_PORTION = data[2];
      SNACK_PORTION = data[3];
      PORTION_SIZE = data[4];
    } else {
      Serial.printf("Error with data. Too many zero values.\nFEED_TRIGGER: %d\nSNACK_TRIGGER: %d\nFEED_PORTION: %d\nSNACK_PORTION: %d");      
      delay(15000);
      ThingSpeak.setField(1, 0);
      ThingSpeak.setField(2, 0);
      ThingSpeak.setField(3, (FEED_PORTION == 0)  ? 3 : FEED_PORTION);
      ThingSpeak.setField(4, (SNACK_PORTION == 0) ? 1 : SNACK_PORTION);
      ThingSpeak.setField(5, (PORTION_SIZE == 0)  ? 1 : PORTION_SIZE);
      
      ThingSpeak.setStatus("This update was a correction.");

      statusCode = ThingSpeak.writeFields(myChannelNumber, WRITE_API_KEY);
      lastWriteMillis = millis();
    }
  }
  return statusCode;
}

int writeThingSpeak(int _feedTrigger = 0, int _snackTrigger = 0, int _feedPortion = FEED_PORTION, int _snackPortion = SNACK_PORTION,  int _portionSize = PORTION_SIZE)
{
  Serial.printf("Writing API values...\n");
  ThingSpeak.setField(1, _feedTrigger);
  ThingSpeak.setField(2, _snackTrigger);
  ThingSpeak.setField(3, (_feedPortion == 0)  ? FEED_PORTION : _feedPortion);
  ThingSpeak.setField(4, (_snackPortion == 0) ? SNACK_PORTION : _snackPortion);
  ThingSpeak.setField(5, (_portionSize == 0)  ? PORTION_SIZE : _portionSize);

  int statusCode = ThingSpeak.writeFields(myChannelNumber, WRITE_API_KEY);
  lastWriteMillis = millis();
  return statusCode;
}

#pragma endregion


void setup() {
  Serial.begin(9600);
  Serial.printf("Starting...\n");
    
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  WiFi.hostname(hostName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    os_printf("STA: Failed!\n");
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);    

  FastLED.addLeds<NEOPIXEL, LED_PIN>(LED, LED_COUNT);

  pcf.begin();
  
  if(switchOpen() == true) {    
    while(switchOpen() == true) {
      motorOn();
      yield();
    }
    motorOff();
    Serial.printf("Starting point found...\n");
  } else {
    while(switchOpen() == false) { 
      motorOn();
      yield();
    }
    Serial.printf("Starting point found...\n");
    motorOff();
  };

  ledGreen();
}

void loop() {
  currentMillis = millis();
  FEED_TRIGGER = 0;
  SNACK_TRIGGER = 0;
  unsigned long timeSinceLastWrite;

  if(currentMillis - lastMillis >= one_second) {      
    if(readThingSpeak() == 200) {
      if(FEED_TRIGGER == 1){
        lastWriteMillis = millis();
        for(int j=1;j<=FEED_PORTION * PORTION_SIZE;j++){
          advanceMotor();                  
          Serial.printf("Advanced %d of %d positions.\n", j, FEED_PORTION * PORTION_SIZE);
        }      
        FEED_TRIGGER = 0;
        Serial.printf("Delaying for API write...\n");
        delay(15250);
        writeThingSpeak(FEED_TRIGGER, 0, FEED_PORTION, SNACK_PORTION, PORTION_SIZE);
      } 

      if(SNACK_TRIGGER == 1){
        lastWriteMillis = millis();
        for(int j=1;j<=SNACK_PORTION * PORTION_SIZE;j++){
        advanceMotor();
        Serial.printf("Advanced %d of %d positions.\n", j, SNACK_PORTION * PORTION_SIZE);
      }
        SNACK_TRIGGER = 0;
        Serial.printf("Delaying for API write...\n");
        delay(15250);
        writeThingSpeak(0, SNACK_TRIGGER, FEED_PORTION, SNACK_PORTION, PORTION_SIZE);
      }

      timeSinceLastWrite = ((currentMillis - lastWriteMillis) / one_second);
              
    }
    
    Serial.printf("Seconds since last write: %d\n", timeSinceLastWrite);    
    
    if(timeSinceLastWrite <= 15){
        ledRed();
      } else {
        ledGreen();
    }
    
    Serial.printf("Switch open: %s - Movements: %d\n", ((switchOpen() == true) ? "True" : "False"), movementCount);    
    lastMillis = millis();
  }

  for(int i=0;i<BUTTON_COUNT;i++)
  {
    int buttonState = pcf.read(BUTTON_PINS[i]);

    if(buttonState == LOW) 
    {
      Serial.printf("You pressed button %d!\n", i);
      switch(i)
      {
        case 0: 
          ledRed();
          motorOff();
          break;
        case 1:                               
          break;
        case 2:
          break;
        case 3:
          ledPurple();
          for(int j=1;j<=SNACK_PORTION * PORTION_SIZE;j++){
            advanceMotor();                  
            Serial.printf("Advanced %d of %d positions.\n", j, SNACK_PORTION * PORTION_SIZE);
          }  
          break;
        case 4:
          ledBlue();
          for(int j=1;j<=FEED_PORTION * PORTION_SIZE;j++){
            advanceMotor();                  
            Serial.printf("Advanced %d of %d positions.\n", j, FEED_PORTION * PORTION_SIZE);
          }      
          FEED_TRIGGER = 0;
          break;
      }
      while(pcf.read(BUTTON_PINS[i]) == LOW) delay(5);
    }    
  }
}