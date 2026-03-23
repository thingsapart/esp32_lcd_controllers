# esp32_lcd_controllers

A reusable ESP32 board-support package that gets LVGL running on any of the supported displays with a single `lcd_controllers_init_and_start()` call.

Designed to be added as a **git submodule** to any PlatformIO / Arduino-ESP32 project.

---

Information to get the boards running collected from various sources.

---

## Supported boards

| Name | MCU | Display | Size | Bus | Touch | Build flag |
|------|-----|---------|------|-----|-------|------------|
| Guition / Sunton 4848S040 | ESP32-S3 (16 MB / 8 MB OPI) | ST7701S | 480×480 | RGB parallel | GT911 I2C | `-D GUITION_4848S040=1` |
| Waveshare / Sunton 2424S012 | ESP32-C3 (4 MB / no PSRAM) | GC9A01 | 240×240 round | SPI | CST816S I2C | `-D ESP32_2424S012=1` |
| Makerfabs / Guition JC3248W535C | ESP32-S3 (8 MB / 8 MB OPI) | AXS15231B | 480×320 | QSPI | AXS15231B I2C | `-D JC3248W535C=1` |
| Wireless-Tag WT32-SC01 Plus | ESP32-S3 (16 MB / 8 MB OPI) | ST7796 | 480×320 | I80 8-bit | FT6336 I2C | `-D WT32_SC01_PLUS=1` |
| Jingcai JC1060P470 | ESP32-P4 (16 MB / 8 MB OPI) | JD9165 | 1024×600 | MIPI-DSI | GT911 I2C | `-D JC1060P470=1` |
| Jingcai JC8012P4A1 | ESP32-P4 (16 MB / 8 MB OPI) | JD9365 | 800×1280 | MIPI-DSI | GSL3680 I2C | `-D JC8012P4A1=1` |

---

## Repository layout

```
esp32_lcd_controllers/
├── esp32_lcd_controllers.ini   ← PlatformIO env fragments (include from your project)
├── library.json                ← PlatformIO library metadata
├── README.md                   ← this file
├── include/
│   └── lcd_controllers.h       ← public API (2 functions)
├── src/
│   ├── lcd_controllers.cpp     ← init + start implementation
│   ├── lvgl_task.cpp/h         ← minimal standalone LVGL FreeRTOS task
│   ├── debug.h                 ← portable LOGI/LOGE/LOGW macros
│   ├── perf_trace.h            ← no-op perf macros (override for real tracing)
│   ├── touch_calib/
│   │   ├── touch_calib.h       ← touch calibration API
│   │   └── touch_calib.cpp     ← identity (no-op) stub
│   ├── ui/touch_calib/
│   │   └── touch_calib.h       ← compatibility redirect → touch_calib/touch_calib.h
│   └── driver/
│       ├── driver_interface.hpp    ← display_alloc() / display_setup() contract
│       ├── esp32_2424s012.cpp      ← FULL: GC9A01+CST816S / ESP32-C3
│       ├── guition_4848s040.cpp    ← STUB: copy from cnc_interface (see below)
│       ├── jc3248w535c.cpp         ← STUB: copy from cnc_interface
│       ├── wt32_sc01_plus.cpp      ← STUB: copy from cnc_interface
│       ├── jc_1060p470.cpp         ← STUB: copy from cnc_interface
│       └── jc_8012p4a1.cpp         ← STUB: copy from cnc_interface
└── example/
    ├── platformio.ini          ← ready-to-build Hello World
    └── src/
        └── main.cpp            ← 3-line setup(), empty loop()
```

---

## Quick start

### 1 — Add the submodule

```bash
mkdir -p ext
git submodule add https://github.com/your-org/esp32_lcd_controllers.git ext/esp32_lcd_controllers
git submodule update --init --recursive
```

### 2 — Create `lv_conf.h`

LVGL requires an `lv_conf.h` in a directory that is in the compiler include path.
The easiest place is the **project root** (next to `platformio.ini`):

```bash
cp ext/esp32_lcd_controllers/include/lv_conf_template.h lv_conf.h
# then edit lv_conf.h to taste
```

The ini fragments already pass `-D LV_CONF_INCLUDE_SIMPLE` and `-I .` so LVGL
finds `lv_conf.h` in your project root.

### 3 — Edit `platformio.ini`

```ini
[platformio]
extra_configs = ext/esp32_lcd_controllers/esp32_lcd_controllers.ini

[env:my_app]
; Choose one of: lcd_ctrl_4848s040 | lcd_ctrl_2424s012 | lcd_ctrl_jc3248w535c
;                lcd_ctrl_wt32_sc01_plus | lcd_ctrl_jc1060p470 | lcd_ctrl_jc8012p4a1
extends = lcd_ctrl_4848s040

; Make PlatformIO compile the library files.
lib_extra_dirs = ext/esp32_lcd_controllers

; Add sub-directory include paths.
build_flags = ${lcd_ctrl_4848s040.build_flags}
    -I ext/esp32_lcd_controllers/src
    -I ext/esp32_lcd_controllers/include
    -I .   ; picks up your lv_conf.h

; Compile your app src/ PLUS the board drivers from the submodule.
; All driver .cpp files carry top-level #ifdef guards so only the active
; board's code is actually compiled.
build_src_filter =
    +<*>
    +<../../ext/esp32_lcd_controllers/src/driver/**>
```

### 4 — Write `src/main.cpp`

```cpp
#include "lcd_controllers.h"
#include <lvgl.h>

static void my_ui(lv_display_t *disp, lv_indev_t *touch) {
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello, World!");
    lv_obj_center(label);
}

void setup() {
    lcd_controllers_init_and_start(my_ui);
}

void loop() {
    vTaskDelay(portMAX_DELAY);     // LVGL runs in its own FreeRTOS task
}
```

### 5 — Build & flash

```bash
pio run -e my_app -t upload
pio device monitor
```

---

## Public API

```cpp
// lcd_controllers.h

// Two-step usage (more control):
void lcd_controllers_init(void);
void lcd_controllers_start(lcd_ui_init_fn_t ui_init_fn);

// Combined convenience:
void lcd_controllers_init_and_start(lcd_ui_init_fn_t ui_init_fn);

// Accessors (valid after lcd_controllers_init()):
lv_display_t *lcd_controllers_get_display(void);
lv_indev_t   *lcd_controllers_get_touch_indev(void);
lv_indev_t   *lcd_controllers_get_encoder_indev(void);  // if encoder pins defined
```

`lcd_controllers_init()` is the MCU-to-LVGL bootstrap sequence:

1. `mcu_setup()` — initialises Serial, checks PSRAM (weak, override if needed)
2. `mcu_startup()` — post-MCU hook (weak no-op, used for WiFi / NVS / etc.)
3. `display_alloc()` — early PSRAM buffer reservation
4. `lv_init()`
5. `lv_display_create(TFT_WIDTH, TFT_HEIGHT)` + `lv_indev_create()`
6. `display_setup(disp, indev)` — board-specific flush + touch callbacks

`lcd_controllers_start()` spawns the LVGL FreeRTOS task.  The task calls
`ui_init_fn(disp, touch)` once before entering the `lv_task_handler()` loop.

---

## Customisation hooks

### MCU init override

Provide your own `mcu_setup()` and/or `mcu_startup()` to override the weak
defaults without modifying the library:

```cpp
// In your src/main.cpp (or any .cpp in your project):
extern "C" void mcu_setup() {
    Serial.begin(115200);
    nvs_flash_init();
    LittleFS.begin();
}

extern "C" void mcu_startup() {
    wifi_manager_connect(ssid, password, 10000);
}
```

### Per-frame work (data binding, sensor reads)

```cpp
extern "C" void lvgl_loop_hook() {
    // Called once per LVGL render iteration from the LVGL task.
    // Keep it fast (< ~5 ms); heavy work will cause UI jank.
    my_sensor_poll();
    my_data_binding_tick();
}
```

### LVGL task tunables (build flags)

| Flag | Default | Effect |
|------|---------|--------|
| `LVGL_TASK_STACK_KB` | 10 (S3/C3) / 24 (P4) | Task stack depth in KiB |
| `LVGL_TASK_PRIORITY` | `tskIDLE_PRIORITY+2` | FreeRTOS task priority |
| `LVGL_TASK_CORE` | `0` | Core affinity (use `-1` for unpinned) |
| `LVGL_TASK_PSRAM` | unset | If defined, stack is allocated from PSRAM |

Example: move the stack to PSRAM on an S3 board with limited internal SRAM:

```ini
build_flags = ${lcd_ctrl_jc3248w535c.build_flags}
    -D LVGL_TASK_PSRAM
    -D LVGL_TASK_STACK_KB=16
```

### Touch calibration

The built-in `touch_calib` stub is an identity transform — raw coordinates
pass through unchanged.  To enable real calibration:

1. Copy `cnc_interface/src/ui/touch_calib/touch_calib.cpp` into
   `src/touch_calib/touch_calib.cpp` (replace the stub).
2. Implement `touch_calib_has()`, `touch_calib_load()`, `touch_calib_save()`,
   and `touch_calib_apply_inplace()` using your NVS scheme.
3. Add the calibration wizard entry to your `ui_init_fn`:
   ```cpp
   if (!touch_calib_has()) {
       ui_run_touch_calib_blocking();
   }
   touch_calib_load();
   ```

---

## Driver migration guide

Driver source files for all boards except `esp32_2424s012` are provided as
stubs that must be populated before that board target will compile.
This is intentional: it keeps the submodule small and avoids bundling large
files that belong to their own repository.

### Automated migration (recommended)

From the firmware root, run:

```bash
boards=(guition_4848s040 jc3248w535c wt32_sc01_plus jc_1060p470 jc_8012p4a1)

for b in "${boards[@]}"; do
    cp cnc_interface/src/driver/${b}.cpp \
       esp32_lcd_controllers/src/driver/${b}.cpp
done

# Sub-directories
for d in jc3248w535c jc1060p470 jc8012p4a1 gt911_touch esp_lvgl; do
    cp -r cnc_interface/src/driver/${d} \
          esp32_lcd_controllers/src/driver/${d}
done
```

No include-path changes are needed — the compatibility headers handle the
`"ui/touch_calib/touch_calib.h"` path automatically.

### What each stub needs

| Driver file | Extra directories to copy |
|-------------|--------------------------|
| `guition_4848s040.cpp` | *(none — self-contained)* |
| `wt32_sc01_plus.cpp` | *(none — self-contained)* |
| `jc3248w535c.cpp` | `jc3248w535c/` |
| `jc_1060p470.cpp` | `jc1060p470/`, `gt911_touch/`, `esp_lvgl/` |
| `jc_8012p4a1.cpp` | `jc8012p4a1/`, `esp_lvgl/` |

---

## Using in the cnc_interface project

The `cnc_interface` project can share this submodule's infrastructure while
keeping its own drivers intact.  Add to `cnc_interface/platformio.ini`:

```ini
[platformio]
extra_configs =
    ../shared_build.ini
    ../esp32_lcd_controllers/esp32_lcd_controllers.ini

; Then derive board envs from lcd_ctrl_<board> sections instead of
; duplicating platform/board/sdkconfig settings.
```

The `mcu_setup()` and `mcu_startup()` weak symbols let the pendant's own
MCU init code override the defaults without any source-level changes.

---

## Adding a new board

1. Add a named section in `esp32_lcd_controllers.ini`:
   ```ini
   [lcd_ctrl_my_new_board]
   extends           = lcd_ctrl_base_s3   ; or _c3 / _p4
   platform          = ${lcd_ctrl_platform_tasmota.platform}
   platform_packages = ${lcd_ctrl_platform_tasmota.platform_packages}
   framework         = ${lcd_ctrl_platform_tasmota.framework}
   board             = esp32s3_qio_opi
   build_flags       = ${lcd_ctrl_base_s3.build_flags}
       -D MY_NEW_BOARD=1
       -D TFT_WIDTH=320
       -D TFT_HEIGHT=240
       ; ... pin defines ...
   lib_deps          = ${lcd_ctrl_base_s3.lib_deps}
       lovyan03/LovyanGFX
   ```

2. Create `src/driver/my_new_board.cpp` with a top-level `#ifdef MY_NEW_BOARD`
   guard implementing `display_alloc()` and `display_setup()`.

3. Add it to `build_src_filter` or leave it in the driver directory — since all
   driver files are compiled but the `#ifdef` guards ensure only the active
   board's code is linked.

---

## Licence

MIT — see [LICENSE](LICENSE).
