#define PIN_A 2
#define PIN_B 3
// On-board LED (most Arduino boards)
#define LED_PIN 13
// Lookup table for rotary encoder state transitions
const int8_t ENCODER_TABLE[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
volatile uint8_t statePinA = 1;
volatile uint8_t statePinB = 1;
volatile uint8_t State = 0; // 4-bit state history index
volatile long Count = 0;
// Blinking/speed measurement settings
const unsigned long SPEED_SAMPLE_MS = 20;       // how often we sample encoder to compute speed (shortened for faster response)
const unsigned long BLINK_MIN_MS = 20;          // fastest blink interval (shortened)
const unsigned long BLINK_MAX_MS = 1000;        // slowest blink interval
const unsigned long MAX_SPEED_FOR_MAP = 3000;   // steps per second that maps to fastest blink (increased for wider range)

// runtime state for blinking
unsigned long lastSpeedCalc = 0;
long lastCount = 0;
unsigned long currentBlinkInterval = BLINK_MAX_MS;
unsigned long lastBlinkToggle = 0;
bool ledState = LOW;

void setup() {
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  // Use digitalPinToInterrupt for portability
  attachInterrupt(digitalPinToInterrupt(PIN_A), ChangePinAB, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B), ChangePinAB, CHANGE);
  Serial.begin(9600);
  // initialize speed sampling baseline
  lastSpeedCalc = millis();
  lastCount = update_value();
}

void loop() {
  unsigned long now = millis();

  // periodically calculate rotation speed (steps per second)
  if (now - lastSpeedCalc >= SPEED_SAMPLE_MS) {
    long cnt = update_value();
    long delta = cnt - lastCount;
    unsigned long elapsed = now - lastSpeedCalc;
    // steps per second (integer)
    unsigned long speedPerSec = (unsigned long)(abs(delta) * 1000UL / (elapsed ? elapsed : 1));
    lastCount = cnt;
    lastSpeedCalc = now;

    // map speed to blink interval (higher speed -> shorter interval)
    unsigned long mapped = map((unsigned long)speedPerSec, 0UL, MAX_SPEED_FOR_MAP, BLINK_MAX_MS, BLINK_MIN_MS);
    // constrain to allowed range
    if (mapped < BLINK_MIN_MS) mapped = BLINK_MIN_MS;
    if (mapped > BLINK_MAX_MS) mapped = BLINK_MAX_MS;
    currentBlinkInterval = mapped;

    // debug
    Serial.print("speed:"); Serial.print(speedPerSec);
    Serial.print(" cps, blink(ms):"); Serial.println(currentBlinkInterval);
  }

  // non-blocking blink according to currentBlinkInterval
  if (now - lastBlinkToggle >= currentBlinkInterval) {
    lastBlinkToggle = now;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
  }
}
void ChangePinAB(){
  // Read pins (0 or 1)
  statePinA = digitalRead(PIN_A) ? 1 : 0;
  statePinB = digitalRead(PIN_B) ? 1 : 0;
  // Build 4-bit index: previous two bits + current two bits
  State = ((State << 2) | (statePinA << 1) | statePinB) & 0x0F;
  Count += ENCODER_TABLE[State];
}
long update_value(){
  long value;
  noInterrupts();
  value = Count;
  interrupts();
  return value;
}
