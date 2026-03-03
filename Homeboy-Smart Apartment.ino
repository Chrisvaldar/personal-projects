/*
	MCD1160 
    Name: G02
    ID: 35438150
    Teacher: Mehran Vahid
    
    Description:
    
      Team Circuit Design and Code for Assignment 2
      Last edited: 14/1/2025 (Jason) 
      (Added DHT11 Sensor)
*/

#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <DHT.h>

#define BEDROOM_LED 6
#define BATHROOM_LED 13
#define LIVING_ROOM_LED 3
#define FRONT_SERVO_PIN 9
#define IN2 5
#define IN1 7
#define EN 11
#define ECHO 12     
#define TRIGGER 8
#define PIEZOSPEAKER 2
#define DHT11_PIN A3
#define KITCHEN_SERVO_PIN 10

#define WINDOW_MAX_TIME 1800

DHT dht11(DHT11_PIN, DHT11);

LiquidCrystal_I2C lcd(0x20,16,2);
Servo frontServo;
Servo kitchenServo;

//Temperature calculation variables
float celsius = 0;
float fahrenheit = 0;

//Variables for password system
char password[8] = {'A', 'P', 'A', 'T', 'E', 'U'};
int currentPassLength = 6;
bool isPassCorrect = false;
int passAttempts = 0;
bool isProgramLocked = false;

//Tracks whether or not the menu should be printed after the alarm goes off
bool printMenuAfterAlarm = false;
//Tracks the window current position, fully open on startup
int windowCurrentPosition = 0; 
     
//Tracks what the current temperature unit is
bool isCelsius = true;

//Tracks whether or not the alarm is on or not
bool alarmOn = false;

//Variable for the ultrasonic sensor
long duration = 0;

//Distance variables for alarm
int currentDistance = 0;
int previousDistance = 0;

//Tracks whether or not there is an intruder or not
bool isAlarmed = false;

//Tracks when the alarm was turned on
long alarmOnTime = 0;
//Tracks whether or not the user is currently changing the password
bool isChangingPass = false;

void setup()
{
  pinMode(BEDROOM_LED, OUTPUT);
  pinMode(LIVING_ROOM_LED, OUTPUT);
  pinMode(BATHROOM_LED, OUTPUT);
  pinMode(ECHO, INPUT);       
  pinMode(TRIGGER, OUTPUT);
  pinMode(PIEZOSPEAKER, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(DHT11_PIN, INPUT);
  
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  dht11.begin();
  
  frontServo.attach(FRONT_SERVO_PIN);
  frontServo.write(0);

  kitchenServo.attach(KITCHEN_SERVO_PIN);
  kitchenServo.write(0);
}

void loop()
{    
  passAttempts = 0;
  //While the user has not entered a correct password and program isn't locked
  while(!isPassCorrect && !isProgramLocked)
  {
    passwordCheck();
    delay(100);
  }
  
  if(!isProgramLocked)
  {
    //Display options menu to user 
    menuDisplay();
   
    //While there is no input
    while(Serial.available() == 0)
    {
      if(printMenuAfterAlarm)
      {
        //Display options menu to user 
        menuDisplay();
        printMenuAfterAlarm = false;
      }
    
      //Calculate temperatures
      float celsius = dht11.readTemperature();
      fahrenheit = (9.0 / 5.0 * celsius) + 32.0; 
    
      //Prints current temperature on LCD with appropriate unit
      lcd.clear();
      lcd.print("Temp: ");
      lcd.setCursor(0,1);
      if(isCelsius)
      {
        lcd.print(celsius);
        lcd.print(" C");
      }
      else
      {
        lcd.print(fahrenheit);
        lcd.print(" F");
      }
      delay(200);
      
      //If the alarm is on, constantly check for movement
      if(alarmOn && (millis() - alarmOnTime) > 30000)
      {
        currentDistance = distanceUsingDuration(duration);
  
        int distanceDiff = currentDistance - previousDistance;

        if(distanceDiff < 0)
        {
          distanceDiff *= -1;
        }
        
        if(distanceDiff > 200 && previousDistance != 0)
        { 
          isAlarmed = true;
        }
        previousDistance = currentDistance;
      }
      
      //If the alarm detects movement, ring buzzer and print on LCD
      if(isAlarmed && alarmOn)
      {
        isPassCorrect = false;
        while(isAlarmed && !isPassCorrect && !isProgramLocked)
        {
          tone(PIEZOSPEAKER, 1000);
          passwordCheck();
          if(isPassCorrect)
          {
            printMenuAfterAlarm = true;
            noTone(PIEZOSPEAKER);
          }
          else if(isProgramLocked)
          {
            while(isProgramLocked)
            {
              
            }
          }
        }
      }
    }
    
    char menuChoice = Serial.read();
  
    //If user chooses lights
    if(menuChoice == 'A')
    {
      //Display menu of rooms to user
      Serial.println("Options: ");
      Serial.println("A. Bedroom");
      Serial.println("B. Bathroom");
      Serial.println("C. Living Room");
      lcd.clear();
      lcd.print("Select room:");
    
      char lightsChoice = '.';
      lightsChoice = waitForInput();
      
      //Show options for bedroom lights and execute accordingly
      if(lightsChoice == 'A')
      {
        analogLightsMenuDisplay();
    
        char bedroomChoice = waitForInput();
        
        controlAnalogLight(bedroomChoice, 'B', BEDROOM_LED);
      }
    
      //Show options for bathroom and execute accordingly
      else if(lightsChoice == 'B')
      {
        onOrOffDisplay();
    
        char bathroomChoice = waitForInput();
        
        controlDigitalLight(bathroomChoice, BATHROOM_LED);
      }
  
      //Show options for living room and execute accordingly
      else if(lightsChoice == 'C')
      {
        analogLightsMenuDisplay();
    
        char livingRoomChoice = waitForInput();
        
        controlAnalogLight(livingRoomChoice, 'L', LIVING_ROOM_LED);
      }
      else
      {
        invalidInput();
      }
    }
  
    //If user chooses door
    else if(menuChoice == 'B')
    {
      lcd.clear();
      lcd.print("Select door:");
      Serial.println("Options: ");
      Serial.println("A. Front Door");
      Serial.println("B. Kitchen Door");

      char doorChoice = waitForInput();
      char lockOrUnlock = '.';
   
      //User chooses front door or kitchen door
      if(doorChoice == 'A')
      {
        lockOrUnlockDisplay();
      
        lockOrUnlock = waitForInput();
        
        doorExecute(lockOrUnlock, 'F');
      }
      else if(doorChoice == 'B')
      {
        lockOrUnlockDisplay();
      
        lockOrUnlock = waitForInput();
        
        doorExecute(lockOrUnlock, 'K');
      }
      else 
      {
        invalidInput();
      }
    }

    //If user chooses window
    else if(menuChoice == 'C')
    {
      lcd.clear();
      lcd.print("Set level: ");
    
      Serial.println("Set level: ");
      Serial.println("A. Fully OPEN (0%)");
      Serial.println("B. Slightly OPEN (25%)");
      Serial.println("C. Half OPENED (50%)");
      Serial.println("D. Slightly OPEN (75%)");
      Serial.println("E. Fully CLOSED (100%)");


      char windowInput = waitForInput();
     
      //Move window according to user input
      windowExecute(windowInput);
    }

    //If user chooses alarm
    else if(menuChoice == 'D')
    {
      onOrOffDisplay();
     
      char alarmChoice = waitForInput();
      
      //Turn alarm ON or OFF
      alarmOnOrOff(alarmChoice);
    }
  
    //If user chooses to swap temperature units
    else if(menuChoice == 'E')
    {
      //Swap the temperature units
      changeTempUnit();
    }
    else if(menuChoice == 'F')
    {
      changePassword();
    }
    else
    {
      invalidInput();
    }
  }
  while(Serial.available() > 0)
  {
    Serial.read();
  }
}

//Displays main menu
void menuDisplay()
{
  Serial.println("Menu: ");      
  Serial.println("A. Lights");
  Serial.println("B. Doors");
  Serial.println("C. Window");
  Serial.println("D. Alarm");
  Serial.println("E. Swap Temp Units");
  Serial.println("F. Change Password");
}

//Displays invalid message on LCD and Serial Monitor
void invalidInput()
{
  lcd.clear();
  lcd.print("Invalid input");
  Serial.println("Invalid input");
  delay(1000);
}

//Checks for correct password
void passwordCheck()
{
  //Prompt user for password attempt
  lcd.clear();
  if(isAlarmed)
  {
    lcd.print("INTRUDER!");
    lcd.setCursor(0,1);
    
    Serial.println("INTRUDER!");
  }
  
  //If changing password, print "Current password: " instead
  if(isChangingPass)
  {
    lcd.print("Current ");
    Serial.print("Current ");
    lcd.setCursor(0,1);
  }
  else
  {
    lcd.print("Enter ");
    Serial.print("Enter ");
  }
  lcd.print("Password: ");
  Serial.println("Password: ");
  
  while(Serial.available() == 0)
  {
          
  }
  char userPassAttempt[8];
  int j = 0;
    
  //Copy each character from user attempt into array
  while(Serial.available() > 0 && j < 8)
  {
    char inputChar = Serial.read();
    
    // Only accept printable characters
    if (inputChar >= 32 && inputChar <= 126) 
    {
      userPassAttempt[j] = inputChar;
      j++;
    }
    delay(10);
  }
  
    
  //Clear anything over the maximum char limit
  while (Serial.available() > 0) 
  {
    Serial.read(); 
  }
  
  //Boolean variable will remain true unless passwords don't match  
  isPassCorrect = true; 
  
  if(j != currentPassLength)
  {
    isPassCorrect = false;
  }
  
  //Compare the password and the attempt
  for(int i = 0; i < j; i++)
  {
    if(userPassAttempt[i] != password[i])
    {
      isPassCorrect = false;
    }
  } 
 
  //Handles maximum attempts
  if(!isPassCorrect && passAttempts < 5)
  {
    passAttempts++;
    Serial.print("Incorrect, Attempts: ");
    Serial.println(5 - passAttempts);
            
    lcd.clear();
    lcd.print("Incorrect");
    lcd.setCursor(0,1);
    lcd.print("Password");
    delay(1000);
  }
  else
  {
    //If password attempts more than 5, lock program
    if(passAttempts >= 5)
    {
      Serial.println("Locking Program...");
      
      lcd.clear();
      lcd.print("Program");
      lcd.setCursor(0,1);
      lcd.print("Locked");
        
      isProgramLocked = true;
      delay(1000);
    }
    else if(!isProgramLocked)
    {
      noTone(PIEZOSPEAKER);
      if(!isChangingPass)
      {
        alarmOn = false;
        isAlarmed = false;
        
        Serial.println("Starting Program...");
      
        lcd.clear();
        lcd.print("Starting");
        lcd.setCursor(0,1);
        lcd.print("Program...");
        delay(1000);
      }
    }
  }
}

//Displays bedroom and living room brightness choices
void analogLightsMenuDisplay()
{
  lcd.clear();
  lcd.print("Set brightness: ");
 
  Serial.println("Set brightness: ");
  Serial.println("A. Off (0%)");
  Serial.println("B. Low (25%)");
  Serial.println("C. Medium (50%)");
  Serial.println("D. High (75%)");
  Serial.println("E. Max (100%)");
}

//Controls analog (bedroom and living room) lights based on choice
void controlAnalogLight(char choice, char room, int pin) 
{
  int brightness = 0;
  int state = 0;

  if (choice == 'A') 
  {
    brightness = 0;
    
    state = 0;
    
  } 
  else if (choice == 'B') 
  {
    brightness = 255 / 4;
   
    state = 25;
    
  } 
  else if (choice == 'C') 
  {
    brightness = 255 / 2;

    state = 50;

  } 
  else if (choice == 'D') 
  {
    brightness = 255 / 4 * 3;
   
    state = 75;
  } 
  else if (choice == 'E') 
  {
    brightness = 255;
    state = 100;
    
  }
  else
  {
    invalidInput();
    return;
  }
  

  // Control the light based on whether it's an analog or digital pin
  analogWrite(pin, brightness); 

  // Display the state on the LCD and Serial Monitor
  lcd.clear();
  if(room == 'B')
  {
    lcd.print("Bedroom");    
    Serial.print("Bedroom");

  }
  else if(room == 'L')
  {
    lcd.print("Living Room");    
    Serial.print("Living Room");

  }
  lcd.print(" set to: ");
  lcd.setCursor(0, 1);
  lcd.print(state);
  lcd.print('%');
  Serial.print(" set to: ");
  Serial.print(state);
  Serial.println('%');
 
  delay(1000);
}

//Dislays ON/OFF menu for the alarm or bathroom
void onOrOffDisplay()
{
  lcd.clear();
  lcd.print("ON/OFF: ");
  Serial.println("Option: ");
  Serial.println("A. ON");
  Serial.println("B. OFF");
}

//Controls digital lights (bathroom)
void controlDigitalLight(char choice, int pin) 
{
  lcd.clear();
  Serial.print("Bathroom ");
  lcd.print("Bathroom ");
  
  if(choice == 'A')
  {
    digitalWrite(pin, HIGH);
    lcd.print("ON");
    Serial.println("ON");
    delay(1000);
  }
  else if(choice == 'B')
  {
    digitalWrite(pin, LOW);
    lcd.print("OFF");
    Serial.println("OFF");
    delay(1000);
  }
  else
  {
    invalidInput();
  }
}

//Display Lock/Unlock menu for both of the doors
void lockOrUnlockDisplay()
{
  lcd.clear();
  lcd.print("Lock/Unlock");
  Serial.println("Options: ");
  Serial.println("A. Lock");
  Serial.println("B. Unlock");
}

//Lock or unlock door
void doorExecute(char choice, char door) 
{
  if (choice == 'A') 
  {
    if(door == 'F')
    {
      frontServo.write(125);
      Serial.println("Front Door Locked");
      lcd.clear();
      lcd.print("Front Door");
      lcd.setCursor(0,1);
      lcd.print("Locked");
      delay(1000);
    }
    else if(door == 'K')
    {
      kitchenServo.write(125);
      Serial.println("Kitchen Door Locked");
      lcd.clear();
      lcd.print("Kitchen Door");
      lcd.setCursor(0,1);
      lcd.print("Locked");
      delay(1000);
    }
  } 
    
  else if (choice == 'B') 
  {
    if(door == 'F')
    {
      frontServo.write(0);
      Serial.println("Front Door Unlocked");
      lcd.clear();
      lcd.print("Front Door");
      lcd.setCursor(0,1);
      lcd.print("Unlocked");
      delay(1000);
    }
    else if(door == 'K')
    {
      kitchenServo.write(0);
      Serial.println("Kitchen Door Unlocked");
      lcd.clear();
      lcd.print("Kitchen Door");
      lcd.setCursor(0,1);
      lcd.print("Unlocked");
      delay(1000);
    }
  } 
  else 
  {
    invalidInput();
  }
}

void FWD_MOTOR() 
{
  //Moves motor forward
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH); 
  analogWrite(EN, 150);
}


void REV_MOTOR() 
{
  //Moves motor in reverse
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW); 
  analogWrite(EN, 150);
}

void BRAKE_MOTOR()
{
  //Brakes motor to prevent coasting
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  analogWrite(EN, 255);
}

//Moves window and keeps track of how much to move
void windowExecute(char choice)
{
  int windowTargetPosition = 0;
  float positionChange = 0;
  float moveTime = 0;

  //Store target position in variable
  if(choice == 'A')
  {
    windowTargetPosition = 100;
  }
  else if(choice == 'B')
  {
    windowTargetPosition = 75;
  }
  else if(choice == 'C')
  {
    windowTargetPosition = 50;
  }
  else if(choice == 'D')
  {
    windowTargetPosition = 25;
  }
  else if(choice == 'E')
  {
    windowTargetPosition = 0;
  }
  //If user doesn't comply, exit to default display
  else
  {
    invalidInput();
    return; 
  }

  //Find how far the window has to move and make sure it's positive
  positionChange = windowTargetPosition - windowCurrentPosition;
  if(positionChange < 0)
  {
    positionChange *= -1;
  }

  /*Calculate move time based on how far it takes to go from 0-100 
    and vice versa*/
  moveTime = positionChange / 100;
  moveTime = moveTime * WINDOW_MAX_TIME;

  /*If target is larger than current position, 
    roll down the window*/
  if(windowTargetPosition > windowCurrentPosition)
  {
    FWD_MOTOR();
    delay(moveTime);
  }
  /*Else if target is lower than current position, 
    roll up the window*/
  else if(windowTargetPosition < windowCurrentPosition)
  {
    REV_MOTOR();
    delay(moveTime);
  }
    
  //Stop the motor
  BRAKE_MOTOR();
  //Update current position
  windowCurrentPosition = windowTargetPosition;   

  lcd.clear();
  lcd.print("Window at: ");
  lcd.setCursor(0, 1);
  lcd.print(windowCurrentPosition);
  lcd.print("% opened");
    
  Serial.print("Window at: ");
  Serial.print(windowCurrentPosition);
  Serial.println("% opened");
    
  delay(1000);
}

//Gets distance using ultrasonic sensor
int distanceUsingDuration(long duration)
{
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(5);
  
  digitalWrite(TRIGGER, LOW);
  
  duration = pulseIn(ECHO, HIGH);
  int speedOfSound = 343;
  long distance = speedOfSound * duration / 2;
  distance = distance / 1000;
  return distance;
}

//Turn alarm on or off 
void alarmOnOrOff(char choice)
{
  lcd.clear();
  lcd.print("Alarm ");
  
  Serial.print("Alarm ");
  //Turn alarm on or off based on user input
  if(choice == 'A')
  {
    alarmOn = true;
    lcd.print("ON");
    
    Serial.println("ON");

    previousDistance = 0;
    alarmOnTime = millis();
    delay(1000);
  }
  else if(choice == 'B')
  {
    alarmOn = false;
    isAlarmed = false;
    digitalWrite(PIEZOSPEAKER, LOW);
      
    lcd.print("OFF");
    
    Serial.println("OFF");
    delay(1000);
  }
  else
  {
    invalidInput();
  }
}

//Swap temperature unit
void changeTempUnit()
{
  //If it's currently celsius, change to fahrenheit    
  lcd.clear();
  lcd.print("Unit: ");
  lcd.setCursor(0,1);

  Serial.print("Unit: ");
  if(isCelsius)
  {
    isCelsius = false;
    lcd.print("Fahrenheit");
    
    Serial.println("Fahrenheit");
  }
  //If it's not celsius (fahrenheit), change to celsius
  else
  {
    isCelsius = true;
    lcd.print("Celsius");
    
    Serial.println("Celsius");
  }      
  delay(1000);

}

char waitForInput()
{
  char choice;
  while (Serial.available() > 0) 
  {
    Serial.read(); 
  }
  while(Serial.available() == 0)
  {

  }
  
  choice = Serial.read();
  
  return choice;
}

//Asks for new password and updates the current one
void changePassword()
{
  isChangingPass = true;
  isPassCorrect = false;
  passAttempts = 0;
  
  while(Serial.available() > 0)
  {
    Serial.read();
  }
  //User must enter the current password to change it
  while(!isPassCorrect)
  {
    passwordCheck();  
    if(isProgramLocked)
    {
      while(isProgramLocked)
      {
        
      }
    }
  }
  
  lcd.clear();
  lcd.print("Enter new");
  lcd.setCursor(0,1);
  lcd.print("password:");
  
  Serial.println("Enter new password:");
  while(Serial.available() == 0)
  {
    
  }
  
  char newPassword[8];
  int j = 0;
    
  while(Serial.available() > 0)
  {
    if(j < 8)
    {
      char inputChar = Serial.read();
    
      // Only accept printable characters
      if (inputChar >= 32 && inputChar <= 126) 
      {
        newPassword[j] = inputChar;
        j++;
      }
      delay(10);
    }
    else
    {
      while (Serial.available() > 0) 
      {
        Serial.read(); 
      }
      lcd.clear();
      lcd.print("Max: 8 char");
      Serial.println("Max 8 chars. Not changed.");
      delay(1000);
      return;
    }
    delay(10);
  }
    
  currentPassLength = j;
  for(int i = 0; i < j; i++)
  {
    password[i] = newPassword[i];
  } 
  
  lcd.clear();
  lcd.print("Password changed");
  Serial.println("Password changed");
  isChangingPass = false;
  
delay(1000);
}