#pragma once

#include "z64.h"

typedef enum {
    BH_OPTIONS_HEARTS,
    BH_OPTIONS_MAGIC,
    BH_OPTIONS_MAX,
} BattleHallOptionEnums;

typedef enum {
    BH_CHOICE_HEARTS_20,
    BH_CHOICE_HEARTS_OHKO,
    BH_CHOICE_HEARTS_3,
    BH_CHOICE_HEARTS_5,
    BH_CHOICE_HEARTS_7,
    BH_CHOICE_HEARTS_10,
    BH_CHOICE_HEARTS_15,
} BattleHallHeartsChoices;

typedef enum {
    BH_CHOICE_MAGIC_NONE,
    BH_CHOICE_MAGIC_SINGLE,
    BH_CHOICE_MAGIC_DOUBLE,
} BattleHallMagicChoices;

#ifdef __cplusplus
extern "C" {
#endif

void BattleHall_InitSave();
void BattleHall_RegisterHooks();

const char* BattleHall_GetSettingName(u8 optionIndex, u8 language);
const char* BattleHall_GetSettingChoiceName(u8 optionIndex, u8 choiceIndex, u8 language);
u8 BattleHall_GetSettingOptionsAmount(u8 optionIndex);
u8 BattleHall_GetSettingsAmount();
#ifdef __cplusplus
};
#endif