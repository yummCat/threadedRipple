## General Info
Demo (progress): https://youtu.be/7Ph1gEYXCcs
Threaded Ripple, created at Undercity, is a draft of a fluid simulation pendant (inspired by [mitxela](https://www.youtube.com/watch?v=jis1MC5Tm8k)). Ours is different, though, because:
- one pendant uses a singular round TFT screen instead of a series of LEDs
- uses metaballs, doesn't do FLIP sim
- it's a lot trashier :D
- we also planned for it to connect to other pendants via Bluetooth, sending a ripple animation for cute interactivity (we could not finish implementing this, reasons down below)

---

# BOM: 
Component | Number
- Micro Servo - 1
- Seeed Studio XIAO RP2040 - 1
- Buttons - 2
- ADXL345 - 1
- Lithium Ion battery 3V- 2
- TP4056 - 1
- Wires

(Previously, the ProMicro NRF52840 was used instead of the XIAO RP2040. But the NRF52840 wasn't connecting properly.)

# Date: July 11 2025

We decided to brainstorm today for what we should make for this opportunity. We decided to go with a fluid simulated necklace that relies on metaballs. We decided to go with this because jewelry can make or break an outfit, and everyone wants to look good. So, this pendant will give one elegance and confidence! Plus, it will be very cute and we like making cute stuff!

# Date: July 12 2025

We finished the firmware in CircuitPython. This was based upon research we looked at on metaballs. The main source was: https://www.cg.tuwien.ac.at/research/publications/2022/Lackner_2022/Lackner_2022-Bachelor%20thesis.pdf. 

We also started working on the schematic around midnight, and one of us started designing the 3d printed case for our pendant. We wanted to make sure it was cute!

<img width="653" height="490" alt="image" src="https://github.com/user-attachments/assets/ed75d0d2-b006-40e6-9592-4f5c64371a50" />

<img width="456" height="298" alt="image" src="https://github.com/yummCat/threadedRipple/blob/main/img/CAD_firstDraft.png?raw=true" />

But then we decided to go with cardboard after looking at the long list of individuals who signed up to 3d print.

<img width="385" height="684" alt="image" src="https://raw.githubusercontent.com/yummCat/threadedRipple/refs/heads/main/img/cardboard_alternative.jpg" />

# Date: July 13/14

We found out that the main reason that all the components did not work together was due to an issue with our microcontroller—through disassembling the whole model. One thing we wished we had done was to consider other options for microcontrollers, their pros and cons, and then decide on the best one.

# Date: July 31

Although the built project so far is encased in a cardboard case, we also made a two-layer case in Fusion 360. The pieces can be joined together without screws, and there is a hole to connect a string/necklace for the pendant.

![assembled side view](https://raw.githubusercontent.com/yummCat/threadedRipple/refs/heads/main/img/assembled_sideView.png)
![assembled top view](https://raw.githubusercontent.com/yummCat/threadedRipple/refs/heads/main/img/assembled_view1.png)
![bottom layer](https://raw.githubusercontent.com/yummCat/threadedRipple/refs/heads/main/img/bottomLayer.png)
![top layer](https://raw.githubusercontent.com/yummCat/threadedRipple/refs/heads/main/img/topLayer.png)



