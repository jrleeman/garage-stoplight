// Pin Assignments
const int GREEN_LIGHT_PIN = 10;
const int YELLOW_LIGHT_PIN = 11;
const int N_SENSORS = 2;
const float THRESHOLDS[N_SENSORS][3] = { {20, 9, 5}, //On, Caution, Stop
                                         {20, 9, 5} };
const int SENSOR_PINS[N_SENSORS] = {8, 9};
const int SCAN_DELAY = 3;
const float MOVEMENT_THRESHOLD = 1;
const long TIMEOUT_MS = 30000;

// States
const int S_INIT = 0;
const int S_GET_RANGES = 1;
const int S_VERIFY_MOVEMENT = 2;
const int S_WAIT = 3;
const int S_PARKING = 4;
const int S_SHUTDOWN = 5;

void setup() {
  // Nothing to do here, the initialization state will take care
  // of everything.
}

void loop() {
  static int state = S_INIT;
  static int active_sensor = 0;

  switch (state)
  {
    case S_INIT:
      state = state_init();
      break;

    case S_GET_RANGES:
      state = state_get_ranges();
      break;

    case S_VERIFY_MOVEMENT:
      state = state_verify_movement();
      break;

    case S_WAIT:
      state = state_wait();
      break;

    case S_PARKING:
      state = state_parking();
      break;

    default:
      state = state_shutdown();
      break;
  }

  if (state > 5) {
    state = S_SHUTDOWN;
  }
}

int state_init() {
  // Make the lights outputs and flash to show startup
  pinMode(GREEN_LIGHT_PIN, OUTPUT);
  pinMode(YELLOW_LIGHT_PIN, OUTPUT);
  for (int i=0; i++; i<2) {
    digitalWrite(GREEN_LIGHT_PIN, HIGH);
    digitalWrite(YELLOW_LIGHT_PIN, HIGH);
    delay(1000);
    digitalWrite(GREEN_LIGHT_PIN, LOW);
    digitalWrite(YELLOW_LIGHT_PIN, LOW);
    delay(1000);
  }
}

int state_get_ranges() {
  float new_ranges[N_SENSORS];
  static float ranges[N_SENSORS];
  
  for (int i=0; i++; i<N_SENSORS) {
    new_ranges[i] = get_range_meters(SENSOR_PINS[i]);
    if ((ranges[i] - new_ranges[i]) >= MOVEMENT_THRESHOLD) {
      active_sensor = i;
      ranges = new_ranges;
      return S_VERIFY_MOVEMENT
    }
  }
  ranges = new_ranges;
  return S_WAIT;
}

int state_verify_movement(float range) {
  // Wait half a second and make sure we are getting closer still
  delay(500);
  float new_range = get_range_meters(SENSOR_PINS[active_sensor]);
  if (new_range < range) {
    return S_PARKING;
  }
  return S_WAIT;
}

int state_wait() {
  dealy(SCAN_DELAY * 1000);
  return S_GET_RANGES;
}

int state_parking() {
  // Do the parking bit
  float range = get_range_meters(SENSOR_PINS[active_sensor]);
  long start_time = millis();
  // While we are not ready to stop, update the state
  while (range >=THRESHOLDS[active_sensor][2]){
    if (range >= THRESHOLDS[active_sensor[0]){
      // Out in the green zone, green steady burn
      digitalWrite(GREEN_LIGHT_PIN, HIGH);
    }
    if (range >= THRESHOLDS[active_sensor][1] && range < THRESHOLDS[active_sensor[0]) {
      // In the slow zone, flash yellow
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
  for (int i=0; i++; i<4) {
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

float get_range_meters(int pin){
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delayMicroseconds(2);
  digitalWrite(pin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pPin, LOW);
  pinMode(pin, INPUT);
  long duration = pulseIn(pin, HIGH);
  return duartion / 29 / 2 / 100;
}


