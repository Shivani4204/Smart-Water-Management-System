Engineering Troubleshooting & Resolution Log
This document records the technical challenges encountered during the development of the ESP32 Smart Water Management System and the engineering solutions implemented to resolve them.

1. Issue: Serial Communication "Command Lag"
Symptom: The motor would start on the 'y' command but fail to stop consistently on the 'n' command.

Root Cause Analysis: The ESP32 was "flooding" the Serial buffer with sensor data strings every few milliseconds. The high frequency of Serial.print commands created a race condition where the incoming RX (receive) buffer was being ignored or overwritten by the outbound TX (transmit) priority.

Resolution:

Implemented Non-blocking Timing using millis().

Restricted data transmission to a 500ms interval, clearing the CPU to listen for user interrupts.

Switched logic to Serial.readStringUntil('\n') with .trim() to handle hidden whitespace characters that were corrupting the command logic.

2. Issue: Conductive Contamination (Water Spill)
Symptom: System reported D: 0.00cm and constant Amp: 3385 despite the motor being physically disconnected.

Root Cause Analysis: An accidental fluid ingress occurred on the breadboard. Water, being conductive, created a "low-resistance bridge" between the 5V power rail and the high-impedance Analog-to-Digital Converter (ADC) pins. This caused the ESP32 to read "ghost" voltages instead of actual sensor signals.

Resolution:

Hardware Migration: The system was immediately powered down, and components were migrated to a dry quadrant of the breadboard.

Mesh Desiccation: Used forced-air drying on the Ultrasonic sensor mesh to clear moisture trapped in the acoustic transducers.

Software Timeout: Added a 30,000µs timeout to the pulseIn() function to prevent the code from hanging (freezing) when a sensor is shorted or non-responsive.

3. Issue: Motor Inhibit (Screen "OFF" while Motor "ON")
Symptom: The Serial Monitor displayed Motor: OFF while the pump was physically running.

Root Cause Analysis: This was a State Synchronization error. The code was reporting the intended state of the variable rather than the actual voltage state of the GPIO pin.

Resolution:

Feedback Loop: Modified the display logic to use digitalRead(PIN_MOTOR_IN1). By reading the actual electrical state of the output pin, the "Digital Twin" (the software) now perfectly reflects the "Physical Asset" (the hardware).

4. Issue: Vibration Sensor "Static" Feedback
Symptom: Sensor reported Vib: OFF even when the motor was active.

Root Cause Analysis: The high-speed rotation of the motor was producing micro-vibrations outside the default sensitivity range of the SW-420 module.

Resolution:

Mechanical Coupling: Secured the sensor directly to the pump housing using high-tension fasteners (zip-ties) to improve kinetic energy transfer.

Threshold Tuning: Calibrated the onboard comparator via the potentiometer to detect the specific frequency of the 12V DC motor.
