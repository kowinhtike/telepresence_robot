#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ESP32Servo.h>

// ---------------- WiFi ----------------
const char* ssid = "Software Engineer";
const char* password = "123459876";

// ---------------- WebSocket ----------------
WebSocketsClient webSocket;

// ---------------- Motor Pins ----------------
const int in1 = 25;
const int in2 = 33;
const int in3 = 27;
const int in4 = 26;
const int ledPin = 2;

// ---------------- Servo Pins ----------------
const int headXPin = 5;
const int headYPin = 18;
const int leftXPin = 19;
const int leftYPin = 21;
const int rightXPin = 22;
const int rightYPin = 23;

// ---------------- Servo Objects ----------------
Servo headX, headY;
Servo leftX, leftY;
Servo rightX, rightY;

// ---------------- Motor Functions ----------------
void forward() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void backward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void left() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void right() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
}

void stopCar() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  digitalWrite(ledPin, LOW);
  Serial.println("🚦 Car STOPPED");
}

// ---------------- Servo Smooth Movement ----------------
void moveServoSmooth(Servo &servo, int target, int speedDelay=10){
  target = constrain(target,0,180);
  int current = servo.read();
  int step = (target>current)?1:-1;
  while(current != target){
    current += step;
    servo.write(current);
    delay(speedDelay);
  }
}

// ---------------- Move All Servos ----------------
void moveServos(String data){
  // Split by comma
  int start=0;
  while(start < data.length()){
    int commaIndex = data.indexOf(',', start);
    String cmd;
    if(commaIndex==-1){
      cmd = data.substring(start);
      start = data.length();
    } else{
      cmd = data.substring(start, commaIndex);
      start = commaIndex + 1;
    }

    cmd.trim();

    if(cmd.startsWith("HX:")) moveServoSmooth(headX, cmd.substring(3).toInt());
    else if(cmd.startsWith("HY:")) moveServoSmooth(headY, cmd.substring(3).toInt());
    else if(cmd.startsWith("LX:")) moveServoSmooth(leftX, cmd.substring(3).toInt());
    else if(cmd.startsWith("LY:")) moveServoSmooth(leftY, cmd.substring(3).toInt());
    else if(cmd.startsWith("RX:")) moveServoSmooth(rightX, cmd.substring(3).toInt());
    else if(cmd.startsWith("RY:")) moveServoSmooth(rightY, cmd.substring(3).toInt());
  }

  Serial.println("🧠 Servos updated: H(" + String(headX.read()) + "," + String(headY.read()) +
                     ") L(" + String(leftX.read()) + "," + String(leftY.read()) +
                     ") R(" + String(rightX.read()) + "," + String(rightY.read()) + ")");
}

// ---------------- WebSocket Event ----------------
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length){
  switch(type){
    case WStype_TEXT:{
      String cmd = (char*)payload;
      Serial.println("CMD: " + cmd);

      // Motor commands
      if(cmd=="F") forward();
      else if(cmd=="B") backward();
      else if(cmd=="L") left();
      else if(cmd=="R") right();
      else if(cmd=="S") stopCar();

      if(cmd=="F" || cmd=="B" || cmd=="L" || cmd=="R") digitalWrite(ledPin,HIGH);

      // Servo command (starts with HX)
      if(cmd.startsWith("HX:")) moveServos(cmd);
    }
    break;

    case WStype_DISCONNECTED:
      Serial.println("⚠️ WebSocket Disconnected → Stopping Car");
      stopCar();
      break;

    case WStype_CONNECTED:
      Serial.println("✅ WebSocket Connected → Default STOP");
      stopCar();
      break;
  }
}

// ---------------- Setup ----------------
void setup(){
  Serial.begin(115200);

  // Motor pins
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  pinMode(in3,OUTPUT);
  pinMode(in4,OUTPUT);
  pinMode(ledPin,OUTPUT);

  // Attach servos
  headX.attach(headXPin); headY.attach(headYPin);
  leftX.attach(leftXPin); leftY.attach(leftYPin);
  rightX.attach(rightXPin); rightY.attach(rightYPin);

  // Initialize to center
  headX.write(90); headY.write(90);
  leftX.write(90); leftY.write(90);
  rightX.write(90); rightY.write(90);

  // WiFi connect
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");

  // SSL WebSocket
  webSocket.beginSSL("arduinocar-nodejs.onrender.com",443,"/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

// ---------------- Loop ----------------
void loop(){
  webSocket.loop();
  if(WiFi.status()!=WL_CONNECTED){
    Serial.println("⚠️ WiFi Lost → Stopping Car");
    stopCar();
  }
}
