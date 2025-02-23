

//Joystick right Pins
const int xAxisPin2 = A1; 
const int yAxisPin2 = A0;
const int zAxisPin2 = A2;
const int buttonPin2 = 2;

//Joystick left Pins 
const int xAxisPin1 = A4;
const int yAxisPin1 = A3; 
const int zAxisPin1 = A5; 
const int buttonPin1 = 3; 

const int centerValue = 512; // Analog midpoint
const int deadZoneMin = 440; // Lower dead zone threshold
const int deadZoneMax = 583; // Upper dead zone threshold
const int minAnalog = 0;     // Min Analog value
const int maxAnalog = 1023;  // Max Analog value

//#define minAnalogZ 0
//#define maxAnalogZ 600

#define RATE_HZ 100

//#define TEST //Uncomment to print raw values instead of adj values

long interval_ms = 99;
long last_time = 0;
long current_time = 0;
void setup() {
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP); 
  Serial.begin(19200);
  interval_ms = 1/RATE_HZ * 1000; //interval in ms
  
}

void loop() {
  current_time = millis();
  if(current_time - last_time > interval_ms){
      last_time = millis();

  
  //Read raw values
  int rawX1 = analogRead(xAxisPin1); 
  int rawY1 = analogRead(yAxisPin1); 
  int rawZ1 = analogRead(zAxisPin1); 
  int buttonState1 = digitalRead(buttonPin1); 

  int rawX2 = analogRead(xAxisPin2);
  int rawY2 = analogRead(yAxisPin2); 
  int rawZ2 = analogRead(zAxisPin2); 
  int buttonState2 = digitalRead(buttonPin2); 

#ifdef TEST
Serial.print("TESING: ");
  // Print joystick values with adjusted dead zone
  Serial.print(rawX1);
  Serial.print(",");
  Serial.print(rawY1);
  Serial.print(",");
  Serial.print(rawZ1);
  Serial.print(",");
  Serial.print(buttonState1 == LOW ? "1" : "0");

  Serial.print(",");
  Serial.print(rawX2);
  Serial.print(",");
  Serial.print(rawY2);
  Serial.print(",");
  Serial.print(rawZ2);
  Serial.print(",");
  Serial.print(buttonState2 == LOW ? "1" : "0");
  Serial.println(")");
  
#else
  int adjX1 = mapJoystickToPercentage(rawX1);
  int adjY1 = mapJoystickToPercentage(rawY1);
  int adjZ1 = mapJoystickToPercentage(rawZ1);

  int adjX2 = mapJoystickToPercentage(rawX2);
  int adjY2 = mapJoystickToPercentage(rawY2);
  int adjZ2 = mapJoystickToPercentage(rawZ2);


  // Print joystick values with adjusted dead zone
  Serial.print("$arm_joy_panel(");
  Serial.print(adjX1, DEC);
  Serial.print(",");
  Serial.print(adjY1);
  Serial.print(",");
  Serial.print(adjZ1);
  Serial.print(",");
  Serial.print(buttonState1 == LOW ? "1" : "0");

  Serial.print(",");
  Serial.print(adjX2, DEC);
  Serial.print(",");
  Serial.print(adjY2);
  Serial.print(",");
  Serial.print(adjZ2);
  Serial.print(",");
  Serial.print(buttonState2 == LOW ? "1" : "0");
  Serial.println(")");
  
#endif
  // Function to map values to 1-100 percentage with deadzone 
  }else{
   delay(10); //delay not really needed but eh
  }

}

// Function to scale joystick input to 1-100 with deadzone 
int mapJoystickToPercentage(int rawValue) {
  if (rawValue >= deadZoneMin && rawValue <= deadZoneMax) {
    return 50; // Keep the dead zone at 50%
  } 
  else if (rawValue < deadZoneMin) {
    return map(rawValue, minAnalog, deadZoneMin - 1, 1, 49); // Scale below dead zone
  } 
  else {
    return map(rawValue, deadZoneMax + 1, maxAnalog, 51, 100); // Scale above dead zone
  }
}
