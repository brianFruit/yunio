/* Copyright (C) 2017-2018 Brian Xia
 * You may use, distribute and modify this code under the
 * terms of the license, which is included in the project
 * root directory.
 */
 
#include <Console.h>
#include <Bridge.h>
//#include <Process.h>
//#include <Wire.h>
//#include <Servo.h>
#include "DHT.h"
#define NUMIO 20

bool DEBUG = false;

typedef struct {
  int type;
  bool isAlloc;
  int numOutputs;
  int *currentValue;
} point;

// Initialization
point* pointArray = malloc(NUMIO * sizeof(point));

//Process p;
//Servo myservo;
DHT dht;

String cmd;
unsigned int pinIdx;
unsigned int loopCount = 0;

void setup() {
  
  for (int i=0; i<NUMIO ; i++) {
    pointArray[i].type = -1;
    pointArray[i].isAlloc = false;
  }

  // Bridge must begin before Console!!
  Bridge.begin();
  delay(500);
  
  Console.begin();
  delay(500);
  
  // Wire.begin(4);
  // delay(500);
  
  Console.println("System all green");
    
}

// Main loop to listen to commands
void loop() {

    while (Console.available() > 0) {

      cmd = Console.readStringUntil('\n');
      cmd.trim();
    
      if (cmd.length() == 5) {
        if (cmd[0] == 'C') {
          configurePoint(cmd);
        } else if (cmd[0] == 'A') {
          actOn(cmd);
        }
      }
    
      Console.print("Command: ");
      Console.println(cmd);
    }
  
    pinIdx = loopCount % NUMIO;
    pushUpdate(pinIdx);
    loopCount++;

    delay(10);
}

void pushUpdate(int pinIdx) {
  
  if (pointArray[pinIdx].type >= 0) {
    pinEvaluate(pinIdx);

    String tuple = "";
    for (int i=0 ; i<pointArray[pinIdx].numOutputs ; i++) {
      tuple += String(pointArray[pinIdx].currentValue[i]) + ",";
    }

    Bridge.put(String(pinIdx), tuple);
  }
}

// Update Arduino I/O Input Value
void pinEvaluate(int pinIdx) {
  if (pointArray[pinIdx].type == 0) {
    int newValue = analogRead(pinIdx);
     pointArray[pinIdx].currentValue[0] = newValue;
  } 
  else if (pointArray[pinIdx].type == 2) {
    int newValue = digitalRead(pinIdx);
    pointArray[pinIdx].currentValue[0] = newValue;
  } 
  else if (pointArray[pinIdx].type == 4) {
    int newHum = dht.getHumidity();
    int newTemp = dht.getTemperature();
    pointArray[pinIdx].currentValue[0] = newTemp;
    pointArray[pinIdx].currentValue[1] = newHum;
  } 
  else if (pointArray[pinIdx].type > 4) {
    for (int i=0 ; i<pointArray[pinIdx].numOutputs ; i++) {
      pointArray[pinIdx].currentValue[0] = -i;
    }
  }
}

// Pre-configure arduino I/O
void configurePoint(String cmd) {
  int pin = ((int) cmd[1]) - 48;
  int type = (int) atoi(&cmd[2]);
  
  pointArray[pin].type = type;
  if (type == 0 || type == 2) {
    pinMode(pin, INPUT);
    pointArray[pin].numOutputs = 1;
    if (!pointArray[pin].isAlloc) {
      pointArray[pin].currentValue = (int*)calloc(1, sizeof(int));
    }
  } else if (type == 1 || type == 3) {
    pinMode(pin, OUTPUT);
    pointArray[pin].numOutputs = 1;
    if (!pointArray[pin].isAlloc) {
      pointArray[pin].currentValue = (int*)calloc(1, sizeof(int));
    }
  } else if (type == 4) {
    dht.setup(pin);
    pointArray[pin].numOutputs = 2;
    if (!pointArray[pin].isAlloc) {
      pointArray[pin].currentValue = (int*)calloc(1, sizeof(int));
    }
  } else if (type == -1) {
    pointArray[pin].type = -1;
  } else {
    Console.print("Invalid Point - pin: ");
    Console.print(pin);
    Console.print(" | type: ");
    Console.println(type);
  }
  // Add more point configuration

  if (DEBUG) {
    Console.print("CONFIG - ");
    Console.print("pin: ");
    Console.print(pin);
    Console.print(" | type: ");
    Console.print(type);
    Console.print(" | no. output: ");
    Console.print(pointArray[pin].numOutputs);
    Console.print(" | currn val: ");
    Console.println(*pointArray[pin].currentValue);
  }
}

// Command Arduino I/O to Output Value
void actOn(String cmd) {
  int pin = ((int) cmd[1]) - 48;
  int type = pointArray[pin].type;
  
  if (type >= 0) {
    int value = atoi(&cmd[2]) % 256;

    if (type == 1) {
      analogWrite(pin, value);
      pointArray[pin].currentValue[0] = value;
      Console.print("write ANALOG value ");
      Console.print(pointArray[pin].currentValue[0]);
      Console.print(" to pin ");
      Console.println(pin);
    } else if (type == 3) {
      digitalWrite(pin, value);
      pointArray[pin].currentValue[0] = value;
      Console.print("write DIGITAL value ");
      Console.print(pointArray[pin].currentValue[0]);
      Console.print(" to pin ");
      Console.println(pin);
    } else {
      Console.println("Invalid Point Type");
    }
    // Add more action type

    if (DEBUG) {
      Console.print("ACTION - ");
      Console.print("pin: ");
      Console.print(pin);
      Console.print(" | type: ");
      Console.print(type);
      Console.print(" | value: ");
      Console.println(value);
    }
  }
}


