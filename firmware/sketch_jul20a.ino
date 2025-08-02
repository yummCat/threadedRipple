#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <TFT_eSPI.h>
#include <math.h>

// ----- Screen Constants -----
#define SCREEN_W    240
#define SCREEN_H    240
#define SCREEN_CX   (SCREEN_W / 2)
#define SCREEN_CY   (SCREEN_H / 2)
#define SCREEN_R    (SCREEN_W / 2)

// ----- Particle Constants -----
#define N_PARTICLES 5
#define P_R         25

// ----- Behavior Tunables -----
float COHESION_STRENGTH = 0.0002;  // larger → more cohesion
bool DRAW_OUTLINE_ONLY = false;    // false → fill entire blob, true → outline only

// ----- Metaball Field Resolution -----
uint8_t FIELD_RES       = 20;     // adjust for finer/coarser blobs
int      CELL_SIZE;               // computed in setup()
float    FIELD_THRESHOLD = 1.0;

// ----- Global Objects -----
Adafruit_ADXL345_Unified accel(12345);
TFT_eSPI tft = TFT_eSPI();
uint16_t framebuffer[SCREEN_W * SCREEN_H];

struct Particle { float x, y, vx, vy; } P[N_PARTICLES];

// ----- Interrupt Flag -----
volatile bool needUpdate = false;

// ----- ISR: Signal when accel triggers -----
void onAccelActivity() {
  needUpdate = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin();
  if (!accel.begin()) {
    Serial.println("❌ I am a POTATO");
    while (1) delay(100);
  }
  accel.setRange(ADXL345_RANGE_4_G);

  // Configure ADXL345 activity interrupt on INT2 (XIAO D2)
  accel.writeRegister(ADXL345_REG_THRESH_ACT,    0x10);
  accel.writeRegister(ADXL345_REG_ACT_INACT_CTL, 0x70);
  accel.writeRegister(ADXL345_REG_INT_MAP,       0x00);
  accel.writeRegister(ADXL345_REG_INT_ENABLE,    0x10);

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), onAccelActivity, RISING);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.startWrite();

  // Compute integer cell size
  CELL_SIZE = SCREEN_W / FIELD_RES;

  randomSeed(micros());
  for (int i = 0; i < N_PARTICLES; i++) {
    P[i].x  = random(SCREEN_CX - 20, SCREEN_CX + 20);
    P[i].y  = random(SCREEN_CY - 20, SCREEN_CY + 20);
    P[i].vx = 0;
    P[i].vy = 0;
  }

  needUpdate = true;  // draw initial frame
}

void loop() {
  if (!needUpdate) return;
  needUpdate = false;

  // 1) Read accelerometer
  sensors_event_t e;
  accel.getEvent(&e);
  float gx =  e.acceleration.x / 9.8;
  float gy = -e.acceleration.y / 9.8;

  // 2) Update particles with tunable cohesion
  for (int i = 0; i < N_PARTICLES; i++) {
    float cx = 0, cy = 0;
    int cnt = 0;
    for (int j = 0; j < N_PARTICLES; j++) {
      if (i == j) continue;
      float dx = P[j].x - P[i].x;
      float dy = P[j].y - P[i].y;
      float d2 = dx*dx + dy*dy;
      if (d2 < (2*P_R)*(2*P_R)) {
        cx += P[j].x;
        cy += P[j].y;
        cnt++;
      }
    }
    if (cnt) {
      cx /= cnt;
      cy /= cnt;
      P[i].vx += (cx - P[i].x) * COHESION_STRENGTH;
      P[i].vy += (cy - P[i].y) * COHESION_STRENGTH;
    }
    // friction + gravity
    P[i].vx = P[i].vx * 0.94 + gx * 0.12;
    P[i].vy = P[i].vy * 0.94 + gy * 0.12;
    P[i].x += P[i].vx;
    P[i].y += P[i].vy;
  }

  // 3) Collisions
  for (int i = 0; i < N_PARTICLES; i++) {
    for (int j = i + 1; j < N_PARTICLES; j++) {
      float dx = P[j].x - P[i].x;
      float dy = P[j].y - P[i].y;
      float d2 = dx*dx + dy*dy;
      if (d2 < (2*P_R)*(2*P_R) && d2 > 0.01f) {
        float d = sqrt(d2);
        float nx = dx/d, ny = dy/d;
        float p = (P[i].vx*nx + P[i].vy*ny - P[j].vx*nx - P[j].vy*ny);
        P[i].vx -= p*nx;
        P[i].vy -= p*ny;
        P[j].vx += p*nx;
        P[j].vy += p*ny;
        float overlap = (2*P_R - d)/2;
        P[i].x -= nx * overlap;
        P[i].y -= ny * overlap;
        P[j].x += nx * overlap;
        P[j].y += ny * overlap;
      }
    }
  }

  // 4) Boundary bounce
  for (int i = 0; i < N_PARTICLES; i++) {
    float dx = P[i].x - SCREEN_CX;
    float dy = P[i].y - SCREEN_CY;
    float dist = sqrt(dx*dx + dy*dy);
    if (dist > SCREEN_R - P_R) {
      float nx = dx/dist, ny = dy/dist;
      float dot = P[i].vx*nx + P[i].vy*ny;
      P[i].vx -= 2*dot * nx;
      P[i].vy -= 2*dot * ny;
      P[i].x = SCREEN_CX + nx*(SCREEN_R - P_R);
      P[i].y = SCREEN_CY + ny*(SCREEN_R - P_R);
    }
  }

  uint16_t c = tft.color565(255, 203, 192); // choosing color - BGR color NOT RGB :)
  // 5) Draw metaballs (outline or fill based on flag)
  memset(framebuffer, 0, sizeof(framebuffer));
  for (int gy_i = 0; gy_i < FIELD_RES; gy_i++) {
    for (int gx_i = 0; gx_i < FIELD_RES; gx_i++) {
      float px = gx_i * CELL_SIZE + CELL_SIZE * 0.5;
      float py = gy_i * CELL_SIZE + CELL_SIZE * 0.5;
      float field = 0;
      for (int i = 0; i < N_PARTICLES; i++) {
        float dx = px - P[i].x;
        float dy = py - P[i].y;
        float d2 = dx*dx + dy*dy;
        if (d2 > 0.1f) field += (P_R*P_R) / d2;
      }
      if (field <= FIELD_THRESHOLD) continue;

      bool draw = !DRAW_OUTLINE_ONLY;
      if (DRAW_OUTLINE_ONLY) {
        // check edge
        bool edge = false;
        const int dxs[4] = {1,-1,0,0};
        const int dys[4] = {0,0,1,-1};
        for (int k=0; k<4; k++) {
          int nx = gx_i + dxs[k];
          int ny = gy_i + dys[k];
          if (nx<0||ny<0||nx>=FIELD_RES||ny>=FIELD_RES) { edge = true; break; }
          float npx = nx*CELL_SIZE + CELL_SIZE*0.5;
          float npy = ny*CELL_SIZE + CELL_SIZE*0.5;
          float nfield=0;
          for (int i=0; i<N_PARTICLES; i++) {
            float ddx = npx - P[i].x;
            float ddy = npy - P[i].y;
            float nd2 = ddx*ddx + ddy*ddy;
            if (nd2>0.1f) nfield += (P_R*P_R)/nd2;
          }
          if (nfield <= FIELD_THRESHOLD) { edge = true; break; }
        }
        draw = edge;
      }
      if (!draw) continue;
      
      for (int by=0; by<CELL_SIZE; by++) {
        int yy = gy_i*CELL_SIZE + by;
        uint16_t* row = framebuffer + yy*SCREEN_W + gx_i*CELL_SIZE;
        for (int bx=0; bx<CELL_SIZE; bx++) row[bx]=c;
      }
    }
  }
  tft.pushImage(0,0,SCREEN_W,SCREEN_H,framebuffer);
}
