#!/usr/bin/env python3
"""
Vocabulary Builder Plugin Installer for CrossPoint Reader / Xteink.

Can install either:
1. Into an xteink-plugins checkout (as plugins/vocabbuilder)
2. Directly into a crosspoint-reader firmware checkout (source-level patch)
"""

import argparse
import os
import shutil
import sys

from patch import patch_crosspoint_reader

PLUGIN_FILES = [
    "VocabBuilderStore.h",
    "VocabBuilderStore.cpp",
    "VocabBuilderActivity.h",
    "VocabBuilderActivity.cpp",
    "VocabBuilderPlugin.h",
    "VocabBuilderPlugin.cpp",
    "PluginListDraw.h",
    "plugin.py",
    "patch.py",
    "README.md",
]


def install_to_xteink_plugins(xteink_dir):
    """Installs the plugin into an xteink-plugins repo."""
    plugins_dir = os.path.join(xteink_dir, "plugins")
    if not os.path.isdir(plugins_dir):
        print(f"Error: {plugins_dir} not found. Is {xteink_dir} an xteink-plugins repository?")
        return False

    dest = os.path.join(plugins_dir, "vocabbuilder")
    os.makedirs(dest, exist_ok=True)

    src_dir = os.path.dirname(os.path.abspath(__file__))
    for fname in PLUGIN_FILES:
        s = os.path.join(src_dir, fname)
        if os.path.exists(s):
            shutil.copy2(s, os.path.join(dest, fname))

    print(f"✓ Vocabulary Builder installed to {dest}")
    print("  You can now run xteink-plugins' install.py to build and flash your firmware.")
    return True


def install_direct_to_crosspoint(crosspoint_dir):
    """Directly copies source files and patches a crosspoint-reader repository."""
    src_dir = os.path.dirname(os.path.abspath(__file__))
    settings_dir = os.path.join(crosspoint_dir, "src", "activities", "settings")
    vocab_dir = os.path.join(crosspoint_dir, "src", "vocabulary")

    if not os.path.isdir(os.path.join(crosspoint_dir, "src")):
        print(f"Error: {crosspoint_dir}/src not found. Is this a crosspoint-reader clone?")
        return False

    os.makedirs(settings_dir, exist_ok=True)
    os.makedirs(vocab_dir, exist_ok=True)

    # Copy files
    shutil.copy2(os.path.join(src_dir, "VocabBuilderActivity.h"), settings_dir)
    shutil.copy2(os.path.join(src_dir, "VocabBuilderActivity.cpp"), settings_dir)
    shutil.copy2(os.path.join(src_dir, "VocabBuilderPlugin.h"), settings_dir)
    shutil.copy2(os.path.join(src_dir, "VocabBuilderPlugin.cpp"), settings_dir)
    shutil.copy2(os.path.join(src_dir, "PluginListDraw.h"), settings_dir)
    shutil.copy2(os.path.join(src_dir, "VocabBuilderStore.h"), vocab_dir)
    shutil.copy2(os.path.join(src_dir, "VocabBuilderStore.cpp"), vocab_dir)
    print("✓ Copied source files into crosspoint-reader tree.")

    # Apply reader selection patches
    patch_crosspoint_reader(crosspoint_dir)

    print("✓ Direct patching complete.")
    return True


def main():
    parser = argparse.ArgumentParser(description="Install Vocabulary Builder plugin for CrossPoint Reader")
    parser.add_argument("--crosspoint", help="Path to crosspoint-reader clone to patch directly")
    parser.add_argument("--xteink-plugins", help="Path to xteink-plugins clone")
    args = parser.parse_args()

    if args.crosspoint:
        ok = install_direct_to_crosspoint(os.path.abspath(args.crosspoint))
        sys.exit(0 if ok else 1)

    if args.xteink_plugins:
        ok = install_to_xteink_plugins(os.path.abspath(args.xteink_plugins))
        sys.exit(0 if ok else 1)

    # Auto-detect common locations
    cwd = os.getcwd()
    if os.path.exists(os.path.join(cwd, "src", "CrossPointSettings.h")):
        print(f"Detected crosspoint-reader in {cwd}")
        ok = install_direct_to_crosspoint(cwd)
        sys.exit(0 if ok else 1)

    if os.path.exists(os.path.join(cwd, "framework", "engine.py")):
        print(f"Detected xteink-plugins in {cwd}")
        ok = install_to_xteink_plugins(cwd)
        sys.exit(0 if ok else 1)

    parser.print_help()
    print("\nPlease specify --crosspoint <path> or --xteink-plugins <path>.")
    sys.exit(1)


if __name__ == "__main__":
    main()
