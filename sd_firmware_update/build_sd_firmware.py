#!/usr/bin/env python3
"""
Helper script to compile CrossPoint firmware with Vocabulary Builder
and copy the resulting firmware.bin ready for SD card installation/updating.
"""

import argparse
import os
import shutil
import subprocess
import sys

ENV_MODELS = {
    "default": "Xteink X3 & X4 (ESP32-C3)",
    "x4pro": "Xteink X4 Pro (ESP32-S3)",
    "x4c": "Xteink X4 Classic (ESP32-S3)",
    "slim": "Xteink X3 & X4 Slim (No serial logging)",
}

def find_pio():
    """Locate platformio executable."""
    if shutil.which("pio"):
        return "pio"
    # Check current python environment
    res = subprocess.run([sys.executable, "-m", "platformio", "--version"],
                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if res.returncode == 0:
        return f'"{sys.executable}" -m platformio'
    return None

def find_crosspoint_dir(custom_path=None):
    """Auto-detect crosspoint-reader directory."""
    if custom_path and os.path.isdir(custom_path):
        return os.path.abspath(custom_path)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.dirname(script_dir)

    candidates = [
        # In parent or sibling directories
        os.path.join(repo_root, "..", "crosspoint-reader"),
        os.path.join(repo_root, "..", "xteink-plugins", "crosspoint-reader"),
        # In current working directory
        os.path.join(os.getcwd(), "crosspoint-reader"),
        os.path.join(os.getcwd(), "..", "crosspoint-reader"),
    ]

    for p in candidates:
        if os.path.isfile(os.path.join(p, "platformio.ini")):
            return os.path.abspath(p)

    return None

def main():
    parser = argparse.ArgumentParser(
        description="Build CrossPoint firmware.bin for SD Card Update"
    )
    parser.add_argument(
        "--env",
        default="default",
        choices=["default", "x4pro", "x4c", "slim"],
        help="Device target environment (default: 'default' for Xteink X3/X4)",
    )
    parser.add_argument(
        "--crosspoint-dir",
        help="Path to patched crosspoint-reader checkout (auto-detected if omitted)",
    )
    parser.add_argument(
        "--output",
        default="firmware.bin",
        help="Output filename (default: firmware.bin)",
    )

    args = parser.parse_args()

    pio_cmd = find_pio()
    if not pio_cmd:
        print("Error: PlatformIO (pio) was not found.")
        print("Install it via: pip install platformio")
        sys.exit(1)

    crosspoint_dir = find_crosspoint_dir(args.crosspoint_dir)
    if not crosspoint_dir:
        print("Error: Could not locate crosspoint-reader directory.")
        print("Please specify it with: python3 build_sd_firmware.py --crosspoint-dir <path>")
        sys.exit(1)

    print(f"Target model:   {ENV_MODELS.get(args.env, args.env)} [env:{args.env}]")
    print(f"CrossPoint dir: {crosspoint_dir}")
    print("\nBuilding firmware with PlatformIO (this may take a moment)...")

    cmd = f"{pio_cmd} run --environment {args.env}"
    res = subprocess.run(cmd, shell=True, cwd=crosspoint_dir)
    if res.returncode != 0:
        print("\n❌ Build failed. Please check compiler errors above.")
        sys.exit(res.returncode)

    built_bin = os.path.join(crosspoint_dir, ".pio", "build", args.env, "firmware.bin")
    if not os.path.isfile(built_bin):
        print(f"Error: Expected build binary not found at {built_bin}")
        sys.exit(1)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    out_file = os.path.join(script_dir, args.output)
    shutil.copy2(built_bin, out_file)
    file_size_kb = os.path.getsize(out_file) / 1024

    print("\n" + "=" * 60)
    print("✓ FIRMWARE BUILD SUCCESSFUL!")
    print(f"✓ Binary created: {out_file} ({file_size_kb:.1f} KB)")
    print("=" * 60)
    print("\nHow to install on your Xteink:")
    print("1. Copy 'firmware.bin' to the root directory of your SD card.")
    print("2. Insert the SD card back into your Xteink.")
    print("3. On your Xteink:")
    print("   • Normal method: Settings → System → SD Card Firmware Update")
    print("   • Recovery method (X3): Hold Left Side Button + Power on boot")
    print("4. Select 'firmware.bin' and confirm to update!\n")

if __name__ == "__main__":
    main()
