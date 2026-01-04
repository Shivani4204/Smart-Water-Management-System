**Installation and Setup Guide**

This document provides a step-by-step guide to assembling the hardware
and configuring the software environment for the ESP32 Smart Water
Management System.

**1. Hardware Assembly**

**Wiring Map**

To ensure system stability, follow the pin mapping defined in
src/config.h.

  -------------------------- --------------- ----------------------------
                                             

                                             

                                             

                                             

                                             

                                             

                                             

                                             
  -------------------------- --------------- ----------------------------

Export to Sheets

**Critical Safety: Common Ground**

**Warning:** The L298N motor driver is powered by a 12V DC adapter,
while the ESP32 is powered by 5V USB. You **must**connect a jumper wire
between the **GND** pin of the L298N and a **GND** pin on the ESP32.
Without this \"Common Ground,\" the logic signals will be unstable, and
the motor may fail to stop.

**2. Software Environment Setup**

**Required Software**

1.  **Arduino IDE** (v2.0 or higher recommended).

2.  **ESP32 Board Package**:

    -   Open Arduino IDE \> Settings \> Additional Boards Manager URLs.

        > Add:
        > https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

        > Go to Boards Manager and install **\"esp32 by Espressif
        > Systems\"**.

**Library Dependencies**

This project is designed with low-level logic to minimize memory
overhead; however, ensure your IDE is configured for the **ESP32 Dev
Module**.

**3. Deployment Steps**

1.  **Connect the ESP32** to your workstation via a high-quality
    > Micro-USB data cable.

2.  **Open the Project**: Load src/esp32_water_system.ino in the Arduino
    > IDE. Ensure config.h is in the same folder tab.

3.  **Select Port**: Go to Tools \> Port and select the active port
    > (e.g., /dev/cu.usbserial-1410).

4.  **Configure Serial Monitor**:

    -   Set Baud Rate to **115200**.

        > Set Line Ending to **Newline**.

5.  **Upload**: Click the Upload button. Once \"Done Uploading\"
    > appears, the system will begin broadcasting diagnostic data.

**4. Post-Installation Verification**

Once the code is running, verify the installation by entering the
following commands in the Serial Monitor:

-   Type **\'y\'**: Verify motor rotation and that Vib: ACT appears.

    > Type **\'n\'**: Verify immediate motor cessation.

    > **Obstacle Test**: Place an object 10cm from the ultrasonic sensor
    > and verify the D: 10.00cm output.
