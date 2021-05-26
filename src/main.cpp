#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

#define CLOCK_I2C_ADDR 0x68
#define RELAY_PIN 2
#define EEPROM_ACTIVATION_START_ADDR 0
#define EEPROM_ACTIVATION_END_ADDR 1

struct {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t dayOfWeek;
  uint8_t dayOfMonth;
  uint8_t month;
  uint8_t year;
} datetime;

struct {
  uint8_t start;
  uint8_t end;
} activationTime;

void readActivationTime();
void printHelp();
void printActivationTime();

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Initializing...");
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  readActivationTime();
  Serial.println("Done");
  Serial.println("Press 'h' for help");
}

void readActivationTime() {
  Serial.println("Reading activation time from EEPROM...");
  activationTime.start = EEPROM.read(EEPROM_ACTIVATION_START_ADDR);
  activationTime.end = EEPROM.read(EEPROM_ACTIVATION_END_ADDR);
  printActivationTime();
}

void printActivationTime() {
  Serial.print("Start: ");
  Serial.println(activationTime.start);
  Serial.print("End: ");
  Serial.println(activationTime.end);
}

void writeActivationTime() {
  Serial.println("Saving activation time to EEPROM...");
  EEPROM.write(EEPROM_ACTIVATION_START_ADDR, activationTime.start);
  EEPROM.write(EEPROM_ACTIVATION_END_ADDR, activationTime.end);
  Serial.println("Done");
}

void writeDatetime() {
  uint8_t data;
  Wire.beginTransmission(CLOCK_I2C_ADDR);
  Wire.write(0);

  // Seconds
  Wire.write(((datetime.seconds / 10) << 4) | (datetime.seconds % 10));

  // Minutes
  Wire.write(((datetime.minutes / 10) << 4) | (datetime.minutes % 10));

  // Hours 23
  uint8_t twenties = (datetime.hours / 20) & 0x01;
  uint8_t tens = ((datetime.hours % 20) / 10) & 0x01;
  uint8_t ones = (datetime.hours - (twenties * 20) - (tens * 10)) & 0x0f;
  Wire.write(twenties << 5 | tens << 4 | ones);


  // Day of week
  Wire.write(datetime.dayOfWeek & 0x07);

  // Day of month
  Wire.write(((datetime.dayOfMonth / 10) << 4) | (datetime.dayOfMonth % 10));

  // Month
  Wire.write(((datetime.month / 10) << 4) | (datetime.month % 10));

  // Year
  Wire.write(((datetime.year / 10) << 4) | (datetime.year % 10));

  Wire.endTransmission();
}

void readDatetime() {
  uint8_t data;
  Wire.beginTransmission(CLOCK_I2C_ADDR);
  Wire.write(0);
  Wire.endTransmission(false);
  Wire.requestFrom(CLOCK_I2C_ADDR, 7);

  // Seconds
  data = Wire.read();
  datetime.seconds = ((data >> 4) * 10) + (data & 0x0f);

  // Minutes
  data = Wire.read();
  datetime.minutes = ((data >> 4) * 10) + (data & 0x0f);

  // Hours
  data = Wire.read();
  uint8_t tens = (data >> 4) & 0x01;
  uint8_t twenties = (data >> 5) & 0x01;
  datetime.hours = (twenties * 20) + (tens * 10) + (data & 0x0f);

  // Day of week
  data = Wire.read();
  datetime.dayOfWeek = data & 0x07;

  // Day of month
  data = Wire.read();
  datetime.dayOfMonth = ((data >> 4) * 10) + (data & 0x0f);

  // Month
  data = Wire.read();
  datetime.month = (((data >> 4) & 0x01) * 10) + (data & 0x0f);

  // Year
  data = Wire.read();
  datetime.year = ((data >> 4) * 10) + (data & 0x0f);
  Wire.endTransmission();
}

void printDatetime() {
  Serial.print(datetime.dayOfMonth);
  Serial.print("/");
  Serial.print(datetime.month);
  Serial.print("/");
  Serial.print(datetime.year);
  Serial.print(" ");
  Serial.print(datetime.hours);
  Serial.print(":");
  Serial.print(datetime.minutes);
  Serial.print(":");
  Serial.println(datetime.seconds);
}

void loop() {
  readDatetime();

  if (Serial.available() > 0) {
    uint8_t c = Serial.read();
    if (c == 's') {
      Serial.println("Enter year (00-99):");
      while (Serial.available() == 0);
      datetime.year = Serial.readString().toInt();
      Serial.println("Enter month:");
      while (Serial.available() == 0);
      datetime.month = Serial.readString().toInt();
      Serial.println("Enter day of week (Sunday=1):");
      while (Serial.available() == 0);
      datetime.dayOfWeek = Serial.readString().toInt();
      Serial.println("Enter day of month:");
      while (Serial.available() == 0);
      datetime.dayOfMonth = Serial.readString().toInt();
      Serial.println("Enter hours:");
      while (Serial.available() == 0);
      datetime.hours = Serial.readString().toInt();
      Serial.println("Enter minutes:");
      while (Serial.available() == 0);
      datetime.minutes = Serial.readString().toInt();
      Serial.println("Enter seconds:");
      while (Serial.available() == 0);
      datetime.seconds = Serial.readString().toInt();
      printDatetime();
      writeDatetime();
    }
    if (c == 't') {
      printDatetime();
    }
    if (c == 'a') {
      printActivationTime();
    }
    if (c == 'c') {
      Serial.println("Enter start time:");
      while (Serial.available() == 0);
      activationTime.start = Serial.readString().toInt();
      Serial.println(activationTime.start);
      Serial.println("Enter end time:");
      while (Serial.available() == 0);
      activationTime.end = Serial.readString().toInt();
      Serial.println(activationTime.end);

      writeActivationTime();
    }
    if (c == 'h') {
      printHelp();
    }
  }
  uint8_t on = (datetime.hours >= activationTime.start) && (datetime.hours <= activationTime.end);

  digitalWrite(RELAY_PIN, !on);
  delay(1000);
}

void printHelp() {
  Serial.println("Usage:");
  Serial.println("'t' (Print time)");
  Serial.println("'a' (Print activation time)");
  Serial.println("'c' (Set activation time)");
  Serial.println("'s' (Set time)");
  Serial.println("'h' (Help)");
}