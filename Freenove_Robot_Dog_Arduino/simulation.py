import numpy as np
import matplotlib.pyplot as plt


class RobotLeg:
    def __init__(self, xOrigin, yOrigin, robotShape):
        self.xOrigin = xOrigin
        self.yOrigin = yOrigin
        self.robotShape = robotShape

    def calculateAngles(self, x, y, z):
        # Simplified IK calculation (port from Robot_Motion.ino)
        a, b, c = 0, 0, 0  # placeholder
        L1 = self.robotShape['L1']
        L2 = self.robotShape['L2']
        L3 = self.robotShape['L3']
        
        x_3 = 0
        x_4 = L1 #* sin(angleA);
        x_5 = L1 #* cos(angleA);

        # Check active radius
        d = np.sqrt((z - x_5)**2 + (y - x_4)**2 + (x - x_3)**2)
        DR = 100 #Active radius
        
        if (d > DR):
            k = DR / d
            x = k * (x - x_3) + x_3
            y = k * (y - x_4) + x_4
            z = k * (z - x_5) + x_5

        # Calculate Angles B and C (Thigh and Calf)
        l23 = np.sqrt((z - x_5)**2 + (y - x_4)**2 + (x - x_3)**2)
        w = (x - x_3) / l23
        v = (L2**2 + l23**2 - L3**2) / (2 * L2 * l23)
        angleA = 0 #PLACEHOLDER
        angleB = np.arcsin(w) - np.arccos(v)
        angleC = np.pi - np.arccos((L2**2 + L3**2 - l23**2) / (2 * L3 * L2))

        # Convert to degrees
        a = angleA / np.pi * 180
        b = angleB / np.pi * 180
        c = angleC / np.pi * 180

        # Map B to servo range
        b = np.interp(b, [-90, 90], [180, 0])

        return a,b,c

def simulate_motion(duration=5, fps=30, amplitude=20):
    """Simulates leg motion and returns servo angles over time."""
    
    robotShape = {'L1': 23.0, 'L2': 55.0, 'L3': 59.0}
    leg = RobotLeg(0, 50, robotShape)
    
    # Simulation parameters
    total_frames = duration * fps
    time = np.linspace(0, duration, total_frames)

    # Initial position
    x_center = 10
    y_center = 99
    z_center = 10

    # Preallocate arrays for servo angles
    servoA = np.zeros_like(time)
    servoB = np.zeros_like(time)
    servoC = np.zeros_like(time)

    # Generate a sinusoidal trajectory for the leg
    for i, t in enumerate(time):
        x = x_center + amplitude * np.sin(2 * np.pi * t)
        y = y_center
        z = z_center + amplitude * np.cos(2 * np.pi * t)
        servoA[i], servoB[i], servoC[i] = leg.calculateAngles(x, y, z)

    return time, servoA, servoB, servoC

def plot_servo_angles(time, servoA, servoB, servoC):
    """Plots the servo angles over time."""
    
    plt.figure(figsize=(10, 6))

    plt.plot(time, servoA, label='Servo A (Root)')
    plt.plot(time, servoB, label='Servo B (Thigh)')
    plt.plot(time, servoC, label='Servo C (Calf)')

    plt.xlabel('Time (s)')
    plt.ylabel('Servo Angle (degrees)')
    plt.title('Simulated Servo Angles During Leg Motion')
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Run the simulation
    time, servoA, servoB, servoC = simulate_motion()
    
    # Plot the results
    plot_servo_angles(time, servoA, servoB, servoC)




"""
Main areas for improvement
* Add multiple legs so there are 4 plots of multiple legs
* make the simulation to be used for walking
* make the inverse kinematics more precise

"""