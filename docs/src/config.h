/*
 * Smart Water Management System - Configuration Header
 * This file contains all calibrated hardware thresholds 
 * and pin assignments to ensure clean, maintainable code.
 */

#ifndef CONFIG_H
#define CONFIG_H

// --- 1. PIN ASSIGNMENTS ---
// Ultrasonic Sensor
#define PIN_TRIG 5
#define PIN_ECHO 18

// L298N Motor Driver
#define PIN_MOTOR_IN1 26
#define PIN_MOTOR_IN2 27

// Diagnostic Sensors
#define PIN_VIBRATION 25
#define PIN_CURRENT 34   // ACS712 Analog Pin
#define PIN_AIR_OUT 35   // HX710B Data Pin

// --- 2. CALIBRATED THRESHOLDS ---
// Distances in Centimeters
#define DIST_FULL 5.0
#define DIST_EMPTY 17.0

// Current (Amp) Logic based on 3337(Water) vs 3378(Air)
// Values > 3300 = Empty/Air Running
// Values < 3300 = Water Load Detected
#define AMP_SYSTEM_THRESHOLD 3300 

// --- 3. TIMING & SAFETY ---
#define SERIAL_BAUD 115200
#define SENSOR_READ_DELAY 400   // Time between data scrolls (ms)
#define ULTRASONIC_TIMEOUT 30000 // Prevents code freeze if sensor is wet

#endif
