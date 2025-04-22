#pragma once

#include "z64.h"

typedef enum {
    BH_OPTIONS_HEARTS,
    BH_OPTIONS_MAGIC,
    BH_OPTIONS_HERO,
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
    BH_CHOICE_MAGIC_INF,
    BH_CHOICE_MAGIC_SINGLE,
    BH_CHOICE_MAGIC_DOUBLE,
} BattleHallMagicChoices;

typedef enum {
    BH_CHOICE_HERO_ON,
    BH_CHOICE_HERO_OFF,
} BattleHallHeroModeChoices;

typedef enum {
    BH_ACTOR_DEKUBABA,
    BH_ACTOR_KEESE,
    BH_ACTOR_MADSCRUB,
    BH_ACTOR_GOHMALARVA,
    BH_ACTOR_STALCHILD,
    BH_ACTOR_STINGER,
    BH_ACTOR_SHABOM,
    BH_ACTOR_SKULLKID,
    BH_ACTOR_BIGSKULLTULA,
    BH_ACTOR_ARMOS,
    BH_ACTOR_BEAMOS,
    BH_ACTOR_SPIKE,
    BH_ACTOR_BARI,
    BH_ACTOR_TAILPASARAN,
    BH_ACTOR_FIREKEESE,
    BH_ACTOR_TEKTITE,
    BH_ACTOR_BABYDODONGO,
    BH_ACTOR_ANUBIS,
    BH_ACTOR_TORCHSLUG,
    BH_ACTOR_WOLFOS,
    BH_ACTOR_BIGDEKUBABA,
    BH_ACTOR_CLUBMOBLIN,
    BH_ACTOR_GREENBUBBLE, // TODO: if time permits, maybe mod these to function without paths?
    BH_ACTOR_REDBUBBLE,
    BH_ACTOR_BLUEBUBBLE,
    BH_ACTOR_POE,
    BH_ACTOR_GUAY,
    BH_ACTOR_ICEKEESE,
    BH_ACTOR_REDEAD,
    BH_ACTOR_SHELLBLADE,
    BH_ACTOR_WHITEWOLFOS,
    BH_ACTOR_LEEVER,
    BH_ACTOR_DODONGO,
    BH_ACTOR_LIZALFOS,
    BH_ACTOR_WHITEBUBBLE,
    BH_ACTOR_FLOORMASTER,
    BH_ACTOR_GIBDO,
    BH_ACTOR_PEAHAT,
    BH_ACTOR_DINOLFOS,
    BH_ACTOR_SHARP,
    BH_ACTOR_FREEZARD,
    BH_ACTOR_STALFOS,
    BH_ACTOR_IRONKNUCKLE,
    BH_ACTOR_DARKLINK,
    BH_ACTOR_MAX
} BattleHallValidActors;

typedef enum {
    BH_ACTORS_VERY_EASY = BH_ACTOR_DEKUBABA,
    BH_ACTORS_EASY = BH_ACTOR_SKULLKID,
    BH_ACTORS_MEDIUM = BH_ACTOR_ANUBIS,
    BH_ACTORS_HARD = BH_ACTOR_WHITEBUBBLE,
    BH_ACTORS_VERY_HARD = BH_ACTOR_IRONKNUCKLE,
    BH_ACTORS_MAX = BH_ACTOR_MAX,
} BattleHallActorTypes;

#define DISTANCE_SCALE 10.0f
#define LOOP_POINT_Z 2571.4f
#define ROOM_BOUND_X 200.0f
#define ROOM_BOUND_Z 800.0f
#define FLAG_OPEN_GATE 0xA

#ifdef __cplusplus

#include <vector>
#include <string>

struct ActorQueueEntry {
    BattleHallValidActors type;
    std::string name;
};

struct BattleHallData {
    Vec3f curLoopOffset = { 0.0f, 0.0f, 0.0f };
    std::vector<Actor*> curAliveActors;
    std::vector<ActorQueueEntry> actorQueue;
    float totalRunDist = 0.0f;
    int loopCount = 0;
    bool isPushPlayer = false;
    bool isSpawnActorsInQueue = true;
    bool isSpawnInitActors = false;
    bool isSpawningActors = false;
};

struct HallActorData {
    ActorID id;
    f32 yOffset;
    s16 params;
};

void BattleHall_WarpPlayer(Vec3f* movePos);
Actor* BattleHall_SpawnActorWithName(BattleHallValidActors type, const char* name);
void BattleHall_RegisterActor(BattleHallValidActors type, const char* name);
int BattleHall_RandRange(int min, int max);
const char* BattleHall_GetRandomTestName();
void BattleHall_GetRandomVec3f(Vec3f* out, const Vec3f& minCoord, const Vec3f& maxCoord);
void BattleHall_QueueAllAvailableActors();

extern "C" {
#endif

void BattleHall_InitSave();
void BattleHall_RegisterHooks();
void BattleHall_InitSystems();

void BattleHall_OnChildActorSpawn(Actor* actor, Actor* parent);

const char* BattleHall_GetSettingName(u8 optionIndex, u8 language);
const char* BattleHall_GetSettingChoiceName(u8 optionIndex, u8 choiceIndex, u8 language);
u8 BattleHall_GetSettingOptionsAmount(u8 optionIndex);
u8 BattleHall_GetSettingsAmount();
void BattleHall_Vec3f_SetRandom(Vec3f* out);

#ifdef __cplusplus
};
#endif