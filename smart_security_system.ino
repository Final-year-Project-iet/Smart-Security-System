#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>

Servo myservo;
int pos = 0;
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
const byte rows = 4;
const byte cols = 3;

char key[rows][cols] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[rows] = {1, 2, 3, 4};
byte colPins[cols] = {5, 6, 7};
Keypad keypad = Keypad(makeKeymap(key), rowPins, colPins, rows, cols);
char* password = "4567";
int currentposition = 0;
int redled = 10;
int greenled = 11;
int buzz = 8;
int invalidcount = 0;

const int ULTRASONIC_PIN = 12;
const int PIR_PIN = 13;

bool isArmed = true; 

long readUltrasonicDistance() {
  pinMode(ULTRASONIC_PIN, OUTPUT);
  digitalWrite(ULTRASONIC_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_PIN, LOW);
  pinMode(ULTRASONIC_PIN, INPUT);
  return pulseIn(ULTRASONIC_PIN, HIGH);
}

void setup() {
  Serial.begin(9600);
  pinMode(redled, OUTPUT);
  pinMode(greenled, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  myservo.attach(9);
  lcd.begin(16, 2);
  displayArmedStatus();
}

void loop() {
  if (currentposition == 0) {
    displayArmedStatus();
  }
  
  char code = keypad.getKey();
  if (code != NO_KEY) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PASSWORD:");
    lcd.setCursor(7, 1);
    lcd.print(" ");
    lcd.setCursor(7, 1);
    for (int l = 0; l <= currentposition; ++l) {
      lcd.print("*");
      keypress();
    }

    if (code == password[currentposition]) {
      ++currentposition;
      if (currentposition == 4) {
        toggleArmStatus();
        currentposition = 0;
      }
    } else {
      ++invalidcount;
      incorrect();
      currentposition = 0;
    }
    
    if (invalidcount == 3) {
      torture1();
    }
    if (invalidcount == 4) {
      torture2();
    }
  }
  
  if (isArmed) {
    long duration = readUltrasonicDistance();
    int distance = duration * 0.034 / 2;
    int pirState = digitalRead(PIR_PIN);
    
    if ((distance > 0 && distance < 50) || pirState == HIGH) {
      triggerAlarm(distance, pirState);
    }
  }
}

void displayArmedStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (isArmed) {
    lcd.print("SYSTEM ARMED");
    digitalWrite(redled, HIGH);
    digitalWrite(greenled, LOW);
  } else {
    lcd.print("SYSTEM DISARMED");
    digitalWrite(redled, LOW);
    digitalWrite(greenled, HIGH);
  }
  lcd.setCursor(0, 1);
  lcd.print("Enter code:");
}

void toggleArmStatus() {
  isArmed = !isArmed;
  if (isArmed) {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("SYSTEM ARMED");
    digitalWrite(redled, HIGH);
    digitalWrite(greenled, LOW);
  } else {
    unlockdoor();
  }
  delay(2000);
  displayArmedStatus();
}

void triggerAlarm(int distance, int pirState) {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (distance > 0 && distance < 50) {
    lcd.print("Object detected!");
    lcd.setCursor(0, 1);
    lcd.print("Distance: ");
    lcd.print(distance);
    lcd.print(" cm");
  } else {
    lcd.print("Motion detected!");
  }
  digitalWrite(redled, HIGH);
  digitalWrite(buzz, HIGH);
  delay(1000);
  digitalWrite(redled, LOW);
  digitalWrite(buzz, LOW);
  displayArmedStatus();
}

void unlockdoor() {
  lcd.setCursor(0, 0);
  lcd.println(" ");
  lcd.setCursor(1, 0);
  lcd.print("Access Granted");
  lcd.setCursor(4, 1);
  lcd.println("WELCOME!!");
  unlockbuzz();

  for (pos = 180; pos >= 0; pos -= 5) {
    myservo.write(pos);
    delay(5);
  }
  delay(2000);
  
  counterbeep();
  
  for (pos = 0; pos <= 180; pos += 5) {
    myservo.write(pos);
    delay(15);
  }
}

void incorrect() {
  delay(500);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("CODE");
  lcd.setCursor(6, 0);
  lcd.print("INCORRECT");
  lcd.setCursor(4, 1);
  lcd.println("GET AWAY!!!");
  Serial.println("CODE INCORRECT YOU ARE UNAUTHORIZED");
  digitalWrite(redled, HIGH);
  digitalWrite(buzz, HIGH);
  delay(3000);
  lcd.clear();
  digitalWrite(redled, LOW);
  digitalWrite(buzz, LOW);
  displayArmedStatus();
}

void keypress() {
  digitalWrite(buzz, HIGH);
  delay(50);
  digitalWrite(buzz, LOW);
}

void unlockbuzz() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(buzz, HIGH);
    delay(80);
    digitalWrite(buzz, LOW);
    delay(80);
  }
}

void counterbeep() {
  for (int i = 5; i > 0; i--) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.println("GET IN WITHIN:");
    lcd.setCursor(4, 1);
    lcd.print(i);
    digitalWrite(buzz, HIGH);
    delay(100);
    digitalWrite(buzz, LOW);
    delay(900);
  }
  
  for (int i = 0; i < 4; i++) {
    digitalWrite(buzz, HIGH);
    delay(40);
    digitalWrite(buzz, LOW);
    delay(40);
  }
  
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("RE-LOCKING");
  for (int i = 0; i < 3; i++) {
    delay(500);
    lcd.setCursor(12 + i, 0);
    lcd.print(".");
  }
  delay(400);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("LOCKED!");
  delay(440);
}

void torture1() {
  delay(1000);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("WAIT FOR ");
  lcd.setCursor(5, 1);
  lcd.print("15 SECONDS");
  digitalWrite(buzz, HIGH);
  delay(15000);
  digitalWrite(buzz, LOW);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("LOL..");
  lcd.setCursor(1, 1);
  lcd.print(" HOW WAS THAT??");
  delay(3500);
  lcd.clear();
}

void torture2() {
  delay(1000);
  lcd.setCursor(2, 0);
  lcd.print("EAR DRUMS ARE");
  lcd.setCursor(0, 1);
  lcd.print(" PRECIOUS!! ");
  delay(1500);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(" WAIT FOR");
  lcd.setCursor(4, 1);
  lcd.print(" 1 MINUTE");
  digitalWrite(buzz, HIGH);
  delay(55000);
  counterbeep();
  lcd.clear();
  digitalWrite(buzz, LOW);
  lcd.setCursor(2, 0);
  lcd.print("WANT ME TO");
  lcd.setCursor(1, 1);
  lcd.print("REDICULE MORE??");
  delay(2500);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Ha Ha Ha Ha");
  delay(1700);
  lcd.clear();
}
