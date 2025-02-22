# *** FOR EXPERIMENTAL USE ONLY ***

# AHRS_SB_F405-Adafruit_CAN_Feather
Connects to a "Speedy Bee F405 Wing APP" radio controlled Airplane flight controller to collect AHRS data:  
[https://www.speedybee.com/speedybee-f405-wing-app-fixed-wing-flight-controller/](https://www.speedybee.com/speedybee-f405-wing-app-fixed-wing-flight-controller/)

The code is specifically for an "Adafruit Feather M4 CAN Express with ATSAME51" board, which has a built-in CAN transceiver and a beefy Cortex M4 chip:  
[https://www.adafruit.com/product/4759](https://www.adafruit.com/product/4759)

## Flight controller configuration
Some configuration needs to be done on the Speedy Bee. I used INAV Configurator version 7.1.2 to enable MAVLink on the UART1 port. 
I also have a MGLRC GPS (model: M100) connected to the Speedy Bee, which includes a magnetometer for magnetic heading. The GPS is enabled on UART3 under the Sensors column. See image below:

![image](https://github.com/user-attachments/assets/11c0cd12-87ae-4637-8880-c8502ce17d73)


# Wiring
The circuit board lists the various connections that go into the terminal blocks. There are six connections:
- F405 Power
  - V+: I used a 90 degree USB-C power cable to plug into the SpeedyBee programming board. The positive wire goes here
  - GND: the negative USB-C power cable goes here to feed ground to the SpeedyBee
- 12V Input
  - V+: This is power that comes from my CAN Bus board (12 volts)
  - GND: this is ground that comes from the CAN Bus Board. I also connected the two black wires from the SpeedyBee's UART1 port (which I configured for MAVLINK data). These two black wires are used as a common ground between the two boards.
- F405 UART
  - Rx: Gray wire from the SpeedyBee UART1 port
  - Tx: White wire from the SpeedyBee UART1 port
 
## Wiring photos
![PXL_20250222_134715603 MP](https://github.com/user-attachments/assets/a85ecff7-47ac-4bce-8985-30a3200936d5)
![PXL_20250222_134739750 MP](https://github.com/user-attachments/assets/b3d780ec-2ce4-4de7-b0aa-1d4054046fc3)
![PXL_20250222_134858510](https://github.com/user-attachments/assets/d19454a8-f472-42db-ab58-b725b5b96f62)
![PXL_20250222_134909562](https://github.com/user-attachments/assets/22c4abb7-ffc2-4c0b-bf04-01ef836c1d2b)
![PXL_20250222_134933491](https://github.com/user-attachments/assets/8207cdc5-3aff-4d63-bc1c-6408082a8326)
![PXL_20250222_135404890](https://github.com/user-attachments/assets/02da123c-7505-4394-b1ad-27839fab191f)


# Arduino Code (loaded on Adafruit Feather M4 CAN Express board)
Start by following the Adafruit tutorial demonstrating how to configure the Arduino IDE for use with this board: 
[https://learn.adafruit.com/adafruit-feather-m4-can-express/arduino-can-examples](https://learn.adafruit.com/adafruit-feather-m4-can-express/arduino-can-examples)

The code is in C++, so you can safely skip the CircuitPython examples (unless you want to learn - in that case, go for it). Once you can program the board with some example code, go ahead and copy mine over.

## What to know about my code


There are 3 boolean values that can be set near the top of the code to determine what kinds of data you would like the Feather board to output:
```
/*************************************/
/******* Select output type **********/
bool OUTPUT_SERIAL  = true;  // Output for the Arduino IDE's Serial Monitor - good for debugging
bool OUTPUT_aEFIS   = false; // Output for my aEFIS Android app - connect the feather's USB port to an OTG cable on an Android
bool OUTPUT_CAN     = true;  // Output CAN data in MakerPlane CAN-FiX format 
/*************************************/
/*************************************/
```

Depending on which type of output you want, you will want to change these boolean values. For example, my aEFIS app is a simple attitude indicator that runs on an Android device. It uses the USB port on the Feather board to plug into the Android device for passing data and for powering the Feather board (e.g. the phone powers the Feather when plugged into the USB ports of both - the phone may require an OTG cable on its end, but most USB-C devices auto-negotiate). 

Because both the Arduino Serial Monitor (e.g. what you use to debug in the Arduino IDE) and aEFIS (e.g. my Android EFIS app) use the USB port on the Feather, you can't have both enabled at the same time or you will get wonky data. However, since the CAN transceiver on the Feather uses a separate set of wires for communication, one of the USB outputs can be used at the same time as the CAN interface. The code snipped above shows the latter (e.g. Serial output to the Arduino IDE and CAN output to a MakerPlane device).

