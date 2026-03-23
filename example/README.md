# Hello World Example

This minimal example brings LVGL up on any supported board with three lines
of application code.

## Prerequisites

- PlatformIO (VS Code extension or `pio` CLI)
- The `esp32_lcd_controllers` submodule checked out at `ext/`

```
my_project/
├── platformio.ini        ← included here
├── lv_conf.h             ← copy from ../include/lv_conf_template.h
├── ext/
│   └── esp32_lcd_controllers/   ← this submodule
└── src/
    └── main.cpp          ← included here
```

## Setup

```bash
# 1. Clone your project and add the submodule
git submodule add https://github.com/your-org/esp32_lcd_controllers.git ext/esp32_lcd_controllers
git submodule update --init

# 2. Copy the LVGL config template
cp ext/esp32_lcd_controllers/include/lv_conf_template.h lv_conf.h

# 3. Populate the driver you need (if not esp32_2424s012):
#    cp cnc_interface/src/driver/guition_4848s040.cpp \
#       ext/esp32_lcd_controllers/src/driver/guition_4848s040.cpp
#    (see README.md "Driver migration guide")

# 4. Build and flash
pio run -e hello_4848s040 -t upload

# 5. Monitor serial output
pio device monitor
```

## What you will see

- Dark blue background
- "Hello, LVGL!" in red, centred
- A pulsing circle animation at the top
- Resolution displayed at the bottom

## Extending the example

- Override `mcu_setup()` to mount LittleFS or connect WiFi before LVGL starts.
- Override `lvgl_loop_hook()` for per-frame data binding.
- Replace the `my_ui()` callback with your own screen hierarchy.
