#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <Keypad.h>
#include <DFPlayerMini_Fast.h>

#define ROWS 4
#define COLS 4
#define PASSWORD_LENGTH 4
#define KEY_BEEP_SOUND 5
#define ARMED_SOUND 3
#define DISARMED_SOUND 2
#define BOMB_BEEP_SOUND 1
#define EXPLOSION 7
#define SECRET_SOUND 10

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {6, 7, 8, 9};
byte colPins[COLS] = {2, 3, 4, 5};

// Оборудование
LiquidCrystal_PCF8574 lcd(0x27);
Keypad keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
DFPlayerMini_Fast player;

// Состояние
char armPassword[PASSWORD_LENGTH] = "2015";
char disarmPassword[PASSWORD_LENGTH] = "1983";
char secretPassword[PASSWORD_LENGTH] = "2008";
bool isArmed = false;
byte passwordLenght = 0;
char password[PASSWORD_LENGTH];
long nextLightTime = 0;
long beepWidth = 0;
bool lightPhase = false;

void resetPassword()
{
 passwordLenght = 0;
 lcd.setCursor(0, 1);
 lcd.print("          ");
 lcd.setCursor(0, 1);
 for (int i=0; i<PASSWORD_LENGTH; i++)
  password[i] = 0;
}

void waitForSound()
{
  while (player.isPlaying())
  {
    delay(500);
  }
}

// Настройка
void setup()
{
  pinMode(A3, OUTPUT);
  digitalWrite(A3, LOW);  
  lcd.begin(16, 2);  // initialize the lcd
  lcd.setBacklight(255);
  lcd.print("D I M A  C 4");  
  Serial.begin(9600);  
  player.begin(Serial);  
  player.volume(80);  
}

void loop()
{
 // Теуущее время
 long curTime = millis();
   
 byte key = keypad.getKey();
 if (key)
 {
  // Сброс
  if (key == '#')
  {
   resetPassword();
  }
  // Кнопка
  else if (passwordLenght < PASSWORD_LENGTH)
  {
   password[passwordLenght] = key;    
   lcd.setCursor(passwordLenght * 2, 1); 
   lcd.print(password[passwordLenght]); 
   lcd.print(' '); 
   passwordLenght++; 
   player.playFromMP3Folder(KEY_BEEP_SOUND);   
    
   // Дергаем лампочку если не взведено
   if (!isArmed)
   {    
    digitalWrite(A3, HIGH);
    delay(200);
    digitalWrite(A3, LOW);
   }
    
   if (!isArmed)
   {
    if (strncmp(password, armPassword, PASSWORD_LENGTH) == 0)
    {
     waitForSound();
     player.playFromMP3Folder(ARMED_SOUND);
     resetPassword();
     lightPhase = true;
     isArmed = true;
     beepWidth = 3000;
     nextLightTime = curTime;
    }
    if (strncmp(password, secretPassword, PASSWORD_LENGTH) == 0)
    {
     player.playFromMP3Folder(SECRET_SOUND);
     resetPassword();      
    }
   }
   else
   {
    if (strncmp(password, disarmPassword, PASSWORD_LENGTH) == 0)
    {
     digitalWrite(A3, LOW);
     resetPassword();
     waitForSound();
     player.playFromMP3Folder(DISARMED_SOUND);     
     isArmed = false;      
    }
   }
  }
 }

 // Моргаем лампочкой
 if (isArmed)
 {
  if (nextLightTime <= curTime)
  {
    if (lightPhase)
    {
     player.playFromMP3Folder(BOMB_BEEP_SOUND);
     digitalWrite(A3, HIGH);
     nextLightTime = curTime + 200;
    }
    else
    {
      digitalWrite(A3, LOW);
      beepWidth -= 80;
      nextLightTime = curTime + beepWidth;
    }
    lightPhase = !lightPhase;
  }

  // Если взрыв
  if (beepWidth <= 0)
  {
    player.playFromMP3Folder(EXPLOSION);
    isArmed = false;
    resetPassword();
  }
 }
}
