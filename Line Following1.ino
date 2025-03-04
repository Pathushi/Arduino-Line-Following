// Define motor driver pins
#define ENA 3
#define IN1 4
#define IN2 5

#define ENB 6
#define IN3 7
#define IN4 8

// Define IR sensor pins
#define IR_LEFT 9
#define IR_RIGHT 10

// Define ultrasonic sensor pins
#define TRIG_PIN 11
#define ECHO_PIN 12

// Variable to track T-junction status
bool atTJunction = false;

// Function to read distance from ultrasonic sensor
long getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout after 30ms
    if (duration == 0) return 9999;  // No valid reading, return large value

    long distance = (duration / 2) / 29.1;  // Convert to cm
    return distance;
}

// Function to move forward with obstacle detection
void moveForward() {
    long distance = getDistance();
    
    if (distance <= 10 && distance > 0) {  // Stop if obstacle detected within 10 cm
        stopMotors();
        Serial.println("Obstacle detected! Stopping.");
        return;
    }

    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 60);
    analogWrite(ENB, 60);
}

// Functions for line corrections
void moveLeftCorrection() {
    Serial.println("Correcting Left...");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 50);
    analogWrite(ENB, 50);
    delay(100);
}

void moveRightCorrection() {
    Serial.println("Correcting Right...");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, 50);
    analogWrite(ENB, 50);
    delay(100);
}

// Stop motors
void stopMotors() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
}

// Handle T-junction
void handleTJunction() {
    stopMotors();
    delay(300);  // Small delay to confirm it's a T-junction
    if (digitalRead(IR_LEFT) == HIGH && digitalRead(IR_RIGHT) == HIGH) {
        Serial.println("T-Junction detected! Waiting for signal...");
        atTJunction = true;
    }
}

// Execute turn based on command
void executeTurn(char signal) {
    if (signal == 'R') {
        moveRight();
    } else if (signal == 'L') {
        moveLeft();
    } else if (signal == 'G') {
        moveForward();
        delay(500);
        stopMotors();
        adjustLine();
    }
    atTJunction = false;
}

// Adjust alignment after turns
void adjustLine() {
    Serial.println("Adjusting line...");
    while (digitalRead(IR_LEFT) == HIGH || digitalRead(IR_RIGHT) == HIGH) {
        if (digitalRead(IR_LEFT) == HIGH && digitalRead(IR_RIGHT) == LOW) {
            moveRightCorrection();
        } else if (digitalRead(IR_LEFT) == LOW && digitalRead(IR_RIGHT) == HIGH) {
            moveLeftCorrection();
        }
    }
    Serial.println("Line adjusted!");
    moveForward();
}

// Move left and correct alignment
void moveLeft() {
    Serial.println("Turning Left...");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, 150);
    analogWrite(ENB, 150);

    delay(1000); 
    stopMotors();
    delay(100);
    correctLeftTurn();
    
    moveForward();
    delay(200);
    stopMotors();
    adjustLine();
}

// Move right and correct alignment
void moveRight() {
    Serial.println("Turning Right...");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, 150);
    analogWrite(ENB, 150);

    delay(1000);
    stopMotors();
    delay(100);
    correctRightTurn();
    
    moveForward();
    delay(200);
    stopMotors();
    adjustLine();
}

// Correct right turn alignment
void correctRightTurn() {
    Serial.println("Correcting Right Turn...");
    while (digitalRead(IR_RIGHT) == LOW) {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        analogWrite(ENA, 150);
        analogWrite(ENB, 150);
    }
    stopMotors();
    adjustLine();
}

// Correct left turn alignment
void correctLeftTurn() {
    Serial.println("Correcting Left Turn...");
    while (digitalRead(IR_LEFT) == LOW) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        analogWrite(ENA, 150);
        analogWrite(ENB, 150);
    }
    stopMotors();
    adjustLine();
}

void setup() {
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    pinMode(IR_LEFT, INPUT);
    pinMode(IR_RIGHT, INPUT);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    Serial.begin(9600);
}

void loop() {
    int leftSensor = digitalRead(IR_LEFT);
    int rightSensor = digitalRead(IR_RIGHT);

    if (!atTJunction) {
        if (leftSensor == LOW && rightSensor == LOW) {
            moveForward();
        } else if (leftSensor == HIGH && rightSensor == LOW) {
            moveRightCorrection();
        } else if (leftSensor == LOW && rightSensor == HIGH) {
            moveLeftCorrection();
        } else if (leftSensor == HIGH && rightSensor == HIGH) {
            handleTJunction();  // Properly detect and stop at T-junction
        } else {
            stopMotors();
        }
    }

    if (atTJunction && Serial.available() > 0) {
        char signal = Serial.read();
        if (signal == 'R' || signal == 'L' || signal == 'G') {
            Serial.print("Received: ");
            Serial.println(signal);
            executeTurn(signal);
        }
    }
}
