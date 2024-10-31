#include <LiquidCrystal_I2C.h>
#include <HUSKYLENS.h>
#include <SoftwareSerial.h>

HUSKYLENS huskylens;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int Relay = 13;

const int speakerPin = 10;

const int tones = 32;

const int ROWS = 4; // 행의 수
const int COLS = 4; // 열의 수

const int numTones = 8;

int check = false;

String inputPassword = "";

String correctPassword = "1234";

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

void printResult(HUSKYLENSResult result);
bool facedetected(int ID);
void lcdInitPassword();
void passwordCorrect();
void passwordWrong();
void lcdInitHusky();

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  pinMode(Relay, OUTPUT);

  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
  }

  for (int j = 0; j < COLS; j++) {
    pinMode(colPins[j], INPUT_PULLUP);
  }

  while (!huskylens.begin(Wire)) {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }

  lcd.setCursor(0,0);

  lcd.print("trun on");

  delay(500);

  lcdInitHusky();
}

void loop() {
  if(check) {
    for (int i = 0; i < ROWS; i++) {
      digitalWrite(rowPins[i], HIGH); // 현재 행 선택
      for (int j = 0; j < COLS; j++) {
        if (digitalRead(colPins[j]) == HIGH) { // 버튼이 눌렸다면
          Serial.print("Pressed: ");
          Serial.println(keys[i][j]);

          if(keys[i][j] == '*') {
            lcdInitPassword();
            
            if(correctPassword == inputPassword) {
              passwordCorrect();
              
              digitalWrite(Relay,HIGH);
              delay(3000);
              digitalWrite(Relay,LOW);
              delay(100);
              digitalWrite(Relay,HIGH);
              delay(100);
              digitalWrite(Relay,LOW);

              check = false;
            } else {
              passwordWrong();
              delay(1000);

              check = false; 
            }

            inputPassword = "";

          } else {
            inputPassword += keys[i][j];

            lcd.setCursor(0,1);

            lcd.print(inputPassword);
          }
          delay(300); // 디바운스 시간
        }
      }
      digitalWrite(rowPins[i], LOW); // 행 선택 해제
    }
  } else {
    if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    else if(!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    else if(!huskylens.available()) {
      Serial.println(F("No block or arrow appears on the screen!"));

      lcd.setCursor(0,0);

      lcdInitHusky();
    }
    else {
      Serial.println(F("--------------------------------------------------"));
      while (huskylens.available())
      {
        HUSKYLENSResult result = huskylens.read();
        printResult(result);
      }
    }
    delay(1000);
  }
}


void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK){
    // Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    
    if(facedetected(result.ID)) {
      check = true;
      lcdInitPassword();
    }

  } else {
    Serial.println("Object unknown!");
  }
}

bool facedetected(int ID) {
  if(ID >= 1) {
    Serial.println("you are right person");
    lcd.clear();

    lcd.setCursor(0,0);
    
    lcd.print("Correct!");

    return true;
  } else {
    Serial.println("who are you?");
    lcd.clear();

    lcd.setCursor(0,0);

    lcd.print("inCorrect!");

    return false;
  }
}

void lcdInitPassword() {
  lcd.clear();

  lcd.print("enter password");

  lcd.setCursor(0, 1);
}

void lcdInitHusky() {
  lcd.clear();

  lcd.print("face recognition");

  lcd.setCursor(0, 1);

  lcd.print("close to husky");
}

void passwordCorrect() {
  lcd.clear();

  lcd.setCursor(0,0);

  lcd.print("correct!!");

  Serial.println("enter the door");

  tone(speakerPin, 261, 500);
}

void passwordWrong() {
  lcd.clear();

  lcd.setCursor(0,0);

  lcd.print("Wrong...");

  Serial.println("enter the door");

  tone(speakerPin, 65, 500);
}
