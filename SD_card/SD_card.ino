#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// ---------------- SD pin configuration ----------------
static const int SD_MISO = 19;
static const int SD_MOSI = 23;
static const int SD_CLK  = 18;
static const int SD_CS   = 5;

// ---------------- Sprite file format ----------------
// Header:
//   0..3   : "SPRT"
//   4..5   : width   (uint16_t, little-endian)
//   6..7   : height  (uint16_t, little-endian)
//   8..9   : count   (uint16_t, little-endian)
//   10..11 : reserved
//
// Per sprite:
//   byte 0     : ASCII symbol
//   bytes 1..7 : padding/reserved
//   bytes 8..519 : 64 rows of uint64_t
//
// Each row uses only the lower 36 bits.
// Bit 0 = leftmost pixel, bit 35 = rightmost pixel.

static const char *SPRITE_FILE_PATH = "/number_sprites_36x64.spr";
static const uint16_t EXPECTED_WIDTH  = 36;
static const uint16_t EXPECTED_HEIGHT = 64;
static const uint16_t MAX_SPRITES     = 11;
static const size_t SPRITE_BLOCK_SIZE = 1 + 7 + (64 * sizeof(uint64_t)); // 520 bytes
static const size_t FILE_HEADER_SIZE  = 12;

struct SpriteFileHeader {
  uint16_t width;
  uint16_t height;
  uint16_t count;
};

struct Sprite36x64 {
  char symbol;
  uint64_t rows[64];
};

SPIClass sdSPI(VSPI);

// ---------------- Utility functions ----------------

bool readExact(File &f, uint8_t *buf, size_t len) {
  return f.read(buf, len) == (int)len;
}

bool loadSpriteHeader(fs::FS &fs, const char *path, SpriteFileHeader &header) {
  File f = fs.open(path, FILE_READ);
  if (!f) {
    Serial.println("Failed to open sprite file");
    return false;
  }

  char magic[4];
  if (!readExact(f, (uint8_t *)magic, 4)) {
    Serial.println("Failed to read sprite magic");
    f.close();
    return false;
  }

  if (memcmp(magic, "SPRT", 4) != 0) {
    Serial.println("Invalid sprite file magic");
    f.close();
    return false;
  }

  uint16_t reserved;
  if (!readExact(f, (uint8_t *)&header.width, 2) ||
      !readExact(f, (uint8_t *)&header.height, 2) ||
      !readExact(f, (uint8_t *)&header.count, 2) ||
      !readExact(f, (uint8_t *)&reserved, 2)) {
    Serial.println("Failed to read sprite header");
    f.close();
    return false;
  }

  f.close();

  if (header.width != EXPECTED_WIDTH || header.height != EXPECTED_HEIGHT) {
    Serial.printf("Unexpected sprite dimensions: %u x %u\n", header.width, header.height);
    return false;
  }

  if (header.count == 0 || header.count > MAX_SPRITES) {
    Serial.printf("Unexpected sprite count: %u\n", header.count);
    return false;
  }

  return true;
}

// Reads a sprite by character, e.g. '0', '5', ':'
bool loadSpriteBySymbol(fs::FS &fs, const char *path, char target, Sprite36x64 &outSprite) {
  File f = fs.open(path, FILE_READ);
  if (!f) {
    Serial.println("Failed to open sprite file");
    return false;
  }

  // Read and validate header
  char magic[4];
  uint16_t width, height, count, reserved;

  if (!readExact(f, (uint8_t *)magic, 4) ||
      !readExact(f, (uint8_t *)&width, 2) ||
      !readExact(f, (uint8_t *)&height, 2) ||
      !readExact(f, (uint8_t *)&count, 2) ||
      !readExact(f, (uint8_t *)&reserved, 2)) {
    Serial.println("Failed to read sprite file header");
    f.close();
    return false;
  }

  if (memcmp(magic, "SPRT", 4) != 0) {
    Serial.println("Invalid sprite file magic");
    f.close();
    return false;
  }

  if (width != EXPECTED_WIDTH || height != EXPECTED_HEIGHT || count > MAX_SPRITES) {
    Serial.println("Sprite file format mismatch");
    f.close();
    return false;
  }

  // Search each sprite block
  for (uint16_t i = 0; i < count; i++) {
    char symbol;
    uint8_t pad[7];

    if (!readExact(f, (uint8_t *)&symbol, 1) ||
        !readExact(f, pad, 7)) {
      Serial.println("Failed to read sprite entry header");
      f.close();
      return false;
    }

    if (symbol == target) {
      outSprite.symbol = symbol;
      if (!readExact(f, (uint8_t *)outSprite.rows, 64 * sizeof(uint64_t))) {
        Serial.println("Failed to read sprite bitmap");
        f.close();
        return false;
      }

      f.close();
      return true;
    } else {
      // Skip bitmap data for non-matching sprite
      if (!f.seek(f.position() + 64 * sizeof(uint64_t))) {
        Serial.println("Failed to skip sprite bitmap");
        f.close();
        return false;
      }
    }
  }

  f.close();
  Serial.printf("Sprite '%c' not found\n", target);
  return false;
}

// Faster version if you already know the sprite index:
// file order is: 0 1 2 3 4 5 6 7 8 9 :
int spriteIndexForChar(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c == ':') return 10;
  return -1;
}

bool loadSpriteByIndex(fs::FS &fs, const char *path, int index, Sprite36x64 &outSprite) {
  if (index < 0 || index >= MAX_SPRITES) {
    Serial.println("Invalid sprite index");
    return false;
  }

  File f = fs.open(path, FILE_READ);
  if (!f) {
    Serial.println("Failed to open sprite file");
    return false;
  }

  char magic[4];
  uint16_t width, height, count, reserved;

  if (!readExact(f, (uint8_t *)magic, 4) ||
      !readExact(f, (uint8_t *)&width, 2) ||
      !readExact(f, (uint8_t *)&height, 2) ||
      !readExact(f, (uint8_t *)&count, 2) ||
      !readExact(f, (uint8_t *)&reserved, 2)) {
    Serial.println("Failed to read sprite file header");
    f.close();
    return false;
  }

  if (memcmp(magic, "SPRT", 4) != 0 ||
      width != EXPECTED_WIDTH ||
      height != EXPECTED_HEIGHT ||
      count > MAX_SPRITES ||
      index >= count) {
    Serial.println("Sprite file format mismatch or index out of range");
    f.close();
    return false;
  }

  size_t spriteOffset = FILE_HEADER_SIZE + ((size_t)index * SPRITE_BLOCK_SIZE);

  if (!f.seek(spriteOffset)) {
    Serial.println("Failed to seek to sprite");
    f.close();
    return false;
  }

  if (!readExact(f, (uint8_t *)&outSprite.symbol, 1)) {
    Serial.println("Failed to read sprite symbol");
    f.close();
    return false;
  }

  uint8_t pad[7];
  if (!readExact(f, pad, 7)) {
    Serial.println("Failed to read sprite padding");
    f.close();
    return false;
  }

  if (!readExact(f, (uint8_t *)outSprite.rows, 64 * sizeof(uint64_t))) {
    Serial.println("Failed to read sprite bitmap");
    f.close();
    return false;
  }

  f.close();
  return true;
}

bool loadSpriteForCharFast(fs::FS &fs, const char *path, char c, Sprite36x64 &outSprite) {
  int idx = spriteIndexForChar(c);
  if (idx < 0) {
    Serial.printf("Unsupported character '%c'\n", c);
    return false;
  }
  return loadSpriteByIndex(fs, path, idx, outSprite);
}

// Loads all sprites for a time string like "12:34"
// maxLen should be large enough for your use
bool loadSpritesForString(fs::FS &fs, const char *path, const char *text, Sprite36x64 *outSprites, size_t maxLen, size_t &loadedCount) {
  loadedCount = 0;
  size_t len = strlen(text);

  if (len > maxLen) {
    Serial.println("Output sprite buffer too small");
    return false;
  }

  for (size_t i = 0; i < len; i++) {
    if (!loadSpriteForCharFast(fs, path, text[i], outSprites[i])) {
      Serial.printf("Failed to load sprite for character '%c'\n", text[i]);
      return false;
    }
    loadedCount++;
  }

  return true;
}

// ---------------- Debug printing ----------------

void printSpriteHexRows(const Sprite36x64 &sprite) {
  Serial.printf("Sprite '%c'\n", sprite.symbol);
  for (int row = 0; row < 64; row++) {
    Serial.printf("row %02d: 0x%09llX\n", row, (unsigned long long)sprite.rows[row]);
  }
}

void printSpriteAscii(const Sprite36x64 &sprite) {
  Serial.printf("ASCII preview for '%c'\n", sprite.symbol);
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 36; x++) {
      bool on = (sprite.rows[y] >> x) & 0x1ULL;
      Serial.print(on ? '#' : '.');
    }
    Serial.println();
  }
}

// ---------------- Setup / loop ----------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting SD sprite reader...");

  sdSPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, sdSPI)) {
    Serial.println("SD.begin() failed");
    return;
  }

  Serial.println("SD card initialized");

  SpriteFileHeader header;
  if (!loadSpriteHeader(SD, SPRITE_FILE_PATH, header)) {
    Serial.println("Sprite header check failed");
    return;
  }

  Serial.printf("Sprite file OK: %u x %u, count=%u\n",
                header.width, header.height, header.count);

  // Example 1: load a single sprite by character
  Sprite36x64 sprite3;
  if (loadSpriteForCharFast(SD, SPRITE_FILE_PATH, '3', sprite3)) {
    Serial.println("Loaded sprite for '3'");
    printSpriteHexRows(sprite3);
    // printSpriteAscii(sprite3);   // uncomment if you want a visual preview
  }

  // Example 2: load colon
  Sprite36x64 spriteColon;
  if (loadSpriteForCharFast(SD, SPRITE_FILE_PATH, ':', spriteColon)) {
    Serial.println("Loaded sprite for ':'");
    printSpriteHexRows(spriteColon);
  }

  // Example 3: load all sprites for a timer string
  const char *timeText = "12:34";
  Sprite36x64 textSprites[8];
  size_t loadedCount = 0;

  if (loadSpritesForString(SD, SPRITE_FILE_PATH, timeText, textSprites, 8, loadedCount)) {
    Serial.printf("Loaded %u sprites for \"%s\"\n", (unsigned)loadedCount, timeText);
    for (size_t i = 0; i < loadedCount; i++) {
      Serial.printf("Character %c loaded as sprite '%c'\n", timeText[i], textSprites[i].symbol);
    }
  }
}

void loop() {
  // Nothing here for now.
}