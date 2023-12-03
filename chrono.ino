#include <Adafruit_GFX.h> //Library for using the I2C screen
#include <Adafruit_SSD1306.h> //Library for using the I2C screen

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint32_t timeStamp[2] = {0,0};
bool hit[2] = {false,false};
uint16_t dartCount = 0;
float velocitySum = 0.0f;
uint16_t velocityMax = 0.0f;
uint16_t velocityMin = 0.0f;

void firstTrigger()
{
  if (!hit[0])
  {
    timeStamp[0] = micros();
    //Serial.println(analogRead(0));
    hit[0] = true;
  }
}

void secondTrigger()
{
  if (hit[0] && !hit[1])
  {
    timeStamp[1] = micros();
    //Serial.println(analogRead(1));
    hit[1] = true;
  }
}

uint16_t getTextShift(uint16_t value)
{
  uint16_t ret = 0;
  ret += value <  10 ? 1 : 0;
  ret += value < 100 ? 1 : 0;
  return ret;
}

void printDisplay(uint16_t velocity, uint16_t velocityAvg)
{
  uint16_t col = 88;
  uint16_t col2 = col+18;
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(col, 1);
  display.print("Ct:");
  display.setCursor(col2+getTextShift(dartCount)*6, 1);
  display.print(dartCount);
  display.setCursor(col, 9);
  display.print("Av:");
  display.setCursor(col2+getTextShift(velocityAvg)*6, 9);
  display.print(velocityAvg);
  display.setCursor(col, 17);
  display.print("Mn:");
  display.setCursor(col2+getTextShift(velocityMin)*6, 17);
  display.print(velocityMin);
  display.setCursor(col, 25);
  display.print("Mx:");
  display.setCursor(col2+getTextShift(velocityMax)*6, 25);
  display.print(velocityMax);
  display.display();
  delay(200);

  uint16_t shift = 8;
  if (velocity < 1000)
  {
    display.setTextSize(4);
    shift += getTextShift(velocity) * 24;
    display.setCursor(shift, 5);
    display.print(velocity);
  }
  else
  {
    display.setTextSize(3);
    display.setCursor(shift, 8);
    display.print(velocity);
  }
  display.display();
}

void setup()
{
  Serial.begin(115200); //Serieller Monitor
  Serial.println("\nStarting up");
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  attachInterrupt(digitalPinToInterrupt(3), firstTrigger, FALLING);
  attachInterrupt(digitalPinToInterrupt(2), secondTrigger, FALLING);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.display();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();     // Clear display
  display.setTextColor(SSD1306_WHITE);
  Serial.println("Startup done");
  printDisplay(0, 0);
}

void loop()
{
  if (hit[0] && !hit[1])
  {
    if (micros() - timeStamp[0] > 500000)
    {
      hit[0] = false;
    }
  }
  if(dartCount && analogRead(0) < 50 && analogRead(1) < 50)
  {
    delay(500);
    if (analogRead(0) < 50 && analogRead(1) < 50)
    {
      display.clearDisplay();
      display.setTextSize(3);
      display.setCursor(22, 8);
      display.print("reset");
      display.display();
      Serial.println("Resetting dart counter");
      Serial.println(analogRead(0));
      Serial.println(analogRead(1));
      dartCount = 0;
      velocitySum = 0;
      velocityMin = 0;
      velocityMax = 0;
      delay(3000);
      printDisplay(0, 0);
      hit[0] = false;
      hit[1] = false;
    }
  }
  if (hit[0] && hit[1])
  {
    float delta = static_cast<float>(timeStamp[1] - timeStamp[0]) * 0.000001;
    float velocity = (0.1/delta)*3.28;
    dartCount += 1;
    velocitySum += velocity;
    uint16_t velocityAvg = round(velocitySum / static_cast<float>(dartCount));
    if (velocity > velocityMax) velocityMax = velocity;
    if (velocity < velocityMin || velocityMin == 0) velocityMin = velocity;
    printDisplay(velocity, velocityAvg);
    delay(250);
    hit[0] = false;
    hit[1] = false;
  }

}
