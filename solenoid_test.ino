// ============================================================
// Solenoid Test
// LoR Core V3 / ESP32
// ============================================================
//
// WIRING:
//   Solenoid MOSFET driver signal  ->  IO slot 12 / GPIO21
//
// CYCLE:
//   Solenoid ON  for  50ms
//   Solenoid OFF for 950ms
//   Total period = 1000ms (1 Hz)
//   Duty cycle   = 5%  --  well under the 30% solenoid limit
//
// SERIAL OUTPUT:
//   Baud rate: 115200
//   Prints hit count every 10 kicks
// ============================================================

#include "esp_task_wdt.h"

// ============================================================
//   Pin Definitions
// ============================================================
#define KICK_PIN  21   // IO slot 12 / GPIO21  --  solenoid MOSFET driver signal

// ============================================================
//   Timing
// ============================================================
#define KICK_ON_MS   50    // How long the solenoid fires per kick (ms)
#define KICK_OFF_MS  950   // How long between kicks (ms) -- cycle = 1000ms total

// ============================================================
//   Reporting
// ============================================================
#define REPORT_EVERY  10   // Print hit count to Serial every N kicks

// ============================================================
//   Watchdog
// ============================================================
#define WDT_TIMEOUT_SEC  5

// ============================================================
//   Setup
// ============================================================
void setup()
{
  Serial.begin(115200);
  delay(500);

  esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
  esp_task_wdt_add(NULL);

  pinMode(KICK_PIN, OUTPUT);
  digitalWrite(KICK_PIN, LOW);

  Serial.println("============================================");
  Serial.println("  Solenoid Endurance Test");
  Serial.println("  LoR Core V3 / ESP32");
  Serial.println("============================================");
  Serial.printf( "  Solenoid : IO slot 12 / GPIO%d\n", KICK_PIN);
  Serial.printf( "  Cycle    : %dms ON / %dms OFF (%.0f%% duty)\n",
                 KICK_ON_MS, KICK_OFF_MS,
                 (float)KICK_ON_MS / (float)(KICK_ON_MS + KICK_OFF_MS) * 100.0f);
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
  static unsigned long hitCount = 0;

  esp_task_wdt_reset();

  // --- Fire ---
  digitalWrite(KICK_PIN, HIGH);
  delay(KICK_ON_MS);
  digitalWrite(KICK_PIN, LOW);

  hitCount++;

  if (hitCount % REPORT_EVERY == 0)
  {
    Serial.printf("Hits: %lu\n", hitCount);
  }

  // --- Wait remainder of cycle ---
  esp_task_wdt_reset();
  delay(KICK_OFF_MS);
}
