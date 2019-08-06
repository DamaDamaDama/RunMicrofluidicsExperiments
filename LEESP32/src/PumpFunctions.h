//PumpFunctions.h
// Contains declarations for global variables and the class PumpFunc
//
//    - Created by Alan Munirji with help from David Schipper
//      University of Pittsburgh, Computational and Systems Biology
//      5/17/2019 REC Lee Lab
//

//Global variables to be used. Any variables that multiple files want are listed here
extern float microstep;
extern int SLP;
extern int buttonState1;
extern int buttonPin1;
extern AccelStepper stepper1;
extern AccelStepper stepper2;
extern AccelStepper stepper3;
extern AccelStepper stepper4;
extern AccelStepper stepper5;
extern AccelStepper stepper6;
extern AccelStepper stepper7;
extern AccelStepper stepper8;

class PumpFunc
{
public:
    //Guarantee motors are awake
    static void wakeMotors();

    //Run motors to the correct position
    static void stepAllMotors();

    //Put motors to sleep
    static void sleepMotors();

    //Move relative to current position
    static void moveBy(int stepperNum, int steps);

    //Return all steppers to home position
    static void homeSteppers();

    //Wait for button press to confirm next part. For splitting experiments into parts
    static void pauseExperiment();

    //Convert height to position in steps. Need to insert height in mm
    static void moveStepper(int stepperNum, float height);
};