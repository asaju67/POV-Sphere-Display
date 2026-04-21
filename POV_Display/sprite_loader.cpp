#include "sprite_loader.h"
#include <SPI.h>
#include <SD.h>

static const int SD_MISO = 19;
static const int SD_MOSI = 23;
static const int SD_CLK  = 18;
static const int SD_CS   = 5;

static SPIClass sdSPI(VSPI);
static SpriteFile36x64 gSpriteFile;
static bool gSpriteFileLoaded = false;

static bool readExact(File &f, uint8_t *buf, size_t len) {
  return f.read(buf, len) == (int)len;
}

bool initSpriteSd() {
  sdSPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, sdSPI)) {
    Serial.println("SD.begin() failed");
    return false;
  }

  return true;
}

bool loadSpriteFile(SpriteFile36x64& outSpriteFile) {
  File f = SD.open(SPRITE_FILE_PATH, FILE_READ);
  if (!f) {
    Serial.printf("Failed to open sprite file: %s\n", SPRITE_FILE_PATH);
    return false;
  }

  char magic[4];
  if (!readExact(f, (uint8_t *)magic, 4) ||
      !readExact(f, (uint8_t *)&outSpriteFile.width, 2) ||
      !readExact(f, (uint8_t *)&outSpriteFile.height, 2) ||
      !readExact(f, (uint8_t *)&outSpriteFile.count, 2)) {
    Serial.println("Failed to read sprite file header");
    f.close();
    return false;
  }

  uint16_t reserved;
  if (!readExact(f, (uint8_t *)&reserved, 2)) {
    Serial.println("Failed to read sprite file reserved bytes");
    f.close();
    return false;
  }

  if (memcmp(magic, "SPRT", 4) != 0) {
    Serial.println("Invalid sprite file magic");
    f.close();
    return false;
  }

  if (outSpriteFile.width != SPRITE_FILE_WIDTH ||
      outSpriteFile.height != SPRITE_FILE_HEIGHT ||
      outSpriteFile.count == 0 ||
      outSpriteFile.count > SPRITE_FILE_MAX_COUNT) {
    Serial.printf("Unexpected sprite file header: %u x %u, count=%u\n",
                  outSpriteFile.width,
                  outSpriteFile.height,
                  outSpriteFile.count);
    f.close();
    return false;
  }

  for (uint16_t i = 0; i < outSpriteFile.count; i++) {
    if (!readExact(f, (uint8_t *)&outSpriteFile.sprites[i].symbol, 1)) {
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

    if (!readExact(f, (uint8_t *)outSpriteFile.sprites[i].rows, SPRITE_ROW_COUNT * sizeof(uint64_t))) {
      Serial.println("Failed to read sprite bitmap rows");
      f.close();
      return false;
    }
  }

  f.close();
  gSpriteFile = outSpriteFile;
  gSpriteFileLoaded = true;
  return true;
}

const SpriteFile36x64* getLoadedSpriteFile() {
  return gSpriteFileLoaded ? &gSpriteFile : nullptr;
}

const Sprite36x64* getSpriteBySymbol(const SpriteFile36x64& spriteFile, char symbol) {
  for (uint16_t i = 0; i < spriteFile.count; i++) {
    if (spriteFile.sprites[i].symbol == symbol) {
      return &spriteFile.sprites[i];
    }
  }
  return nullptr;
}

bool buildPatternsFromText(const SpriteFile36x64& spriteFile, const char* text, uint64_t* outPatterns, int& outPatternCount, int maxPatterns) {
  if (!text || !outPatterns || maxPatterns <= 0) return false;

  outPatternCount = 0;
  size_t len = strlen(text);
  if (len == 0) return false;

  for (size_t ti = 0; ti < len; ti++) {
    char symbol = text[ti];

    if (symbol == ' ') {
      for (int s = 0; s < SPRITE_INTER_CHAR_SPACING; s++) {
        if (outPatternCount >= maxPatterns) return false;
        outPatterns[outPatternCount++] = 0ULL;
      }
      continue;
    }

    const Sprite36x64* sprite = getSpriteBySymbol(spriteFile, symbol);
    if (!sprite) {
      Serial.printf("Unsupported sprite character: '%c'\n", symbol);
      return false;
    }

    for (int x = 0; x < SPRITE_FILE_WIDTH; x++) {
      if (outPatternCount >= maxPatterns) return false;

      uint64_t columnPattern = 0ULL;
      for (int row = 0; row < SPRITE_FILE_HEIGHT; row++) {
        if ((sprite->rows[row] >> x) & 1ULL) {
          columnPattern |= (1ULL << row);
        }
      }

      outPatterns[outPatternCount++] = columnPattern;
    }

    if (ti + 1 < len) {
      for (int s = 0; s < SPRITE_INTER_CHAR_SPACING; s++) {
        if (outPatternCount >= maxPatterns) return false;
        outPatterns[outPatternCount++] = 0ULL;
      }
    }
  }

  return outPatternCount > 0;
}
