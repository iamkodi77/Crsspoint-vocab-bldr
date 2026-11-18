# CrossPoint Vocabulary Builder Plugin (`Crsspoint-vocab-bldr`)

A vocabulary builder plugin for [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader) firmware running on **Xteink** e-readers (such as the Xteink X3 and X4).

This plugin allows you to highlight any word in a book and save it directly to your Vocabulary Builder with a single tap or button press. Words are saved as clean, plain words to `/vocabulary_builder.txt` on the device's storage for later review or export to flashcard apps (such as Anki or Quizlet).

---

## Features

- **Highlight & Save Directly from Books**: While reading any book in CrossPoint, selecting text displays a dedicated **`[Vocab]`** button alongside `[Lookup]`, `[Clip]`, and `[Bookmark]`.
- **Pure Plain Words Only**: Per design, only the clean word itself is saved—no source information, no book title, no author, and no timestamps cluttering the file.
- **Smart Punctuation & Quote Stripping**: Trims leading and trailing quotes (`"`, `'`, `“`, `”`, `‘`, `’`, `«`, `»`), punctuation (`.`, `,`, `!`, `?`, `;`, `:`, `(`, `)`), brackets, and dashes (`—`, `–`), while correctly preserving internal apostrophes (`don't`), hyphens (`state-of-the-art`), and UTF-8 accented characters (`café`).
- **Instant On-Screen Confirmation**: Shows an immediate toast popup on your e-ink screen (`Saved: <word>` or `Already saved: <word>`) without interrupting your reading flow.
- **Automatic Deduplication**: Case-insensitive duplicate detection ensures your list stays tidy without duplicate entries.
- **On-Device Vocabulary Builder App**: Accessible from **Settings → Plugins → Vocabulary Builder**:
  - **List View**: Browse all saved words with pagination, scrollbar, and total word count.
  - **Word Options Dialog**: Tap or press Confirm on any word to:
    - **Look Up in Dictionary**: View definition instantly via StarDict.
    - **Flashcard Review**: Enter full-screen distraction-free flashcard mode.
    - **Delete Word**: Remove the word from your list and TXT file with confirmation.
    - **Clear All**: Reset your vocabulary list when desired.
  - **Flashcard Review Mode**: Full-screen flashcard display for studying saved words. Cycle through cards with Previous/Next buttons or swipe gestures, and press Confirm to look up the definition.
- **Plain Text Storage (`/vocabulary_builder.txt`)**: All words are stored one per line on the root of your SD card / internal storage. Easily readable in any text editor or ready for Anki / Quizlet import.
- **Touch & Button Support**:
  - **Touchscreen devices (Xteink X4)**: Tap `[Vocab]` on the selection bar; tap words to open options.
  - **Physical button devices (Xteink X3)**: Set the **Confirm Action** setting to "Save to Vocab" so pressing Confirm on a highlighted word immediately saves it to your vocabulary list!
- **Optional Auto-Save on Dictionary Lookup**: Optionally auto-save words to your vocabulary builder whenever you look them up in the StarDict dictionary.

---

## File Format

Saved to `/vocabulary_builder.txt` on your SD card root (or custom path configured in Settings):

```text
ephemeral
serendipity
ubiquitous
mellifluous
ineffable
```

Plain text, one word per line. No JSON, no timestamps, no metadata.

---

## Installation

### Method 1: Via `xteink-plugins` (Recommended)

1. Clone [xteink-plugins](https://github.com/JustinOros/xteink-plugins) if you haven't already:
   ```bash
   git clone https://github.com/JustinOros/xteink-plugins.git
   ```

2. Copy this repository into `xteink-plugins/plugins/vocabbuilder`:
   ```bash
   cp -r /path/to/Crsspoint-vocab-bldr /path/to/xteink-plugins/plugins/vocabbuilder
   ```
   Or use the included `install.py`:
   ```bash
   python3 install.py --xteink-plugins /path/to/xteink-plugins
   ```

3. Run the xteink-plugins installer to build and flash your firmware:
   ```bash
   python3 install.py
   ```
   Select **Vocabulary Builder** when prompted for plugins to install.

---

### Method 2: Direct Source Patch to `crosspoint-reader`

If you are building CrossPoint firmware directly from [crosspoint-reader](https://github.com/crosspoint-reader/crosspoint-reader):

1. Run `install.py` pointing to your crosspoint-reader directory:
   ```bash
   python3 install.py --crosspoint /path/to/crosspoint-reader
   ```

2. Build and flash with PlatformIO:
   ```bash
   pio run -e x3 -t upload   # for Xteink X3
   # or
   pio run -e x4 -t upload   # for Xteink X4
   ```

---

## Settings & Configuration

In CrossPoint, open **Settings → Plugins**:

| Setting | Options | Default | Description |
| :--- | :--- | :--- | :--- |
| **Vocabulary Builder** | Open | — | Launches the Vocabulary Builder activity |
| **Confirm Action** | `Clip` / `Save to Vocab` | `Clip` | When pressing Confirm on a selected word (useful for non-touch button devices) |
| **Lowercase Words** | `Disabled` / `Enabled` | `Enabled` | Automatically convert saved words to lowercase |
| **Auto-Save Lookups** | `Disabled` / `Enabled` | `Disabled` | Automatically add words to Vocab Builder when looking them up in StarDict |
| **Vocab TXT File** | String | `/vocabulary_builder.txt` | Path on storage where words are saved |

---

## Project Structure

```text
Crsspoint-vocab-bldr/
├── VocabBuilderActivity.h/.cpp   # On-device UI (Word list, Flashcard review, Dictionary integration)
├── VocabBuilderStore.h/.cpp      # Plain word storage, cleaning/trimming, TXT file append & rewrite
├── VocabBuilderPlugin.h/.cpp     # Settings glue and mode helpers
├── PluginListDraw.h              # Standalone list renderer helper for activities
├── plugin.py                     # xteink-plugins manifest definition
├── patch.py                      # Reader hook patcher (adds [Vocab] action to book selection)
├── install.py                    # Standalone installation script for crosspoint or xteink-plugins
├── test/
│   ├── test_vocab_store.cpp      # Native C++ unit tests (cleaning, deduplication, file IO)
│   ├── test_patch.py             # Patch application and idempotency tests
│   ├── test_xteink_integration.py# Integration test running xteink-plugins test_harness
│   └── run_all_tests.sh          # One-command test runner
└── README.md
```

---

## Testing

Run all unit and integration tests locally:

```bash
./test/run_all_tests.sh
```

This runs:
1. Native C++ host unit tests (validates punctuation trimming, UTF-8 safety, file format, deduplication).
2. Patch application tests (validates clean patching and idempotency on fresh CrossPoint sources).
3. Integration tests with `xteink-plugins` test harness.

---

## License

MIT License. Free to use, modify, and distribute.
