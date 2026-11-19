# SD Card Firmware Update Guide

This folder contains the tools and instructions for updating your **Xteink** e-reader running **CrossPoint** directly from your **MicroSD card**—with **no USB cable or PC connection required during flashing!**

---

## Why Update via SD Card?

- **No USB cable connection issues**: Avoids driver conflicts, dialout permissions, or loose cable disconnects.
- **Works on any operating system**: Simply copy a single file (`firmware.bin`) to your SD card like copying a book.
- **Safe & Verified**: CrossPoint firmware automatically runs an integrity check (header magic, partition boundary, XOR checksum, SHA256) on the `.bin` before flashing. If the file is damaged or meant for a different model, it safely refuses to flash.

---

## Supported Devices & Firmware Files

| Device Model | Processor | PlatformIO Environment | Output Binary |
| :--- | :--- | :--- | :--- |
| **Xteink X3** (Buttons) | ESP32-C3 | `default` | `firmware.bin` |
| **Xteink X4** (Touchscreen) | ESP32-C3 | `default` | `firmware.bin` |
| **Xteink X4 Pro** (Frontlight) | ESP32-S3 | `x4pro` | `firmware.bin` |
| **Xteink X4 Classic** (Buttons) | ESP32-S3 | `x4c` | `firmware.bin` |

---

## Step 1: Get the `firmware.bin` File

### Method A: Build It Using `build_sd_firmware.py` (Fastest)

If you have already applied the Vocabulary Builder plugin:

```bash
# On your computer in the repository directory:
cd sd_firmware_update

# For Xteink X3 or X4:
python3 build_sd_firmware.py --env default

# For Xteink X4 Pro:
python3 build_sd_firmware.py --env x4pro
```

The script will compile the firmware and copy `firmware.bin` right into this folder!

### Method B: From an Existing Build in `xteink-plugins`

If you previously ran the installer in `xteink-plugins`, the compiled `.bin` file is already saved in:

```text
xteink-plugins/crosspoint-reader/.pio/build/default/firmware.bin
```

---

## Step 2: Copy `firmware.bin` to Your MicroSD Card

1. Insert your MicroSD card into your computer (or connect your reader via USB card-reader mode).
2. Copy `firmware.bin` directly onto the **root** of the SD card.

### SD Card Layout:
```text
MicroSD Card (Root)
├── Books/                 <-- Your regular books folder
│   ├── Book1.epub
│   └── Book2.epub
├── firmware.bin           <-- COPY YOUR FIRMWARE FILE HERE
└── vocabulary_builder.txt <-- (Created automatically when you save words)
```

3. Safely eject the SD card and insert it back into your Xteink reader.

---

## Step 3: Trigger the Update on Your Device

You can launch the SD firmware update using either of the following two methods:

### Method 1: From the Settings Menu (Recommended)

1. Turn on your Xteink reader.
2. Go to **Settings** → **System**.
3. Select **SD Card Firmware Update** (or **Check for Updates**).
4. The file browser will open in firmware mode, showing all `.bin` files on your SD card.
5. Select `firmware.bin`.
6. CrossPoint will validate the file and show a confirmation prompt:
   ```text
   Update firmware?
   firmware.bin
   ```
7. Select **Confirm** (or press the Confirm button).
8. The screen will show:
   ```text
   Updating...
   [████████████░░░░░░░░] 60%
   Do not power off
   ```
9. When finished, it will say **"Update Complete - Restarting"** and reboot automatically with **Vocabulary Builder** installed!

---

### Method 2: Hardware Button Recovery Shortcut (Xteink X3)

If you cannot access the Settings menu or prefer a direct shortcut:

1. Turn off your Xteink X3.
2. Press and hold the **Left Side Button** (on the side of the device), then press the **Power Button**.
3. The device will boot directly into **Recovery Mode** and open the firmware file browser.
4. Select `firmware.bin` and confirm to flash!

---

## Troubleshooting

- **Error: "Firmware open failed"**: Make sure the file is named `firmware.bin` and located on the root of your SD card.
- **Error: "Wrong device"**: Ensure you built with `--env default` for X3/X4 (ESP32-C3) or `--env x4pro` for X4 Pro (ESP32-S3).
- **Books & Reading Progress**: Updating firmware via SD card does **not** erase your books, bookmarks, or reading progress.
