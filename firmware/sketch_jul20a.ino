#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <TFT_eSPI.h>

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240
#define SCREEN_CENTER_X (SCREEN_WIDTH/2)
#define SCREEN_CENTER_Y (SCREEN_HEIGHT/2)
#define SCREEN_RADIUS   (SCREEN_WIDTH/2)

#define NUM_PARTICLES    80
#define PARTICLE_RADIUS  4

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

TFT_eSPI tft = TFT_eSPI();

struct Particle {
  float x, y;
  float vx, vy;
};
Particle particles[NUM_PARTICLES];

float gx = 0, gy = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  if (!accel.begin()) {
    Serial.println("Failed to find ADXL345");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_4_G);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].x  = random(SCREEN_CENTER_X - 20, SCREEN_CENTER_X + 20);
    particles[i].y  = random(SCREEN_CENTER_Y - 20, SCREEN_CENTER_Y + 20);
    particles[i].vx = 0;
    particles[i].vy = 0;
  }
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);
  gx = event.acceleration.x / 9.8;
  gy = -event.acceleration.y / 9.8;

  tft.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_BLACK);

  for (int i = 0; i < NUM_PARTICLES; i++) {
    auto &p = particles[i];
    
    p.vx = p.vx * 0.96 + gx * 0.08;
    p.vy = p.vy * 0.96 + gy * 0.08;
    
    p.x += p.vx;
    p.y += p.vy;
    
    float dx = p.x - SCREEN_CENTER_X;
    float dy = p.y - SCREEN_CENTER_Y;
    float dist = sqrt(dx*dx + dy*dy);
    if (dist > SCREEN_RADIUS - PARTICLE_RADIUS) {
      float nx = dx / dist;
      float ny = dy / dist;
      float dot = p.vx*nx + p.vy*ny;
      p.vx -= 2*dot*nx;
      p.vy -= 2*dot*ny;
      p.x = SCREEN_CENTER_X + nx*(SCREEN_RADIUS - PARTICLE_RADIUS);
      p.y = SCREEN_CENTER_Y + ny*(SCREEN_RADIUS - PARTICLE_RADIUS);
    }

    tft.fillCircle((int)p.x, (int)p.y, PARTICLE_RADIUS, TFT_CYAN);
  }

  delay(16);
}
