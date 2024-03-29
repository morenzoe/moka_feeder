/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <WiFi.h>
#include <PubSubClient.h>
#define BUILTIN_LED 4
#define Laser 2
#include <Servo.h>
Servo ServoX;
Servo ServoY;
Servo ServoF;
int angle1 = 0;
int angle2 = 0;
int angle3 = 0;

// Update these with values suitable for your network.

const char* ssid = "New-DU83 3";
const char* password = "Dipatiukur.83-3";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  //Servo

  if (strcmp(topic, "mokaFeeder/puter_kiri") == 0){
    if(angle1 <= 170) {
      angle1 += 10;
      ServoX.write(angle1);
    }
  }

  if (strcmp(topic, "mokaFeeder/puter_kanan") == 0){
    if(angle1 >= 10) {
      angle1 -= 10;
      ServoX.write(angle1); 
    }
  }

  if (strcmp(topic, "mokaFeeder/puter_atas") == 0){
    if(angle2 >= 10) {
      angle2 -= 10;
      ServoY.write(angle2);
    }
  }

  if (strcmp(topic, "mokaFeeder/puter_bawah") == 0){
     if(angle2 <= 170) {
        angle2 += 10;
        ServoY.write(angle2);
      }
  }

   if (strcmp(topic, "mokaFeeder/feeding_moka") == 0){
      // open
      for(angle3 = 180; angle3>=1; angle3-=5)     // command to move from 180 degrees to 0 degrees 
      {                                
        ServoF.write(angle3);              //command to rotate the servo to the specified angle
        delay(5);                       
      } 
    
      delay(550); // 30 grams of cat food
    
      // close
      for(angle3 = 0; angle3 < 180; angle3 += 5)    // command to move from 0 degrees to 180 degrees 
      {                                  
        ServoF.write(angle3);                 //command to rotate the servo to the specified angle
        delay(5);                       
      } 
   }

   if (strcmp(topic, "mokaFeeder/laser") == 0){
     digitalWrite(Laser, LOW);
      } else if (strcmp(topic, "mokaFeeder/laser") == 1){
        digitalWrite(Laser, HIGH);
  }

//  // Switch on the LED if an 1 was received as first character
//  if ((char)payload[0] == '1') {
//    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
//    // but actually the LED is on; this is because
//    // it is active low on the ESP-01)
//  } else {
//    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
//  }

}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("mokaFeeder/status", "meow");
      // ... and resubscribe
      uint16_t keKiri = client.subscribe("mokaFeeder/puter_kiri");
      uint16_t keKanan = client.subscribe("mokaFeeder/puter_kanan");
      uint16_t keAtas = client.subscribe("mokaFeeder/puter_atas");
      uint16_t keBawah = client.subscribe("mokaFeeder/puter_bawah");
      uint16_t ayoMakan = client.subscribe("mokaFeeder/feeding_moka");
      uint16_t laserOn = client.subscribe("mokaFeeder/laser");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  ServoX.attach(16);
  ServoY.attach(3);
  ServoF.attach(1);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(Laser, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "meow #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("mokaFeeder/status", msg);
  }
}
