#ifndef SPRITE_LOADER_H
#define SPRITE_LOADER_H

#include <Arduino.h>
#include <stdint.h>

static constexpr const char SPRITE_FILE_PATH[] = "/number_sprites_36x64.spr";
static constexpr uint16_t SPRITE_FILE_WIDTH = 36;
static constexpr uint16_t SPRITE_FILE_HEIGHT = 64;
static constexpr uint16_t SPRITE_FILE_MAX_COUNT = 11;
static constexpr size_t SPRITE_ROW_COUNT = SPRITE_FILE_HEIGHT;
static constexpr size_t SPRITE_FILE_BLOCK_SIZE = 1 + 7 + (SPRITE_ROW_COUNT * sizeof(uint64_t));
static constexpr size_t SPRITE_FILE_HEADER_SIZE = 12;
static constexpr int SPRITE_INTER_CHAR_SPACING = 2;

struct Sprite36x64 {
  char symbol;
  uint64_t rows[SPRITE_ROW_COUNT];
};

struct SpriteFile36x64 {
  uint16_t width;
  uint16_t height;
  uint16_t count;
  Sprite36x64 sprites[SPRITE_FILE_MAX_COUNT];
};

bool initSpriteSd();
bool loadSpriteFile(SpriteFile36x64& outSpriteFile);
const SpriteFile36x64* getLoadedSpriteFile();
const Sprite36x64* getSpriteBySymbol(const SpriteFile36x64& spriteFile, char symbol);
bool buildPatternsFromText(const SpriteFile36x64& spriteFile, const char* text, uint64_t* outPatterns, int& outPatternCount, int maxPatterns);

#endif
