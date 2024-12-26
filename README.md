# *** FOR EXPERIMENTAL USE ONLY ***

# AHRS_SB_F405-Adafruit_CAN_Feather
Connects to a "Speedy Bee F405 Wing APP" radio controlled Airplane flight controller to collect AHRS data:  
[https://www.speedybee.com/speedybee-f405-wing-app-fixed-wing-flight-controller/](https://www.speedybee.com/speedybee-f405-wing-app-fixed-wing-flight-controller/)

The code is specifically for an "Adafruit Feather M4 CAN Express with ATSAME51" board, which has a built-in CAN transceiver and a beefy Cortex M4 chip (which does floating point math):  
[https://www.adafruit.com/product/4759](https://www.adafruit.com/product/4759)

## Flight controller configuration
Some configuration needs to be done on the Speedy Bee. I used INAV Configurator version 7.1.2 to enable MAVLink on the UART1 port. 
I also have a MGLRC GPS (model: M100) connected to the Speedy Bee, which includes a magnetometer for magnetic heading. The GPS is enabled on UART3 under the Sensors column. See image below:

![image](https://github.com/user-attachments/assets/11c0cd12-87ae-4637-8880-c8502ce17d73)


# Arduino Code (loaded on Adafruit Feather M4 CAN Express board)
## Need to know

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
Depending on which type of output you want, you will want to change these boolean values. For example, my aEFIS app is a simple attitude indicator that runs on an Android device. It uses the USB port on the Feather board to plug into the Android device for passing data and for powering the Feather board (e.g. the phone powers the Feather when plugged into the USB ports of both - the phone should have an OTG cable on its end). Because both the Arduino Serial Monitor and aEFIS use the USB port on the Feather, you can't have both enabled at the same time or you will get wonky data. However, since the CAN transceiver on the Feather uses a separate set of wires for communication, one of the USB outputs can be used at the same time as the CAN interface. The code snipped above shows the latter (e.g. Serial output to the Arduino IDE and CAN output to a MakerPlane device).

