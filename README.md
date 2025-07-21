## General Info
Demo (progress): https://youtu.be/7Ph1gEYXCcs
Threaded Ripple, created at Undercity, is a draft of a fluid simulation pendant (inspired by [mitxela](https://www.youtube.com/watch?v=jis1MC5Tm8k)). Ours is different, though, because:
- one pendant uses a singular round TFT screen instead of a series of LEDs
- we also planned for it to connect to other pendants via Bluetooth, sending a ripple animation for cute interactivity (we could not finish implementing this, reasons down below)

---

Date: July 11 2025

We decided to brainstorm today for what we should make for this opportunity. We decided to go with a fluid simulated necklace that relies on metaballs. We decided to go with this because jewelry can make or break an outfit, and everyone wants to look good. So, this pendant will give one elegance and confidence! Plus, it will be very cute and we like making cute stuff!

Date: July 12 2025

We finished the firmware in CircuitPython. This was based upon research we conducted on metaballs. The main source to our research was: https://www.cg.tuwien.ac.at/research/publications/2022/Lackner_2022/Lackner_2022-Bachelor%20thesis.pdf. 

Our Firmware(took wayy too many hours): 

import board, time, displayio, busio
from adafruit_st7789 import ST7789
from math import floor, sqrt
from array import array


# Grid dimensions
H = 16
W = 16


# Display resolution
SCREEN_W = 240
SCREEN_H = 240


# Fluid simulation parameters
dt = 0.02
radius = 5.0
inv_radius = 1.0 / radius
surface_tension = 0.5
damping = 0.98
N_PRESSURE_ITERS = 2


# Buffers
velocityX = [[0.0 for _ in range(W)] for _ in range(H)]
velocityY = [[0.0 for _ in range(W)] for _ in range(H)]
velocityX_new = [[0.0 for _ in range(W)] for _ in range(H)]
velocityY_new = [[0.0 for _ in range(W)] for _ in range(H)]
pressure = [[0.0 for _ in range(W)] for _ in range(H)]
divergence = [[0.0 for _ in range(W)] for _ in range(H)]
phi = [[0.0 for _ in range(W)] for _ in range(H)]


# Setup display (ST7789 example — adapt if needed)
spi = busio.SPI(clock=board.SCK, MOSI=board.MOSI)
tft_cs = board.D5
tft_dc = board.D6
display_bus = displayio.FourWire(spi, command=tft_dc, chip_select=tft_cs)
display = ST7789(display_bus, width=SCREEN_W, height=SCREEN_H, rotation=0)
bitmap = displayio.Bitmap(SCREEN_W, SCREEN_H, 65536)
palette = displayio.Palette(1)
tilegrid = displayio.TileGrid(bitmap, pixel_shader=palette)
group = displayio.Group()
group.append(tilegrid)
display.show(group)


# Metaball structure
class Metaball:
    def __init__(self, x, y, mass=1.0, ax=0.0, ay=0.0):
        self.posX = x
        self.posY = y
        self.mass = mass
        self.accelX = ax
        self.accelY = ay


# Bilinear interpolation sampling
def sample(f, y, x):
    x0 = max(0, min(W - 1, int(floor(x))))
    x1 = max(0, min(W - 1, x0 + 1))
    y0 = max(0, min(H - 1, int(floor(y))))
    y1 = max(0, min(H - 1, y0 + 1))
    sx = x - x0
    sy = y - y0
    v00 = f[y0][x0]
    v10 = f[y0][x1]
    v01 = f[y1][x0]
    v11 = f[y1][x1]
    return (v00 + sx * (v10 - v00)) + sy * ((v01 + sx * (v11 - v01)) - (v00 + sx * (v10 - v00)))


# Compute phi field from metaballs
def compute_phi(metaballs):
    for y in range(H):
        for x in range(W):
            val = 0.0
            for mb in metaballs:
                dx = x - mb.posX
                dy = y - mb.posY
                dist = sqrt(dx * dx + dy * dy)
                if dist < radius:
                    val += 1.0 - dist * inv_radius
            phi[y][x] = val


# Fluid simulation step
def fluid_step(metaballs):
    compute_phi(metaballs)


    # Step 1: Advect velocity
    for y in range(H):
        for x in range(W):
            floatX = x - dt * velocityX[y][x]
            floatY = y - dt * velocityY[y][x]
            velocityX_new[y][x] = sample(velocityX, floatY, floatX)
            velocityY_new[y][x] = sample(velocityY, floatY, floatX)


    # Step 2: Apply surface tension force
    for y in range(1, H - 1):
        for x in range(1, W - 1):
            laplacian = phi[y][x + 1] + phi[y][x - 1] + phi[y + 1][x] + phi[y - 1][x] - 4.0 * phi[y][x]
            gx = 0.5 * (phi[y][x + 1] - phi[y][x - 1])
            gy = 0.5 * (phi[y + 1][x] - phi[y - 1][x])
            magnitude = sqrt(gx * gx + gy * gy) + 1e-6  # Prevent divide by 0
            nx = gx / magnitude
            ny = gy / magnitude
            force = surface_tension * laplacian
            velocityX_new[y][x] += dt * force * nx
            velocityY_new[y][x] += dt * force * ny


    # Step 3: Apply forces from metaballs
    for mb in metaballs:
        startX = max(0, int(mb.posX - radius))
        endX = min(W, int(mb.posX + radius))
        startY = max(0, int(mb.posY - radius))
        endY = min(H, int(mb.posY + radius))
        for y in range(startY, endY):
            for x in range(startX, endX):
                dx = x - mb.posX
                dy = y - mb.posY
                dist = sqrt(dx * dx + dy * dy)
                if dist < radius:
                    falloff = 1.0 - dist * inv_radius
                    velocityX_new[y][x] += dt * mb.accelX * mb.mass * falloff
                    velocityY_new[y][x] += dt * mb.accelY * mb.mass * falloff


    # Step 4: Compute divergence
    for y in range(1, H - 1):
        for x in range(1, W - 1):
            divergence[y][x] = 0.5 * (
                velocityX_new[y][x + 1] - velocityX_new[y][x - 1] +
                velocityY_new[y + 1][x] - velocityY_new[y - 1][x]
            )


    # Step 5: Pressure solve
    for _ in range(N_PRESSURE_ITERS):
        for y in range(1, H - 1):
            for x in range(1, W - 1):
                pressure[y][x] = 0.25 * (
                    pressure[y][x + 1] + pressure[y][x - 1] +
                    pressure[y + 1][x] + pressure[y - 1][x] -
                    divergence[y][x]
                )


    # Step 6: Subtract pressure gradient and apply damping
    for y in range(1, H - 1):
        for x in range(1, W - 1):
            velocityX_new[y][x] -= 0.5 * (pressure[y][x + 1] - pressure[y][x - 1])
            velocityY_new[y][x] -= 0.5 * (pressure[y + 1][x] - pressure[y - 1][x])
            velocityX_new[y][x] *= damping
            velocityY_new[y][x] *= damping


    # Swap velocity buffers
    for y in range(H):
        for x in range(W):
            velocityX[y][x] = velocityX_new[y][x]
            velocityY[y][x] = velocityY_new[y][x]


# Render the phi field as color
def render(metaballs):
    for y in range(SCREEN_H):
        simY = int((y * H) / SCREEN_H)
        for x in range(SCREEN_W):
            simX = int((x * W) / SCREEN_W)
            val = phi[simY][simX]
            if val <= 0.2:
                alpha = 0.0
            elif val >= 0.5:
                alpha = 1.0
            else:
                alpha = (val - 0.2) / (0.5 - 0.2)
            r = int(50 * alpha)
            g = int(180 * alpha)
            b = int(255 * alpha)
            color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            bitmap[x, y] = color


# Main loop
metaballs = [Metaball(8.0, 8.0)]
while True:
    fluid_step(metaballs)
    render(metaballs)
    time.sleep(0.05)


We also started working on the schematic around midnight, and one of us started designing the 3d printed case for our pendant. We wanted to make sure it was cute!

<img width="653" height="490" alt="image" src="https://github.com/user-attachments/assets/ed75d0d2-b006-40e6-9592-4f5c64371a50" />

<img width="456" height="298" alt="image" src="https://github.com/yummCat/threadedRipple/blob/main/img/CAD_firstDraft.png?raw=true" />

But then we decided to go with cardboard after looking at the long list of individuals who signed up to 3d print.

<img width="385" height="684" alt="image" src="https://raw.githubusercontent.com/yummCat/threadedRipple/refs/heads/main/img/cardboard_alternative.jpg" />

# Date: July 13/14

We found out that the main reason that all the components did not work together was due to an issue with our microcontroller—through disassembling the whole model. One thing we wished we had done was to consider other options for microcontrollers, their pros and cons, and then decide on the best one.



# BOM: 
Component | Number
- Micro Servo - 1
- ProMicro NRF52840 - 1
- Buttons - 2
- ADXL345 - 1
- Lithium Ion battery 3V- 2
- TP4056 - 1
- Wires

