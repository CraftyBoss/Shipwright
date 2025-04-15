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

#define DISTANCE_SCALE 10.0f
#define LOOP_POINT_Z 2571.4f

#ifdef __cplusplus

struct BattleHallData {
    Vec3f curLoopOffset = { 0.0f, 0.0f, 0.0f };
    float totalRunDist;
};

void BattleHall_WarpPlayer(Vec3f* movePos);
Actor* BattleHall_SpawnActorWithName(ActorID id, u32 params, Vec3f* pos, const char* name);

extern "C" {
#endif

void BattleHall_InitSave();
void BattleHall_RegisterHooks();
void BattleHall_InitSystems();

const char* BattleHall_GetSettingName(u8 optionIndex, u8 language);
const char* BattleHall_GetSettingChoiceName(u8 optionIndex, u8 choiceIndex, u8 language);
u8 BattleHall_GetSettingOptionsAmount(u8 optionIndex);
u8 BattleHall_GetSettingsAmount();

#ifdef __cplusplus
};
#endif