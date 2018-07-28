/*
Name:    ground_station.ino
Created:  7/23/2018
Author: Richad Bamford (FOSSA Systems)
*/
#include <LoRaLib.h>
#include "configuration.h"
#include "debugging_utilities.h"
#include "state_machine_declerations.h"
#include "communication.h"

void setup()
{
  Serial.begin(9600);

  // Initialize the SX1278 interface with default settings.
  // See the PDF reports on previous PocketQube attempts for more info.
  Debugging_Utilities_DebugLog("SX1278 interface :\nCARRIER_FREQUENCY "
    + String(CARRIER_FREQUENCY) + " MHz\nBANDWIDTH: "
    + String(BANDWIDTH) + " kHz\nSPREADING_FACTOR: "
    + String(SPREADING_FACTOR) + "\nCODING_RATE: "
    + String(CODING_RATE) + "\nSYNC_WORD: "
    + String(SYNC_WORD) + "\nOUTPUT_POWER: "
    + String(OUTPUT_POWER));
    
  byte err_check = LORA.begin(CARRIER_FREQUENCY, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SYNC_WORD, OUTPUT_POWER);
  
  if (err_check == ERR_NONE)
  {
    Debugging_Utilities_DebugLog("(S) SX1278 Online!");
  }
  else
  {
    Debugging_Utilities_DebugLog("(E) SX1278 Error code = 0x" + String(err_check, HEX));
    while (true);
  }
}

void loop()
{
  String str;
  byte state = LORA.receive(str);
  
  String signature = str.substring(0, 10);
  String withoutSignature = str.substring(10);

  int indexOfS1 = withoutSignature.indexOf('S');
  String message = withoutSignature.substring(indexOfS1);

  String function_id = withoutSignature.substring(0, indexOfS1);

  /*
  Serial.println("Signature: " + String(signature));
  Serial.println("Function ID: " + functionId);
  Serial.println("Message: " + message);
  */

  if (state == ERR_NONE)
  {
    ///////////////
    // recieving //
    ///////////////
    if (function_id == "1")
    {
      Communication_ReceivedStartedSignal();
    }
    if (function_id == "2")
    {
      Communication_ReceivedStoppedSignal();
    }
    if (function_id == "3")
    {
      Communication_ReceivedTransmittedOnline();
    }
    if (function_id == "4")
    {
      Communication_ReceivedDeploymentSuccess();
    }
    if (function_id == "6")
    {
      Communication_ReceivedPong();
    }
    if (function_id == "9")
    {
      Communication_ReceivedPowerInfo(message);
    }
    if (function_id == "10")
    {
      // Frequency error for automatic tuning...
      float frequencyError = LORA.getFrequencyError();
      
      Communication_ReceivedTransceiverSettings(message, frequencyError);
    }
  
    ///////////////////////////////
    // transmitting to satellite //
    ///////////////////////////////
    if (STATE_TRANSMIT_PING)
    {
      Communication_TransmitPing();
      STATE_TRANSMIT_PING = false;
    }
    if (STATE_TRANSMIT_STOP_TRANSMITTING)
    {
      Communication_TransmitStopTransmitting();
      STATE_TRANSMIT_STOP_TRANSMITTING = false;
    }
    if (STATE_TRANSMIT_START_TRANSMITTING)
    {
      Communication_TransmitStartTransmitting();
      STATE_TRANSMIT_START_TRANSMITTING = false;
    }
  }
  else if (state == ERR_RX_TIMEOUT)
  {
    // timeout occurred while waiting for a packet
    Debugging_Utilities_DebugLog("timeout!");

  }
  else if (state == ERR_CRC_MISMATCH)
  {
    // packet was received, but is malformed
    Debugging_Utilities_DebugLog("CRC error!"); 
  }

  delay(200);
}
