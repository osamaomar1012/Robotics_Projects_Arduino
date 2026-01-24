#ifndef ROBOT_MOTION_CONTROL_H
#define ROBOT_MOTION_CONTROL_H

#include "Robot_Global_Definitions.h"
#include "Robot_Hardware_Drivers.h"

namespace Motion {
    
    // --- Kinematics & State ---
    extern float currentPos[4][3];
    extern float calibration[4][3];
    extern float offsets[4][3];
    
    void loadCalibration();
    void standUp();
    void setHeight(int h);
    
    // --- Core Movement ---
    // Moves legs to target coordinates (Inverse Kinematics)
    void moveTo(float target[4][3], int speed);
    
    // Calculates angles for a specific leg point
    void calcAngles(int leg, float x, float y, float z, float& a, float& b, float& c);
    
    // --- High Level Actions ---
    void twist(float x, float y, float z);
    void turn(int direction, int speed);
    void updateTrot(int speed, float strideX, float strideZ);
    
    // --- Dance Routines ---
    namespace Dance {
        void sayHello();
        void pushUp();
        void stretchSelf();
        void sitDown();
        void turnAround();
        void dancing();
    }

    // --- Task ---
    void taskMotion(void *pvParameters);
}

#endif