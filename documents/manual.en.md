# milligram image viewer — Manual

**Version 3.4.2** | Last update 2026-09-17

This document is the detailed manual for milligram image viewer. For an overview and feature highlights, see the [README](../README.en.md).

---

## Table of contents

- [Introduction](#introduction)
- [Installation](#installation)
- [Updating](#updating)
- [Susie plugins](#susie-plugins)
- [Uninstallation](#uninstallation)
- [System requirements](#system-requirements)
- [Usage](#usage)
- [Keyboard shortcuts](#keyboard-shortcuts)
- [Mouse operations](#mouse-operations)
- [Multimedia key operations](#multimedia-key-operations)
- [Startup options](#startup-options)
- [Troubleshooting](#troubleshooting)
- [Advanced settings](#advanced-settings)
- [Disclaimer & support](#disclaimer--support)
- [Commercial use](#commercial-use)
- [Contact & copyright](#contact--copyright)

---

## Introduction

milligram image viewer is a simple image viewer. It has no frame, and is minimal in both functionality and appearance. Built-in support covers BMP / JPEG / GIF (animated) / PNG / WebP (animated) / AVIF (animated, HDR supported) / JpegXL (animated) and various video formats. Additional image formats can be supported by adding **Susie plugins**. **Free for non-commercial use only.**

Key features:

- Slideshow
- Various fullscreen modes
- Browsing images inside compressed folders (uncompressed ZIP files are supported without plugins)
- Image organization
- Multiple simultaneous instances with different settings

milligram was designed around the concept of keeping images always visible on your desktop — like a desktop photo frame. Additional features include image organization (Ctrl + right-click) and saving favorite image lists.

---

## Installation

1. Extract the ZIP file to any location.
2. Run `milligram.exe`.
3. You will be prompted to choose where to create a shortcut. Select as appropriate.
4. To display formats other than BMP, obtain Susie plugins and place the `.spi` files in the same folder as `milligram.exe`. They will be loaded at startup.

Shift + Right-click inside the black frame and choose **Options → File Association** to allow opening the viewer by double-clicking image files.

---

## Updating

Overwrite all existing files with the new ones.

---

## Susie plugins

By default, milligram searches for Susie plugins in the folder where `milligram.exe` is located. If your plugins are stored elsewhere, right-click and choose **Specify Plugin Folder**, then select a plugin file. milligram will then search the folder containing that file.

To specify two or more plugin folders, edit the INI file directly.

### Susie resources

- [Susie's Room](http://digitalpad.co.jp/~takechin/)
- [Susie Plugin Downloads](http://www.vector.co.jp/vpack/filearea/win95/art/graphics/loader/susie/index.html)
- [TORO's Library](http://www2f.biglobe.ne.jp/~kana/link.html) (the x64 version follows this specification)

> Note: some plugins are not supported by milligram image viewer.

---

## Uninstallation

1. Launch milligram.
2. Right-click and choose **Options → Uninstall**.
3. Delete the folder containing `milligram.exe`.

---

## System requirements

Windows 10 or later (64-bit)

---

## Usage

Launch the application normally and drop the files you want to view onto the window. Any supported image file will be displayed immediately. Dropping files onto a shortcut works as well.

### Basic operations

- **Double-click an image** — shows a list of all dropped files. Double-click any file in the list to display it.
- **Mouse wheel on an image** — cycles through images. Sensitivity can be adjusted under **Options → Wheel Sensitivity**.
- **Drag the image** — since there is no title bar, you can drag the window itself by dragging anywhere on the image.
- **Resize the display** — move the cursor to the edge of the image; the cursor shape will change, allowing you to resize. Hold **Alt** while dragging to rotate the window to any angle.

### Settings

- **Specify Plugin Folder** — sets the folder where Susie plugins are searched. To specify two or more folders, edit the INI file directly.
- **Internal Loader Priority File Settings** — sets a file mask to prioritize the internal loader. For WebP and animated GIF, the internal loader is always used regardless of this setting, due to animation support requirements.
- **Instances** — milligram supports the concept of "instances." By preparing multiple configuration files and file lists, you can run several viewers simultaneously with different settings. For example, one instance can be locked to display a slideshow as a desktop wallpaper image, while another is used as a regular viewer.

### File organization (Ctrl + right-click)

Holding Ctrl while right-clicking shows a list of destination folders for the currently displayed file — provided that the file is **not** inside a compressed archive. Clicking a folder name immediately moves the file there.

- **Ctrl + folder name click** — automatically renames the file if a file with the same name already exists in the destination.
- **Folder name click without Ctrl** — opens a confirmation dialog asking whether to overwrite.

Files can be moved consecutively — useful for organizing image collections.

### History menu

- **Click** — opens the corresponding history entry.
- **Shift + click** — deletes that history entry.

The last item in the history menu is "Clear History," which removes all entries at once.

---

## Keyboard shortcuts

Most operations can be performed via the keyboard.

### Image display & navigation

| Key | Action |
|---|---|
| `↓` `Space` | Show next image |
| `↑` `BackSpc` | Show previous image |
| `Enter` / Double-click | Toggle file list / image |
| `/` | Display at actual size (1:1) |
| `*` or `\` | Fit image to screen |
| `+` | Zoom in |
| `-` | Zoom out |
| `,` (comma) | Rotate image left |
| `.` (period) | Rotate image right |

### Display modes

| Key | Action |
|---|---|
| `A` | Display at actual size (1:1) |
| `S` | Fit to screen |
| `C` | Center |
| `T` | Always on top |
| `B` | Fix zoom level |
| `G` | Fixed size display |
| `W` | Fit to window |
| `H` | Use entire screen |
| `F` | Fullscreen |
| `D` | Slideshow |
| `Ctrl + R` | Set window size |
| `E` | PeepMode |
| `I` | Show File Property |

### File operations

| Key | Action |
|---|---|
| `O` | Open file |
| `P` | Open folder |
| `Ctrl + F` | Load folder of the currently displayed file |
| `Ctrl + O` | Open existing folder |
| `Shift + S` | Save JPEG file |
| `Ctrl + Shift + S` | Save JPEG at displayed size |
| `Shift + S` | Save file list |
| `Ctrl + T` | Sort file list by name |
| `Ctrl + C` | Copy file (pasteable in Explorer) |
| `Ctrl + V` | Paste file (from Explorer) |
| `Ctrl + X` | Cut file |
| `Ctrl + N` | Rename |
| `Shift + C` | Copy image to clipboard |
| `Ctrl + Shift + C` | Copy file path to clipboard |
| `Shift + Z` | Pick color from image (copy RGB at cursor to clipboard) |
| `Del` | Remove file from list |
| `Ctrl + Del` | Move file to Recycle Bin |
| `R` / `F5` | Reload |
| `X` / `BS` | Close archive |
| `I` | Show file information |

### Video playback

| Key | Action |
|---|---|
| `Z` | Play / pause |
| `←` | Seek back 10 s |
| `→` | Seek forward 10 s |
| `Ctrl` + `+` | Volume up |
| `Ctrl` + `-` | Volume down |
| `M` | Toggle mute |

### Multi-instance (when several instances of the same config are open)

| Key | Action |
|---|---|
| `Ctrl + →` | Arrange same instances to the right |
| `Ctrl + ←` | Arrange same instances to the left |
| `Ctrl + ↓` | Show previous image across same instances |
| `Ctrl + ↑` | Show next image across same instances |
| `Shift + ↓` | Step back by the number of instances |
| `Shift + ↑` | Step forward by the number of instances |

### System

| Key | Action |
|---|---|
| `Esc` | Quit |
| `Right-click` | Show short menu |
| `Shift + right-click` | Show full menu |
| `Ctrl + right-click` | Move file |
| `Ctrl + Shift + right-click` | Move file by Use Homephas |
| `Ctrl + Space` | Hide the window (right-click the system tray icon and choose "Show" to bring it back) |

### File list operations

| Key | Action |
|---|---|
| `Alt + ↑` | Move selected file up by one |
| `Alt + ↓` | Move selected file down by one |
| `Ctrl + A` | Select all |
| `Ctrl + I` | Invert selection |
| `Del` | Remove from list |
| `Ctrl + click` | Multi-select |
| `Shift + click` | Range select |

---

## Mouse operations

### File operations

- **Drop a file** — dropping a folder loads the files inside; dropping a file displays that file.
- **`Ctrl` + drop** — appends the dropped files to the current file list.

### Image operations

- **Wheel** — cycle through images.
- **Wheel click** — quit.
- **`Ctrl` + wheel** or **right-click + wheel** — zoom in / zoom out.
- **`Ctrl` + wheel click** or **right-click + wheel click** — display at actual size (1:1).
- **`Alt` + wheel** or **left-click + wheel** — rotate image.
- **`Alt` + wheel click** or **left-click + wheel click** — reset rotation.
- **`Ctrl` + `Alt` + wheel** or **right-click + left-click + wheel** — adjust window transparency.
- **`Ctrl` + `Alt` + wheel click** or **right-click + left-click + wheel click** — set opacity to 100%.
- **`Shift` +** — modify view area
- **Left-click + right-click** — move the image so it fits within the screen.
- **`Shift` + operate described above** — Oparate only image.

---

## Startup options

- **Hold `Ctrl` while launching** — starts without loading the previous file list, even if "Load last file list on startup" is enabled.
- **Hold `Shift + Ctrl` while launching** — starts in a fresh initial state. Try this if the application behaves unexpectedly.

---

## Troubleshooting

### The application will not start

If "Load last file list on startup" is enabled and a file causes an error preventing startup, try one of the following:

1. Hold `Ctrl` while launching.
2. Drop any file onto the shortcut to launch.
3. Hold `Shift` + `Ctrl` while launching (starts in a fresh initial state).

### The window has moved off-screen

If the window seems missing (and you have not loaded a tiny image), right-click the system tray icon and choose **Move to Center**.

### Still not displayed

There may be a problem with a plugin. Right-click the tray icon and choose **Close** to quit, then launch with `Ctrl` + `Shift` held. The application should start normally.

---

## Advanced settings

Edit the INI file for fine-grained configuration. Its location:

```
C:\Users\(username)\AppData\Local\milligram
```

(On older Windows versions: `C:\Documents and Settings\(username)\Local Settings\Application Data\milligram`)

---

## Disclaimer & support

milligram image viewer is freeware for non-commercial use only. The author assumes no responsibility for any damages resulting from the use of this software. For support, please contact the email address below. Responses may take time, but questions will be answered to the best of the author's ability.

---

## Commercial use

For commercial use, please contact the email address below (replace `(a)` with `@`) for payment information. **License fee: 3,000 JPY per license.**

---

## Contact & copyright

**DOWNSPADE.**
- Email: `downspade(a)downspade.com`
- Web: http://www.downspade.com

### Copyright

All copyrights to the contents of this archive are retained by the program's author and will not be relinquished. Unauthorized modification may be a violation of law. Redistribution is freely permitted as long as the archive file (`milligram.zip`) is not modified. No notification is required.
