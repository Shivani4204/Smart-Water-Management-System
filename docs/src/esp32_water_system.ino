// ESP32 Smart Water Management System - CALIBRATED Version
// Calibrated with real sensor data for accurate operation

// ===== PIN DEFINITIONS =====
const int TRIG_PIN = 5;
const int ECHO_PIN = 18;
const int PUMP_IN1 = 26;
const int PUMP_IN2 = 27;
const int PUMP_ENA = 25;

const int FLOW_SENSOR_PIN = 19;
const int PRESSURE_SENSOR_PIN = 34;
const int VIBRATION_SENSOR_PIN = 21;
const int CURRENT_SENSOR_PIN = 35;

// ===== CALIBRATED THRESHOLDS (Your Real Values) =====
// Water Level Thresholds
const float WATER_EMPTY_THRESHOLD = 17.0;  // > 17cm = Tank almost EMPTY
const float WATER_FULL_THRESHOLD = 4.0;    // < 4cm = Tank almost FULL

// Pressure Thresholds (Analog readings)
const int PRESSURE_DRY_RUN = 3337;    // Motor running WITHOUT water (DANGER!)
const int PRESSURE_WITH_WATER = 2700; // Motor running WITH water (SAFE)
const int PRESSURE_MOTOR_OFF = 221;   // Motor OFF baseline

// ===== GLOBAL VARIABLES =====
volatile int flowPulses = 0;
float flowRate = 0.0;
float totalLiters = 0.0;
unsigned long lastFlowCheck = 0;
bool pumpRunning = false;
bool autoMode = true;
int rawPressure = 0;

// Data logging
unsigned long sessionStartTime = 0;
int pumpCycles = 0;
int dryRunEvents = 0;
int safetyStops = 0;

// ===== INTERRUPT FOR FLOW SENSOR =====
void IRAM_ATTR countFlow() {
  flowPulses++;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PUMP_IN1, OUTPUT);
  pinMode(PUMP_IN2, OUTPUT);
  pinMode(PUMP_ENA, OUTPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(VIBRATION_SENSOR_PIN, INPUT);
  pinMode(PRESSURE_SENSOR_PIN, INPUT);
  pinMode(CURRENT_SENSOR_PIN, INPUT);
  
  stopPump();
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), countFlow, FALLING);
  
  sessionStartTime = millis();
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║  ESP32 Water Management System v2.0   ║");
  Serial.println("║   Calibrated Dry-Run Detection        ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println("\n📊 CALIBRATED VALUES:");
  Serial.println("   Tank EMPTY: > 17 cm distance");
  Serial.println("   Tank FULL:  < 4 cm distance");
  Serial.println("   Dry Run Pressure: ~3337");
  Serial.println("   Normal Pressure: ~2700");
  Serial.println("\n📋 Commands:");
  Serial.println("  y - Start pump    | n - Stop pump");
  Serial.println("  a - Auto mode     | m - Manual mode");
  Serial.println("  s - Show status   | d - Data log (CSV)");
  Serial.println("  r - Reset stats   | c - Calibration info");
  Serial.println("\n🛡️  Safety Systems ACTIVE");
  Serial.println("✅ System Ready!\n");
}

void loop() {
  // Handle serial commands
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
    delay(10);
    while(Serial.available() > 0) Serial.read();
  }
  
  // Read all sensors
  float distance = getDistance();
  rawPressure = getRawPressure();
  float current = getCurrent();
  bool vibration = getVibration();
  
  // Calculate flow every second
  if (millis() - lastFlowCheck >= 1000) {
    calculateFlow();
    lastFlowCheck = millis();
  }
  
  // === CRITICAL SAFETY: DRY RUN DETECTION ===
  if (pumpRunning && rawPressure > 3000) {
    stopPump();
    dryRunEvents++;
    safetyStops++;
    Serial.println("\n🚨🚨🚨 CRITICAL ALERT 🚨🚨🚨");
    Serial.println("⚠️  DRY RUN DETECTED!");
    Serial.println("⚠️  Pump running WITHOUT water!");
    Serial.print("   Pressure: ");
    Serial.print(rawPressure);
    Serial.println(" (Expected: ~2700 with water)");
    Serial.println("   ACTION: Pump STOPPED to prevent damage");
    Serial.println("   Please check water supply before restart!\n");
  }
  
  // === AUTOMATIC WATER LEVEL CONTROL ===
  if (autoMode) {
    if (distance > 0) {
      // Tank EMPTY - Start filling
      if (distance > WATER_EMPTY_THRESHOLD && !pumpRunning) {
        startPump();
        pumpCycles++;
        Serial.println("\n💧 AUTO: Tank EMPTY (distance > 17cm)");
        Serial.print("   Current level: ");
        Serial.print(distance);
        Serial.println(" cm");
        Serial.println("   Starting pump to FILL tank...\n");
      }
      // Tank FULL - Stop filling
      else if (distance < WATER_FULL_THRESHOLD && pumpRunning) {
        stopPump();
        Serial.println("\n✅ AUTO: Tank FULL (distance < 4cm)");
        Serial.print("   Current level: ");
        Serial.print(distance);
        Serial.println(" cm");
        Serial.println("   Stopping pump - Tank is full!\n");
      }
    }
    
    // Additional safety checks
    if (vibration && pumpRunning) {
      stopPump();
      safetyStops++;
      Serial.println("\n⚠️  ALERT: Excessive vibration → STOPPED!");
    }
    
    if (current > 5.0 && pumpRunning) {
      stopPump();
      safetyStops++;
      Serial.println("\n⚠️  ALERT: Overcurrent detected → STOPPED!");
    }
  }
  
  // Print status every 3 seconds
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 3000) {
    printStatus(distance, rawPressure, current, vibration);
    lastPrint = millis();
  }
  
  delay(100);
}

// ===== COMMAND HANDLER =====
void handleCommand(char cmd) {
  switch(cmd) {
    case 'y':
    case 'Y':
      autoMode = false;
      startPump();
      pumpCycles++;
      Serial.println("\n✅ MANUAL: Pump STARTED");
      Serial.println("   Monitoring for dry-run condition...");
      break;
      
    case 'n':
    case 'N':
      autoMode = false;
      stopPump();
      Serial.println("\n🛑 MANUAL: Pump STOPPED");
      break;
      
    case 'a':
    case 'A':
      autoMode = true;
      Serial.println("\n🤖 AUTO mode ENABLED");
      Serial.println("   System will maintain water level automatically");
      Serial.println("   Empty threshold: > 17 cm");
      Serial.println("   Full threshold: < 4 cm");
      break;
      
    case 'm':
    case 'M':
      autoMode = false;
      Serial.println("\n👤 MANUAL mode ENABLED");
      Serial.println("   Use 'y' to start, 'n' to stop pump");
      break;
      
    case 's':
    case 'S':
      Serial.println("\n📊 DETAILED STATUS REQUEST...\n");
      break;
      
    case 'd':
    case 'D':
      printDataLog();
      break;
      
    case 'c':
    case 'C':
      printCalibrationInfo();
      break;
      
    case 'r':
    case 'R':
      totalLiters = 0;
      pumpCycles = 0;
      dryRunEvents = 0;
      safetyStops = 0;
      sessionStartTime = millis();
      Serial.println("\n🔄 All counters RESET");
      break;
  }
}

// ===== SENSOR FUNCTIONS =====

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  
  float dist = (duration * 0.034) / 2;
  if (dist < 2 || dist > 400) return -1;
  
  return dist;
}

int getRawPressure() {
  // Return RAW analog value (0-4095)
  return analogRead(PRESSURE_SENSOR_PIN);
}

void calculateFlow() {
  flowRate = (flowPulses / 7.5);
  totalLiters += (flowRate / 60.0);
  flowPulses = 0;
}

float getCurrent() {
  int raw = analogRead(CURRENT_SENSOR_PIN);
  float voltage = raw * (3.3 / 4095.0);
  float amps = abs((voltage - 2.5) / 0.185);
  return amps;
}

bool getVibration() {
  return digitalRead(VIBRATION_SENSOR_PIN) == HIGH;
}

// ===== PUMP CONTROL =====

void startPump() {
  analogWrite(PUMP_ENA, 255);
  digitalWrite(PUMP_IN1, HIGH);
  digitalWrite(PUMP_IN2, LOW);
  pumpRunning = true;
}

void stopPump() {
  analogWrite(PUMP_ENA, 0);
  digitalWrite(PUMP_IN1, LOW);
  digitalWrite(PUMP_IN2, LOW);
  delay(50);
  digitalWrite(PUMP_IN1, LOW);
  digitalWrite(PUMP_IN2, LOW);
  pumpRunning = false;
}

// ===== STATUS DISPLAY =====

void printStatus(float dist, int press, float curr, bool vibe) {
  Serial.println("╔═══════════════ STATUS ═══════════════╗");
  
  // Mode and pump status
  Serial.print("║ Mode: ");
  Serial.print(autoMode ? "AUTO    " : "MANUAL  ");
  Serial.print(" | Pump: ");
  Serial.println(pumpRunning ? "🟢 ON      ║" : "🔴 OFF     ║");
  
  // Water level with status
  Serial.print("║ Water Level: ");
  if (dist > 0) {
    Serial.print(dist, 1);
    Serial.print(" cm ");
    if (dist > WATER_EMPTY_THRESHOLD) {
      Serial.println("⚠️  EMPTY    ║");
    } else if (dist < WATER_FULL_THRESHOLD) {
      Serial.println("✅ FULL     ║");
    } else {
      Serial.println("        ║");
    }
  } else {
    Serial.println("ERROR            ║");
  }
  
  // Pressure with water status
  Serial.print("║ Pressure: ");
  Serial.print(press);
  Serial.print(" ");
  if (pumpRunning) {
    if (press > 3000) {
      Serial.println("🚨 DRY!    ║");
    } else if (press > 2500) {
      Serial.println("✅ WATER   ║");
    } else {
      Serial.println("          ║");
    }
  } else {
    Serial.println("(OFF)      ║");
  }
  
  // Flow data
  Serial.print("║ Flow Rate: ");
  Serial.print(flowRate, 1);
  Serial.println(" L/min          ║");
  
  Serial.print("║ Total Pumped: ");
  Serial.print(totalLiters, 1);
  Serial.println(" L           ║");
  
  // Current draw
  Serial.print("║ Current: ");
  Serial.print(curr, 2);
  Serial.println(" A              ║");
  
  // Vibration
  Serial.print("║ Vibration: ");
  Serial.println(vibe ? "⚠️  DETECTED     ║" : "✅ Normal        ║");
  
  // Session stats
  Serial.print("║ Pump Cycles: ");
  Serial.print(pumpCycles);
  Serial.print(" | Dry Runs: ");
  Serial.print(dryRunEvents);
  Serial.println("      ║");
  
  Serial.println("╚══════════════════════════════════════╝\n");
}

// ===== DATA LOGGING =====

void printDataLog() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║         DATA LOG (CSV FORMAT)          ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("Timestamp,WaterLevel_cm,Pressure_Raw,Current_A,FlowRate_Lmin,TotalVolume_L,PumpStatus,WaterStatus");
  
  // Current snapshot
  float dist = getDistance();
  int press = getRawPressure();
  float curr = getCurrent();
  
  unsigned long runtime = (millis() - sessionStartTime) / 1000;
  
  Serial.print(runtime);
  Serial.print(",");
  Serial.print(dist, 2);
  Serial.print(",");
  Serial.print(press);
  Serial.print(",");
  Serial.print(curr, 2);
  Serial.print(",");
  Serial.print(flowRate, 2);
  Serial.print(",");
  Serial.print(totalLiters, 2);
  Serial.print(",");
  Serial.print(pumpRunning ? "ON" : "OFF");
  Serial.print(",");
  
  if (pumpRunning && press > 3000) {
    Serial.println("DRY_RUN");
  } else if (pumpRunning && press > 2500) {
    Serial.println("WATER_FLOW");
  } else {
    Serial.println("IDLE");
  }
  
  Serial.println("\n📊 Session Summary:");
  Serial.print("   Runtime: ");
  Serial.print(runtime / 60);
  Serial.println(" minutes");
  Serial.print("   Pump Cycles: ");
  Serial.println(pumpCycles);
  Serial.print("   Dry Run Events: ");
  Serial.println(dryRunEvents);
  Serial.print("   Safety Stops: ");
  Serial.println(safetyStops);
  Serial.print("   Total Water Pumped: ");
  Serial.print(totalLiters, 1);
  Serial.println(" L\n");
}

void printCalibrationInfo() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║       CALIBRATION INFORMATION          ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("🔧 WATER LEVEL (Ultrasonic Distance):");
  Serial.println("   Tank EMPTY: > 17.0 cm (sensor far from water)");
  Serial.println("   Tank FULL:  < 4.0 cm (sensor close to water)");
  Serial.println("   Logic: Greater distance = Less water\n");
  
  Serial.println("🔧 PRESSURE SENSOR (Analog Reading):");
  Serial.println("   Motor OFF: ~221 (baseline)");
  Serial.println("   Dry Run: ~3337 (motor ON, NO water)");
  Serial.println("   With Water: ~2700 (motor ON, water flowing)");
  Serial.println("   Safety: Stops if > 3000 during operation\n");
  
  Serial.println("🔧 CURRENT SENSOR:");
  Serial.println("   Idle: ~0.1 A");
  Serial.println("   Running: ~1.8-4.5 A");
  Serial.println("   Overcurrent threshold: > 5.0 A\n");
  
  Serial.println("📊 Current Readings:");
  float dist = getDistance();
  int press = getRawPressure();
  float curr = getCurrent();
  
  Serial.print("   Distance: ");
  Serial.print(dist);
  Serial.println(" cm");
  Serial.print("   Pressure: ");
  Serial.println(press);
  Serial.print("   Current: ");
  Serial.print(curr);
  Serial.println(" A\n");
}
