/* Copyright (C) 2017-2018 Brian Xia
 * You may use, distribute and modify this code under the
 * terms of the license, which is included in the project
 * root directory.
 */

#include <Bridge.h>
#include <Console.h>
// #include <Process.h>
// #include <Wire.h>
// #include <Servo.h>
#include "DHT.h"
#define NUMIO 54

bool DEBUG = false;

typedef struct {
  int type;
  bool isAlloc;
  int numOutputs;
  int *currentValue;
} point;

// Initialization
point* pointArray = (point*) malloc(NUMIO * sizeof(point));

// Process p;
// Servo myservo;
DHT dht;

String cmd;
unsigned int pinIdx;
unsigned int loopCount = 0;

void setup() {
  
  for (int i=0; i<NUMIO ; i++) {
    pointArray[i].type = -1;
    pointArray[i].isAlloc = false;
  }

  // Serial.begin(9600);
  delay(1000);
  Bridge.begin();

  delay(1000);
  Console.begin(); 

  // Wire.begin(4);
  delay(1000);
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
  bool valueChange = false;
  
  if (pointArray[pinIdx].type >= 0) {
    pinEvaluate(pinIdx, &valueChange);

    if (valueChange) {
      String tuple = "";
      for (int i=0 ; i<pointArray[pinIdx].numOutputs ; i++) {
        tuple += String(pointArray[pinIdx].currentValue[i]) + ",";
      }

      Bridge.put(String(pinIdx), tuple);

    }
  }
}

// Update Arduino I/O Input Value
void pinEvaluate(int pinIdx, bool *valueChange) {
  if (pointArray[pinIdx].type == 0) {
    int newValue = analogRead(pinIdx);
    if (newValue > pointArray[pinIdx].currentValue[0]*1.01 || 
        newValue < pointArray[pinIdx].currentValue[0]*0.99) {
       pointArray[pinIdx].currentValue[0] = newValue;
       *valueChange = true;
    }
  } else if (pointArray[pinIdx].type == 2) {
    int newValue = digitalRead(pinIdx);
    if (newValue > pointArray[pinIdx].currentValue[0]*1.01 || 
        newValue < pointArray[pinIdx].currentValue[0]*0.99) {
       pointArray[pinIdx].currentValue[0] = newValue;
       *valueChange = true;
    }
  } else if (pointArray[pinIdx].type == 4) {
    int newHum = dht.getHumidity();
    int newTemp = dht.getTemperature();
    if (newTemp > pointArray[pinIdx].currentValue[0]*1.01 || 
        newTemp < pointArray[pinIdx].currentValue[0]*0.99 ||
        newHum < pointArray[pinIdx].currentValue[1]*1.01 ||
        newHum < pointArray[pinIdx].currentValue[1]*0.99){
       pointArray[pinIdx].currentValue[0] = newTemp;
       pointArray[pinIdx].currentValue[1] = newHum;
       *valueChange = true;
    }
  } else if (pointArray[pinIdx].type > 4) {
    for (int i=0 ; i<pointArray[pinIdx].numOutputs ; i++) {
      pointArray[pinIdx].currentValue[0] = -i;
      *valueChange = true; // ??
    }
  }
}

// Pre-configure arduino I/O
void configurePoint(String cmd) {
  int pin = ((int) cmd[1]) - 48;
  int type = cmd.substring(2).toInt();
  
  pointArray[pin].type = type;
  if (type == 0 || type == 2) {
    pinMode(pin, INPUT_PULLUP);
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
    int value = cmd.substring(2).toInt() % 256;

    if (type == 1) {
      analogWrite(pin, value);
      *(pointArray[pin].currentValue) = value;
    } else if (type == 3) {
      digitalWrite(pin, value);
      *(pointArray[pin].currentValue) = value;
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
