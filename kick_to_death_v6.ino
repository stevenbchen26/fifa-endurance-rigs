// ============================================================
// Kick-to-Death Solenoid Endurance Test -- v6
// LoR Core V3 / ESP32
// Humber Polytechnic -- FIFA Bot Builder Project
// ============================================================
//
// WIRING:
//   Solenoid MOSFET driver signal  ->  IO slot 1 / GPIO32
//
//   E18-D80NK proximity sensor:
//     Brown  ->  5V  on LoR Core V3
//     Blue   ->  GND on LoR Core V3
//     Black  ->  IO slot 10 / GPIO4
//
// CYCLE:
//   Solenoid ON  for  50ms
//   Solenoid OFF for 950ms
//   Total period = 1000ms (1 Hz)
//   Duty cycle   = 5%  --  well under the 30% solenoid limit
//
// FAILURE DETECTION:
//   After each kick the sensor is polled for SENSOR_WINDOW_MS.
//   If the ball passes the sensor beam, output goes LOW = hit.
//   If MISS_FAIL_THRESHOLD consecutive kicks produce no detection,
//   the test halts and reports the final hit count to Serial.
//
// SERIAL OUTPUT:
//   Baud rate: 115200
//   Prints hit count every REPORT_EVERY successful hits.
//   Prints each miss event with running miss count.
//   Prints final summary on failure detection.
// ============================================================

#include "esp_task_wdt.h"

// ============================================================
//   Pin Definitions
// ============================================================
#define KICK_PIN    32   // IO slot 1  / GPIO32  --  solenoid MOSFET driver signal
#define SENSOR_PIN   4   // IO slot 10 / GPIO4   --  E18-D80NK output (direct connection)

// ============================================================
//   Timing
// ============================================================
#define KICK_ON_MS        50   // How long the solenoid fires per kick (ms)
#define KICK_OFF_MS      950   // How long between kicks (ms) -- cycle = 1000ms total
#define SENSOR_WINDOW_MS 400   // How long to watch for ball detection after each kick (ms)
                               // Must be less than KICK_OFF_MS

// ============================================================
//   Failure Detection
// ============================================================
#define MISS_FAIL_THRESHOLD  5   // Consecutive missed detections before test halts

// ============================================================
//   Reporting
// ============================================================
#define REPORT_EVERY  10   // Print hit count to Serial every N successful hits

// ============================================================
//   Watchdog
// ============================================================
#define WDT_TIMEOUT_SEC  5   // ESP32 resets if loop hangs longer than this

// ============================================================
//   Setup
// ============================================================
void setup()
{
  Serial.begin(115200);
  delay(500);

  // Watchdog protects against firmware hangs during long unattended runs
  esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
  esp_task_wdt_add(NULL);

  // Solenoid output -- start LOW so solenoid is off at boot
  pinMode(KICK_PIN, OUTPUT);
  digitalWrite(KICK_PIN, LOW);

  // Sensor input -- INPUT_PULLUP holds the pin HIGH at idle
  // The E18-D80NK pulls the line LOW when it detects the ball
  pinMode(SENSOR_PIN, INPUT_PULLUP);

  Serial.println("============================================");
  Serial.println("  Kick-to-Death Solenoid Endurance Test");
  Serial.println("  LoR Core V3 / ESP32");
  Serial.println("============================================");
  Serial.printf( "  Solenoid : IO slot 1  / GPIO%d\n", KICK_PIN);
  Serial.printf( "  Sensor   : IO slot 10 / GPIO%d\n", SENSOR_PIN);
  Serial.printf( "  Cycle    : %dms ON / %dms OFF (%.0f%% duty)\n",
                 KICK_ON_MS, KICK_OFF_MS,
                 (float)KICK_ON_MS / (float)(KICK_ON_MS + KICK_OFF_MS) * 100.0f);
  Serial.printf( "  Window   : %dms post-kick sensor watch\n", SENSOR_WINDOW_MS);
  Serial.printf( "  Threshold: %d consecutive misses = failure\n", MISS_FAIL_THRESHOLD);
  Serial.printf( "  Reporting: every %d successful hits\n", REPORT_EVERY);
  Serial.println("============================================");
  Serial.println("  Starting in 3 seconds...");
  Serial.println("============================================");

  delay(3000);
  Serial.println("GO.");
}

// ============================================================
//   Main Loop
// ============================================================
void loop()
{
  static unsigned long hitCount          = 0;
  static int           consecutiveMisses = 0;

  esp_task_wdt_reset();

  // ----------------------------------------------------------
  // Fire solenoid
  // ----------------------------------------------------------
  digitalWrite(KICK_PIN, HIGH);
  delay(KICK_ON_MS);
  digitalWrite(KICK_PIN, LOW);

  // ----------------------------------------------------------
  // Watch for ball detection within the sensor window
  // The sensor output goes LOW when the ball passes the beam
  // ----------------------------------------------------------
  bool          ballDetected = false;
  unsigned long windowStart  = millis();

  while (millis() - windowStart < SENSOR_WINDOW_MS)
  {
    esp_task_wdt_reset();

    if (digitalRead(SENSOR_PIN) == LOW)
    {
      ballDetected = true;
      break;
    }

    delay(5);  // Poll every 5ms -- fast enough to catch a moving ball
  }

  // ----------------------------------------------------------
  // Evaluate result
  // ----------------------------------------------------------
  if (ballDetected)
  {
    hitCount++;
    consecutiveMisses = 0;

    if (hitCount % REPORT_EVERY == 0)
    {
      Serial.printf("Hits: %lu\n", hitCount);
    }
  }
  else
  {
    consecutiveMisses++;
    Serial.printf("MISS %d/%d  (total hits: %lu)\n",
                  consecutiveMisses, MISS_FAIL_THRESHOLD, hitCount);

    if (consecutiveMisses >= MISS_FAIL_THRESHOLD)
    {
      // Ensure solenoid is off before halting
      digitalWrite(KICK_PIN, LOW);

      Serial.println("============================================");
      Serial.println("  FAILURE DETECTED");
      Serial.printf( "  Ball did not reach sensor for %d consecutive kicks.\n",
                     MISS_FAIL_THRESHOLD);
      Serial.printf( "  Total successful hits: %lu\n", hitCount);
      Serial.println("  Test halted. Power cycle to restart.");
      Serial.println("============================================");

      // Hold here indefinitely -- keep feeding watchdog
      while (true)
      {
        esp_task_wdt_reset();
        delay(1000);
      }
    }
  }

  // ----------------------------------------------------------
  // Wait out the remainder of the OFF period
  // SENSOR_WINDOW_MS was already spent above, subtract it
  // ----------------------------------------------------------
  int remainingMs = KICK_OFF_MS - SENSOR_WINDOW_MS;
  if (remainingMs > 0)
  {
    esp_task_wdt_reset();
    delay(remainingMs);
  }
}
