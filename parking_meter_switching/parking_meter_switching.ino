/*
   This code starts the parking meter and lets it connect to any available PAN.
   Once it starts up, it does not allow any other devices to join through it.
   However, there is a trip wire, that when it goes high (just from unplugging it
   from ground), the other devices will be allowed to join for any amount of time.
*/

#include <XBee.h>

// list of commands we will be sending, and some of the parameters
uint8_t commandNJ[] = {'N', 'J'};
uint8_t commandAI[] = {'A', 'I'};
uint8_t commandAC[] = {'A', 'C'};
uint8_t joinNever = 0x00;
uint8_t joinAlways = 0xFF;

XBee xbee = XBee();

// Set up the commands with parameters that will be sent at some later time
AtCommandRequest joinAlwaysRequest = AtCommandRequest(commandNJ, &joinAlways, 0x1);
AtCommandRequest joinNeverRequest = AtCommandRequest(commandNJ, &joinNever, 0x1);
AtCommandRequest applyChangesXbee = AtCommandRequest(commandAC);
// TODO: look into how to recieve API status frame instead of polling with AtCommands
AtCommandRequest readAIXbee = AtCommandRequest(commandAI);

// Variables to hold information about a response
AtCommandResponse commandResponse = AtCommandResponse();
int commandResponseStatus = 0;
uint8_t *commandResponseValue;

// Set up the button for the trip-wire
const int buttonPin = 2; 
int buttonState = 0;

// Keep track of state
boolean sentAI = false;

void setup () {
  pinMode(buttonPin, INPUT);
  xbee.begin(9600);
  xbee.send(joinNeverRequest);
  Serial.println("Telling Xbee to allow joins never");
  xbee.send(applyChangesXbee);
  Serial.println("Applying Changes");
}

void loop () {
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    // turn xbee node join time always on
    xbee.send(joinAlwaysRequest);
    Serial.println("Telling Xbee to allow joins always");
    xbee.send(applyChangesXbee);
    Serial.println("Applying Changes");
  }
  
  // read any data available
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
        if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
                xbee.getResponse().getAtCommandResponse(commandResponse);
                commandResponseStatus = commandResponse.getStatus();
                if (commandResponseStatus == 0) {
                  Serial.println("Command Response Success");
                  if (!sentAI) {
                    // xbee send ai
                    xbee.send(readAIXbee);
                    sentAI = true;
                  } else {
                    // was this an AI command response?
                    uint8_t *commandResponseCommand = commandResponse.getCommand();
                    if (isAICommand(commandResponseCommand)) {
                      commandResponseValue = commandResponse.getValue();
                      if (*commandResponseValue == 255) {
                        delay(1000);
                        // send it again
                        xbee.send(readAIXbee);
                      } else if (*commandResponseValue == 0) {
                        // print join succeeeded
                        Serial.println("Network join success");
                      } else {
                        Serial.print("AI = ");
                        Serial.println(*commandResponseValue, HEX);
                      }
                    }
                  }
                }
        }
  }
}

boolean isAICommand(uint8_t *command) {
  return command[0] == 65 && command[1] == 73;
}
