#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <MFRC522.h>
#include <SPI.h>
#include <ESP32Servo.h>

#define MAXLEN 128
#define GATE_OPEN 15
#define GATE_CLOSED 90

#define SS_PIN 21
#define RST_PIN 5

#define SERVO_PIN 4

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

Servo sg90;

const char* ssid = "DIGI_e3e78f";
const char* pass = "38866c68";

const char* mqtt_broker_ip = "192.168.1.9";
int mqtt_broker_port = 8883;
const char* mqtt_username = "proiect-pr";
const char* mqtt_password = "difficult";

const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEHTCCAwWgAwIBAgIUNP65Mcq3I2LhhGV81E/GA7JU5igwDQYJKoZIhvcNAQEL
BQAwgZ0xCzAJBgNVBAYTAlJPMRIwEAYDVQQIDAlCdWNoYXJlc3QxEjAQBgNVBAcM
CUJ1Y2hhcmVzdDEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRcw
FQYDVQQDDA5BbmRyZWlNYXJ1bnRpczEqMCgGCSqGSIb3DQEJARYbYW5kcmVpLm1h
cnVudGlzbW1AZ21haWwuY29tMB4XDTI1MDExNjExMTQwN1oXDTM1MDExNDExMTQw
N1owgZ0xCzAJBgNVBAYTAlJPMRIwEAYDVQQIDAlCdWNoYXJlc3QxEjAQBgNVBAcM
CUJ1Y2hhcmVzdDEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRcw
FQYDVQQDDA5BbmRyZWlNYXJ1bnRpczEqMCgGCSqGSIb3DQEJARYbYW5kcmVpLm1h
cnVudGlzbW1AZ21haWwuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC
AQEAy/1UhiTzGcd1vT0V5rhtDsMSTllXZ+8Yfraw704sOc4hex9fd79cIgru74S1
CarM7MX7JovLCJkjADFVieff5CFQ1+/lvsXVe3vs8r7HVah/OAz0atrLuwdIq+Sf
/LK7W4AW2hcp6s0ekZpsr+VymjttPlmNNYW1dNwss+6FPM7vUQOpxPas9EIUGtVx
NN9HBG1jJBiPItYdZcl/ycR3QTqflETwtz08YAgypEizfguZDraSNFj2GWt+HWj0
4R4lilw1qXkr1h6YhL0Ljyn35SPN91bp/DAuwTNvEuuNz+xASa+C0l/AsJYjnALY
96OcqVsENewuMR+1ZEW2+gblHwIDAQABo1MwUTAdBgNVHQ4EFgQUhdbwWrbUVUDb
ZJvFRSlz+Z/AXVowHwYDVR0jBBgwFoAUhdbwWrbUVUDbZJvFRSlz+Z/AXVowDwYD
VR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAPKvrEdLhUGScq/9OgiwU
m+drgSPYGwQ9AJwHJUW8kaWRHr9Wu43N4Gz/s6xtditHdW4HkZeOIxaMn4hYdnK9
uSqdkw5OOfunLZ0Pau1P7uQbIEsF1iKG+rQqDicMlXuE6fNe+BAuxMM12KmjQVyQ
x/Py3s3fsb7EQMWgzSpLURyXKMjSMd5bFMu5UJuUKFTNsxCUJyMwXaOQg0vegAoR
AluXPfuJTqpllcyfo8twlCyWm5k82k2jfhAsGG4vhrIW/D1kr+QZMw8zNZsI4dO4
34IAOQfYT08LT5LhwkwR2uTdN17LVDXk8IB2Majw69Hwox1SQW+oava8ZkDopjMd
pQ==
-----END CERTIFICATE-----
)EOF";

WiFiClientSecure espClient;
PubSubClient client(espClient);

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as hex values to a String. 
 */
String parseHex(byte *buffer, byte bufferSize) {
  String output = "";
  for (byte i = 0; i < bufferSize; i++) {
    output += buffer[i] < 0x10 ? " 0" : " ";
    output += String(buffer[i], HEX);
  }
  return output;
}

void wait_for_access(char *topic, byte *payload, unsigned int length) {
  char buffer[MAXLEN];
  for (int i = 0; i < length; i++) {
    buffer[i] = (char)payload[i];
  }
  buffer[length] = '\0';
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print(buffer);
  Serial.println();

  if (!strcmp(buffer, "granted")) {
    Serial.println("Access granted");
    sg90.write(GATE_OPEN);
    delay(5000);
    sg90.write(GATE_CLOSED);
  } else {
    Serial.println("Access denied");
  }
}

void WiFi_init() {
  espClient.setCACert(ca_cert);
  espClient.setInsecure();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
     Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());

     if (WiFi.status() == WL_CONNECTED) {
    client.setServer(mqtt_broker_ip, mqtt_broker_port);
    client.setCallback(wait_for_access);
    while (!client.connected()) {
      String client_id = "ESP32Client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Connected to the public MQTT broker");
        client.subscribe("andrei/scan1/access");
      } else {
        Serial.print("Failed to connect to the public MQTT broker, state: ");
        Serial.print(client.state());
        delay(5000);
      }
    }
  }
}

void RFID_init() {
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void Servo_init() {
  sg90.setPeriodHertz(50); // Standard 50hz servo
  sg90.attach(SERVO_PIN, 500, 2400); // attaches the servo on pin 4 to the servo object
  sg90.write(GATE_CLOSED);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi_init();
  RFID_init();
  Servo_init();
}

String rfid_scan() {
  String aux = "";

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return aux;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return aux;

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && 
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return aux;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));

  aux = parseHex(rfid.uid.uidByte, rfid.uid.size);

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  return aux;
}

void loop() {
  client.loop();

  String out = rfid_scan();

  if (out != "") {
    client.publish("andrei/scan1", out.c_str());
  }
}