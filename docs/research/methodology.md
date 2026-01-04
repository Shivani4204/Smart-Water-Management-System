System Methodology
The development of this system followed a Hardware-in-the-Loop (HIL) approach, focusing on real-time diagnostics and fail-safe automation.

1. Architectural Strategy

The system utilizes a Decentralized Control Logic where the ESP32 acts as the central processing unit, interfacing with three distinct data layers:

Actuation Layer: L298N H-Bridge controlling a 12V submersible pump.

Monitoring Layer: Ultrasonic (Level) and Vibration (Mechanical) sensors.

Safety/Diagnostic Layer: ACS712 Current sensor for Current Signature Analysis (CSA).

2. Empirical Calibration (The "Current Signature")

Rather than using simple float switches, this system identifies "Dry Run" states by measuring the motor's electrical load.

Hydraulic Resistance (Water): High torque requirement leads to a lower ADC value (~3337).

Air Resistance (Dry): Minimal torque leads to a higher ADC value (~3378).

Threshold: A logic boundary was set at 3300 ADC to trigger a safety shutdown when the pump sucks air.

3. Asynchronous Firmware Design

To prevent "Processor Hang," the firmware was written using Non-Blocking I/O:

Polling: Sensors are sampled every 500ms using millis() instead of delay().

Priority Interrupts: The Serial command buffer ('y'/'n') is checked every loop cycle to ensure the Manual Override always overrides the automation logic.

4. Failure Mode & Effects Analysis (FMEA)

During testing, the system encountered conductive contamination (moisture). This was mitigated by:

Software Timeouts: Preventing the pulseIn() function from freezing the CPU.

Hardware Isolation: Moving logic components to a dry quadrant and establishing a Common Ground to stabilize signal noise.
