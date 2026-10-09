# Motion Sword

![The foam Minecraft sword, breadboard and parts on the desk next to Minecraft](images/sword_setup.jpg)

A motion-controlled Minecraft sword built for StormHacks. Swing a real (foam) sword and your character attacks in Minecraft.

The sword pairs with your laptop as a **Bluetooth mouse**, so there are no Minecraft mods and nothing to install on the laptop. Every hard swing sends a left click.

| With the sword | Laptop sees | In Minecraft |
| --- | --- | --- |
| Swing hard | Left click | Attack |

## What you need

- ESP32 dev board (30-pin, USB-C, CP2102)
- MPU6050 (GY-521) accelerometer/gyro
- Breadboard and jumper wires (male-to-male and female-to-male)
- 9V battery + clip (into **VIN**), or a USB power bank
- Something sword-shaped to mount it on. Ours is cut from black, teal and brown craft foam and held together with duct tape.
- Optional: a joystick module (used in the first version, see [The build process](#the-build-process))

## Quick start

1. **Install the ESP32 boards in the Arduino IDE.** Go to *Settings*, add `https://espressif.github.io/arduino-esp32/package_esp32_index.json` to *Additional boards manager URLs*, then install **esp32 by Espressif Systems** in the Boards Manager.
2. **Install the Bluetooth mouse library.** Download the ZIP from [T-vK/ESP32-BLE-Mouse](https://github.com/T-vK/ESP32-BLE-Mouse), then use *Sketch > Include Library > Add .ZIP Library*.
3. **Wire it up** using the table below.
4. **Upload.** Open `sword_wireless/sword_wireless.ino`, select **ESP32 Dev Module** and your USB port, and click Upload.
5. **Pair.** On your laptop, open the Bluetooth settings and connect to **Minecraft Sword**.
6. **Play.** Open Minecraft, hold a sword, and swing.

Open the Serial Monitor at **115200 baud** to see `SWING DETECTED!` every time a swing registers.

## Wiring

| From | To |
| --- | --- |
| ESP32 3V3 | + rail |
| ESP32 GND | − rail |
| MPU6050 VCC / GND | + rail / − rail |
| MPU6050 SDA | D21 |
| MPU6050 SCL | D22 |
| 9V battery red / black | VIN / GND (never 3V3 or the + rail) |

## The build process

We didn't get this working on the first try. Here's how the sword was put together, including what didn't work, so you can skip our mistakes.

### Step 1: Plan the pins

We started by deciding which ESP32 pins to use. D21 and D22 are the ESP32's default I2C pins, which the motion sensor uses. D32, D34 and D35 were set aside for the joystick, because D34 and D35 are input-only pins that can read analog values.

![ESP32 pins used](images/step2_esp32.png)

### Step 2: Power rails

Two short jumpers connect the ESP32's **3V3** and **GND** to the breadboard's + and − rails. Everything else gets power from these rails.

![Power rails](images/step3_power.png)

### Step 3: Add the motion sensor

The MPU6050 gets power from the rails, with SDA going to D21 and SCL to D22. Before writing any game code, we uploaded a small test sketch and watched the readings in the Arduino **Serial Plotter**. When the sensor is still, the total acceleration sits around 9.8 (gravity), and when you swing it spikes well above that. That spike is the "swing".

![MPU6050 wired up](images/step4_mpu.png)

### Step 4: First version, with Wi-Fi, a joystick, and a laptop script

The first full version did a lot:

- **Swing** to attack, **hold the sword sideways** to block with a shield, the **joystick** to walk with W/A/S/D, and **click the joystick** to jump.
- The ESP32 sent text messages over Wi-Fi (through a phone hotspot) to `bridge/sword_bridge.py`, a Python script on the laptop that pressed the keys and mouse buttons.
- It used the Adafruit MPU6050 library to read the sensor.

![Full wiring with joystick](images/step5_full.png)

### Step 5: The sensor wouldn't show up

This is where we got stuck. The Adafruit library kept reporting that it **couldn't find the MPU6050**, even with correct wiring. Many cheap GY-521 boards use a clone or similar chip that reports a different ID than the library expects, and the library refuses to talk to it.

The fix was to **skip the library** and talk to the sensor directly over I2C. We wake it up by writing to its power register, then read the raw acceleration numbers ourselves. It only takes a few lines, and it works with clone boards.

### Step 6: Simplify to a Bluetooth mouse

With the clock running out, we cut the project down to what mattered most: **swing to attack**. Instead of Wi-Fi plus a Python script, the ESP32 now acts as a Bluetooth mouse. That means:

- No hotspot and no laptop script, just pair it like any mouse.
- Fewer things to break during a demo.

The joystick, blocking and the Wi-Fi bridge aren't in the current sketch. The Python bridge is still in `bridge/`, and the full Wi-Fi firmware is in the git history if you want to bring them back:

```bash
git show 33a2488:sword_wireless/sword_wireless.ino
```

### Step 7: Go wireless with a battery

To swing without a USB cable, a 9V battery clip goes into **VIN** and **GND**. VIN runs through the board's voltage regulator, so it's safe for 9V. **Never** connect the battery to 3V3 or the + rail, or you'll fry the sensor and the board.

![9V battery into VIN](images/step7_battery.png)

Finally, we taped the breadboard and sensor to the foam sword so the sensor moves with the blade.

## Tuning and troubleshooting

| Problem | Try this |
| --- | --- |
| Have to swing really hard | Lower `swingThreshold` in `sword_wireless.ino` (for example to `12000`) |
| Clicks when you're not swinging | Raise `swingThreshold`, or turn the sensor so its X axis isn't pointing straight down (gravity alone reads about 16384 on that axis) |
| One swing gives several hits | Increase the `delay(300)` cooldown after a click |
| "Minecraft Sword" doesn't show up in Bluetooth | Press the ESP32's EN/reset button, and remove any old pairing of the sword from your laptop |
| Nothing happens even though it's paired | Open the Serial Monitor at 115200 baud and check the MPU6050 wiring (SDA → D21, SCL → D22) |
| Library won't compile | The BLE Mouse library can have trouble with the newest ESP32 board package. Try installing esp32 version 2.0.x in the Boards Manager |

## Files

| Path | What it is |
| --- | --- |
| `sword_wireless/sword_wireless.ino` | The sword firmware: reads the MPU6050 and sends a Bluetooth left click on each swing |
| `bridge/sword_bridge.py` | Laptop script from the first, Wi-Fi version (swing, block, joystick). Not needed for the Bluetooth version |
| `images/` | Photo of the build and wiring diagrams for each step |
