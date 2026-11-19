//for node a
// C++ code
//
#include <Wire.h>

const int gasPin = A0;
const int tempPin = A1;
const int lightPin = A2;

int gasValue;
int lightValue;
int temperature10;

void setup() {
  Serial.begin(9600);

  // I2C Slave Address
  Wire.begin(8);

  // Send sensor data when Arduino 2 requests it
  Wire.onRequest(sendData);
}

void loop() {

  gasValue = analogRead(gasPin);
  lightValue = analogRead(lightPin);

  int tempRaw = analogRead(tempPin);

  float voltage = tempRaw * (5.0 / 1023.0);

  float temperature =
    (voltage - 0.5) * 100.0;

  temperature10 = (int)(temperature * 10);

  Serial.print("Gas: ");
  Serial.print(gasValue);

  Serial.print(" | Temperature: ");
  Serial.print(temperature);

  Serial.print(" C | Light: ");
  Serial.println(lightValue);

  delay(500);
}

// Send 6 bytes to Arduino 2
void sendData() {

  Wire.write(gasValue & 0xFF);
  Wire.write((gasValue >> 8) & 0xFF);

  Wire.write(temperature10 & 0xFF);
  Wire.write((temperature10 >> 8) & 0xFF);

  Wire.write(lightValue & 0xFF);
  Wire.write((lightValue >> 8) & 0xFF);
}
//for node b
#include <Wire.h>
#include <LiquidCrystal.h>

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// Output pins
const int motorPin = 9;
const int redPin = 10;
const int greenPin = 11;
const int bluePin = 12;
const int buzzerPin = 13;

// Sensor readings
int gasValue = 0;
int lightValue = 0;
int temperature10 = 0;

float temperature = 0;

// I2C address of Arduino 1
const int sensorAddress = 8;

// Gas thresholds (simulation)
const int gasWarning = 150;
const int gasEmergency = 250;

// Temperature thresholds
const float tempWarning = 35.0;
const float tempEmergency = 45.0;

void setup() {

  Serial.begin(9600);

  // I2C Master
  Wire.begin();

  // LCD
  lcd.begin(16, 2);
  lcd.clear();

  pinMode(motorPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(motorPin, LOW);
  digitalWrite(buzzerPin, LOW);

  lcd.setCursor(0, 0);
  lcd.print("SCADA SYSTEM");

  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(1500);
  lcd.clear();
}

void loop() {

  // Request 6 bytes from Arduino 1
  Wire.requestFrom(sensorAddress, 6);

  if (Wire.available() == 6) {

    gasValue = Wire.read();
    gasValue |= Wire.read() << 8;

    temperature10 = Wire.read();
    temperature10 |= Wire.read() << 8;

    lightValue = Wire.read();
    lightValue |= Wire.read() << 8;

    // Convert temperature
    temperature = temperature10 / 10.0;

    // Serial Monitor
    Serial.print("Gas: ");
    Serial.print(gasValue);

    Serial.print(" | Temp: ");
    Serial.print(temperature);

    Serial.print(" C | Light: ");
    Serial.println(lightValue);

    // Emergency condition
    if (gasValue >= gasEmergency ||
        temperature >= tempEmergency) {

      emergencyMode();

    }

    // Warning condition
    else if (gasValue >= gasWarning ||
             temperature >= tempWarning) {

      warningMode();

    }

    // Normal condition
    else {

      normalMode();

    }

    // Display readings and status together
    updateLCD();

  }

  else {

    sensorFault();

  }

  delay(500);
}

// NORMAL MODE
void normalMode() {

  analogWrite(motorPin, 0);

  noTone(buzzerPin);

  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);

}

// WARNING MODE
void warningMode() {

  analogWrite(motorPin, 0);

  tone(buzzerPin, 1000, 200);

  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);

}

// EMERGENCY MODE
void emergencyMode() {

  analogWrite(motorPin, 200);

  tone(buzzerPin, 2000);

  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);

}

// SENSOR FAULT
void sensorFault() {

  analogWrite(motorPin, 0);

  noTone(buzzerPin);

  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);

  lcd.setCursor(0, 0);
  lcd.print("SENSOR ERROR    ");

  lcd.setCursor(0, 1);
  lcd.print("CHECK I2C       ");

}

// LCD DISPLAY FUNCTION
void updateLCD() {

  // First row: Gas and Temperature
  lcd.setCursor(0, 0);
  lcd.print("                ");

  lcd.setCursor(0, 0);
  lcd.print("G:");
  lcd.print(gasValue);
  lcd.print(" T:");
  lcd.print(temperature, 1);
  lcd.print("C");

  // Second row: System status
  lcd.setCursor(0, 1);
  lcd.print("                ");

  lcd.setCursor(0, 1);

  if (gasValue >= gasEmergency ||
      temperature >= tempEmergency) {

    lcd.print("EMERG FAN:ON");

  }

  else if (gasValue >= gasWarning ||
           temperature >= tempWarning) {

    lcd.print("WARNING BUZZ:ON");

  }

  else {

    lcd.print("NORMAL FAN:OFF");

  }

}
