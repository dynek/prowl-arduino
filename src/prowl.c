// Include libraries
#include <SPI.h>
#include <Ethernet.h>
#include <HTTPClient.h>

// Variables for ethernet connection (you know how it goes right ?)
byte mac[] = {  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
byte serverIP[] = { 209,20,72,170 }; // api.prowlapp.com
EthernetClient client;

// Define Prowl API information
#define PROWL_API_KEY "<your api key>"
#define PROWL_API_SRV "api.prowlapp.com"
#define PROWL_API_URL "http://api.prowlapp.com/publicapi/add"

// Define notification application, event and description
#define PUSH_APPLICATION "Arduino"
#define PUSH_EVENT "Mailbox"
#define PUSH_DESCRIPTION "You've got mail!"

// Variables in regard to timing
const unsigned long sendInterval = 60000; // 1 minute minimum between notifications
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

// Variables in regard to INPUT / OUTPUT
int switchPin = 2;
int switchCurrentState;
int switchPreviousState;

void setup() {
  // Hold on for 2 secs so Serial Monitor can be opened
  delay(2000);
  
  // Start the serial port
  Serial.begin(9600);
  
  // Start the ethernet connection
  Serial.println("Configuring Ethernet Connection:");
  if( Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // No point in carrying on, so do nothing forevermore
    for(;;)
      ;
  }
  
  // Connection successful, display IP address
  Serial.println("Connection successful");
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
}

void loop() {
  // Read switch value
  switchCurrentState = digitalRead(switchPin);
  delay(10); // Fix to button boucing issue
  if (switchCurrentState == digitalRead(switchPin)) { // Read switch value a second time to fix button boucing issue
    // Get number of milliseconds since the Arduino board began running the current program
    currentMillis = millis();
    // If switch state changed, is LOW and delay has expired
    if (switchCurrentState != switchPreviousState && switchCurrentState == LOW && (previousMillis == 0 || currentMillis - previousMillis >= sendInterval)) {
      // Set previousMillis as currentMillis
      previousMillis = currentMillis;
      
      // Send message
      char apikey[] = PROWL_API_KEY;
      char application[] = PUSH_APPLICATION;
      char event[] = PUSH_EVENT;
      char description[] = PUSH_DESCRIPTION;
      sendProwlNotification(apikey, application, event, description);
    }
    // Set current value as last value for next iteration
    switchPreviousState = switchCurrentState;
  }
}

void sendProwlNotification(char* apikey, char* application, char* event, char* description) {
  char url[] = "";
  sendProwlNotification(apikey, application, event, description, 0, url);
}

void sendProwlNotification(char* apikey, char* application, char* event, char* description, int priority) {
  char url[] = "";
  sendProwlNotification(apikey, application, event, description, priority, url);
}

void sendProwlNotification(char* apikey, char* application, char* event, char* description, int priority, char* url) {
  // Convert priority from int to char*
  char sPriority[1];
  sprintf(sPriority, "%d", priority);

  // Create HTTPClient
  char serverHostName[] = PROWL_API_SRV;
  HTTPClient client(serverHostName, serverIP);
  
  // Create POST parameters
  http_client_parameter postParameters[] = {
    {"apikey", apikey},
    {"application", application},
    {"event", event},
    {"description", description},
    {"priority", sPriority},
    {NULL, NULL}
  };
  
  // Send POST to server
  char data[] = "";
  char apiURL[] = PROWL_API_URL;
  FILE* result = client.postURI(apiURL, postParameters, data);
  
  // Could we connect to API server ?
  if (result == NULL) {
    Serial.println("Failed to connect to API server");
  } else {
    // Check error code
    int returnCode = client.getLastReturnCode();
    if(returnCode == 200) {
      // Successfully sent notification
      Serial.println("Notification sent!");
      
      // Display HTML page returned for debugging
      /*char buffer [40];
      while ( ! feof (result) )
      {
        if ( fgets (buffer , 100 , result) != NULL )
          Serial.print(buffer);
      }*/
    } else {
      // Display error code on serial
      Serial.print("Error sending notification! Error Code: ");
      Serial.println(returnCode);
    }
    client.closeStream(result);
  }
} 
