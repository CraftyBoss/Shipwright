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

BattleHallData sHallData;
std::shared_ptr<BattleHallDebugWindow> sDbgWindow;
std::shared_ptr<BattleHallDistWindow> sDistWindow;

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
    sDistWindow->SetDistance(sHallData.totalRunDist);

    auto relativeHallPos = Vec3f_();
    Math_Vec3f_Diff(&playerPos, &sHallData.curLoopOffset, &relativeHallPos);

    float offsetVal = (LOOP_POINT_Z * 2) + 2.0f;

    if (relativeHallPos.z > LOOP_POINT_Z) {
        Vec3f_ movePos = Vec3f_(playerPos.x, playerPos.y, playerPos.z - offsetVal);
        BattleHall_WarpPlayer(&movePos);
    } else if (relativeHallPos.z < -LOOP_POINT_Z) {
        Vec3f_ movePos = Vec3f_(playerPos.x, playerPos.y, playerPos.z + offsetVal);
        BattleHall_WarpPlayer(&movePos);
    }
}

void BattleHall_RegisterHooks() {
    static u32 onVanillaBehaviorHook = 0;
    static u32 onGameFrameUpdateHook = 0;
    static u32 onPlayerUpdate = 0;

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>([](int32_t fileNum) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnVanillaBehavior>(onVanillaBehaviorHook);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameFrameUpdate>(onGameFrameUpdateHook);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameFrameUpdate>(onPlayerUpdate);

        onVanillaBehaviorHook = 0;
        onGameFrameUpdateHook = 0;
        onPlayerUpdate = 0;

        onVanillaBehaviorHook =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnVanillaBehavior>(BattleHallOnVanillaBehaviour);
        onGameFrameUpdateHook = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameFrameUpdate>(
            BattleHallOnGameFrameUpdateHandler);
        onPlayerUpdate = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerUpdate>(BattleHallOnPlayerUpdate);
    });
}

void BattleHall_InitSystems() {
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();

    sDistWindow = std::make_shared<BattleHallDistWindow>(CVAR_WINDOW("BHDistanceWindow"),
                                                                    "Battle Hall Distance Window", ImVec2(100, 50));
    gui->AddGuiWindow(sDistWindow);

    sDbgWindow = std::make_shared<BattleHallDebugWindow>(CVAR_WINDOW("BHDebugWindow"), "Battle Hall Debug Window",
                                                        ImVec2(600, 800));

    sDbgWindow->SetData(&sHallData);

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

Actor* BattleHall_SpawnActorWithName(ActorID id, u32 params, Vec3f* pos, const char* name) {
    auto actorEntry = ActorDB::Instance->RetrieveEntry(id);
    if (!actorEntry.entry.valid) {
        return nullptr;
    }

    auto* actor = Actor_Spawn(&gPlayState->actorCtx, gPlayState, id, pos->x, pos->y, pos->z, 0, 0, 0,
                                   params, false);
    NameTag_RegisterForActor(actor, name);
    return actor;
}