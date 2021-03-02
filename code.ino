#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include<EEPROM.h>
#include <TM1637.h>
#include<Servo.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

char password[4];
char initial_password[4],new_password[4];
int i=0;

char key_pressed=0;
const byte rows = 4; 
const byte columns = 4; 
char hexaKeys[rows][columns] = {

{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}

};

byte row_pins[rows] = {2,3,4,5};
byte column_pins[columns] = {6,7,8,9};   
Keypad keypad_key = Keypad( makeKeymap(hexaKeys), row_pins, column_pins, rows, columns);


int selectButton = 14;
int leftButton = 12;
int rightButton = 13;
int selectButtonState, leftButtonState, rightButtonState; 
char modes[5][16] = {"Documents", "Fruits & Veg", "Groceries", "Electronics", "Others"};
int index = 0;

int CLK = 10;
int DIO = 11;
TM1637 tm1637(CLK,DIO);

int Pulse = 17;
int timer_val; // Countdown value in minutes 
int timer_seconds; // Display seconds 
// Variables used to store individual numbers 
int firstnum=0; 
int secondnum=0; 
int thirdnum=0; 
int fournum=0; 

int processComplete;
char lock = 0;

//Servo upServo
Servo downServo;

int upRelay = 16;
//int downRelay = 0;

void setup()
{
  Serial.begin(9600);
  lcd.init(); 
  lcd.init(); 
  lcd.backlight();
  pinMode(selectButton,INPUT);
  pinMode(leftButton,INPUT);
  pinMode(rightButton,INPUT);
  pinMode(Pulse, OUTPUT);
    
  //upServo.attach(A2);
  downServo.attach(A1);
  //upServo.write(0);
  downServo.write(0);

  pinMode(upRelay,OUTPUT);
  //pinMode(downRelay,OUTPUT);
  digitalWrite(upRelay,HIGH);
  //digitalWrite(downRelay,LOW);
  
    
  digitalWrite(Pulse, LOW); 
  
  Serial.println("Reached Here");
  tm1637.init(); // Display Reset 
  tm1637.set(BRIGHT_TYPICAL); // Brightness Level 
  // BRIGHT_DARKEST = 0,BRIGHTEST = 7 BRIGHT_TYPICAL = 2;
  tm1637.point(POINT_ON); // Centre "colons" ON 
  delay(1000); // Delay of 1 second 

  
  lcd.setCursor(0,0);
  lcd.print("Hello");
  lcd.setCursor(0,1);
  lcd.print("I am UVSANITIZE");
  delay(2000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Enter Passowrd:");
  lcd.setCursor(0,1);
  initialpassword();



}



void loop()
{
  selectButtonState=leftButtonState=rightButtonState=index=0; 
  lock=0;
  
  key_pressed = keypad_key.getKey();
  if(key_pressed=='#')
    change();

  if (key_pressed)
  {
    password[i++]=key_pressed;
    lcd.print(key_pressed);
    Serial.println(key_pressed);

   }

  if(i==4)
  {
    delay(200);
    for(int j=0;j<4;j++)
      initial_password[j]=EEPROM.read(j);
    if(!(strncmp(password, initial_password,4)))
    {
      lcd.clear();
      lcd.print("Pass Accepted");
      preset();
      delay(2000);

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Press * to Lock");

      while(lock != '*')
      {
        lock = keypad_key.getKey();
      }
      if(lock=="*");
      {
      //upServo.write(90);
      downServo.write(90);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Door Locked");
      lcd.setCursor(0,1);
      lcd.print("Confirm to Begin!");
      }

       while(selectButtonState!=1)
       {
        selectButtonState=digitalRead(selectButton);
       }

     digitalWrite(upRelay,LOW);
     //digitalWrite(downRelay,HIGH);      

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Sanitization in");
      lcd.setCursor(0,1);
      lcd.print("Progress!");
      
      calcModeTime(index);

      processComplete=0;
      while(processComplete!=1)
      {
        countdown();
      }

      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Sanitization is");
      lcd.setCursor(0,1);
      lcd.print("Complete!");

      digitalWrite(upRelay,HIGH);
      //digitalWrite(downRelay,LOW);
      delay(2000);
      //upServo.write(0);
      downServo.write(0);
      
      i=0;
    }

    else
    {
      //digitalWrite(relay_pin, LOW);
      lcd.clear();
      lcd.print("Wrong Password");
      lcd.setCursor(0,1);
      lcd.print("Pres # to Change");
      delay(2000);
      lcd.clear();
      lcd.print("Enter Password");
      lcd.setCursor(0,1);
      i=0;
    }

  }

}

void change()
{
  int j=0;
  lcd.clear();
  lcd.print("Current Password");
  lcd.setCursor(0,1);
  while(j<4)
  {
    char key=keypad_key.getKey();
    if(key)
    {
      new_password[j++]=key;
      lcd.print(key);
    }
    key=0;
  }

  delay(500);


  if((strncmp(new_password, initial_password, 4)))
  {
    lcd.clear();
    lcd.print("Wrong Password");
    lcd.setCursor(0,1);
    lcd.print("Try Again");
    delay(1000);
  }

  else
  {
    j=0;
    lcd.clear();
    lcd.print("New Password:");
    lcd.setCursor(0,1);

    while(j<4)
    {
      char key=keypad_key.getKey();
      if(key)
      {
        initial_password[j]=key;
        lcd.print(key);
        EEPROM.write(j,key);
        j++;
      }
    }

    lcd.print("Pass Changed");
    delay(1000);
  }

  lcd.clear();
  lcd.print("Enter Password");
  lcd.setCursor(0,1);
  key_pressed=0;
}




void initialpassword()
{
  for(int j=0;j<4;j++)
      EEPROM.write(j, j+49);

  for(int j=0;j<4;j++)
      initial_password[j]=EEPROM.read(j);

} 


void preset()
{
  lcdPreset(modes[index]);
  while(selectButtonState!=1)
  {
      //Serial.println(digitalRead(selectButton));
         leftButtonState = digitalRead(leftButton);
         while(digitalRead(leftButton)==1)
         {
          continue;
         }
         rightButtonState = digitalRead(rightButton);
         while(digitalRead(rightButton)==1)
         {
          continue;
         }
        
     
     //Serial.println("rightButtonState");
     //Serial.println(rightButtonState);
     if(leftButtonState==1 && index!=0)
      {
        index--;
        lcdPreset(index);
       }

      else if(rightButtonState==1 && index!=4)
      {
       index++;
        lcdPreset(index);
      }
  
      else
     {
 
     }  
      selectButtonState = digitalRead(selectButton);
  }

  selectButtonState = 0;
}

void lcdPreset(int pos)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Choose Preset:");
  lcd.setCursor(0,1);
  lcd.print(modes[pos]);
  //delay(200);
  
}

void calcModeTime(int pos)
{
  switch(pos)
  {
      case 0:
            timer_val = 0;
            timer_seconds = 30;
            break;

      case 1:
            timer_val = 0;
            timer_seconds = 45;            
            break;
            
      case 2:
            timer_val = 1;
            timer_seconds = 00;              
            break;
            
      case 3:
            timer_val = 1;
            timer_seconds = 30;
            break;

      case 4:
            timer_val = 2;
            timer_seconds = 0;

            break;
            
      default:
    
            break;
    
  }
}

void countdown() { 
   //digitalWrite(upRelay,HIGH);
   //digitalWrite(downRelay,HIGH);
   // Check if timer is elapsed 
   while (timer_val == 0 && timer_seconds == 0) { 
      tm1637.clearDisplay(); // Clear display 
      tm1637.display(0,0); 
      tm1637.display(1,0); 
      tm1637.display(2,0); 
      tm1637.display(3,0); 
      digitalWrite(Pulse, HIGH); // Alarm Trigger ON 
      delay(1000);  // Wait
      tm1637.clearDisplay(); 
      digitalWrite(Pulse, LOW); // Alarm Trigger OFF
      delay(500); // Wait
      processComplete=1;
      break; 
   } 
   // Breakdown minutes and seconds in individual numbers 
   if (timer_val > 9) { 
      firstnum = timer_val/10%10; 
      secondnum = timer_val%10; 
   } 
   else { 
      secondnum = timer_val; 
   } 
   if (timer_seconds > 9) { 
      thirdnum = timer_seconds/10%10; 
      fournum = timer_seconds%10; 
   } 
   else { 
      thirdnum = 0; 
      fournum = timer_seconds; 
   } 
   // Show countdown on 4 digit 7 segment display 
   tm1637.clearDisplay(); // Clear display 
   if (timer_val > 9) { 
      tm1637.display(0,firstnum); 
   } 
   if (timer_val > 0) { 
      tm1637.display(1,secondnum); 
   } 
   if (timer_seconds > 9 || timer_val > 0) { 
      tm1637.display(2,thirdnum); 
   } 
   tm1637.display(3,fournum); 
   // Decrease seconds 
   timer_seconds=timer_seconds-1; 
   delay(1000); // Delay of 1 second 
   // Decrease timer 
   if (timer_seconds == -1) { 
      timer_val=timer_val-1; 
      timer_seconds=59; 
   } 
}
