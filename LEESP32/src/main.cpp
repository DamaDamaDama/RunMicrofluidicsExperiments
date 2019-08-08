/*
*  Created by Alan Munirji
*  University of Pittsburgh, Computational and Systems Biology
*  5/17/2019 REC Lee Lab
*/
//#define _GLIBCXX_USE_CXX11_ABI 0
#include <iostream>
#include <string>
#include <stdlib.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>
#include <Wire.h> //For OLED
#include "SSD1306.h" //For OLED
#include <AccelStepper.h> //For practicing with stepper motors
#include "PumpFunctions.h"
#include "Adafruit_MCP23017.h" //For controlling IO expander
#include "sha256.h" //For hashing (validation)

////~~~Experiment associated declarations~~~
Adafruit_MCP23017 mcp;
//Define the solenoid controller pins
int Solenoid1 = 6;
int Solenoid2 = 5;
int Solenoid3 = 4;
int Solenoid4 = 3;
int Solenoid5 = 2;
int Solenoid6 = 1;
int Solenoid7 = 0;
//Define sleep control pin
int SLP = 14;
//Define the steppers and the pins each will use
//AccelStepper stepperName(1[defines stepper driver],pinStep,pinDirection)
AccelStepper stepper1(1, 15, 2); 
AccelStepper stepper2(1, 4, 16); 
AccelStepper stepper3(1, 17, 5); 
AccelStepper stepper4(1, 18, 19);
AccelStepper stepper5(1, 23, 13);
AccelStepper stepper6(1, 12, 27);
AccelStepper stepper7(1, 26, 25);
AccelStepper stepper8(1, 33, 32);

//Switch 1 (left): GPIO 35
//Switch 2 (right): GPIO 34
//Switch 3: GPIO 39
int buttonReset = 35;     //Reset button (Button 1)
int buttonResetState = 0; //Holds state of reset button (Button 1)
int buttonPause = 34;     //Pause button (Button 2)
int buttonPauseState = 0; //Holds state of pause button (Button 2)
int buttonLimit = 39;     //Limit button (Button 3, used for zeroing out the motors)
int buttonLimitState = 0;     //Holds state of limit button (Button 3)
int count = 0;
float microstep = .0550; //Going to set this to a much larger value in the hopes that heights more than 15 can be
                        //used. Originally was .00625 (Maybe try 0.0555 or 0.0575 next! 0.0550 gets ~5.1mm 
                        //consistently).
//Set Initial Global Motor speed and acceleration 
int motorAcceleration = 5000; // [step/second] (use values between 0 and 3000, 
//smaller number = smaller acceleration) Recommend 2000
int motorMaxSpeed = 10000; // [steps/second]
  //*Note speed will also depend on step sizes and some step sizes will exclude certain speeds

////~~~OLED declarations~~~
SSD1306 display(0x3c, 21, 22);
int line = 0;

////~~~All BLE associated declarations~~~
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

////~~~Declarations for data/byte manipulation~~~
int pos = 0;
int oldpos = 0;
unsigned long start = 0;
unsigned long curr = 0;
unsigned long stop = 0;
std::vector<std::string> shkrings; //Vector with many strings received via BLE
std::vector<std::vector<std::string>> motors; //Vector containing separate string data for each of 8 motors
std::vector<std::string> motor0;
std::vector<std::string> motor1;
std::vector<std::string> motor2;
std::vector<std::string> motor3;
std::vector<std::string> motor4;
std::vector<std::string> motor5;
std::vector<std::string> motor6;
std::vector<std::string> motor7;
std::vector<int> delays;
std::vector<std::string>::iterator i; //For iterating over our strings vector
std::string instruction(""); //What the client will eventually send to the server LEESP32
std::string recHash = ""; //The hash sent by bluetooth device, compared to the hash of instruction
std::string token; //To be used with parsing
std::string packet(""); //What is immediately sent to the server
std::string oldPacket(""); //Carries an old copy of packet, for comparison to detect changes

#define SERVICE_UUID        "9c45be4a-130b-404f-9c57-7d77f8ed33d1" //Advertises itself as this ID
#define CHARACTERISTIC_UUID "dee7d621-3fd3-42cb-9498-2042caf3d20f" //Has a custom characteristic of this ID

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);

  //BEGIN EXPERIMENT SETUP
  //Initialize the MCP23017 and give address of IO expander
  mcp.begin();    // use default address 0
  //Define the Input button switches. By default these all read to 0, unless the button is held down, they will read 1.
  pinMode(buttonReset, INPUT);
  pinMode(buttonPause, INPUT);
  pinMode(buttonLimit, INPUT);

  // sets the digital pins controlling solenoids as output
  mcp.pinMode(Solenoid1, OUTPUT);
  mcp.pinMode(Solenoid2, OUTPUT);
  mcp.pinMode(Solenoid3, OUTPUT);
  mcp.pinMode(Solenoid4, OUTPUT);
  mcp.pinMode(Solenoid5, OUTPUT);
  mcp.pinMode(Solenoid6, OUTPUT);
  mcp.pinMode(Solenoid7, OUTPUT);
  //set sleep pin as output
  pinMode(SLP, OUTPUT);
  //Make sure motors are awake ---------------
  digitalWrite(SLP, HIGH);
  //Set all Solenoids to off to begin
  mcp.digitalWrite(Solenoid1, LOW);
  mcp.digitalWrite(Solenoid2, LOW);
  mcp.digitalWrite(Solenoid3, LOW);
  mcp.digitalWrite(Solenoid4, LOW);
  mcp.digitalWrite(Solenoid5, LOW);
  mcp.digitalWrite(Solenoid6, LOW);
  mcp.digitalWrite(Solenoid7, LOW);
  //set Parameters for each stepper motor 
  stepper1.setMaxSpeed(motorMaxSpeed);
  stepper1.setAcceleration(motorAcceleration);
  stepper1.setCurrentPosition(0);
  stepper2.setMaxSpeed(motorMaxSpeed);
  stepper2.setAcceleration(motorAcceleration);
  stepper2.setCurrentPosition(0);
  stepper3.setMaxSpeed(motorMaxSpeed);
  stepper3.setAcceleration(motorAcceleration);
  stepper3.setCurrentPosition(0);
  stepper4.setMaxSpeed(motorMaxSpeed);
  stepper4.setAcceleration(motorAcceleration);
  stepper4.setCurrentPosition(0);
  stepper5.setMaxSpeed(motorMaxSpeed);
  stepper5.setAcceleration(motorAcceleration);
  stepper5.setCurrentPosition(0);
  stepper6.setMaxSpeed(motorMaxSpeed);
  stepper6.setAcceleration(motorAcceleration);
  stepper6.setCurrentPosition(0);
  stepper7.setMaxSpeed(motorMaxSpeed);
  stepper7.setAcceleration(motorAcceleration);
  stepper7.setCurrentPosition(0);
  stepper8.setMaxSpeed(motorMaxSpeed);
  stepper8.setAcceleration(motorAcceleration);
  stepper8.setCurrentPosition(0);
  delay(500);
  PumpFunc::wakeMotors();

  Serial.println("Experiment motor setup complete"); //tell the computer that the setup is complete (if connected via USB)
  PumpFunc::sleepMotors();
  //END EXPERIMENT SETUP

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); //Set to low at start

  //BEGIN OLED SETUP
  display.init();
  display.drawString(3, 3, "OLED Initialized");
  display.display();
  //END OLED SETUP
  
  // Create the BLE Device
  BLEDevice::init("LEESP32");
  BLEDevice::setMTU(515);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a client connection to notify...");
}

//Takes the shkrings vector containing all strings, sorts data using the number in front to know
//which height goes to which motor
int sortMotors(std::vector<std::string> aggregate){
  motors.clear();
  motor0.clear();
  motor1.clear();
  motor2.clear();
  motor3.clear();
  motor4.clear();
  motor5.clear();
  motor6.clear();
  motor7.clear();
  delays.clear();
  motors.push_back(motor0);
  motors.push_back(motor1);
  motors.push_back(motor2);
  motors.push_back(motor3);
  motors.push_back(motor4);
  motors.push_back(motor5);
  motors.push_back(motor6);
  motors.push_back(motor7);

  for(i = aggregate.begin(); i < aggregate.end(); ++i){
    switch(atoi((*i).substr(0, 1).c_str())){ //Take the first character of string, and convert to integer
      //For stupid reasons atoi returns 0... when input is NaN... which means if you actually 
      //have 0 as your first character... you can't differentiate between NaN and 0.  -__-
      case 1 :  motors[0].push_back((*i).substr(1, (*i).length())); //Add this value except the number
                 break;

      case 2 :  motors[1].push_back((*i).substr(1, (*i).length())); //Add this value except the number
                 break;

      case 3 :  motors[2].push_back((*i).substr(1, (*i).length())); //Add this value except the number
                 break;

      case 4 :  motors[3].push_back((*i).substr(1, (*i).length())); //Add this value except the number
                 break;

      case 5 :  motors[4].push_back((*i).substr(1, (*i).length())); //Add this value except the number
                 break;

      case 6 :  motors[5].push_back((*i).substr(1, (*i).length())); //Add this value except the number
                 break;

      case 7 :  motors[6].push_back((*i).substr(1, (*i).length())); //Add this value except the number
                 break;

      case 8 :  motors[7].push_back((*i).substr(1, (*i).length())); //Add this value except the number
                 break;

      case 9 :  delays.push_back(atoi((*i).substr(1,(*i).length()).c_str()));
                 break;
    }
  }

  for(int j = 0; j < 8; j++){
    Serial.print("Motor ");
    Serial.print(j);
    Serial.println("'s heights look like this: (if blank no data provided for this motor)");

    if(motors[j].size() > 0){ //Only if it has elements, loop through them
      for(i = motors[j].begin(); i < motors[j].end(); ++i){
        Serial.println((*i).c_str());
      }
    }
  }

  return 0;
}

//Verifies that received data is valid by comparing with a hash sent to it
bool isValid(std::string receivedData, std::string receivedHash){
  //receivedData is all the packets appended together EXCEPT the hash sent, with alterations 
  //other than them being joined back. This means delimiters like _ are left in. 
  //receivedHash is the hash sent from the client to the ESP32. They will be compared to
  //validate the data.
  //Test agg data: 1testdata_2testdata_3testdata_4testdata_5testdata_6fakedata_7fake_8fake_9substitutedata
  
  std::string dataHash = sha256(receivedData);
  if(dataHash.compare(receivedHash) != 0){
    Serial.println("Hash Error: the two hashes do NOT match.");
    Serial.print("The hash sent to the ESP32: ");
    Serial.println(receivedHash.c_str());
    Serial.print("The hash calculated by appending packets together:");
    Serial.println(dataHash.c_str());

    return false;
  }
  Serial.println("It is valid!");
  return true;
} //6e6bd72e8b01078024bf66c1a59af48e4cebb7e2af59949d7f71e14c94e0761a

int zeroMotors(){
  PumpFunc::wakeMotors();
  delay(500);

  while(digitalRead(buttonLimit) == 1){
    PumpFunc::moveBy(1, -3000); //Go down a step
  }
  stepper1.setCurrentPosition(0);
  PumpFunc::moveStepper(1, 5); //Take it 5mm off the limit switch. This will actually be 0.
  PumpFunc::stepAllMotors();
  stepper1.setCurrentPosition(0);
  while(digitalRead(buttonLimit) == 1){
    PumpFunc::moveBy(2, -3000); //Go down a step
  }
  stepper2.setCurrentPosition(0);
  PumpFunc::moveStepper(2, 5); //Take it 5mm off the limit switch. This will actually be 0.
  PumpFunc::stepAllMotors();
  stepper2.setCurrentPosition(0);
  while(digitalRead(buttonLimit) == 1){
    PumpFunc::moveBy(3, -3000); //Go down a step
  }
  stepper3.setCurrentPosition(0);
  PumpFunc::moveStepper(3, 5); //Take it 5mm off the limit switch. This will actually be 0.
  PumpFunc::stepAllMotors();
  stepper3.setCurrentPosition(0);
  while(digitalRead(buttonLimit) == 1){
    PumpFunc::moveBy(4, -3000); //Go down a step
  }
  stepper4.setCurrentPosition(0);
  PumpFunc::moveStepper(4, 5); //Take it 5mm off the limit switch. This will actually be 0.
  PumpFunc::stepAllMotors();
  stepper4.setCurrentPosition(0);
  while(digitalRead(buttonLimit) == 1){
    PumpFunc::moveBy(5, -3000); //Go down a step
  }
  stepper5.setCurrentPosition(0);
  PumpFunc::moveStepper(5, 5); //Take it 5mm off the limit switch. This will actually be 0.
  PumpFunc::stepAllMotors();
  stepper5.setCurrentPosition(0);
  while(digitalRead(buttonLimit) == 1){
    PumpFunc::moveBy(6, -3000); //Go down a step
  }
  stepper6.setCurrentPosition(0);
  PumpFunc::moveStepper(6, 5); //Take it 5mm off the limit switch. This will actually be 0.
  PumpFunc::stepAllMotors();
  stepper6.setCurrentPosition(0);
  while(digitalRead(buttonLimit) == 1){
    PumpFunc::moveBy(7, -3000); //Go down a step
  }
  stepper7.setCurrentPosition(0);
  PumpFunc::moveStepper(7, 5); //Take it 5mm off the limit switch. This will actually be 0.
  PumpFunc::stepAllMotors();
  stepper7.setCurrentPosition(0);
  while(digitalRead(buttonLimit) == 1){
    PumpFunc::moveBy(8, -3000); //Go down a step
  }
  stepper8.setCurrentPosition(0);
  PumpFunc::moveStepper(8, 5); //Take it 5mm off the limit switch. This will actually be 0.
  PumpFunc::stepAllMotors();
  stepper8.setCurrentPosition(0);
  return 0;
}

//Busy waits for press of button or specific bluetooth signal before proceding
float pauseExperiment(){
  std::string goPacket = "";
  long startOfPause = millis();
  display.resetDisplay();
  display.drawString(1, 3, "Tap \"Go\" or");
  display.drawString(1, 13, "Press button");
  display.drawString(1, 23, "to continue.");
  display.display();
  pCharacteristic->setValue(""); //Wipe it in case it was already set to something to prevent side effects
  buttonPauseState = 0;

  Serial.println("Waiting at pauseExperiment call...");
  while(buttonPauseState == 0 && goPacket.compare("go") != 0){ //While button has not been pressed again or Go has 
    goPacket = pCharacteristic->getValue();                   //not been pressed on app
    buttonPauseState = digitalRead(buttonPause);
  }
  return ((millis() - startOfPause) / 1000 ); //Returning the elapsed time spent pausing
}

//Executes the actual experiment. Takes stringvars, a vector containing vectors that each hold strings in them. The strings
//are data for pumps sorted chronologically.
//Returns: nothing
void runExp(std::vector<std::vector<std::string>> stringvars){
  Serial.println("Building variables from strings in vars vector...");
  int startDelay = 0;
  int currDelay = 0;
  std::vector<std::vector<int>> vars;
  std::vector<int> mot0;
  std::vector<int> mot1;
  std::vector<int> mot2;
  std::vector<int> mot3;
  std::vector<int> mot4;
  std::vector<int> mot5;
  std::vector<int> mot6;
  std::vector<int> mot7;
  vars.push_back(mot0);
  vars.push_back(mot1);
  vars.push_back(mot2);
  vars.push_back(mot3);
  vars.push_back(mot4);
  vars.push_back(mot5);
  vars.push_back(mot6);
  vars.push_back(mot7);

  for(int i = 0; i < 8; i++){ //Outer loop iterating through 8 possible motors
    if(stringvars[i].size() > 0){
      for(int k = 0; k < stringvars[i].size(); k++){
        vars[i].push_back(atoi(stringvars[i].at(k).c_str()));
      }
      Serial.print("Vars at motor");
      Serial.print(i);
      Serial.println(" looks like: ");
      for(int t = 0; t < vars[i].size(); t++){
        Serial.println(vars[i][t]);
      }
    }
  }

  Serial.println("Zeroing out motors before start...");
  zeroMotors();

  //pauseExperiment at the start
  pauseExperiment();
  start = millis()/1000;
  int counter = 0;
  display.resetDisplay();
  Serial.println("Executing experiment now...");

  //Wake up motors
  PumpFunc::wakeMotors();

  Serial.println("Delays size is: ");
  Serial.println(delays.size());

  Serial.print("mot0 size: ");
  Serial.println(vars[0].size());

  Serial.print("mot1 size: ");
  Serial.println(vars[1].size());

  while(counter < delays.size()){ //Since for one motor operation there is one delay, we can use delays to track exp status
    // Move Steppers Units are in mm (Max height = 80mm)
    if(vars[0].size() > counter) PumpFunc::moveStepper(1, vars[0][counter]); // PumpFunc::moveStepper(stepper#, Height(mm))
    if(vars[1].size() > counter) PumpFunc::moveStepper(2, vars[1][counter]); // PumpFunc::moveStepper(stepper#, Height(mm))
    if(vars[2].size() > counter) PumpFunc::moveStepper(3, vars[2][counter]); // PumpFunc::moveStepper(stepper#, Height(mm))
    if(vars[3].size() > counter) PumpFunc::moveStepper(4, vars[3][counter]); // PumpFunc::moveStepper(stepper#, Height(mm))
    if(vars[4].size() > counter) PumpFunc::moveStepper(5, vars[4][counter]); // PumpFunc::moveStepper(stepper#, Height(mm))
    if(vars[5].size() > counter) PumpFunc::moveStepper(6, vars[5][counter]); // PumpFunc::moveStepper(stepper#, Height(mm))
    if(vars[6].size() > counter) PumpFunc::moveStepper(7, vars[6][counter]); // PumpFunc::moveStepper(stepper#, Height(mm))
    if(vars[7].size() > counter) PumpFunc::moveStepper(8, vars[7][counter]); // PumpFunc::moveStepper(stepper#, Height(mm))

    //Step to that position
    PumpFunc::stepAllMotors();

    //delay(delays[counter++]); //Use appropriate delay and increment to next set of data, after using the delay
    
    //Delay loop. Delays until next round of instructions while also checking to see if pause or reset was sent
    startDelay = millis();
    currDelay = millis();
    while(1){
      currDelay = millis();
      if(pCharacteristic->getValue().compare("pause") == 0 || digitalRead(buttonPause) != 0){
        pauseExperiment();
        startDelay += (millis() - currDelay); //"resume" startDelay so it continues where it left off
        currDelay = millis(); //"resume" currDelay so it continues where it left off
        Serial.println("Was paused, resuming...");
      }
      if(pCharacteristic->getValue().compare("reset") == 0 || digitalRead(buttonReset) != 0){
        //Reset all motors back to 0, and reset counter to 0. Simply reuse motor vectors.
        PumpFunc::moveStepper(1, 0);
        PumpFunc::moveStepper(2, 0);
        PumpFunc::moveStepper(3, 0);
        PumpFunc::moveStepper(4, 0);
        PumpFunc::moveStepper(5, 0);
        PumpFunc::moveStepper(6, 0);
        PumpFunc::moveStepper(7, 0);
        PumpFunc::moveStepper(8, 0);
        PumpFunc::stepAllMotors();
        counter = 0;
        Serial.println("Counter is set back to 0");
        Serial.println("Experiment reset. Press Go to continue.");
        pauseExperiment();
        break;
      }
      if(currDelay - startDelay >= delays[counter]) break; //If the difference in times is greater than delays[counter], break out
    }
    counter++;

    Serial.print("counter is now: ");
    Serial.println(counter);
  }

  Serial.println("Closing out now...");

  // Sleep the motors
  PumpFunc::sleepMotors();

  delay(1000);

  //Wake up motors
  PumpFunc::wakeMotors();

  // Move Steppers Units are in mm (Max height = 80mm)
  PumpFunc::moveStepper(1, 0); // PumpFunc::moveStepper(stepper#, Height(mm)) ***
  PumpFunc::moveStepper(2, 0); // PumpFunc::moveStepper(stepper#, Height(mm)) ***
  PumpFunc::moveStepper(3, 0); // PumpFunc::moveStepper(stepper#, Height(mm)) ***
  PumpFunc::moveStepper(4, 0); // PumpFunc::moveStepper(stepper#, Height(mm)) ***
  PumpFunc::moveStepper(5, 0); // PumpFunc::moveStepper(stepper#, Height(mm)) ***
  PumpFunc::moveStepper(6, 0); // PumpFunc::moveStepper(stepper#, Height(mm)) ***
  PumpFunc::moveStepper(7, 0); // PumpFunc::moveStepper(stepper#, Height(mm)) ***
  PumpFunc::moveStepper(8, 0); // PumpFunc::moveStepper(stepper#, Height(mm)) ***

  //Step to that position
  PumpFunc::stepAllMotors(); //Takes about 162 seconds to complete? 

  // Sleep the motors
  PumpFunc::sleepMotors();

  delay(1000);
  Serial.println("Done executing experiment/trial.");
  stop = millis()/1000;
}

//Interprets incoming packets and builds them into variables for an experiment
void interpretExp(){
  pCharacteristic->setValue("next"); //As soon as function is called set it to "next" to indicate
  pCharacteristic->notify(true);
                              
  Serial.println("At start of interpretExp: ");
  Serial.println(pCharacteristic->getValue().c_str());
  instruction = "";
  oldPacket = "";
  packet = "";
  pos = 0;
  oldpos = 0;
  float timeout = millis();
  bool timeoutBool = false;
  display.resetDisplay();
  display.drawString(1, 1, "Receiving Experiment");
  display.drawString(1, 11, "via BLE");
  display.display();
  while(true){
    if(millis() - timeout >= 10000) {
      timeoutBool = true;
      Serial.println("Error: Timeout (10 seconds passed without packet)");
      break;
    }
    delay(10); //50ms is definitely long enough
    Serial.print(".");
    packet = pCharacteristic->getValue();
    pCharacteristic->notify(true);
    if(packet.compare("0") == 0){
      timeout = millis(); //Reset timeout since we received another real packet
      Serial.println("Got 0, waiting for final hash to come in...");
      pCharacteristic->setValue("next");
      while(packet.compare("next") == 0 || packet.compare("0") == 0){ //While the packet is still 0
        delay(10); //Needs time to fully acquire the value over BLE
        packet = pCharacteristic->getValue(); //Busy wait until a characteristic has new value
      }
      recHash = packet;
      Serial.println("Got final hash from app: ");
      Serial.println(recHash.c_str());
      Serial.println("String before applying changes: ");
      Serial.println(instruction.c_str());
      Serial.println("It's length in total was: ");
      Serial.println(strlen(instruction.c_str()));
      //Aggregate the data into their individual strings, packed into one vector
      while(pos <= strlen(instruction.c_str())){
        pos = instruction.find('_', oldpos);
        token = instruction.substr(oldpos, pos-oldpos);
        oldpos = pos + 1; //Skip over _
        shkrings.push_back(token); //Add it to our list of std::strings
        if(pos == -1) break; //If pos is -1, find function failed to detect anymore _
      }
      //Print out the vector's contents to verify everything is appropriately received & handled
      Serial.println("Shkrings has this many members: ");
      Serial.println(shkrings.size());
      //Currently only flushing display when new write is received
      display.resetDisplay();
      Serial.println("Individual data pieces received (variables for pump): ");
      line = 0;
      for(i = shkrings.begin(); i < shkrings.end(); ++i){
        display.drawStringMaxWidth(3, line, 120, (*i).c_str());
        line += 10;
        Serial.println((*i).c_str());
      }
      break;
    }

    //Is the packet different from the last packet, AND not a exit packet AND not an entrance 
    //packet, AND not a "next" packet? If so, append
    if(packet.compare(oldPacket) != 0 && packet.compare("0") != 0 && packet.compare("1") != 0 && packet.compare("next") != 0){
      timeout = millis(); //Reset the timeout since we just received another packet
      instruction = instruction + packet;
      oldPacket = packet;
      Serial.println("Built string is now:");
      Serial.println(instruction.c_str());
      pCharacteristic->setValue("next"); //Received packet, done with it, and ready for next
      pCharacteristic->notify(true);
    }

    if(!deviceConnected && oldDeviceConnected) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Restarting advertising, device disconnected");
        display.resetDisplay();
        display.drawString(1, 1, "Disconnected");
        display.display();
        oldDeviceConnected = deviceConnected;
        Serial.println("Unexpectedly lost connection during data transfer");
        return;
    }
  }
  if(!timeoutBool){ //If you never proc a timeout, execute whatever is left in this function. If you did, skip and go back to loop()
    if(recHash.compare("") != 0 && isValid(instruction, recHash)){ //If our received hash is defined as anything
      display.display();
      sortMotors(shkrings);
      runExp(motors);
      Serial.print("Experiment finished after ");
      Serial.print(stop - start);
      Serial.println(" seconds.");
      shkrings.clear(); //Wipe the vector so its size will be 0 again.
                      //Ensures code if conditional won't spam
      recHash = "";
    }
  }
}

void loop() {
    //while(1) runExpTest();
    //connectED
    if (deviceConnected) {
        //connectING
        if (!oldDeviceConnected) { //Originally was (deviceConnected && !oldDeviceConnected) (now redundant due to moving inside of the other conditional)
          //do stuff here on connecting
          Serial.println("New client device connected to LEESP32 server.");
          display.resetDisplay();
          display.drawString(1, 1, "Connected");
          display.display();
          digitalWrite(LED_BUILTIN, HIGH);
          oldDeviceConnected = deviceConnected;
        }
        if(pCharacteristic->getValue().compare("1") == 0){ 
          interpretExp();
          pCharacteristic->setValue("");
        }
        delay(1000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if(!deviceConnected && oldDeviceConnected) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Restarting advertising, device disconnected");
        display.resetDisplay();
        display.drawString(1, 1, "Disconnected");
        display.display();
        oldDeviceConnected = deviceConnected;
    }
}