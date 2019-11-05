#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

struct state{
  int rotate_A;       // range: -1023 to 1023
  int rotate_B;       // range: -1023 to 1023
  bool soft_stop_A;   // opposite to hard stop when rotate = 0
  bool soft_stop_B;   // opposite to hard stop when rotate = 0
};

const int MAX_PACKET_SIZE = 256;
const char MAGIC_NUMBER = 123;

struct state vehicle = {0, 0, false, false};
int AIN1, AIN2, PWMA;
int BIN1, BIN2, PWMB;
WiFiUDP Udp;
unsigned int localUdpPort = 6111;
char incomingPacket[MAX_PACKET_SIZE];
bool paired = false;
IPAddress pairedIp;
unsigned int pairedPort;

void setup() {
  Serial.begin(115200);
  WiFi.begin("Detonate-2137","roszki123");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
//
//  WiFi.mode(WIFI_STA);
//  WiFi.disconnect();
//  delay(100);
//  
//  IPAddress localIp(192,168,1,1);
//  IPAddress gateway(192,168,1,1);
//  IPAddress subnet(255,255,255,0);
//  
//  WiFi.softAPConfig(localIp, gateway, subnet);
//  WiFi.softAP("Dominik Config");

  
  static const uint8_t D0 = 16;
  static const uint8_t D1 = 5;
  static const uint8_t D2 = 4;
  static const uint8_t D3 = 0; //Never use GPIO0 (D3) as input pin, this pin is flash button. If it is low at power on state this will put ESP in programming mode.
  static const uint8_t D4 = 2;
  static const uint8_t D5 = 14;
  static const uint8_t D6 = 12;
  static const uint8_t D7 = 13;
  static const uint8_t D8 = 15;
  static const uint8_t D9 = 3;
  static const uint8_t D10 = 1;

  pinMode(D0, OUTPUT);     // Initialize pin as an output AIN1
  pinMode(D1, OUTPUT);     // Initialize pin as an output AIN2
  pinMode(D2, OUTPUT);     // Initialize pin as an output PWMA
  pinMode(D5, OUTPUT);     // Initialize pin as an output BIN1
  pinMode(D6, OUTPUT);     // Initialize pin as an output BIN2
  pinMode(D7, OUTPUT);     // Initialize pin as an output PWMB

  pinMode(D9, OUTPUT);     // Initialize pin as an output STBY
  digitalWrite(D9, LOW);   // STBY in LOW -> sleep
  delay(1000);
  digitalWrite(D9, HIGH);  // STBY in HIGH -> work
  
}

void loop() {
  //tank_test();
  if(get_udp_packet()){
    parse_udp_packet();
    set_engines();
  }
  
  
}
bool get_udp_packet(){
    int packetSize = Udp.parsePacket();
    int len = 0;
    if(packetSize <= MAX_PACKET_SIZE && packetSize > 0) {
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      len = Udp.read(incomingPacket, MAX_PACKET_SIZE);
      if(len > 0) incomingPacket[len] = '\0';
      Serial.printf("UDP packet contents: %s\n", incomingPacket);    
      }
    return len == 6;
}

void parse_udp_packet(){
  if(incomingPacket[0] != MAGIC_NUMBER) return;
  if(paired && Udp.remoteIP() == pairedIp){
    pairedPort = Udp.remotePort();
  }else if(!paired){
    paired = true;
    pairedIp = Udp.remoteIP();
    pairedPort = Udp.remotePort();
  }else return;
  if(incomingPacket[1] == 255){
    paired = false;
  }
  int temp;
  temp =  (incomingPacket[2] - 127) * 8;
  if(temp >= -1023 && temp <= 1023) vehicle.rotate_A = temp;
  temp =  (incomingPacket[3] - 127) * 8;
  if(temp >= -1023 && temp <= 1023) vehicle.rotate_B = temp;
  temp =  incomingPacket[4];
  if(temp == 1 && vehicle.rotate_A == 0) vehicle.soft_stop_A = true;
  else vehicle.soft_stop_A = false;
  temp =  incomingPacket[5];
  if(temp == 1 && vehicle.rotate_B == 0) vehicle.soft_stop_B = true;
  else vehicle.soft_stop_B = false;
  Serial.printf("Got: %d %d %d %d \n", incomingPacket[2], incomingPacket[3], incomingPacket[4], incomingPacket[5]);
  Serial.printf("Translated: %d %d %d %d \n", vehicle.rotate_A, vehicle.rotate_B, vehicle.soft_stop_A, vehicle.soft_stop_B);
}
void tank_test(){
  for(int i = -1023; i < 1024; i++){
    vehicle.rotate_A = i;
    vehicle.rotate_B = i;
    set_engines();
    delay(2);
  }
}

void set_engines(){
  if(vehicle.rotate_A < -1023 || vehicle.rotate_A > 1023 || vehicle.rotate_B < -1023 || vehicle.rotate_B > 1023)  return;

  if(vehicle.soft_stop_A == true){
    AIN1 = LOW;   // the same when HIGH
    AIN2 = LOW;   // the same when HIGH
    PWMA = 500;   // doesn't matter
  }
  else{
    if(vehicle.rotate_A > 0){
      AIN1 = HIGH;
      AIN2 = LOW;
    } else{
      AIN1 = LOW;
      AIN2 = HIGH;
    }
    PWMA = abs(vehicle.rotate_A);
  }

  if(vehicle.soft_stop_B == true){
    BIN1 = HIGH;   // the same when HIGH
    BIN2 = HIGH;   // the same when HIGH
    PWMB = 500;   // doesn't matter
  }
  else{
    if(vehicle.rotate_B > 0){
      BIN1 = HIGH;
      BIN2 = LOW;
    } else{
      BIN1 = LOW;
      BIN2 = HIGH;
    }
    PWMB = abs(vehicle.rotate_B);
  }

  
  digitalWrite(D0, AIN1);
  digitalWrite(D1, AIN2);
  analogWrite(D2, PWMA);
  digitalWrite(D5, BIN1);
  digitalWrite(D6, BIN2);
  analogWrite(D7, PWMB);
}
