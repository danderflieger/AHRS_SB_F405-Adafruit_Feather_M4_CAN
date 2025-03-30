#include <Arduino.h>
#include <MAVLink.h>
// #include <Adafruit_NeoPixel.h>

#include <CANSAME5x.h>
CANSAME5x CAN;

// Define serial port for MAVLink communication (Serial1 is available on RP2040)
#define MAVLINK_SERIAL Serial1

// MAVLink system and component IDs
#define MAVLINK_SYSTEM_ID 1   // This is the ID for this script
#define MAVLINK_COMPONENT_ID 1 // Component ID

// NeoPixel stuff
// #define LED_PIN 6
// #define LED_COUNT 1

// Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

/*************************************/
/******* Select output type **********/
bool OUTPUT_SERIAL  = true;  // Output for the Arduino IDE's Serial Monitor - good for debugging
bool OUTPUT_aEFIS   = false; // Output for my aEFIS Android app - connect the feather's USB port to an OTG cable on an Android
bool OUTPUT_CAN     = true;  // Output CAN data in MakerPlane CAN-FiX format 
/*************************************/
/*************************************/


/******* MakerPlane CAN-FiX IDs ********/
// These values can be changed if you want to use another CAN standard
const uint16_t FIX_PITCH = 384;
const uint16_t FIX_ROLL = 385;
const uint16_t FIX_HEADING = 389;
// const uint16_t FIX_GROUND_SPEED = 453;
const uint16_t FIX_INDICATED_AIRSPEED = 387;
const uint16_t FIX_INDICATED_ALTITUDE = 388;

const uint16_t nodeId = 0x82;
/***************************************/


/**************************************/
// * Variables to store attitude data *
float roll = 0.0;
float pitch = 0.0;
float yaw = 0.0;
float ground_speed = 0.0;
float gps_altitude = 0.0;
int32_t global_heading = 0;

signed long lngPitch = 0;
signed long lngRoll = 0;
signed long lngGlobalHeading = 0;
unsigned long lngGroundSpeed = 0;
signed long lngGpsAltitude = 0;

/**************************************/


// Function to process incoming MAVLink messages
void handleMavlinkMessage() {
  mavlink_message_t msg;
  mavlink_status_t status;

  while (MAVLINK_SERIAL.available()) {
    uint8_t c = MAVLINK_SERIAL.read();

    if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
      // Message received, handle it
      switch (msg.msgid) {
        case MAVLINK_MSG_ID_ATTITUDE:
        {
          mavlink_attitude_t attitude;
          mavlink_msg_attitude_decode(&msg, &attitude);
          roll = attitude.roll;
          pitch = attitude.pitch;
          yaw = attitude.yaw;
          
          roll = roll * RAD_TO_DEG;
          pitch = pitch * RAD_TO_DEG;
          yaw = yaw * RAD_TO_DEG;
        
          break;
        }
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        {
          // mavlink_global_position_int_t global_position;
          // mavlink_msg_global_position_int_decode(&msg, &global_position);
          // global_heading = global_position.hdg;
          // global_heading = global_heading / 10.0f;
          
          
          break;
        }
        case MAVLINK_MSG_ID_GPS_RAW_INT: {
          // Handle GPS_RAW_INT message (older MAVLink versions)
          mavlink_gps_raw_int_t gps_raw_int;
          mavlink_msg_gps_raw_int_decode(&msg, &gps_raw_int);
          gps_altitude = gps_raw_int.alt * 0.00328084; // Altitude in Feet //1000.0; // Altitude in meters (altitude is in millimeters in the raw message)
          break;
        }
        default:
          // Ignore other messages or handle them as needed
          break;
      }

      lngPitch = pitch * 100;
      lngRoll = roll * 100;
      lngGlobalHeading = getHeadingReciprocal( yaw * 10 );
      lngGpsAltitude = gps_altitude;
      // long lngGlobalHeading = yaw * 10;

      if (OUTPUT_CAN) {
        sendCanMessage(FIX_PITCH, lngPitch);
        sendCanMessage(FIX_ROLL, lngRoll);
        sendCanMessage(FIX_HEADING, lngGlobalHeading);
        sendCanMessage(FIX_INDICATED_ALTITUDE, lngGpsAltitude);
        // sendCanMessage(FIX_GROUND_SPEED, lngGroundSpeed);
      }

      if (OUTPUT_SERIAL) {
        Serial.print("pitch:"); Serial.print(lngPitch);
        Serial.print(",roll:"); Serial.print(lngRoll);
        Serial.print(",heading:");Serial.print(lngGlobalHeading);
        Serial.print(",altitude:");Serial.println(lngGpsAltitude);

      }

      if (OUTPUT_aEFIS) {
        send_canfix_frame_to_aefis(FIX_PITCH, lngPitch);
        send_canfix_frame_to_aefis(FIX_ROLL, lngRoll);
        send_canfix_frame_to_aefis(FIX_HEADING, lngGlobalHeading);
      }
    }
  }
}

void sendCanMessage(int messageType, int data) {
  
  byte byteData[4];
  byteData[0] = data;
  byteData[1] = data >> 8;
  byteData[2] = data >> 16;
  byteData[3] = data >> 24;

  CAN.beginPacket(messageType);
  CAN.write(nodeId);
  CAN.write(0x00);
  CAN.write(0x00);
  for (int i = 0; i<4; i++) {
    CAN.write(byteData[i]);
  }

  CAN.endPacket();
  
}

// void send_canfix_frame_to_serial(CanFixFrame frame) {
void send_canfix_frame_to_aefis(int messageType, long data) {
  
  byte message [] = {
    messageType,
    messageType >> 8,
    messageType >> 16,
    0x78,//frame.data[0],
    0x00,//frame.data[1],
    0x00,//frame.data[2],
    data,
    data>>8,
    data>>16,
    data>>24

  };
  Serial.write(message, 11);
}

long getHeadingReciprocal(long heading) {
  // long reciprocal = (heading + 1800) % 3600;
  long reciprocal = (heading + 3600) % 3600;
  return reciprocal;
}


void setup() {
  
  Serial.begin(115200);
  
  // Prepare the USB port on the Adafruit board for use with either the
  // Arduino IDE or the aEFIS Android app, but not both.
  if (OUTPUT_SERIAL) {
    // For debugging on USB
    
    delay(1000);
    if (!Serial){
      // it's probably not starting, so don't try to send any messages over the Serial port anymore
      // OUTPUT_SERIAL = false;
    } else {
      Serial.println ("Serial Started ...");
    }
    delay(100);
  } else if (OUTPUT_aEFIS) {
    // For debugging on USB
    // Serial.begin(115200);
    delay(1000);
    while (!Serial) {
      delay(100);
    }
  }


  // Set up the connection from the Adafruit board to the Speedy Bee over Rx/Tx lines
  // Note: whichever UART port you're using on the Speedy Bee has to be configured to
  //        output MAVLink data or this entire project is basically useless
  MAVLINK_SERIAL.begin(115200); // MAVLink Serial Port Speed  (Commonly 57600 for Ardupilot)
  delay(100);
  if (!MAVLINK_SERIAL) {
    if (OUTPUT_SERIAL) {
      Serial.println("Error starting MAVLink ... continuing");
    }
  } else {
    if (OUTPUT_SERIAL) {
      Serial.println("MAVLink Attitude Listener Started");
    }
  }

  // From when I was playing around with NeoPixel stuff
  // strip.begin();  // initialize the strip
  // strip.show();   // make sure it is visible
  // strip.clear();  // Initialize all pixels to 'off'
  // strip.setPixelColor(0, 255, 0, 0);
  // strip.show();
  // delay(10);

  if (OUTPUT_CAN) {
    pinMode(PIN_CAN_STANDBY, OUTPUT);
    digitalWrite(PIN_CAN_STANDBY, false);
    pinMode(PIN_CAN_BOOSTEN, OUTPUT);
    digitalWrite(PIN_CAN_BOOSTEN, true);

    while (!CAN.begin(250000)) { // start the CAN bus at 250 kbps
      if (OUTPUT_SERIAL) {
        Serial.println("CAN failed to initialize! Will try again in 1 second ...");
        delay(1000);
      }
    }

    if (OUTPUT_SERIAL) {
      Serial.println("CAN started!");
    }

  }
  
  delay(500);
}


void loop() {
  handleMavlinkMessage();

  // More NeoPixel stuff
  // strip.setPixelColor(0, 255, 0, 0);
  // strip.show();
  // delay(10);

  //delay(3); //Small delay for loop rate
}

