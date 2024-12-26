#include <Arduino.h>
#include <MAVLink.h>

#include <CANSAME5x.h>
CANSAME5x CAN;

// Define serial port for MAVLink communication (Serial1 is available on RP2040)
#define MAVLINK_SERIAL Serial1

// MAVLink system and component IDs
#define MAVLINK_SYSTEM_ID 1   // This is the ID for this script
#define MAVLINK_COMPONENT_ID 1 // Component ID

/*************************************/
/******* Select output type **********/
bool OUTPUT_SERIAL  = true;  // Output for the Arduino IDE's Serial Monitor - good for debugging
bool OUTPUT_aEFIS   = false; // Output for my aEFIS Android app - connect the feather's USB port to an OTG cable on an Android
bool OUTPUT_CAN     = true;  // Output CAN data in MakerPlane CAN-FiX format 
/*************************************/
/*************************************/


/******* MakerPlane CAN-FiX IDs ********/
// These values can be changed if you want to use another CAN standard
int FIX_PITCH = 384;
int FIX_ROLL = 385;
int FIX_HEADING = 389;
int FIX_GROUND_SPEED = 387;
int nodeId = 0x82;
/***************************************/


/**************************************/
// * Variables to store attitude data *
float roll = 0.0;
float pitch = 0.0;
float yaw = 0.0;
int32_t global_heading = 0;

signed long lngPitch = 0;
signed long lngRoll = 0;
signed long lngGlobalHeading = 0;
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
        default:
          // Ignore other messages or handle them as needed
          break;
      }

      lngPitch = pitch * 100;
      lngRoll = roll * 100;
      lngGlobalHeading = getHeadingReciprocal( yaw * 10 );
      //long lngGlobalHeading = yaw * 10;

      if (OUTPUT_CAN) {
        sendCanMessage(FIX_PITCH, lngPitch);
        sendCanMessage(FIX_ROLL, lngRoll);
        sendCanMessage(FIX_HEADING, lngGlobalHeading);
        // sendCanMessage(FIX_GROUND_SPEED, lngGroundSpeed);
      }

      if (OUTPUT_SERIAL) {
        Serial.print("pitch:"); Serial.print(lngPitch);
        Serial.print(",roll:"); Serial.print(lngRoll);
        Serial.print(",heading:");Serial.println(lngGlobalHeading);
        
        
        // Serial.print(",lngHeading:");Serial.print(lngHeading);
        // Serial.print(",lngPitch:"); Serial.print(lngPitch);
        // Serial.print(",lngRoll:"); Serial.println(lngRoll);
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
  long reciprocal = (heading + 1800) % 3600;
  return reciprocal;
}


void setup() {
  Serial.begin(115200); // For debugging on USB
  MAVLINK_SERIAL.begin(115200); // MAVLink Serial Port Speed  (Commonly 57600 for Ardupilot)
  while(!Serial){delay(100);}
  
  if (OUTPUT_SERIAL) {
    Serial.println("MAVLink Attitude Listener Started");
  }

  while (!CAN.begin(250000)) { // start the CAN bus at 250 kbps
    Serial.println("CAN failed to initialize! Will try again in 1 second ...");
    delay(1000);
  }

  if (OUTPUT_SERIAL) {
    Serial.println("CAN started!");
  }
  
  delay(1000);
}


void loop() {
  handleMavlinkMessage();

  delay(3); //Small delay for loop rate
}


