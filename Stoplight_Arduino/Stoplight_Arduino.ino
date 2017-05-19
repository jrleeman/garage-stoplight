// Pin Assignments
const int GREEN_LIGHT_PIN = 6;
const int YELLOW_LIGHT_PIN = 7;
const int N_SENSORS = 2;
const float THRESHOLDS[N_SENSORS][3] = { {0.7, 0.5, 0.2}, //On, Caution, Stop
                                         {0.7, 0.5, 0.2} };
const int SENSOR_PINS[N_SENSORS][2] = { {8, 9},
                                        {10, 11}};
const int SCAN_DELAY = 1;
const float MOVEMENT_THRESHOLD = 0.1;
const long TIMEOUT_MS = 30000;

// States
const int S_INIT = 0;
const int S_GET_RANGES = 1;
const int S_VERIFY_MOVEMENT = 2;
const int S_WAIT = 3;
const int S_PARKING = 4;
const int S_SHUTDOWN = 5;

// Variables
int active_sensor = 0;
float ranges[N_SENSORS];

void setup() {
  // Nothing to do here, the initialization state will take care
  // of everything.
  Serial.begin(115200); // Useful for debugging
}

void loop() {
  static int state = S_INIT;
  for (int i=0; i<N_SENSORS; i++) {
    Serial.print("RANGE ARRAY ");
    Serial.println(ranges[i]);
  }
  switch (state)
  {
    case S_INIT:
      state = state_init();
      Serial.println("State: Init");
      break;

    case S_GET_RANGES:
      state = state_get_ranges();
      Serial.println("State: Get Ranges");
      break;

    case S_VERIFY_MOVEMENT:
      state = state_verify_movement();
      Serial.println("State: Verify Movement");
      break;

    case S_WAIT:
      state = state_wait();
      Serial.println("State: Wait");
      break;

    case S_PARKING:
      state = state_parking();
      Serial.println("State: Parking");
      break;

    default:
      state = state_shutdown();
      Serial.println("State: Shutdown");
      break;
  }

  if (state > 5) {
    state = S_SHUTDOWN;
  }
}

int state_init() {
  Serial.println("IN INIT FUNC");
  // Setup the trigger and echo pins for each sensor
  for (int i=0; i<N_SENSORS; i++) {
    Serial.println(i);
    pinMode(SENSOR_PINS[i][0], OUTPUT);
    pinMode(SENSOR_PINS[i][1], INPUT);
  }
  
  // Make the lights outputs and flash to show startup
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  pinMode(YELLOW_LIGHT_PIN, OUTPUT);
  for (int i=0; i<2; i++) {
    Serial.println(i);
    Serial.println("FLASH");
    digitalWrite(GREEN_LIGHT_PIN, HIGH);
    digitalWrite(YELLOW_LIGHT_PIN, HIGH);
    delay(1000);
    digitalWrite(GREEN_LIGHT_PIN, LOW);
    digitalWrite(YELLOW_LIGHT_PIN, LOW);
    delay(1000);
  }
  return S_GET_RANGES;
}

int state_get_ranges() {
  float new_ranges[N_SENSORS];
  for (int i=0; i<N_SENSORS; i++) {
    new_ranges[i] = get_range_meters(SENSOR_PINS[i]);
    Serial.println(ranges[i] - new_ranges[i]);
    if ((ranges[i] - new_ranges[i]) >= MOVEMENT_THRESHOLD) {
      active_sensor = i;
      memcpy(ranges, new_ranges, sizeof(ranges));
      return S_VERIFY_MOVEMENT;
    }
  }
   memcpy(ranges, new_ranges, sizeof(ranges));
  return S_WAIT;
}

int state_verify_movement() {
  // Wait half a second and make sure we are getting closer still
  delay(500);
  float new_range = get_range_meters(SENSOR_PINS[active_sensor]);
  Serial.print("VERIFY: ");
  Serial.println(ranges[active_sensor]);
  Serial.println(new_range);
  if (new_range < ranges[active_sensor]) {
    return S_PARKING;
  }
  return S_WAIT;
}

int state_wait() {
  delay(SCAN_DELAY * 1000);
  return S_GET_RANGES;
}

int state_parking() {
  // Do the parking bit
  float range = get_range_meters(SENSOR_PINS[active_sensor]);
  long start_time = millis();
  // While we are not ready to stop, update the state
  while (range >=THRESHOLDS[active_sensor][2]){
    range = get_range_meters(SENSOR_PINS[active_sensor]);
    if (range <= THRESHOLDS[active_sensor][0] && range > THRESHOLDS[active_sensor][1]){
      // Out in the green zone, green steady burn
      Serial.println("Green zone");
      digitalWrite(GREEN_LIGHT_PIN, HIGH);
    }
    if (range <= THRESHOLDS[active_sensor][1] && range > THRESHOLDS[active_sensor][2]) {
      // In the slow zone, flash yellow
      Serial.println("Slow zone");
      digitalWrite(GREEN_LIGHT_PIN, LOW);
      digitalWrite(YELLOW_LIGHT_PIN, HIGH);
      delay(500);
      digitalWrite(YELLOW_LIGHT_PIN, LOW);
      delay(500);
   }

   
   // Check to make sure we haven't timed out. If we have, break out.
   if (millis() - start_time > TIMEOUT_MS) {
    break;
   }
  }

  // We're done and in the red zone, flash both lights, turn off
  for (int i=0; i<4; i++) {
    digitalWrite(GREEN_LIGHT_PIN, HIGH);
    digitalWrite(YELLOW_LIGHT_PIN, HIGH);
    delay(200);
    digitalWrite(GREEN_LIGHT_PIN, LOW);
    digitalWrite(YELLOW_LIGHT_PIN, LOW);
    delay(200);
  }
  return S_WAIT;
}

int state_shutdown() {
  // Turn the lights off and set all pins to inputs. A safe state.
  digitalWrite(GREEN_LIGHT_PIN, LOW);
  digitalWrite(YELLOW_LIGHT_PIN, LOW);
  pinMode(GREEN_LIGHT_PIN, INPUT);
  pinMode(YELLOW_LIGHT_PIN, INPUT);

  for (int pin=SENSOR_PINS[0]; pin++; pin<N_SENSORS) {
    pinMode(pin, INPUT);
  }
}

float get_range_meters(int pins[]){
  Serial.println(pins[0]);
  Serial.println(pins[1]);
  pinMode(pins[0], OUTPUT);
  pinMode(pins[1], INPUT);
  digitalWrite(pins[0], LOW);
  delayMicroseconds(5);
  digitalWrite(pins[0], HIGH);
  delayMicroseconds(10);
  digitalWrite(pins[0], LOW);
  long duration = pulseIn(pins[1], HIGH);
  Serial.print("Range : ");
  Serial.println(duration /29.1/2/100);
  return duration / 29.1 / 2 / 100;
}


