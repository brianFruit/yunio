# YunIO

YunIO allows your Arduino Yun or Yun Shielded Arduino to dynamically (1) configure I/O pins, (2) command output pins, and (3) receive input pin updates, via simple web APIs. With such interface, virtually any language can be used to program the Arduino. There is one catch, this is not suitable for time critical tasks that require instantaneous response as the TCP/IP communication comes with latency.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. 

### Prerequisites

They are the hardware and software prerequisites:

* [Arduino Yun](https://store.arduino.cc/usa/arduino-yun) or Yun Shielded Arduino
* [Arduino IDE](https://www.arduino.cc/en/Main/Software)

### Installing

This is the step-by-step process to get it working:

```
1. Upload yunio.ino into your Yun or Yun Shielded Arduino
2. Edit ServerComm.py; update the URL variable with the actual endpoint that you would like your Arduino to push the I/O values to
3. Upload both ServerComm.py and linkstart.sh onto the Linino side
4. Configure a cron job to run the linkstart.sh every minute: `* * * * * /root/linkstart.sh`
```

## Usage

Below explains how to use the API.

An Arduino pin can be configured to 4 different types: Analog Input (AI), Analog Output (AO), Digital Input (DI), Digital Output (DO). Please note that NOT ALL PINS ARE CREATED EQUAL, so check Arduino sepcifications to see if the certain pin number can indeed achieve the intended state. 

```
Pin Type Code:
AI - 0 
A0 - 1
DI - 2
DO - 3
```

### Configuring Pin

Use GET request to configure a pin, the URL should be constructed as follow:
```
URL Path:
1st digit "C" stands for Configure mode
2nd digit specifies the pin number plus a constant 48 in ASCII (ie. pin 12 is '<')
3rd-5th digits specify the pin type code

Example: configure pin 12 to DO
http://[arduino_ip_address]/C<003
```


### Commanding Output

Use GET request to command a pin, the URL should be constructed as follow:
```
URL Path:
1st digit "A" stands for Action mode
2nd digit specifies the pin number plus a constant 48 in ASCII (ie. pin 12 is '<')
3rd-5th digits specify value the pin should output (ie. 0 - 255). Please note that any value >255 will overflow.

Example: configure pin 12 to output high
http://[arduino_ip_address]/A<001
```

### Receiving Pin Update

If there is any pin with >10% change in value, it will trigger an update. The ServerComm will periodically post a JSON string to the endpoint URL specified during step 2 of the Installation. The JSON string has pin number as the key and pin state/value as the value. 

Make sure to always parse the value by comma separation as there could be more than one value returned by a pin. The "None" denotes that the specific pin was not yet configured.

```
Example JSON String:
{'11': None, '10': None, '13': None, '12': None, '15': None, '14': None, '17': None, '16': None, '19': None, '18': None, '1': '1,', '0': None, '3': '0,', '2': '0,', '5': u'0,', '4': '0,', '7': '0,', '6': '0,', '9': '0,', '8': '0,'}

```

## Authors

* **Brian Xia** - *Initial work* - [YunIO](https://github.com/brianfruit)

## License

see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* [PyMOTW](https://pymotw.com/2/BaseHTTPServer/) - on the lightweight Python server in Linino
