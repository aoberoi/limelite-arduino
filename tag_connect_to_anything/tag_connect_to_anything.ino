/*
   This code tells the tag to connect to anything available.
   Once it has connected, it will print to the Serial monitor that it has
   ----------------
   We assume that the Xbee WILL NOT go to sleep. To ensure this, we set the
   SM value to 1 (Pin Sleep) and make sure pin 9 is low all the time.
*/

#include <XBee.h>

XBee xbee = XBee();

uint8_t commandAI[] = {'A', 'I'};
AtCommandRequest readAIXbee = AtCommandRequest(commandAI);

AtCommandResponse commandResponse = AtCommandResponse();
int commandResponseStatus = 0;
uint8_t *commandResponseValue;

void setup () {
  xbee.begin(9600);
  xbee.send(readAIXbee);
  Serial.println("Read AI Sent");
}

void loop () {
  // wait 1 second so that we don't fire tons of commands all at once
  delay(1000);
  
  // read any data available (the next 2 lines are magic that I don't understand)
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      
      // put the response information into the variable commandResponse
      xbee.getResponse().getAtCommandResponse(commandResponse);
      
      // read the status from the commandResponse information
      commandResponseStatus = commandResponse.getStatus();
      
      // status was successful
      if (commandResponseStatus == 0) {
        
        // read value from commandResponse info
        commandResponseValue = commandResponse.getValue();
        if (*commandResponseValue == 255) {
          // still scanning for network, send the request again
          Serial.print("...");
          xbee.send(readAIXbee);
        } else if (*commandResponseValue == 0) {
          // join succeeded, print it
          Serial.println("Network join success");
        } else {
          // something else happened. stop trying and tell us what the status is.
          Serial.print("AI = ");
          Serial.println(*commandResponseValue, HEX);
        }
        
      // status was not successful
      } else {
        Serial.println("bad response");
      }
    }
  }
}
