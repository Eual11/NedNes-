The PPU (Picture Processing Unit) in the NES is responsible for generating the video output. It does this by working through a specific sequence of operations for each clock cycle. Here's a detailed explanation of what the PPU does during every clock cycle, including pseudocode to help illustrate the process.

### Overview of the PPU's Operation
The PPU works in a cycle of scanlines and cycles:
- **Scanlines:** Each frame has 262 scanlines.
  - **Visible Scanlines:** 0 to 239 (where the actual picture is drawn).
  - **VBlank Scanlines:** 240 to 260 (where the screen is not being drawn, and the CPU can safely update PPU memory).
  - **Pre-render Scanline:** 261 (prepares the PPU for the next frame).
- **Cycles:** Each scanline has 341 cycles.
  - **Visible Cycles:** 1 to 256 (where the pixels for the scanline are fetched and drawn).
  - **Post-render Cycles:** 257 to 320 (horizontal blanking, fetching sprites for the next line).
  - **VBlank Cycles:** 321 to 340 (prepares the next scanline).

### PPU Operations Per Clock Cycle

#### 1. **Pixel Fetching and Rendering**
During the visible scanlines (0 to 239), the PPU fetches pixel data from the pattern tables, attribute tables, and name tables to render the current pixel.

**Pseudocode:**
```plaintext
if (scanline >= 0 and scanline < 240) {
    if (cycle >= 1 and cycle <= 256) {
        // Fetch tile data for the current pixel
        fetchBackgroundData();
        fetchSpriteData();

        // Combine background and sprite data to determine the final pixel color
        finalPixelColor = renderPixel();
        outputPixel(finalPixelColor);
    }
}
```

- **Fetch Background Data:** The PPU fetches the background tile and color information from the name and attribute tables.
- **Fetch Sprite Data:** The PPU fetches sprite data if a sprite overlaps the current pixel.
- **Render Pixel:** The PPU determines the final color for the pixel, giving priority to sprites that are on top of the background.

#### 2. **Sprite Evaluation**
During the cycles 257 to 320, the PPU evaluates sprites that will appear on the next scanline. It checks which sprites are in range and fetches their data.

**Pseudocode:**
```plaintext
if (scanline >= 0 and scanline < 240) {
    if (cycle >= 257 and cycle <= 320) {
        evaluateSprites();
    }
}
```

- **Evaluate Sprites:** The PPU checks the OAM (Object Attribute Memory) to find sprites that intersect with the next scanline and loads their data into secondary OAM.

#### 3. **VBlank Period (Vertical Blanking)**
During the VBlank period (scanlines 240 to 260), the PPU does not render any pixels. This is the time when the CPU can safely update PPU memory without causing visual artifacts.

**Pseudocode:**
```plaintext
if (scanline >= 240 and scanline <= 260) {
    if (scanline == 241 and cycle == 1) {
        // Set the VBlank flag at the beginning of VBlank period
        PPUSTATUS.vblank = 1;
        triggerNMI();
    }

    // The CPU can update PPU memory during this time
    allowCPUMemoryAccess();
}

if (scanline == 261 and cycle == 1) {
    // Clear the VBlank flag at the end of the VBlank period
    PPUSTATUS.vblank = 0;
    resetFrameState();
}
```

- **Set VBlank Flag:** The PPU sets the VBlank flag in the `PPUSTATUS` register at the beginning of the VBlank period, which may trigger an NMI (Non-Maskable Interrupt) if enabled.
- **Clear VBlank Flag:** The VBlank flag is cleared at the end of the VBlank period, preparing the PPU for the next frame.

#### 4. **Pre-render Scanline**
The pre-render scanline (scanline 261) prepares the PPU for the next frame. This includes resetting various internal counters and latches.

**Pseudocode:**
```plaintext
if (scanline == 261) {
    if (cycle == 1) {
        resetVerticalCounters();
        clearSpriteZeroHit();
        clearOverflowFlag();
    }

    if (cycle >= 280 and cycle <= 304) {
        // Set vertical scroll bits from temporary to current register
        copyVerticalScrollBits();
    }

    // Fetch initial background data for the first scanline
    fetchInitialBackgroundData();
}
```

- **Reset Counters:** The PPU resets the vertical scroll counters and prepares for the first scanline of the next frame.
- **Copy Vertical Scroll Bits:** The PPU copies vertical scroll bits from a temporary register to the current scroll register to start rendering from the correct position.
- **Fetch Initial Background Data:** The PPU pre-fetches background tile data to be ready for the first scanline.

### Summary
The PPU's operation involves fetching and rendering pixels during visible scanlines, evaluating and loading sprite data, and handling VBlank periods where the CPU can update PPU memory. The pre-render scanline prepares the PPU for the next frame by resetting certain states and fetching initial data. Each clock cycle advances these processes in a structured and repeatable manner, ensuring smooth video output.
