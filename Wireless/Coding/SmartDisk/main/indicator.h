// the loop function runs over and over again forever

#include <math.h>

int neopixelSetRGBHue(float hue) {
  float red = 0.0;
  float green = 0.0;
  float blue = 0.0;
  float realHue = hue * 5;

  if (hue > 3) {
    red = RGB_BRIGHTNESS;
    blue = RGB_BRIGHTNESS;
    green = RGB_BRIGHTNESS;
    //  Serial.printf("hue:white");
  } else {
    // Serial.printf("hue: %0.2f , realHue : %0.1f  \n\r", hue, realHue);
    //red
    float red_center = 1;
    float red_width = 4;
    if (abs((realHue - red_center)) < (red_width / 2.0)) {
      red = RGB_BRIGHTNESS * (1 + ((red_width / 2.0) - abs((realHue - red_center))) / (red_width / 2.0));
    }
    //blue
    float blue_center = 5;
    float blue_width = 3;
    if (abs((realHue - blue_center)) < (blue_width / 2.0)) {
      blue = RGB_BRIGHTNESS * (1 + ((blue_width / 2.0) - abs((realHue - blue_center))) / (blue_width / 2.0));
    }
    //green
    float green_center = 3;
    float green_width = 4;
    if (abs((realHue - green_center)) < (green_width / 2.0)) {
      green = RGB_BRIGHTNESS * (1 + ((green_width / 2.0) - abs((realHue - green_center))) / (green_width / 2.0));
    }
  }
  // Serial.printf("rgb(%d,%d,%d)", (int)red, (int)green, (int)blue);
  neopixelWrite(RGB_BUILTIN, (int)red, (int)green, (int)blue);
  return 1;
}

// test the rgb led
void testRGB() {
  Serial.println("testing RGB LED");
  digitalWrite(RGB_BUILTIN, HIGH);  // Turn the RGB LED white
  delay(1000);
  digitalWrite(RGB_BUILTIN, LOW);  // Turn the RGB LED off
  delay(1000);

  for (int i = 0; i < 100; i++) {
    // Serial.println();
    // Serial.printf("the value of i is : %.2f", (i / 100.00));
    // Serial.println();
    int bug = neopixelSetRGBHue((i / 100.00));
    bug = bug + 2;
    delay(10);
  }

  neopixelWrite(RGB_BUILTIN, 0, 0, 0);  // Off / black
  delay(1000);
}
