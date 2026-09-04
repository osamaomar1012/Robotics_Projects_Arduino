#include "ard_Robot_Motion.h"
#include "ard_Robot_Comms.h" // For parser

namespace Motion {

    // State
    // Initialize to "Rest" position (Height 55) so standUp() performs a smooth rise from the ground
    // Coordinates are [LegIndex][Axis] in mm relative to the shoulder pivot.
    // Leg Index: 0=FrontLeft, 1=BackLeft, 2=BackRight, 3=FrontRight
    // Axis: 0=X (Forward/Back), 1=Y (Up/Down), 2=Z (Left/Right)
    float currentPos[4][3] = {{10, 55, 10}, {10, 55, 10}, {10, 55, -10}, {10, 55, -10}};
    float calibration[4][3] = {{10, 99, 10}, {10, 99, 10}, {10, 99, -10}, {10, 99, -10}};
    float offsets[4][3] = {{0}};
    
    Comms::Parser parser;

    // ===================================================================================
    //  CALIBRATION
    // ===================================================================================
    // Loads servo offsets from Non-Volatile Storage (NVS).
    // These offsets correct mechanical misalignments in the servo horns.
    void loadCalibration() {
        if (Drivers::NVS::prefs.getBytesLength("OFFSET") == sizeof(offsets)) {
            Drivers::NVS::prefs.getBytes("OFFSET", &offsets, sizeof(offsets));
            
            // SAFETY CHECK: Validate loaded offsets
            bool corrupt = false;
            for(int i=0; i<4; i++) {
                for(int j=0; j<3; j++) {
                    // If offset is NaN or unreasonably large (> 45 degrees), assume corruption
                    if (isnan(offsets[i][j]) || abs(offsets[i][j]) > 45.0f) corrupt = true;
                }
            }
            if (corrupt) {
                Serial.println("[Motion] Corrupt Calibration Data detected! Resetting to 0.");
                memset(offsets, 0, sizeof(offsets));
                Drivers::Buzzer::play(MELODY_CAM_FAILURE);
            }
        } else {
            memset(offsets, 0, sizeof(offsets)); // Ensure clean state if no config
        }
    }

    // ===================================================================================
    //  INVERSE KINEMATICS
    // ===================================================================================
    /**
     * @brief Calculates the servo angles required for a leg to reach a specific point in 3D space.
     * @param leg Leg index (0-3).
     * @param x Target X coordinate [mm] (Forward/Back relative to shoulder).
     * @param y Target Y coordinate [mm] (Up/Down relative to shoulder).
     * @param z Target Z coordinate [mm] (Left/Right relative to shoulder).
     * @param a, b, c Output variables for servo angles (Alpha, Beta, Gamma) in degrees.
     */
    void calcAngles(int leg, float x, float y, float z, float& a, float& b, float& c) {
        float x_3 = 0, x_4, x_5 = 0, l23 = 0, w = 0, v = 0;
        
        // Calculate Angle A (Root)
        float angleA = PI / 2 - atan2(z, y);
        
        x_3 = 0;
        x_4 = ROBOT_L1 * sin(angleA);
        x_5 = ROBOT_L1 * cos(angleA);

        // Check active radius (Physical limit of the leg extension)
        float d = sqrt(pow((z - x_5), 2) + pow((y - x_4), 2) + pow((x - x_3), 2));
        if (d > ROBOT_DR) {
            float k = ROBOT_DR / d;
            x = k * (x - x_3) + x_3;
            y = k * (y - x_4) + x_4;
            z = k * (z - x_5) + x_5;
        }

        // Calculate Angles B and C (Thigh and Calf) using the Law of Cosines
        l23 = sqrt(pow((z - x_5), 2) + pow((y - x_4), 2) + pow((x - x_3), 2));
        w = (x - x_3) / l23;
        v = (ROBOT_L2 * ROBOT_L2 + l23 * l23 - ROBOT_L3 * ROBOT_L3) / (2 * ROBOT_L2 * l23);

        // Clamp values to [-1, 1] to prevent NaN errors in asin/acos due to floating point noise
        if (w > 1.0) w = 1.0; else if (w < -1.0) w = -1.0;
        if (v > 1.0) v = 1.0; else if (v < -1.0) v = -1.0;
        
        float angleB = asin(w) - acos(v);
        float valC = (pow(ROBOT_L2, 2) + pow(ROBOT_L3, 2) - pow(l23, 2)) / (2 * ROBOT_L3 * ROBOT_L2);
        if (valC > 1.0) valC = 1.0; else if (valC < -1.0) valC = -1.0;
        float angleC = PI - acos(valC);

        // Convert to degrees
        a = angleA / PI * 180;
        b = angleB / PI * 180;
        c = angleC / PI * 180;
        
        // Map B to servo range
        b = 90.0f - b; // Float equivalent of map(b, -90, 90, 180, 0)
    }

    // ===================================================================================
    //  SERVO UPDATE
    // ===================================================================================
    /**
     * @brief Updates all 12 servos based on the `currentPos` array.
     * @details Calls calcAngles() for each leg, applies calibration offsets, and sends commands to the PCA9685.
     */
    void updateServos() {
        float a, b, c;
        for(int i=0; i<4; i++) {
            calcAngles(i, currentPos[i][0], currentPos[i][1], currentPos[i][2], a, b, c);
            
            // Apply calibration offsets and map to specific PCA9685 channels
            // Leg 0: 0,1,2; Leg 1: 7,6,5; Leg 2: 8,9,10; Leg 3: 15,14,13
            if (i==0) {
                Drivers::PCA9685::setServoAngle(0, a + offsets[i][0]);
                Drivers::PCA9685::setServoAngle(1, b + offsets[i][1]);
                Drivers::PCA9685::setServoAngle(2, c + offsets[i][2]);
            } else if (i==1) {
                Drivers::PCA9685::setServoAngle(7, a + offsets[i][0]);
                Drivers::PCA9685::setServoAngle(6, b + offsets[i][1]);
                Drivers::PCA9685::setServoAngle(5, c + offsets[i][2]);
            } else if (i==2) {
                Drivers::PCA9685::setServoAngle(8, a + offsets[i][0]);
                Drivers::PCA9685::setServoAngle(9, 180 - b - offsets[i][1]);
                Drivers::PCA9685::setServoAngle(10, 180 - c - offsets[i][2]);
            } else if (i==3) {
                Drivers::PCA9685::setServoAngle(15, a + offsets[i][0]);
                Drivers::PCA9685::setServoAngle(14, 180 - b - offsets[i][1]);
                Drivers::PCA9685::setServoAngle(13, 180 - c - offsets[i][2]);
            }
        }
    }

    // ===================================================================================
    //  MOVEMENT PRIMITIVES
    // ===================================================================================
    /**
     * @brief Smoothly moves all legs from their current position to a target position.
     * @param target Target coordinates [4][3] in mm.
     * @param speed Movement speed factor (Higher = Faster, Lower = Smoother).
     */
    void moveTo(float target[4][3], int speed) {
        // Calculate max distance to determine steps
        float maxDist = 0;
        for (int i = 0; i < 4; i++) {
            float dist = sqrt(pow(target[i][0] - currentPos[i][0], 2) + 
                              pow(target[i][1] - currentPos[i][1], 2) + 
                              pow(target[i][2] - currentPos[i][2], 2));
            if (dist > maxDist) maxDist = dist;
        }

        // Ensure speed is valid (avoid div by 0)
        if (speed < 1) speed = 1;
        int steps = (int)(maxDist / speed) + 1; // +1 to ensure at least 1 step

        for(int t=1; t<=steps; t++) {
            for(int i=0; i<4; i++) {
                for(int j=0; j<3; j++) {
                    // Linear interpolation: Start + (End - Start) * (t / TotalSteps)
                    // We update currentPos incrementally to keep state valid
                    float diff = target[i][j] - currentPos[i][j];
                    // Calculate the increment for this remaining portion
                    // A simpler way is to interpolate from the *start* of this moveTo call, 
                    // but since we don't store 'start', we nudge currentPos towards target.
                    // Better approach for stability:
                    currentPos[i][j] += diff / (steps - t + 1); 
                }
            }
            updateServos();
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }

    /**
     * @brief Resets the robot to the neutral standing position.
     * @details Uses the `calibration` coordinates as the target.
     */
    void standUp() {
        float target[4][3];
        memcpy(target, calibration, sizeof(calibration));
        moveTo(target, 2); // Slower speed (2mm/step) for smooth startup
    }

    /**
     * @brief Adjusts the robot's body height.
     * @param h Target height in mm (Safe range: 55-110mm).
     */
    void setHeight(int h) {
        h = constrain(h, 55, 110); // Limit height to safe range (55mm - 110mm)
        float target[4][3];
        // Preserve current X/Z positions to prevent feet from sliding/moving
        memcpy(target, currentPos, sizeof(currentPos));
        // Only update Y (Height)
        for(int i=0; i<4; i++) target[i][1] = h;
        moveTo(target, 10);
    }

    /**
     * @brief Rotates the robot's body around its center (Pitch/Roll/Yaw).
     * @param alpha Pitch angle (X-axis rotation) in degrees.
     * @param beta Roll angle (Z-axis rotation) in degrees.
     * @param gama Yaw angle (Y-axis rotation) in degrees.
     */
    void twist(float alpha, float beta, float gama) {
        // Constrain angles to prevent mechanical binding
        alpha = constrain(alpha, -25, 25);
        beta = constrain(beta, -25, 25);
        gama = constrain(gama, -25, 25);
        
        // alpha: Pitch (X-axis rotation)
        // beta: Roll (Z-axis rotation)
        // gama: Yaw (Y-axis rotation)
        float a = alpha * PI / 180;
        float b = beta * PI / 180;
        float c = gama * PI / 180;

        float v = sqrt(pow(ROBOT_LEN_BD, 2) + pow(ROBOT_WID_BD, 2));
        float theta = atan(ROBOT_WID_BD / ROBOT_LEN_BD);

        float v_x_sin_theta_p_c = v * sin(theta + c);
        float v_x_sin_theta_m_c = v * sin(theta - c);
        float v_x_cos_theta_p_c = v * cos(theta + c);
        float v_x_cos_theta_m_c = v * cos(theta - c);

        float l_x_sin_a = ROBOT_LEN_BD * sin(a);
        float w_x_sin_b = ROBOT_WID_BD * sin(b);

        float target[4][3];

        target[0][0] = calibration[0][0] - v_x_cos_theta_p_c + ROBOT_LEN_BD;
        target[1][0] = calibration[1][0] + v_x_cos_theta_m_c - ROBOT_LEN_BD;
        target[2][0] = calibration[2][0] + v_x_cos_theta_p_c - ROBOT_LEN_BD;
        target[3][0] = calibration[3][0] - v_x_cos_theta_m_c + ROBOT_LEN_BD;

        target[0][1] = calibration[0][1] + l_x_sin_a + w_x_sin_b;
        target[1][1] = calibration[1][1] - l_x_sin_a + w_x_sin_b;
        target[2][1] = calibration[2][1] - l_x_sin_a - w_x_sin_b;
        target[3][1] = calibration[3][1] + l_x_sin_a - w_x_sin_b;

        target[0][2] = calibration[0][2] - v_x_sin_theta_p_c + ROBOT_WID_BD;
        target[1][2] = calibration[1][2] - v_x_sin_theta_m_c + ROBOT_WID_BD;
        target[2][2] = calibration[2][2] + v_x_sin_theta_p_c - ROBOT_WID_BD;
        target[3][2] = calibration[3][2] + v_x_sin_theta_m_c - ROBOT_WID_BD;

        Serial.printf("Twist: %.1f %.1f %.1f\n", alpha, beta, gama);
        moveTo(target, 5);
    }

    // ===================================================================================
    //  GAITS
    // ===================================================================================
    /**
     * @brief Executes one tick of the Trot Gait (Diagonal pairs moving together).
     * @param speed Movement speed (1-10).
     * @param strideX Forward/Backward stride length [mm].
     * @param strideZ Left/Right stride length [mm].
     * @details This function is non-blocking and updates the leg positions incrementally.
     */
    void updateTrot(int speed, float strideX, float strideZ) {
        static float phaseProgress = 0;
        static int currentPhase = 0; // 0 or 1
        
        float lift = 20;
        float cogOffsetX = -strideX * 0.25; // Shift CoG forward/backward
        float cogOffsetZ = -strideZ * 0.20; // Shift CoG left/right
        
        // Baseline position with CoG shift
        float basePos[4][3];
        memcpy(basePos, calibration, sizeof(calibration));
        for(int i=0; i<4; i++) { basePos[i][0] += cogOffsetX; basePos[i][2] += cogOffsetZ; }

        // Determine legs based on phase
        // Phase 0: 0&2 Swing (Forward), 1&3 Stance (Backward)
        // Phase 1: 1&3 Swing (Forward), 0&2 Stance (Backward)
        int swing[2], stance[2];
        if (currentPhase == 0) {
            swing[0]=0; swing[1]=2; stance[0]=1; stance[1]=3;
        } else {
            swing[0]=1; swing[1]=3; stance[0]=0; stance[1]=2;
        }

        // Swing Legs: Move Forward + Lift
        for (int i = 0; i < 2; i++) {
            int leg = swing[i];
            float startX = basePos[leg][0] - strideX/2.0;
            float endX   = basePos[leg][0] + strideX/2.0;
            currentPos[leg][0] = startX + (endX - startX) * phaseProgress;
            
            float startZ = basePos[leg][2] - strideZ/2.0;
            float endZ   = basePos[leg][2] + strideZ/2.0;
            currentPos[leg][2] = startZ + (endZ - startZ) * phaseProgress;
            
            currentPos[leg][1] = basePos[leg][1] - lift * sin(phaseProgress * PI);
        }
        
        // Stance Legs: Move Backward (Propel body)
        for (int i = 0; i < 2; i++) {
            int leg = stance[i];
            float startX = basePos[leg][0] + strideX/2.0;
            float endX   = basePos[leg][0] - strideX/2.0;
            currentPos[leg][0] = startX + (endX - startX) * phaseProgress;
            
            float startZ = basePos[leg][2] + strideZ/2.0;
            float endZ   = basePos[leg][2] - strideZ/2.0;
            currentPos[leg][2] = startZ + (endZ - startZ) * phaseProgress;
            
            currentPos[leg][1] = basePos[leg][1];
        }
        
        updateServos();
        
        // Advance Phase
        // Speed determines how much progress per tick (0.0 to 1.0)
        // Base speed factor: 0.05 per tick (20 ticks per phase)
        // Adjust based on speed input (1-10)
        float stepInc = 0.02 + (speed * 0.005); 
        phaseProgress += stepInc;
        
        if (phaseProgress >= 1.0) {
            phaseProgress = 0;
            currentPhase = 1 - currentPhase; // Toggle 0/1
        }
    }

    /**
     * @brief Executes a Spot Turn (rotating in place).
     * @param direction +1 for Left (CCW), -1 for Right (CW).
     * @param speed Movement speed.
     */
    void turn(int direction, int speed) {
        Serial.printf("Turn: Dir=%d Speed=%d\n", direction, speed);
        float lift = 20;
        float stride = 30; 
        float target[4][3];
        memcpy(target, currentPos, sizeof(currentPos));

        // Sequence: FL(0), BR(2), BL(1), FR(3)
        int sequence[] = {0, 2, 1, 3};

        for (int k = 0; k < 1; k++) { // One cycle
            for (int i = 0; i < 4; i++) {
                int leg = sequence[i];
                
                // 1. Lift
                target[leg][1] = calibration[leg][1] - lift;
                
                // 2. Reach (Left legs back, Right legs fwd for Left Turn)
                // Leg Indices: 0(FL), 1(BL), 2(BR), 3(FR)
                float dirSign = (direction > 0) ? 1.0 : -1.0;
                if (leg == 0 || leg == 1) target[leg][0] = calibration[leg][0] - (stride * dirSign); // Left side
                else target[leg][0] = calibration[leg][0] + (stride * dirSign); // Right side
                
                moveTo(target, speed);

                // 3. Place
                target[leg][1] = calibration[leg][1];
                moveTo(target, speed);

                // 4. Propel (All legs return to center, rotating the body)
                for(int j=0; j<4; j++) target[j][0] = calibration[j][0];
                moveTo(target, speed);
            }
        }
        // Removed standUp() for consistency.
    }

    // ===================================================================================
    //  DANCE ROUTINES
    // ===================================================================================
    namespace Dance {
        void sayHello() {
            float target[4][3];
            memcpy(target, calibration, sizeof(calibration));
            
            // Shift body weight to Left-Rear to prevent falling
            // Move body Left (Feet move Right -> Z decreases)
            // Move body Back (Feet move Forward -> X increases)
            float shiftX = 30;
            float shiftZ = -30;
            for(int i=0; i<4; i++) {
                target[i][0] += shiftX;
                target[i][2] += shiftZ;
            }
            moveTo(target, 5); // Execute body shift first to ensure stability

            // Lift Right Front (3) High
            target[3][0] += 45;
            target[3][1] -= 90; // Retract leg significantly
            target[3][2] -= 10; // Move Outwards (Right) instead of Inwards
            moveTo(target, 2);  // Slower speed for stability
            
            // Wave
            for(int i=0; i<3; i++) {
                target[3][2] -= 15; moveTo(target, 3); // Wave Out
                target[3][2] += 15; moveTo(target, 3); // Wave In
            }
            standUp();
        }
        
        void pushUp() {
            float target[4][3];
            memcpy(target, calibration, sizeof(calibration));
            
            // Get into position: Back legs back, body slightly up
            target[1][0] -= 80; target[1][1] += 10;
            target[2][0] -= 80; target[2][1] += 10;
            target[0][1] += 20;
            target[3][1] += 20;
            moveTo(target, 2);
            vTaskDelay(200);
            
            // Perform Pushups
            for(int i=0; i<5; i++) {
                // Down (Front legs retract)
                target[0][1] = calibration[0][1] - 20;
                target[3][1] = calibration[3][1] - 20;
                moveTo(target, 2);
                vTaskDelay(100);
                
                // Up (Front legs extend)
                target[0][1] = calibration[0][1] + 20;
                target[3][1] = calibration[3][1] + 20;
                moveTo(target, 2);
                vTaskDelay(100);
            }
            standUp();
        }

        void stretchSelf() {
            float target[4][3];
            memcpy(target, calibration, sizeof(calibration));
            
            // Front legs forward and up (stretch)
            target[0][0] += 95; target[0][1] -= 56;
            target[3][0] += 95; target[3][1] -= 56;
            
            // Back legs down
            target[1][1] += 30;
            target[2][1] += 30;
            
            moveTo(target, 2);
            vTaskDelay(2000);
            standUp();
        }

        void sitDown() {
            float target[4][3];
            memcpy(target, calibration, sizeof(calibration));
            
            // Front legs up (body high)
            target[0][1] += 30;
            target[3][1] += 30;
            
            // Back legs down (body low/retracted)
            target[1][1] -= 30;
            target[2][1] -= 30;
            
            moveTo(target, 2);
            vTaskDelay(3000);
            standUp();
        }

        void turnAround() {
            // Simulate looking around using body twist (Yaw)
            for(int i=0; i<3; i++) {
                twist(0, 0, 20); vTaskDelay(400);
                twist(0, 0, -20); vTaskDelay(400);
            }
            twist(0, 0, 0);
        }

        void dancing() {
            // Sequence of twists (Roll/Pitch)
            for(int i=0; i<3; i++) {
                twist(0, 15, 0); vTaskDelay(250);
                twist(0, -15, 0); vTaskDelay(250);
                twist(15, 0, 0); vTaskDelay(250);
                twist(-15, 0, 0); vTaskDelay(250);
            }
            twist(0, 0, 0);
        }
    }

    // ===================================================================================
    //  MOTION TASK
    // ===================================================================================
    // Motion State Variables
    float g_strideX = 0;
    float g_strideZ = 0;
    float g_rotation = 0;
    int g_speed = 0;
    bool g_isMoving = false;
    bool g_wasMoving = false;
    
    // Idle Timer
    uint32_t g_lastActivityTime = 0;
    bool g_isSleeping = false;

    /**
     * @brief Main Motion Control Task (FreeRTOS).
     * @details Handles command parsing, gait execution, and idle timeout logic.
     */
    void taskMotion(void *pvParameters) {
        g_lastActivityTime = millis();
        while(1) {
            // 1. Process ALL waiting commands to get the latest state
            while (!mqMotion.isEmpty()) {
                g_lastActivityTime = millis(); // Reset timer on any command
                String msg = mqMotion.out();
                Serial.print("Motion Task Rx: "); Serial.println(msg);
                parser.parse(msg);
                Serial.printf("Parsed: Cmd='%c' (%d) Params=%d [0]=%.1f\n", parser.cmdChar, (int)parser.cmdChar, parser.paramCount, parser.params[0]);
                
                switch(parser.cmdChar) {
                    case ACTION_INSTALLATION:
                        if (parser.params[0] == 1) {
                            // Set all to 90 degrees
                            if (g_isSleeping) { standUp(); g_isSleeping = false; }
                            for(int i=0; i<16; i++) Drivers::PCA9685::setServoAngle(i, 90);
                        } else if (parser.params[0] == 2) { // Verify / Reset
                            standUp(); // Move to Calibrated Standing Position
                            g_isSleeping = false;
                        }
                        break;
                        
                    case ACTION_CALIBRATE:
                        // J#Leg#Mode#X#Y#Z
                        // Mode 1: Test, Mode 2: Save, Mode 3: Reset
                        {
                        if (g_isSleeping) { standUp(); g_isSleeping = false; }
                        int leg = parser.params[0];
                        int mode = parser.params[1];
                        if (mode == 1) { // Test
                            // Move leg to: Calibration Point + Input Offsets (X,Y,Z) WITHOUT saved offsets
                            float targetX = calibration[leg][0] + parser.params[2];
                            float targetY = calibration[leg][1] + parser.params[3];
                            float targetZ = calibration[leg][2] + parser.params[4];
                            
                            float a, b, c;
                            calcAngles(leg, targetX, targetY, targetZ, a, b, c);
                            
                            // Apply to specific leg channels immediately (Raw angles)
                            if (leg == 0) {
                                Drivers::PCA9685::setServoAngle(0, a);
                                Drivers::PCA9685::setServoAngle(1, b);
                                Drivers::PCA9685::setServoAngle(2, c);
                            } else if (leg == 1) {
                                Drivers::PCA9685::setServoAngle(7, a);
                                Drivers::PCA9685::setServoAngle(6, b);
                                Drivers::PCA9685::setServoAngle(5, c);
                            } else if (leg == 2) {
                                Drivers::PCA9685::setServoAngle(8, a);
                                Drivers::PCA9685::setServoAngle(9, 180 - b);
                                Drivers::PCA9685::setServoAngle(10, 180 - c);
                            } else if (leg == 3) {
                                Drivers::PCA9685::setServoAngle(15, a);
                                Drivers::PCA9685::setServoAngle(14, 180 - b);
                                Drivers::PCA9685::setServoAngle(13, 180 - c);
                            }
                            
                            Drivers::Buzzer::play(MELODY_BB_CLEAR_1);
                        } else if (mode == 2) { // Save
                            // Calculate angles for the target point (params 2,3,4 correspond to X,Y,Z offset from calibration)
                            float ta, tb, tc;
                            float ca, cb, cc;
                            // Target angles (Current adjusted position)
                            calcAngles(leg, parser.params[2] + calibration[leg][0], parser.params[3] + calibration[leg][1], parser.params[4] + calibration[leg][2], ta, tb, tc);
                            // Calibration angles (Neutral position)
                            calcAngles(leg, calibration[leg][0], calibration[leg][1], calibration[leg][2], ca, cb, cc);
                            
                            offsets[leg][0] = ta - ca; offsets[leg][1] = tb - cb; offsets[leg][2] = tc - cc;
                            Drivers::NVS::prefs.putBytes("OFFSET", &offsets, sizeof(offsets));
                            Drivers::Buzzer::play(MELODY_BB_CLEAR_2);
                            standUp(); // Apply new calibration immediately
                        } else if (mode == 3) { // Reset
                            // Move to low height (55mm) without offsets, then release
                            float h = 55;
                            for(int i=0; i<4; i++) {
                                float a, b, c;
                                calcAngles(i, calibration[i][0], h, calibration[i][2], a, b, c);
                                if (i==0) {
                                    Drivers::PCA9685::setServoAngle(0, a); Drivers::PCA9685::setServoAngle(1, b); Drivers::PCA9685::setServoAngle(2, c);
                                } else if (i==1) {
                                    Drivers::PCA9685::setServoAngle(7, a); Drivers::PCA9685::setServoAngle(6, b); Drivers::PCA9685::setServoAngle(5, c);
                                } else if (i==2) {
                                    Drivers::PCA9685::setServoAngle(8, a); Drivers::PCA9685::setServoAngle(9, 180-b); Drivers::PCA9685::setServoAngle(10, 180-c);
                                } else if (i==3) {
                                    Drivers::PCA9685::setServoAngle(15, a); Drivers::PCA9685::setServoAngle(14, 180-b); Drivers::PCA9685::setServoAngle(13, 180-c);
                                }
                            }
                            vTaskDelay(500);
                            Drivers::PCA9685::releaseAll();
                            Drivers::Buzzer::play(MELODY_BB_CLEAR_1);
                        }
                        }
                        break;
                        
                    case ACTION_UP_DOWN:
                        // 0: Toggle, 1: Stand, 2: Rest
                        static bool isStanding = true;
                        if (parser.params[0] == 0) {
                            if (isStanding) { 
                                setHeight(55); // Lean down first
                                Drivers::PCA9685::releaseAll(); 
                                isStanding = false; 
                                g_isSleeping = true;
                            } else { 
                                standUp(); 
                                isStanding = true; 
                                g_isSleeping = false;
                            }
                        }
                        else if (parser.params[0] == 1) {
                            standUp(); isStanding = true;
                            g_isSleeping = false;
                        }
                        else if (parser.params[0] == 2) {
                            setHeight(55); // Lean down first
                            Drivers::PCA9685::releaseAll(); 
                            isStanding = false;
                            g_isSleeping = true;
                        }
                        break;
                        
                    case ACTION_BODY_HEIGHT:
                        if (g_isSleeping) { standUp(); g_isSleeping = false; }
                        setHeight(parser.params[0]);
                        break;

                    case ACTION_TWIST:
                        Serial.println("Executing Twist...");
                        if (g_isSleeping) { standUp(); g_isSleeping = false; }
                        if (parser.paramCount >= 3) {
                            twist(parser.params[0], parser.params[1], parser.params[2]);
                        }
                        break;
                        
                    case ACTION_MOVE_ANY:
                        { // Instead of blocking, update global state
                        float direction = parser.params[0];
                        float stepLen = parser.params[1];
                        float rotation = parser.params[2];
                        int speed = parser.params[3];

                        if (stepLen > 0) {
                            // Calculate stride components
                            float magnitude = 30.0; 
                            float rad = direction * PI / 180.0;
                            float targetStrideX = magnitude * cos(rad);
                            float targetStrideZ = magnitude * sin(rad);
                            // Simple Low Pass Filter to smooth out erratic app commands
                            g_strideX = g_strideX * 0.7 + targetStrideX * 0.3;
                            g_strideZ = g_strideZ * 0.7 + targetStrideZ * 0.3;
                            g_rotation = 0;
                            g_speed = speed;
                            g_isMoving = true;
                        } else if (abs(rotation) > 0) {
                            g_rotation = rotation;
                            g_strideX = 0; g_strideZ = 0;
                            g_speed = speed;
                            g_isMoving = true;
                        } else {
                            // Stop
                            g_strideX = 0; g_strideZ = 0; // Reset stride when stopped
                            g_isMoving = false;
                        }
                        }
                        break;
                        
                    case ACTION_DANCING:
                        // Corrected IDs to match Freenove App (0-based)
                        if (g_isSleeping) { standUp(); g_isSleeping = false; }
                        if (parser.params[0] == 0) Dance::sayHello();
                        else if (parser.params[0] == 1) Dance::pushUp();
                        else if (parser.params[0] == 2) Dance::stretchSelf();
                        else if (parser.params[0] == 3) Dance::turnAround();
                        else if (parser.params[0] == 4) Dance::sitDown();
                        else if (parser.params[0] == 5) Dance::dancing();
                        break;
                }
            }

            // 2. Execute Motion based on latest state
            if (g_isMoving) {
                g_lastActivityTime = millis(); // Reset timer while moving
                if (g_isSleeping) {
                    standUp();
                    g_isSleeping = false;
                }

                if (abs(g_rotation) > 0) {
                    turn((g_rotation > 0) ? 1 : -1, g_speed);
                } else {
                    updateTrot(g_speed, g_strideX, g_strideZ);
                }
                g_wasMoving = true;
            } else {
                if (g_wasMoving) {
                    standUp(); // Smoothly return to neutral posture once when stopping
                    g_wasMoving = false;
                    g_lastActivityTime = millis(); // Reset timer on stop
                }

                // 3. Idle Timeout Check
                if (!g_isSleeping && (millis() - g_lastActivityTime > 10000)) {
                    Serial.println("Idle Timeout: Leaning down...");
                    setHeight(55);
                    Drivers::PCA9685::releaseAll();
                    g_isSleeping = true;
                }
            }

            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}