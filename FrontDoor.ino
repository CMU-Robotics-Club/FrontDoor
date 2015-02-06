#include <PS2Keyboard.h>

#include <SPI.h>
#include <Ethernet.h>
#include <RC.h>

// SET THESE
#define PUBLIC_KEY ""
#define PRIVATE_KEY ""
byte mac[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#define FRONT_DOOR_RFID_CHANNEL_ID 10

#define RFID_IRQ_PIN 2
#define RFID_DATA_PIN 5

EthernetClient ethernetClient;

APIClient api(ethernetClient, PUBLIC_KEY, PRIVATE_KEY);

PS2Keyboard rfidReader;
String rfid;

uint32_t swap_uint32(uint32_t val);

void setup() {
  Serial.begin(9600);
  Serial.println(F("Welcome to the FrontDoor Project"));

  rfidReader.begin(RFID_DATA_PIN, RFID_IRQ_PIN);
  
  if(!Ethernet.begin(mac)) {
    Serial.println(F("Failed to initialize ethernet shield"));
    return; 
  }
  
  Serial.println(F("Connected to Internet"));

  rfid = "";
}

void loop() {  
  if (rfidReader.available()) {
    delay(200);
    
    while(rfidReader.available()) {
      char c = rfidReader.read();
      rfid += c;
    }
    
    char buffer[rfid.length()];
    rfid.toCharArray(buffer, rfid.length());
    
    long rfidValue = atol((const char*)buffer);
    
    rfidValue = swap_uint32(rfidValue);

    String rfidHex = String(rfidValue, HEX);
    
    if(rfidHex.length() == 7) {
      rfidHex = "0" + rfidHex; 
    }
    
    rfidHex.toUpperCase();

    Serial.print("RFID: ");
    Serial.println(rfidHex);
    
    UserLookup userLookup;
    if(!api.rfid(rfidHex, "", userLookup)) {
      Serial.println("RFID lookup failed"); 
    } else {
      if(!userLookup.found) {
        Serial.println("No User found");
         
        if(!api.channelWriteValue(FRONT_DOOR_RFID_CHANNEL_ID, "")) {
          Serial.println("Channel Write Failed"); 
        } else {
          Serial.println("Channel Write Success"); 
        }
      } else {
        Serial.print("User ID: ");
        Serial.println(userLookup.user_id);
       
        if(!api.channelWriteValue(FRONT_DOOR_RFID_CHANNEL_ID, String(userLookup.user_id, DEC))) {
          Serial.println("Channel Write Failed"); 
        } else {
          Serial.println("Channel Write Success"); 
        }
      } 
    }
    
    rfid = "";
    
    while(rfidReader.available()) {
      rfidReader.read(); 
    }
  }
}

uint32_t swap_uint32(uint32_t val)
{
  val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
  return (val << 16) | (val >> 16);
}
