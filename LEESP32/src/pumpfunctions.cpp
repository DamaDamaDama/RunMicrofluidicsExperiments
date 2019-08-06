// PumpFunctions.cpp
// Contains definitions for PumpFunc class functions
// PumpFunc contains functions that are designed to make working with motors easier
// especially for managing 8 motors. It uses the AccelStepper library by Mike McCauley
//
//    - Created by Alan Munirji with help from David Schipper 
//      University of Pittsburgh, Computational and Systems Biology
//      5/17/2019 REC Lee Lab
//

#include <AccelStepper.h>
#include "PumpFunctions.h"

//--------------------Function to make sure the motors are awake--------------------------------
void PumpFunc::wakeMotors()
{
  digitalWrite(SLP, HIGH);
}

//--------------------Function to run the motors to the correct position---------------------------
//Step to that position
void PumpFunc::stepAllMotors()
{
  //Logic of this function is as follows:
  //While ANY motor still has some distance to cover:
  //  Take a step IF this motor still had some distance to cover
  //  Take a step IF this motor still had some distance to cover
  //  Take a step IF this motor still had some distance to cover
  //etc
  while(stepper1.distanceToGo() != 0 || stepper2.distanceToGo() != 0 
  || stepper3.distanceToGo() != 0 || stepper4.distanceToGo() != 0 
  || stepper5.distanceToGo() != 0 || stepper6.distanceToGo() != 0 
  || stepper7.distanceToGo() != 0 || stepper8.distanceToGo() != 0 )
  {
    if(stepper1.distanceToGo() != 0) stepper1.run();
    if(stepper2.distanceToGo() != 0) stepper2.run();
    if(stepper3.distanceToGo() != 0) stepper3.run();
    if(stepper4.distanceToGo() != 0) stepper4.run();
    if(stepper5.distanceToGo() != 0) stepper5.run();
    if(stepper6.distanceToGo() != 0) stepper6.run();
    if(stepper7.distanceToGo() != 0) stepper7.run();
    if(stepper8.distanceToGo() != 0) stepper8.run();
  }
}

//-------------------------Function to put the motors to sleep-------------------------------------
void PumpFunc::sleepMotors()
{
  digitalWrite(SLP, LOW);
}

//----------------------Function to move a single stepper relative to its current position----------
void PumpFunc::moveBy(int stepperNum, int steps)
{
  if (stepperNum == 1){
    stepper1.move(steps); 
  }
  else if (stepperNum == 2){
    stepper2.move(steps); 
  }
  else if (stepperNum == 3){
    stepper3.move(steps); 
  }
  else if (stepperNum == 4){
    stepper4.move(steps); 
  }
  else if (stepperNum == 5){
    stepper5.move(steps); 
  }
  else if (stepperNum == 6){
    stepper6.move(steps); 
  }
  else if (stepperNum == 7){
    stepper7.move(steps); 
  }
  else {
    stepper8.move(steps); 
  }
  stepAllMotors();
}

//----------------------Function to return all steppers to home position----------------------------
void PumpFunc::homeSteppers()
{
  stepper1.moveTo(0);
  stepper2.moveTo(0);
  stepper3.moveTo(0);
  stepper4.moveTo(0);
  stepper5.moveTo(0);
  stepper6.moveTo(0);
  stepper7.moveTo(0);
  stepper8.moveTo(0);
  stepAllMotors();
}

//-----------------------Convert height to position in steps-------------------------------------------
void PumpFunc::moveStepper(int stepperNum, float height)  //need to insert height in mm
{
  float stepSize = .025*microstep; //previously was .025*.00625
  float numSteps = height/stepSize;
  int roundedStepNum = numSteps;
  if (stepperNum == 1){
    stepper1.moveTo(roundedStepNum); 
  }
  else if (stepperNum == 2){
    stepper2.moveTo(roundedStepNum); 
  }
  else if (stepperNum == 3){
    stepper3.moveTo(roundedStepNum); 
  }
  else if (stepperNum == 4){
    stepper4.moveTo(roundedStepNum); 
  }
  else if (stepperNum == 5){
    stepper5.moveTo(roundedStepNum); 
  }
  else if (stepperNum == 6){
    stepper6.moveTo(roundedStepNum); 
  }
  else if (stepperNum == 7){
    stepper7.moveTo(roundedStepNum); 
  }
  else {
    stepper8.moveTo(roundedStepNum); 
  }
}

  //.5mm/20steps = .025mm/step (for no micro-stepping)
  // 
  //.0125mm/step for 1/2 micro-stepping 
  // 
  //.00625mm/step for 1/4 micro-stepping
  // 
  //.003125mm/step for 1/8 micro-stepping
