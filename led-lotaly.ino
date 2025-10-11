#define PIN_A 2
#define PIN_B 3
const int8_t Encoder_TABLE[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
volatile bool StetePinA = 1;
volatile bool StetePinB = 1;
volatile uint8_t State = 0;
volatile long Count = 0;

void setup() {
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
 
  attachInterrupt(0, ChangePinAB, CHANGE);
  attachInterrupt(1, ChangePinAB, CHANGE);
  Serial.begin(9600);
}

void loop() {
  Serial.println(update_value());
  delay(10);
}
void ChangePinAB(){
  StatePinA = PIND & 0b00000100;
  StatePinB = PIND & 0b00001000;
  State = (State<<1) + StatePinA;
  State = (State<<1) + StatePinB;
  State = State & 0b00001111;
  Count += ENCODER_TABLE[State];
}
void update_value(){
  noInterrupts();
  value = Count;
  interrupts();
  Return value;
}
