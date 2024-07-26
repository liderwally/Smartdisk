
#define LED_WEB 45
#define LED_PROC 46
#define LED_USB 47


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 45 as an output.
  pinMode(LED_WEB, OUTPUT);
  pinMode(LED_PROC, OUTPUT);
  pinMode(LED_USB, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_WEB, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_WEB, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
    digitalWrite(LED_PROC, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_PROC, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);  
    digitalWrite(LED_USB, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_USB, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);  
}
