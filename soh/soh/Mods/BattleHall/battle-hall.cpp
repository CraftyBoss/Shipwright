#include "battle-hall.h"
#include "soh/OTRGlobals.h"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/Enhancements/enhancementTypes.h"

#include <array>
#include <string>
#include <vector>
#include <StringHelper.h>

#include "BattleHallDistanceWindow.h"
#include "BattleHallDebugWindow.h"

extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
#include <soh/ActorDB.h>
#include <soh/Enhancements/nametag.h>
extern PlayState* gPlayState;
}

#define GetBHOptionFlag(Flag) gSaveContext.ship.quest.data.bossRush.options[Flag]
#define CheckBHOptionFlag(Flag, Value) gSaveContext.ship.quest.data.bossRush.options[Flag] == Value

template <typename T, std::size_t LL, std::size_t RL>
constexpr std::array<T, LL + RL> join(std::array<T, LL> rhs, std::array<T, RL> lhs) {
    std::array<T, LL + RL> ar;

    auto current = std::copy(rhs.begin(), rhs.end(), ar.begin());
    std::copy(lhs.begin(), lhs.end(), current);

    return ar;
}

struct {
    std::array<std::string, LANGUAGE_MAX> name;
    std::vector<std::array<std::string, LANGUAGE_MAX>> choices;
} BattleHallOptions[] = {
    { { "HEARTS:", "HERZEN:", "COEURS:" },
      {
          { "20", "20", "20" },
          { "OHKO", "OHKO", "OHKO" },
          { "3", "3", "3" },
          { "5", "5", "5" },
          { "7", "7", "7" },
          { "10", "10", "10" },
          { "15", "15", "15" },
      } },
    { { "MAGIC:", "MAGIE:", "MAGIE:" },
      {
          { "None", "None", "None" },
          { "Single", "Einzel", "Simple" },
          { "Double", "Doppel", "Double" },
      } }
};

const char* testPlayerNames[] = { "CraftyBoss", "xGeoff",       "Eri",   "Fenix",   "Kasai",
                                  "Duprific",   "SuperMCGamer", "jyggy", "Smallant" };

auto hallActorsVeryEasy = std::to_array<HallActorData>({
    { ACTOR_EN_DEKUBABA, 0.0f, 0x0 },        // Deku Baba (controls size)
    { ACTOR_EN_FIREFLY, 80.0f, 0x2 },        // Keese (determines type, visibility, and something else)
    { ACTOR_EN_DEKUNUTS, 0.0f, 0x300 },      // Mad Scrub (param controls amount of shots) TODO: find a way to despawn the flower that the scrub leaves behind on death)
    { ACTOR_EN_GOMA, 0.0f, 0x7 },            // Gohma Larva (Invisible until Hatch) (0x6 = Normal Egg on Ground)
    { ACTOR_EN_SKB, 0.0f, 0x4 },             // Stalchild (param is size in steps of 2)
    { ACTOR_EN_EIYER, 0.0f, 0xA },           // Stinger (param determines group type, here we select single)
    { ACTOR_EN_BUBBLE, 40.0f, (s16)0xFFFF }, // Shabom (param determines bubble type)
});

auto hallActorsEasy = std::to_array<HallActorData>({
    { ACTOR_EN_SKJ, 0.0f, (s16)0xFC00 },    // Skull Kid
    { ACTOR_EN_ST, 0.0f, 0x3 },             // Big Skulltula (maybe 0x3?)
    { ACTOR_EN_AM, 0.0f, 0x1 },             // Armos
    { ACTOR_EN_VM, 0.0f, 0x500 },           // Beamos (param determines sight dist in increments of 40.0f)
    { ACTOR_EN_NY, 0.0f, 0x0 },             // Spike
    { ACTOR_EN_VALI, 300.0f, -1 },          // Bari
    { ACTOR_EN_TP, 0.0f, (s16)0xFFFF },     // Tail Pasaran
    { ACTOR_EN_FIREFLY, 0.0f, 0x0 },        // Fire Keese
    { ACTOR_EN_TITE, 0.0f, 0x0 },           // Tektite
    { ACTOR_EN_DODOJR, 0.0f, (s16)0xFFFF }, // Baby Dodongo
});

auto hallActorsMedium = std::to_array<HallActorData>({
    //{ ACTOR_EN_ANUBICE, 0.0f, 0x0 }, // Anubis (how to kill???)
    { ACTOR_EN_BW, 0.0f, 0x0 },        // Torch Slug
    { ACTOR_EN_WF, 0.0f, 0x0 },        // Wolfos
    //{ ACTOR_EN_DH, 0.0f, 0x0 },      // Dead Hand
    { ACTOR_EN_DEKUBABA, 0.0f, 0x1 },  // Big Deku Baba
    { ACTOR_EN_MB, 0.0f, 0x0 },        // Club Moblin (0x1 = Spear Moblin)
    { ACTOR_EN_BB, 0.0f, 0xFB },       // Large Green Bubble
    { ACTOR_EN_BB, 0.0f, 0xFE },       // Red Bubble
    { ACTOR_EN_BB, 0.0f, 0xFF },       // Blue Bubble
    { ACTOR_EN_POH, 0.0f, 0x0 },       // Poe
    { ACTOR_EN_CROW, 0.0f, 0x0 },      // Guay
    { ACTOR_EN_FIREFLY, 0.0f, 0x4 },   // Ice Keese
    { ACTOR_EN_RD, 0.0f, 0x0 },        // ReDead
    { ACTOR_EN_SB, 0.0f, 0x0 },        // Shell Blade
    { ACTOR_EN_WF, 0.0f, (s16)0xFF01 },        // White Wolfos
    { ACTOR_EN_REEBA, 0.0f, 0x0 },     // Leever
    { ACTOR_EN_DODONGO, 0.0f, 0x0 },   // Dodongo
    { ACTOR_EN_ZF, 0.0f, 0x80 },       // Lizalfos
});

auto hallActorsHard = std::to_array<HallActorData>({
    { ACTOR_EN_BB, 0.0f, 0xFD },     // White Bubble
    { ACTOR_EN_FLOORMAS, 0.0f, 0x0 }, // Floormaster
    { ACTOR_EN_RD, 0.0f, 0xFE },      // Gibdo
    { ACTOR_EN_PEEHAT, 0.0f, -1 },   // Peahat
    { ACTOR_EN_ZF, 0.0f, 0xFE },      // Dinolfos
    { ACTOR_EN_POH, 0.0f, 0x2 },      // Sharp (or Flat)
    { ACTOR_EN_FZ, 0.0f, 0x0 },       // Freezard
    { ACTOR_EN_TEST, 0.0f, 0x2 },     // Stalfos
    //{ ACTOR_EN_FD, 0.0f, 0x0 },     // Flare Dancer (oof)
});

auto hallActorsVeryHard = std::to_array<HallActorData>({
    { ACTOR_EN_IK, 0.0f, 0x3 }, // Iron Knuckle
    { ACTOR_EN_TORCH2, 0.0f, -1 }, // Dark Link
    //{ ACTOR_BOSS_GANON2, 0.0f, 0x0 }, // Probably not Ganon
});

//auto hallActorsAll = join(hallActorsEasy, hallActorsVeryEasy);
//auto hallActorsAll = join(hallActorsMedium, join(hallActorsEasy, hallActorsVeryEasy));
    //auto hallActorsAll = join(hallActorsHard, join(hallActorsMedium, join(hallActorsEasy, hallActorsVeryEasy)));
//auto hallActorsAll = join(hallActorsVeryHard, join(hallActorsHard, join(hallActorsMedium, join(hallActorsEasy, hallActorsVeryEasy))));
auto hallActorsAll =
    join(hallActorsVeryEasy, join(hallActorsEasy, join(hallActorsMedium, join(hallActorsHard, hallActorsVeryHard))));

auto hallSubActors = std::to_array<ActorID>({
    ACTOR_EN_BILI
});

BattleHallData sHallData;
std::shared_ptr<BattleHallDebugWindow> sDbgWindow;
std::shared_ptr<BattleHallDistWindow> sDistWindow;

const char* BattleHall_GetRandomTestName() {
    int randIndex = BattleHall_RandRange(0, std::size(testPlayerNames) - 1);

    if (randIndex < 0 || randIndex >= std::size(testPlayerNames)) {
        return "NULL";
    }

    return testPlayerNames[randIndex];
}

const char* BattleHall_GetSettingName(u8 optionIndex, u8 language) {
    return BattleHallOptions[optionIndex].name[language].c_str();
}

const char* BattleHall_GetSettingChoiceName(u8 optionIndex, u8 choiceIndex, u8 language) {
    return BattleHallOptions[optionIndex].choices[choiceIndex][language].c_str();
}

u8 BattleHall_GetSettingOptionsAmount(u8 optionIndex) {
    return BattleHallOptions[optionIndex].choices.size();
}

u8 BattleHall_GetSettingsAmount() {
    return ARRAYSIZE(BattleHallOptions);
}

// used by game actors
void BattleHall_Vec3f_SetRandom(Vec3f* out) {
    BattleHall_GetRandomVec3f(out, Vec3f(-ROOM_BOUND_X, 0, -ROOM_BOUND_Z), Vec3f(ROOM_BOUND_X, 0, ROOM_BOUND_Z));
}

// gets a random val between [min..max)
int BattleHall_RandRange(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void BattleHall_GetRandomVec3f(Vec3f* out, const Vec3f& minCoord, const Vec3f& maxCoord) {
    out->x = (f32)BattleHall_RandRange(minCoord.x, maxCoord.x);
    out->y = (f32)BattleHall_RandRange(minCoord.y, maxCoord.y);
    out->z = (f32)BattleHall_RandRange(minCoord.z, maxCoord.z);
}

void BattleHall_QueueAllAvailableActors() {
    for (size_t i = BH_ACTORS_VERY_EASY; i < BH_ACTORS_MAX; i++) {
        BattleHall_RegisterActor((BattleHallValidActors)i, BattleHall_GetRandomTestName());
    }
}

bool BattleHall_IsInAllowedActors(ActorID id) {
    for (size_t i = 0; i < hallActorsAll.size(); i++) {
        if (hallActorsAll[i].id == id)
            return true;
    }
    return false;
}

bool BattleHall_IsValidSubActor(ActorID id) {
    return std::find(hallSubActors.begin(), hallSubActors.end(), id) != hallSubActors.end();
}

f32 BattleHall_GetActorSpawnOffset(BattleHallValidActors type) {
    return hallActorsAll[type].yOffset;
}

s16 BattleHall_GetActorParams(BattleHallValidActors type) {
    return hallActorsAll[type].params;
}

void BattleHall_SetEquipment(u8 linkAge) {
    std::array<u8, 8> brButtonItems;
    std::array<u8, 7> brCButtonSlots;

    // Set Child Equipment.
    if (linkAge == LINK_AGE_CHILD) {
        brButtonItems = {
            ITEM_SWORD_KOKIRI, ITEM_STICK, ITEM_NUT, ITEM_BOMB, ITEM_NONE, ITEM_NONE, ITEM_NONE, ITEM_NONE
        };

        brCButtonSlots = { SLOT_STICK, SLOT_NUT, SLOT_BOMB, SLOT_NONE, SLOT_NONE, SLOT_NONE, SLOT_NONE };

        Inventory_ChangeEquipment(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_KOKIRI);
        Inventory_ChangeEquipment(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_DEKU);
        // Set Adult equipment.
    }
    else {
        brButtonItems = { ITEM_SWORD_MASTER, ITEM_BOW,  ITEM_HAMMER, ITEM_BOMB,
                          ITEM_NONE,         ITEM_NONE, ITEM_NONE,   ITEM_NONE };

        brCButtonSlots = { SLOT_BOW, SLOT_HAMMER, SLOT_BOMB, SLOT_NONE, SLOT_NONE, SLOT_NONE, SLOT_NONE };

        Inventory_ChangeEquipment(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_MASTER);
        Inventory_ChangeEquipment(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_MIRROR);
        Inventory_ChangeEquipment(EQUIP_TYPE_TUNIC, EQUIP_VALUE_TUNIC_GORON);
    }

    // Button Items
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.equips.buttonItems); button++) {
        gSaveContext.equips.buttonItems[button] = brButtonItems[button];
    }

    // C buttons
    for (int button = 0; button < ARRAY_COUNT(gSaveContext.equips.cButtonSlots); button++) {
        gSaveContext.equips.cButtonSlots[button] = brCButtonSlots[button];
    }
}

void BattleHall_InitSave() {

    // Set player name to Lonk for the few textboxes that show up during Boss Rush. Player can't input their own name.
    std::array<char, 8> brPlayerName = { 21, 50, 49, 46, 62, 62, 62, 62 };
    for (int i = 0; i < ARRAY_COUNT(gSaveContext.playerName); i++) {
        gSaveContext.playerName[i] = brPlayerName[i];
    }

    gSaveContext.ship.quest.id = QUEST_BATTLEHALL;
    gSaveContext.entranceIndex = ENTR_BATTLE_HALL_0;
    gSaveContext.cutsceneIndex = 0x8000;

    // Set magic
    if (!CheckBHOptionFlag(BH_OPTIONS_MAGIC, BH_CHOICE_MAGIC_NONE)) {
        gSaveContext.isMagicAcquired = 1;

        if (CheckBHOptionFlag(BH_OPTIONS_MAGIC, BR_CHOICE_MAGIC_SINGLE)) {
            gSaveContext.magicLevel = 1;
            gSaveContext.magic = 48;
        } else {
            gSaveContext.isDoubleMagicAcquired = 1;
            gSaveContext.magicLevel = 2;
            gSaveContext.magic = 96;
        }
    } else {
        gSaveContext.isMagicAcquired = 0;
    }

    // Set health
    u16 health = 16;
    switch (GetBHOptionFlag(BH_OPTIONS_HEARTS)) {
        case BH_CHOICE_HEARTS_3:
            health *= 3;
            break;
        case BH_CHOICE_HEARTS_5:
            health *= 5;
            break;
        case BH_CHOICE_HEARTS_7:
            health *= 7;
            break;
        case BH_CHOICE_HEARTS_10:
            health *= 10;
            break;
        case BH_CHOICE_HEARTS_15:
            health *= 15;
            break;
        case BH_CHOICE_HEARTS_20:
            health *= 20;
            break;
    default:
        break;
    }

    if (CheckBHOptionFlag(BH_OPTIONS_HEARTS, BH_CHOICE_HEARTS_OHKO)) {
        CVarSetInteger(CVAR_ENHANCEMENT("DamageMult"), DAMAGE_OHKO);
    } else {
        CVarSetInteger(CVAR_ENHANCEMENT("DamageMult"), DAMAGE_VANILLA);
    }

    gSaveContext.healthCapacity = health;
    gSaveContext.health = health;

    // Skip boss cutscenes
    gSaveContext.eventChkInf[7] |= 1;    // gohma
    gSaveContext.eventChkInf[7] |= 2;    // dodongo
    gSaveContext.eventChkInf[7] |= 4;    // phantom ganon
    gSaveContext.eventChkInf[7] |= 8;    // volvagia
    gSaveContext.eventChkInf[7] |= 0x10; // morpha
    gSaveContext.eventChkInf[7] |= 0x20; // twinrova
    gSaveContext.eventChkInf[7] |= 0x40; // barinade
    gSaveContext.eventChkInf[7] |= 0x80; // bongo bongo

    // Sets all rando flags to false (we use the infinite item flags for this gamemode)
    for (s32 i = 0; i < ARRAY_COUNT(gSaveContext.ship.randomizerInf); i++) {
        gSaveContext.ship.randomizerInf[i] = 0;
    }

    // set infinite items
    Flags_SetRandomizerInf(RAND_INF_HAS_INFINITE_BOMBCHUS);
    Flags_SetRandomizerInf(RAND_INF_HAS_INFINITE_BOMB_BAG);
    Flags_SetRandomizerInf(RAND_INF_HAS_INFINITE_BULLET_BAG);
    Flags_SetRandomizerInf(RAND_INF_HAS_INFINITE_NUT_UPGRADE);
    Flags_SetRandomizerInf(RAND_INF_HAS_INFINITE_QUIVER);
    Flags_SetRandomizerInf(RAND_INF_HAS_INFINITE_STICK_UPGRADE);

    // Set items
    static std::array<u8, 24> bhItems = {
        ITEM_STICK,     ITEM_NUT,          ITEM_BOMB,        ITEM_BOW,        ITEM_ARROW_FIRE,  ITEM_DINS_FIRE,
        ITEM_SLINGSHOT, ITEM_OCARINA_TIME, ITEM_BOMBCHU,     ITEM_LONGSHOT,   ITEM_ARROW_ICE,   ITEM_FARORES_WIND,
        ITEM_BOOMERANG, ITEM_LENS,         ITEM_BEAN,        ITEM_HAMMER,     ITEM_ARROW_LIGHT, ITEM_NAYRUS_LOVE,
        ITEM_FAIRY,     ITEM_FAIRY,        ITEM_POTION_BLUE, ITEM_POTION_RED, ITEM_CLAIM_CHECK, ITEM_MASK_BUNNY,
    };

    // remove fairies if in OHKO mode
    if (CheckBHOptionFlag(BH_OPTIONS_HEARTS, BH_CHOICE_HEARTS_OHKO)) {
        bhItems[18] = ITEM_BOTTLE;
        bhItems[19] = ITEM_BOTTLE;
    }

    for (int item = 0; item < ARRAY_COUNT(gSaveContext.inventory.items); item++) {
        gSaveContext.inventory.items[item] = bhItems[item];
    }

    // Set consumable counts
    std::array<s8, 16> brAmmo = { 30, 40, 40, 50, 0, 0, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    for (int ammo = 0; ammo < ARRAY_COUNT(gSaveContext.inventory.ammo); ammo++) {
        gSaveContext.inventory.ammo[ammo] = brAmmo[ammo];
    }

    // Equipment
    gSaveContext.inventory.equipment = 0x7777;
    gSaveContext.inventory.upgrades = 3597531;
    gSaveContext.inventory.questItems = 33554431;

    gSaveContext.bgsFlag = 1;

    // Upgrades (unsure if need to keep for infinite upgrades)

    u8 upgradeLevel = 3;

    Inventory_ChangeUpgrade(UPG_QUIVER, upgradeLevel);
    Inventory_ChangeUpgrade(UPG_BOMB_BAG, upgradeLevel);
    Inventory_ChangeUpgrade(UPG_BULLET_BAG, upgradeLevel);
    Inventory_ChangeUpgrade(UPG_STICKS, upgradeLevel);
    Inventory_ChangeUpgrade(UPG_NUTS, upgradeLevel);
    Inventory_ChangeUpgrade(UPG_STRENGTH, upgradeLevel);

    // Set flags and Link's age based on chosen settings.
    gSaveContext.linkAge = LINK_AGE_ADULT;
    BattleHall_SetEquipment(LINK_AGE_ADULT);
}

void BattleHall_RegisterActor(BattleHallValidActors type, const char* name) {
    sHallData.actorQueue.push_back({ 
        type,
        name
    });
}

void BattleHall_HandleActorSpawn() {
    if (sHallData.isSpawnActorsInQueue) {
        sHallData.isSpawnActorsInQueue = false;
        sHallData.isSpawningActors = true;

        sHallData.curAliveActors.clear();

        for (const auto& entry : sHallData.actorQueue) {
            BattleHall_SpawnActorWithName(entry.type, entry.name.c_str());
        }

        sHallData.actorQueue.clear();
        sHallData.isSpawningActors = false;
    }

    if (!sHallData.isSpawnActorsInQueue && sHallData.curAliveActors.empty()) {
        Flags_SetSwitch(gPlayState, FLAG_OPEN_GATE);
    }
}

void BattleHallOnVanillaBehaviour(GIVanillaBehavior id, bool* should, va_list originalArgs) {
    va_list args;
    va_copy(args, originalArgs);

    switch (id) {
        case VB_DRAW_AMMO_COUNT: {
            s16 item = *va_arg(args, s16*);
            // don't draw ammo count if you have the infinite upgrade
            if ((item == ITEM_NUT && Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_NUT_UPGRADE)) ||
                (item == ITEM_STICK && Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_STICK_UPGRADE)) ||
                (item == ITEM_BOMB && Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_BOMB_BAG)) ||
                ((item == ITEM_BOW || item == ITEM_BOW_ARROW_FIRE || item == ITEM_BOW_ARROW_ICE ||
                    item == ITEM_BOW_ARROW_LIGHT) &&
                    Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_QUIVER) && gPlayState->shootingGalleryStatus < 2 &&
                    gSaveContext.minigameState != 1) ||
                (item == ITEM_SLINGSHOT && Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_BULLET_BAG) &&
                    gPlayState->shootingGalleryStatus < 2) ||
                (item == ITEM_BOMBCHU && Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_BOMBCHUS) &&
                    gPlayState->bombchuBowlingStatus < 1)) {
                *should = false;
            }
            break;
        }
        case VB_CLOSE_PAUSE_MENU: {
            if (CHECK_BTN_ALL(gPlayState->state.input[0].press.button, BTN_B)) {
                *should = true;
            }
            break;
        }
        // Prevent saving
        case VB_BE_ABLE_TO_SAVE:
        // Rupees are useless in boss rush
        case VB_RENDER_RUPEE_COUNTER: {
            *should = false;
            break;
        }
        // Prevent warning spam
        default: {
            break;
        }
    }
}

void BattleHallOnGameFrameUpdateHandler() {
    if (Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_QUIVER)) {
        AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
    }

    if (Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_BOMB_BAG)) {
        AMMO(ITEM_BOMB) = CUR_CAPACITY(UPG_BOMB_BAG);
    }

    if (Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_BULLET_BAG)) {
        AMMO(ITEM_SLINGSHOT) = CUR_CAPACITY(UPG_BULLET_BAG);
    }

    if (Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_STICK_UPGRADE)) {
        AMMO(ITEM_STICK) = CUR_CAPACITY(UPG_STICKS);
    }

    if (Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_NUT_UPGRADE)) {
        AMMO(ITEM_NUT) = CUR_CAPACITY(UPG_NUTS);
    }

    if (Flags_GetRandomizerInf(RAND_INF_HAS_INFINITE_BOMBCHUS)) {
        AMMO(ITEM_BOMBCHU) = 50;
    }
}

void BattleHallOnPlayerUpdate() {
    Player* player = GET_PLAYER(gPlayState);
    Camera* camera = GET_ACTIVE_CAM(gPlayState);
    auto& playerPos = player->actor.world.pos;
    auto& camPos = camera->eye;

    sHallData.totalRunDist = Math_Vec3f_DistXYZ(&sHallData.curLoopOffset, &playerPos);

    BattleHall_HandleActorSpawn();

    float offsetVal = (LOOP_POINT_Z * 2) + 2.0f;

    if (playerPos.z > LOOP_POINT_Z) {
        Vec3f movePos = Vec3f(playerPos.x, playerPos.y, playerPos.z - offsetVal);
        sHallData.curLoopOffset.z -= offsetVal;
        sHallData.loopCount++;
        sHallData.isSpawnActorsInQueue = true;

        BattleHall_WarpPlayer(&movePos);
        Flags_UnsetSwitch(gPlayState, FLAG_OPEN_GATE);
    } else if (playerPos.z < -LOOP_POINT_Z) {
        sHallData.isPushPlayer = true;
        player->pushedSpeed = 6.0f;
        player->pushedYaw = 0;
    } else if (sHallData.isPushPlayer) {
        sHallData.isPushPlayer = false;
        player->pushedSpeed = 0.0f;
    }
}

void BattleHallOnSceneInit(u16 sceneNum) {
    if (sceneNum != SCENE_BATTLEHALL)
        return;

    sHallData.curAliveActors.clear();
    sHallData.actorQueue.clear();

    //BattleHall_QueueAllAvailableActors();
}

void BattleHallOnActorKillHook(void* actorPtr) {
    if (gPlayState->sceneNum != SCENE_BATTLEHALL)
        return;
    Actor* actor = (Actor*)actorPtr;
    ActorID id = (ActorID)actor->id;

    if (!BattleHall_IsInAllowedActors(id) && !BattleHall_IsValidSubActor(id))
        return;

    auto actorIter = std::find(sHallData.curAliveActors.begin(), sHallData.curAliveActors.end(), actor);
    if (actorIter != sHallData.curAliveActors.end()) {
        sHallData.curAliveActors.erase(actorIter);
    }
}

void BattleHallOnActorInitHook(void* actorPtr) {
    if (gPlayState->sceneNum != SCENE_BATTLEHALL)
        return;
    Actor* actor = (Actor*)actorPtr;
    ActorID id = (ActorID)actor->id;

    // allowed actors get handled by our spawning func, no need to catch it here
    if (BattleHall_IsInAllowedActors(id) || !BattleHall_IsValidSubActor(id))
        return;

    sHallData.curAliveActors.push_back(actor);
}

void BattleHall_RegisterHooks() {
    static u32 onVanillaBehaviorHook = 0;
    static u32 onGameFrameUpdateHook = 0;
    static u32 onPlayerUpdate = 0;
    static u32 onSceneInitHook = 0;
    static u32 onActorInitHook = 0;
    static u32 onActorKillHook = 0;

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>([](int32_t fileNum) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnVanillaBehavior>(onVanillaBehaviorHook);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameFrameUpdate>(onGameFrameUpdateHook);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnPlayerUpdate>(onPlayerUpdate);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(onSceneInitHook);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorKill>(onActorKillHook);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorInit>(onActorInitHook);

        onVanillaBehaviorHook = 0;
        onGameFrameUpdateHook = 0;
        onPlayerUpdate = 0;
        onSceneInitHook = 0;
        onActorKillHook = 0;
        onActorInitHook = 0;

        onVanillaBehaviorHook =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnVanillaBehavior>(BattleHallOnVanillaBehaviour);
        onGameFrameUpdateHook = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameFrameUpdate>(
            BattleHallOnGameFrameUpdateHandler);
        onPlayerUpdate =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(BattleHallOnPlayerUpdate);
        onSceneInitHook =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>(BattleHallOnSceneInit);
        onActorKillHook =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorKill>(BattleHallOnActorKillHook);
        onActorInitHook =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnActorInit>(BattleHallOnActorInitHook);
    });
}

void BattleHall_InitSystems() {
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();

    sDistWindow = std::make_shared<BattleHallDistWindow>(CVAR_WINDOW("BHDistanceWindow"),
                                                                    "Battle Hall Distance Window", ImVec2(100, 50));
   
    sDbgWindow = std::make_shared<BattleHallDebugWindow>(CVAR_WINDOW("BHDebugWindow"), "Battle Hall Debug Window",
                                                        ImVec2(600, 800));

    sDistWindow->SetHallData(&sHallData);
    sDbgWindow->SetData(&sHallData);

    gui->AddGuiWindow(sDistWindow);
    gui->AddGuiWindow(sDbgWindow);
}

void BattleHall_WarpPlayer(Vec3f* movePos) {
    Player* player = GET_PLAYER(gPlayState);
    Camera* camera = GET_ACTIVE_CAM(gPlayState);

    auto& playerPos = player->actor.world.pos;

    Vec3f relEyePos = Vec3f(camera->eye.x - playerPos.x, camera->eye.y - playerPos.y, camera->eye.z - playerPos.z);
    Vec3f camEyePos = Vec3f(relEyePos.x + movePos->x, relEyePos.y + movePos->y, relEyePos.z + movePos->z);
    Vec3f camAtPos =
        Vec3f(camera->posOffset.x + movePos->x, camera->posOffset.y + movePos->y, camera->posOffset.z + movePos->z);

    Math_Vec3f_Copy(&playerPos, movePos);
    Play_CameraSetAtEye(gPlayState, gPlayState->activeCamera, &camAtPos, &camEyePos);
}

void BattleHall_PushPlayer(float zSpeed) {
    Player* player = GET_PLAYER(gPlayState);
    auto& playerVel = player->actor.velocity;

    player->actor.velocity.z += zSpeed;
}

Actor* BattleHall_SpawnActorWithName(BattleHallValidActors type, const char* name) {
    auto& actorEntry = hallActorsAll[type];
    
    auto& dbEntry = ActorDB::Instance->RetrieveEntry(actorEntry.id);

    SPDLOG_TRACE("Spawned Actor: %s", dbEntry.desc.c_str());

    Vec3f spawnPos = Vec3f();
    BattleHall_GetRandomVec3f(&spawnPos, Vec3f(-ROOM_BOUND_X, 0, -ROOM_BOUND_Z), Vec3f(ROOM_BOUND_X, 0, ROOM_BOUND_Z));
    spawnPos.y = BattleHall_GetActorSpawnOffset(type);

    auto* actor = Actor_Spawn(&gPlayState->actorCtx, gPlayState, actorEntry.id, spawnPos.x, spawnPos.y, spawnPos.z, 0,
                              0, 0,
                              BattleHall_GetActorParams(type), false);
    if (actor == nullptr) {
        return actor;
    }

    // actor was killed during init
    if (actor->update == NULL && actor->draw == NULL) {
        return nullptr;
    }

    NameTag_RegisterForActor(actor, name);

    sHallData.curAliveActors.push_back(actor);

    return actor;
}