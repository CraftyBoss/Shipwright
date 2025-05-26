/*
 * File: z_door_shutter.c
 * Overlay: ovl_Door_Shutter
 * Description: Sliding doors, Phantom Ganon room bars, Gohma room rock slab
 */

#include "z_door_shutter.h"
#include "overlays/actors/ovl_Boss_Goma/z_boss_goma.h"

#include "objects/object_gnd/object_gnd.h"
#include "objects/object_goma/object_goma.h"
#include "objects/object_ydan_objects/object_ydan_objects.h"
#include "objects/object_ddan_objects/object_ddan_objects.h"
#include "objects/object_bdan_objects/object_bdan_objects.h"
#include "objects/gameplay_keep/gameplay_keep.h"
#include "objects/object_bdoor/object_bdoor.h"
#include "objects/object_hidan_objects/object_hidan_objects.h"
#include "objects/object_ganon_objects/object_ganon_objects.h"
#include "objects/object_jya_door/object_jya_door.h"
#include "objects/object_mizu_objects/object_mizu_objects.h"
#include "objects/object_haka_door/object_haka_door.h"
#include "objects/object_ice_objects/object_ice_objects.h"
#include "objects/object_menkuri_objects/object_menkuri_objects.h"
#include "objects/object_demo_kekkai/object_demo_kekkai.h"
#include "objects/object_ouke_haka/object_ouke_haka.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/Enhancements/randomizer/randomizer_entrance.h"

#define FLAGS ACTOR_FLAG_UPDATE_CULLING_DISABLED

void DoorShutter_Init(Actor* thisx, PlayState* play);
void DoorShutter_Destroy(Actor* thisx, PlayState* play);
void DoorShutter_Update(Actor* thisx, PlayState* play);
void DoorShutter_Draw(Actor* thisx, PlayState* play);

void func_8099803C(PlayState* play, s16 y, s16 countdown, s16 arg3);
void DoorShutter_SetupType(DoorShutter* this, PlayState* play);
void func_80996A54(DoorShutter* this, PlayState* play);
void func_80996B00(DoorShutter* this, PlayState* play);
void func_80996B0C(DoorShutter* this, PlayState* play);
void func_80996EE8(DoorShutter* this, PlayState* play);
void func_80996F98(DoorShutter* this, PlayState* play);
void func_80997004(DoorShutter* this, PlayState* play);
void func_80997150(DoorShutter* this, PlayState* play);
void func_809973E8(DoorShutter* this, PlayState* play);
void func_80997528(DoorShutter* this, PlayState* play);
void func_80997568(DoorShutter* this, PlayState* play);
void func_809975C0(DoorShutter* this, PlayState* play);
void func_809976B8(DoorShutter* this, PlayState* play);
void func_80997744(DoorShutter* this, PlayState* play);

const ActorInit Door_Shutter_InitVars = {
    ACTOR_DOOR_SHUTTER,
    ACTORCAT_DOOR,
    FLAGS,
    OBJECT_GAMEPLAY_KEEP,
    sizeof(DoorShutter),
    (ActorFunc)DoorShutter_Init,
    (ActorFunc)DoorShutter_Destroy,
    (ActorFunc)DoorShutter_Update,
    (ActorFunc)DoorShutter_Draw,
    NULL,
};

typedef enum DoorShutterGfxType {
    /*  0 */ DOORSHUTTER_GFX_DEKU_TREE_1,
    /*  1 */ DOORSHUTTER_GFX_DEKU_TREE_2,
    /*  2 */ DOORSHUTTER_GFX_DODONGOS_CAVERN,
    /*  3 */ DOORSHUTTER_GFX_JABU_JABU,
    /*  4 */ DOORSHUTTER_GFX_PHANTOM_GANON_BARS,
    /*  5 */ DOORSHUTTER_GFX_GOHMA_BLOCK,
    /*  6 */ DOORSHUTTER_GFX_SPIRIT_TEMPLE,
    /*  7 */ DOORSHUTTER_GFX_BOSS_DOOR,
    /*  8 */ DOORSHUTTER_GFX_GENERIC,
    /*  9 */ DOORSHUTTER_GFX_FIRE_TEMPLE_1,
    /* 10 */ DOORSHUTTER_GFX_FIRE_TEMPLE_2,
    /* 11 */ DOORSHUTTER_GFX_GANONS_TOWER,
    /* 12 */ DOORSHUTTER_GFX_WATER_TEMPLE_1,
    /* 13 */ DOORSHUTTER_GFX_WATER_TEMPLE_2,
    /* 14 */ DOORSHUTTER_GFX_SHADOW_TEMPLE_1,
    /* 15 */ DOORSHUTTER_GFX_SHADOW_TEMPLE_2,
    /* 16 */ DOORSHUTTER_GFX_ICE_CAVERN,
    /* 17 */ DOORSHUTTER_GFX_GERUDO_TRAINING_GROUND,
    /* 18 */ DOORSHUTTER_GFX_GANONS_CASTLE,
    /* 19 */ DOORSHUTTER_GFX_ROYAL_FAMILYS_TOMB
} DoorShutterGfxType;

typedef enum DoorShutterStyleType {
    /* -1 */ DOORSHUTTER_STYLE_FROM_SCENE = -1, // Style is taken from `sSceneInfo`
    /*  0 */ DOORSHUTTER_STYLE_PHANTOM_GANON,
    /*  1 */ DOORSHUTTER_STYLE_GOHMA_BLOCK,
    /*  2 */ DOORSHUTTER_STYLE_DEKU_TREE,
    /*  3 */ DOORSHUTTER_STYLE_DODONGOS_CAVERN,
    /*  4 */ DOORSHUTTER_STYLE_JABU_JABU,
    /*  5 */ DOORSHUTTER_STYLE_FOREST_TEMPLE,
    /*  6 */ DOORSHUTTER_STYLE_BOSS_DOOR,
    /*  7 */ DOORSHUTTER_STYLE_GENERIC, // Default for some `DoorShutterType`s
    /*  8 */ DOORSHUTTER_STYLE_FIRE_TEMPLE,
    /*  9 */ DOORSHUTTER_STYLE_GANONS_TOWER,
    /* 10 */ DOORSHUTTER_STYLE_SPIRIT_TEMPLE,
    /* 11 */ DOORSHUTTER_STYLE_WATER_TEMPLE,
    /* 12 */ DOORSHUTTER_STYLE_SHADOW_TEMPLE,
    /* 13 */ DOORSHUTTER_STYLE_ICE_CAVERN,
    /* 14 */ DOORSHUTTER_STYLE_GERUDO_TRAINING_GROUND,
    /* 15 */ DOORSHUTTER_STYLE_GANONS_CASTLE,
    /* 16 */ DOORSHUTTER_STYLE_ROYAL_FAMILYS_TOMB
} DoorShutterStyleType;

typedef struct {
    s16 objectId;
    u8 index1;
    u8 index2;
} DoorShutterStyleInfo;

static DoorShutterStyleInfo sStyleInfo[] = {
    /* DOORSHUTTER_STYLE_PHANTOM_GANON */
    {
        OBJECT_GND,
        DOORSHUTTER_GFX_PHANTOM_GANON_BARS,
        DOORSHUTTER_GFX_PHANTOM_GANON_BARS,
    },
    /* DOORSHUTTER_STYLE_GOHMA_BLOCK */
    {
        OBJECT_GOMA,
        DOORSHUTTER_GFX_GOHMA_BLOCK,
        DOORSHUTTER_GFX_GOHMA_BLOCK,
    },
    /* DOORSHUTTER_STYLE_DEKU_TREE */
    {
        OBJECT_YDAN_OBJECTS,
        DOORSHUTTER_GFX_DEKU_TREE_1,
        DOORSHUTTER_GFX_DEKU_TREE_2,
    },
    /* DOORSHUTTER_STYLE_DODONGOS_CAVERN */
    {
        OBJECT_DDAN_OBJECTS,
        DOORSHUTTER_GFX_DODONGOS_CAVERN,
        DOORSHUTTER_GFX_DODONGOS_CAVERN,
    },
    /* DOORSHUTTER_STYLE_JABU_JABU */
    {
        OBJECT_BDAN_OBJECTS,
        DOORSHUTTER_GFX_JABU_JABU,
        DOORSHUTTER_GFX_JABU_JABU,
    },
    /* DOORSHUTTER_STYLE_FOREST_TEMPLE */
    {
        OBJECT_GAMEPLAY_KEEP,
        DOORSHUTTER_GFX_GENERIC,
        DOORSHUTTER_GFX_GENERIC,
    },
    /* DOORSHUTTER_STYLE_BOSS_DOOR */
    {
        OBJECT_BDOOR,
        DOORSHUTTER_GFX_BOSS_DOOR,
        DOORSHUTTER_GFX_BOSS_DOOR,
    },
    /* DOORSHUTTER_STYLE_GENERIC */
    {
        OBJECT_GAMEPLAY_KEEP,
        DOORSHUTTER_GFX_GENERIC,
        DOORSHUTTER_GFX_GENERIC,
    },
    /* DOORSHUTTER_STYLE_FIRE_TEMPLE */
    {
        OBJECT_HIDAN_OBJECTS,
        DOORSHUTTER_GFX_FIRE_TEMPLE_1,
        DOORSHUTTER_GFX_FIRE_TEMPLE_2,
    },
    /* DOORSHUTTER_STYLE_GANONS_TOWER */
    {
        OBJECT_GANON_OBJECTS,
        DOORSHUTTER_GFX_GANONS_TOWER,
        DOORSHUTTER_GFX_GANONS_TOWER,
    },
    /* DOORSHUTTER_STYLE_SPIRIT_TEMPLE */
    {
        OBJECT_JYA_DOOR,
        DOORSHUTTER_GFX_SPIRIT_TEMPLE,
        DOORSHUTTER_GFX_SPIRIT_TEMPLE,
    },
    /* DOORSHUTTER_STYLE_WATER_TEMPLE */
    {
        OBJECT_MIZU_OBJECTS,
        DOORSHUTTER_GFX_WATER_TEMPLE_1,
        DOORSHUTTER_GFX_WATER_TEMPLE_2,
    },
    /* DOORSHUTTER_STYLE_SHADOW_TEMPLE */
    {
        OBJECT_HAKA_DOOR,
        DOORSHUTTER_GFX_SHADOW_TEMPLE_1,
        DOORSHUTTER_GFX_SHADOW_TEMPLE_2,
    },
    /* DOORSHUTTER_STYLE_ICE_CAVERN */
    {
        OBJECT_ICE_OBJECTS,
        DOORSHUTTER_GFX_ICE_CAVERN,
        DOORSHUTTER_GFX_ICE_CAVERN,
    },
    /* DOORSHUTTER_STYLE_GERUDO_TRAINING_GROUND */
    {
        OBJECT_MENKURI_OBJECTS,
        DOORSHUTTER_GFX_GERUDO_TRAINING_GROUND,
        DOORSHUTTER_GFX_GERUDO_TRAINING_GROUND,
    },
    /* DOORSHUTTER_STYLE_GANONS_CASTLE */
    {
        OBJECT_DEMO_KEKKAI,
        DOORSHUTTER_GFX_GANONS_CASTLE,
        DOORSHUTTER_GFX_GANONS_CASTLE,
    },
    /* DOORSHUTTER_STYLE_ROYAL_FAMILYS_TOMB */
    {
        OBJECT_OUKE_HAKA,
        DOORSHUTTER_GFX_ROYAL_FAMILYS_TOMB,
        DOORSHUTTER_GFX_ROYAL_FAMILYS_TOMB,
    },
};

typedef struct DoorShutterGfxInfo {
    /* 0x0000 */ Gfx* doorDL;
    /* 0x0004 */ Gfx* barsDL;
    /* 0x0008 */ u8 barsOpenOffsetY;
    /* 0x0009 */ u8 barsOffsetZ;
    /* 0x000A */ u8 rangeSides;
    /* 0x000B */ u8 rangeY;
} DoorShutterGfxInfo;

static DoorShutterGfxInfo sGfxInfo[] = {
    { gDTDungeonDoor1DL, gDoorMetalBarsDL, 130, 12, 20, 15 }, // DOORSHUTTER_GFX_DEKU_TREE_1
    { gDTDungeonDoor2DL, gDoorMetalBarsDL, 130, 12, 20, 15 }, // DOORSHUTTER_GFX_DEKU_TREE_2
    { gDodongoDoorDL, gDodongoBarsDL, 240, 14, 70, 15 },      // DOORSHUTTER_GFX_DODONGOS_CAVERN
#if OOT_VERSION < NTSC_1_1
    { gJabuDoorSection1DL, gJabuWebDoorDL, 0, 110, 70, 15 }, // DOORSHUTTER_GFX_JABU_JABU
    { gPhantomGanonBarsDL, NULL, 130, 12, 70, 15 },          // DOORSHUTTER_GFX_PHANTOM_GANON_BARS
    { gGohmaDoorDL, NULL, 130, 12, 70, 15 },                 // DOORSHUTTER_GFX_GOHMA_BLOCK
    { gSpiritDoorDL, gJyaDoorMetalBarsDL, 240, 14, 50, 15 }, // DOORSHUTTER_GFX_SPIRIT_TEMPLE
    { gBossDoorDL, NULL, 130, 12, 70, 15 },                  // DOORSHUTTER_GFX_BOSS_DOOR
#else
    { gJabuDoorSection1DL, gJabuWebDoorDL, 0, 110, 50, 15 }, // DOORSHUTTER_GFX_JABU_JABU
    { gPhantomGanonBarsDL, NULL, 130, 12, 50, 15 },          // DOORSHUTTER_GFX_PHANTOM_GANON_BARS
    { gGohmaDoorDL, NULL, 130, 12, 50, 15 },                 // DOORSHUTTER_GFX_GOHMA_BLOCK
    { gSpiritDoorDL, gJyaDoorMetalBarsDL, 240, 14, 50, 15 }, // DOORSHUTTER_GFX_SPIRIT_TEMPLE
    { gBossDoorDL, NULL, 130, 12, 50, 15 },                  // DOORSHUTTER_GFX_BOSS_DOOR
#endif
    { gDungeonDoorDL, gDoorMetalBarsDL, 130, 12, 20, 15 },                         // DOORSHUTTER_GFX_GENERIC
    { gFireTempleDoorFrontDL, gDoorMetalBarsDL, 130, 12, 20, 15 },                 // DOORSHUTTER_GFX_FIRE_TEMPLE_1
    { gFireTempleDoorBackDL, gDoorMetalBarsDL, 130, 12, 20, 15 },                  // DOORSHUTTER_GFX_FIRE_TEMPLE_2
    { object_ganon_objects_DL_0000C0, gDoorMetalBarsDL, 130, 12, 20, 15 },         // DOORSHUTTER_GFX_GANONS_TOWER
    { gObjectMizuObjectsDoorShutterDL_005D90, gDoorMetalBarsDL, 130, 12, 20, 15 }, // DOORSHUTTER_GFX_WATER_TEMPLE_1
    { gObjectMizuObjectsDoorShutterDL_007000, gDoorMetalBarsDL, 130, 12, 20, 15 }, // DOORSHUTTER_GFX_WATER_TEMPLE_2
    { object_haka_door_DL_002620, gDoorMetalBarsDL, 130, 12, 20, 15 },             // DOORSHUTTER_GFX_SHADOW_TEMPLE_1
    { object_haka_door_DL_003890, gDoorMetalBarsDL, 130, 12, 20, 15 },             // DOORSHUTTER_GFX_SHADOW_TEMPLE_2
    { object_ice_objects_DL_001D10, gDoorMetalBarsDL, 130, 12, 20, 15 },           // DOORSHUTTER_GFX_ICE_CAVERN
    { gGTGDoorDL, gDoorMetalBarsDL, 130, 12, 20, 15 },                 // DOORSHUTTER_GFX_GERUDO_TRAINING_GROUND
    { gGanonsCastleDoorDL, gDoorMetalBarsDL, 130, 12, 20, 15 },        // DOORSHUTTER_GFX_GANONS_CASTLE
    { object_ouke_haka_DL_0000C0, gDoorMetalBarsDL, 130, 12, 20, 15 }, // DOORSHUTTER_GFX_ROYAL_FAMILYS_TOMB
};

static s8 sTypeStyles[] = {
    DOORSHUTTER_STYLE_FROM_SCENE,    // SHUTTER
    DOORSHUTTER_STYLE_FROM_SCENE,    // SHUTTER_FRONT_CLEAR
    DOORSHUTTER_STYLE_FROM_SCENE,    // SHUTTER_FRONT_SWITCH
    DOORSHUTTER_STYLE_FROM_SCENE,    // SHUTTER_BACK_LOCKED
    DOORSHUTTER_STYLE_PHANTOM_GANON, // SHUTTER_PG_BARS
    DOORSHUTTER_STYLE_BOSS_DOOR,     // SHUTTER_BOSS
    DOORSHUTTER_STYLE_GOHMA_BLOCK,   // SHUTTER_GOHMA_BLOCK
    DOORSHUTTER_STYLE_FROM_SCENE,    // SHUTTER_FRONT_SWITCH_BACK_CLEAR
    DOORSHUTTER_STYLE_PHANTOM_GANON, // SHUTTER_8
    DOORSHUTTER_STYLE_FROM_SCENE,    // SHUTTER_9
    DOORSHUTTER_STYLE_FROM_SCENE,    // SHUTTER_A
    DOORSHUTTER_STYLE_FROM_SCENE,    // SHUTTER_KEY_LOCKED
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F(scale, 1, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneForward, 4000, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneScale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneDownward, 400, ICHAIN_STOP),
};

typedef struct {
    s16 sceneNum;
    u8 index;
} DoorShutterSceneInfo;

static DoorShutterSceneInfo sSceneInfo[] = {
    { SCENE_DEKU_TREE, DOORSHUTTER_STYLE_DEKU_TREE },
    { SCENE_DODONGOS_CAVERN, DOORSHUTTER_STYLE_DODONGOS_CAVERN },
    { SCENE_DODONGOS_CAVERN_BOSS, DOORSHUTTER_STYLE_DODONGOS_CAVERN },
    { SCENE_JABU_JABU, DOORSHUTTER_STYLE_JABU_JABU },
    { SCENE_FOREST_TEMPLE, DOORSHUTTER_STYLE_FOREST_TEMPLE },
    { SCENE_FIRE_TEMPLE, DOORSHUTTER_STYLE_FIRE_TEMPLE },
    { SCENE_GANONS_TOWER, DOORSHUTTER_STYLE_GANONS_TOWER },
    { SCENE_GANONDORF_BOSS, DOORSHUTTER_STYLE_GANONS_TOWER },
    { SCENE_SPIRIT_TEMPLE, DOORSHUTTER_STYLE_SPIRIT_TEMPLE },
    { SCENE_SPIRIT_TEMPLE_BOSS, DOORSHUTTER_STYLE_SPIRIT_TEMPLE },
    { SCENE_WATER_TEMPLE, DOORSHUTTER_STYLE_WATER_TEMPLE },
    { SCENE_SHADOW_TEMPLE, DOORSHUTTER_STYLE_SHADOW_TEMPLE },
    { SCENE_BOTTOM_OF_THE_WELL, DOORSHUTTER_STYLE_SHADOW_TEMPLE },
    { SCENE_ICE_CAVERN, DOORSHUTTER_STYLE_ICE_CAVERN },
    { SCENE_GERUDO_TRAINING_GROUND, DOORSHUTTER_STYLE_GERUDO_TRAINING_GROUND },
    { SCENE_INSIDE_GANONS_CASTLE, DOORSHUTTER_STYLE_GANONS_CASTLE },
    { SCENE_ROYAL_FAMILYS_TOMB, DOORSHUTTER_STYLE_ROYAL_FAMILYS_TOMB },
    { -1, DOORSHUTTER_STYLE_GENERIC },
};

typedef enum DoorShutterBossDoorTexIndex {
    /* 0 */ DOORSHUTTER_BOSSDOORTEX_0,
    /* 1 */ DOORSHUTTER_BOSSDOORTEX_FIRE,
    /* 2 */ DOORSHUTTER_BOSSDOORTEX_WATER,
    /* 3 */ DOORSHUTTER_BOSSDOORTEX_SHADOW,
    /* 4 */ DOORSHUTTER_BOSSDOORTEX_GANON,
    /* 5 */ DOORSHUTTER_BOSSDOORTEX_FOREST,
    /* 6 */ DOORSHUTTER_BOSSDOORTEX_SPIRIT
} DoorShutterBossDoorTexIndex;

typedef struct {
    s16 dungeonScene;
    s16 bossScene;
    u8 index;
    s16 bossSpawn; // rando addition for boss door textures matches boss
} DoorShutterBossDoorInfo;

static DoorShutterBossDoorInfo sBossDoorInfo[] = {
    { SCENE_FIRE_TEMPLE, SCENE_FIRE_TEMPLE_BOSS, DOORSHUTTER_BOSSDOORTEX_FIRE, ENTR_FIRE_TEMPLE_BOSS_ENTRANCE },
    { SCENE_WATER_TEMPLE, SCENE_WATER_TEMPLE_BOSS, DOORSHUTTER_BOSSDOORTEX_WATER, ENTR_WATER_TEMPLE_BOSS_ENTRANCE },
    { SCENE_SHADOW_TEMPLE, SCENE_SHADOW_TEMPLE_BOSS, DOORSHUTTER_BOSSDOORTEX_SHADOW, ENTR_SHADOW_TEMPLE_BOSS_ENTRANCE },
    { SCENE_GANONS_TOWER, SCENE_GANONDORF_BOSS, DOORSHUTTER_BOSSDOORTEX_GANON, ENTR_GANONDORF_BOSS_0 }, // unknown atm
    { SCENE_FOREST_TEMPLE, SCENE_FOREST_TEMPLE_BOSS, DOORSHUTTER_BOSSDOORTEX_FOREST, ENTR_FOREST_TEMPLE_BOSS_ENTRANCE },
    { SCENE_SPIRIT_TEMPLE, SCENE_SPIRIT_TEMPLE_BOSS, DOORSHUTTER_BOSSDOORTEX_SPIRIT, ENTR_SPIRIT_TEMPLE_BOSS_ENTRANCE },
    { -1, -1, DOORSHUTTER_BOSSDOORTEX_0, -1 },
};

static Gfx* sJabuDoorDLists[] = {
    gJabuDoorSection1DL, gJabuDoorSection2DL, gJabuDoorSection7DL, gJabuDoorSection4DL,
    gJabuDoorSection5DL, gJabuDoorSection4DL, gJabuDoorSection3DL, gJabuDoorSection2DL,
};

static void* sBossDoorTextures[] = {
    gBossDoorDefaultTex,      // DOORSHUTTER_BOSSDOORTEX_0
    gBossDoorFireTex,         // DOORSHUTTER_BOSSDOORTEX_FIRE
    gBossDoorWaterTex,        // DOORSHUTTER_BOSSDOORTEX_WATER
    gBossDoorShadowTex,       // DOORSHUTTER_BOSSDOORTEX_SHADOW
    gBossDoorGanonsCastleTex, // DOORSHUTTER_BOSSDOORTEX_GANON
    gBossDoorForestTex,       // DOORSHUTTER_BOSSDOORTEX_FOREST
    gBossDoorSpiritTex,       // DOORSHUTTER_BOSSDOORTEX_SPIRIT
};

s16 GetTexIndexFromEntrance(EntranceIndex entrance) {
    DoorShutterBossDoorInfo* bossDoorInfo;
    s32 i;

    if (entrance == ENTR_DEKU_TREE_BOSS_DOOR || entrance == ENTR_DODONGOS_CAVERN_BOSS_DOOR ||
        entrance == ENTR_JABU_JABU_BOSS_DOOR) {
        return DOORSHUTTER_BOSSDOORTEX_0;
    }

    for (bossDoorInfo = &sBossDoorInfo[0], i = 0; i < ARRAY_COUNT(sBossDoorInfo) - 1; i++, bossDoorInfo++) {
        if (bossDoorInfo->bossSpawn == entrance)
            return bossDoorInfo->index;
    }
    return DOORSHUTTER_BOSSDOORTEX_0; // use default door texture if not found
}

void DoorShutter_SetupAction(DoorShutter* this, DoorShutterActionFunc actionFunc) {
    this->actionFunc = actionFunc;
    this->actionTimer = 0;
}

s32 DoorShutter_SetupDoor(DoorShutter* this, PlayState* play) {
    TransitionActorEntry* transitionEntry = &play->transiActorCtx.list[(u16)this->dyna.actor.params >> 0xA];
    s8 frontRoom = transitionEntry->sides[0].room;
    s32 doorType = this->doorType;
    DoorShutterStyleInfo* temp_t0 = &sStyleInfo[this->styleType];

    if (doorType != SHUTTER_KEY_LOCKED) {
        if (frontRoom == transitionEntry->sides[1].room) {
            if (ABS((s16)(this->dyna.actor.shape.rot.y - this->dyna.actor.yawTowardsPlayer)) < 0x4000) {
                frontRoom = -1;
            }
        }
        if (frontRoom == this->dyna.actor.room) {
            if (doorType == SHUTTER_FRONT_SWITCH_BACK_CLEAR) { // Swap the back clear to the front clear
                doorType = SHUTTER_FRONT_CLEAR;
            } else {
                doorType = (doorType == SHUTTER_BOSS) ? SHUTTER_BACK_LOCKED : SHUTTER;
            }
        }
    }
    this->gfxType = (doorType == SHUTTER) ? temp_t0->index1 : temp_t0->index2;

    if (doorType == SHUTTER_FRONT_CLEAR) {
        if (!Flags_GetClear(play, this->dyna.actor.room)) {
            DoorShutter_SetupAction(this, func_80996A54);
            this->barsClosedAmount = 1.0f;
            return true;
        }
    } else if (doorType == SHUTTER_FRONT_SWITCH || doorType == SHUTTER_FRONT_SWITCH_BACK_CLEAR) {
        if (!Flags_GetSwitch(play, this->dyna.actor.params & 0x3F)) {
            DoorShutter_SetupAction(this, func_80996EE8);
            this->barsClosedAmount = 1.0f;
            return true;
        }
        DoorShutter_SetupAction(this, func_80996F98);
        return false;
    } else if (doorType == SHUTTER_BACK_LOCKED) {
        DoorShutter_SetupAction(this, func_80996B00);
        return false;
    }
    DoorShutter_SetupAction(this, func_80996B0C);
    return false;
}

void DoorShutter_Init(Actor* thisx, PlayState* play2) {
    DoorShutter* this = (DoorShutter*)thisx;
    PlayState* play = play2;
    s32 styleType;
    s32 pad;
    s32 objectIndex;
    s32 i;

    Actor_ProcessInitChain(&this->dyna.actor, sInitChain);
    this->dyna.actor.home.pos.z = this->dyna.actor.shape.yOffset;
    DynaPolyActor_Init(&this->dyna, DPM_UNK);
    this->doorType = (this->dyna.actor.params >> 6) & 0xF;
    styleType = sTypeStyles[this->doorType];
    if (styleType < 0) {
        DoorShutterSceneInfo* sceneInfo;

        for (sceneInfo = &sSceneInfo[0], i = 0; i < ARRAY_COUNT(sSceneInfo) - 1; i++, sceneInfo++) {
            if (play->sceneNum == sceneInfo->sceneNum) {
                break;
            }
        }
        styleType = sceneInfo->index;
    } else if (styleType == DOORSHUTTER_STYLE_BOSS_DOOR) {
        DoorShutterBossDoorInfo* bossDoorInfo;

        for (bossDoorInfo = &sBossDoorInfo[0], i = 0; i < ARRAY_COUNT(sBossDoorInfo) - 1; i++, bossDoorInfo++) {
            if (play->sceneNum == bossDoorInfo->dungeonScene || play->sceneNum == bossDoorInfo->bossScene) {
                break;
            }
        }

        if (CVarGetInteger(CVAR_RANDOMIZER_ENHANCEMENT("BossDoorTexMatchesBoss"), 0)) {
            EntranceIndex overrideEntranceIndex = Entrance_GetOverride(bossDoorInfo->bossSpawn);
            this->bossDoorTexIndex = GetTexIndexFromEntrance(overrideEntranceIndex);
        } else {
            this->bossDoorTexIndex = bossDoorInfo->index;
        }

    } else { // DOORSHUTTER_STYLE_PHANTOM_GANON, DOORSHUTTER_STYLE_GOHMA_BLOCK
        this->dyna.actor.room = -1;
    }
    if (this->requiredObjBankIndex = objectIndex = Object_GetIndex(&play->objectCtx, sStyleInfo[styleType].objectId),
        (s8)objectIndex < 0) {
        Actor_Kill(&this->dyna.actor);
        return;
    }
    DoorShutter_SetupAction(this, DoorShutter_SetupType);
    this->styleType = styleType;
    if (this->doorType == SHUTTER_KEY_LOCKED || this->doorType == SHUTTER_BOSS) {
        if (GameInteractor_Should(VB_LOCK_BOSS_DOOR, !Flags_GetSwitch(play, this->dyna.actor.params & 0x3F), this)) {
            this->unlockTimer = 10;
        }
        Actor_SetFocus(&this->dyna.actor, 60.0f);
    } else if (styleType == 4) {
        Actor_SetScale(&this->dyna.actor, 0.1f);
        this->jabuDoorClosedAmount = 100;
        this->dyna.actor.uncullZoneScale = 200.0f;
        Actor_SetFocus(&this->dyna.actor, 0.0f);
    } else {
        Actor_SetFocus(&this->dyna.actor, 60.0f);
    }
}

void DoorShutter_Destroy(Actor* thisx, PlayState* play) {
    DoorShutter* this = (DoorShutter*)thisx;

    DynaPoly_DeleteBgActor(play, &play->colCtx.dyna, this->dyna.bgId);
    if (this->dyna.actor.room >= 0) {
        s32 transitionActorId = (u16)this->dyna.actor.params >> 0xA;

        play->transiActorCtx.list[transitionActorId].id *= -1;
    }
}

void DoorShutter_SetupType(DoorShutter* this, PlayState* play) {
    if (Object_IsLoaded(&play->objectCtx, this->requiredObjBankIndex)) {
        this->dyna.actor.objBankIndex = this->requiredObjBankIndex;
        if (this->doorType == SHUTTER_PG_BARS || this->doorType == SHUTTER_GOHMA_BLOCK) {
            // Init dynapoly for shutters of the type that uses it
            CollisionHeader* colHeader = NULL;

            Actor_SetObjectDependency(play, &this->dyna.actor);
            this->gfxType = sStyleInfo[this->styleType].index1;
            CollisionHeader_GetVirtual((this->doorType == SHUTTER_GOHMA_BLOCK) ? &gGohmaDoorCol : &gPhantomGanonBarsCol,
                                       &colHeader);
            this->dyna.bgId = DynaPoly_SetBgActor(play, &play->colCtx.dyna, &this->dyna.actor, colHeader);
            if (this->doorType == SHUTTER_GOHMA_BLOCK) {
                this->dyna.actor.velocity.y = 0.0f;
                this->dyna.actor.gravity = -2.0f;
                Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_SLIDE_DOOR_CLOSE);
                DoorShutter_SetupAction(this, func_809975C0);
            } else {
                DoorShutter_SetupAction(this, func_80997744);
                this->isActive = 7;
            }
        } else {
            DoorShutter_SetupDoor(this, play);
        }
    }
}

f32 func_80996840(PlayState* play, DoorShutter* this, f32 arg2, f32 arg3, f32 arg4) {
    s32 pad;
    Vec3f sp28;
    Vec3f sp1C;
    Player* player = GET_PLAYER(play);

    sp28.x = player->actor.world.pos.x;
    sp28.y = player->actor.world.pos.y + arg2;
    sp28.z = player->actor.world.pos.z;
    Actor_WorldToActorCoords(&this->dyna.actor, &sp1C, &sp28);
    if (arg3 < fabsf(sp1C.x) || arg4 < fabsf(sp1C.y)) {
        return FLT_MAX;
    } else {
        return sp1C.z;
    }
}

s32 func_809968D4(DoorShutter* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if (!Player_InCsMode(play)) {
        DoorShutterGfxInfo* temp_v1 = &sGfxInfo[this->gfxType];
        f32 temp_f2 = func_80996840(play, this, (this->gfxType != 3) ? 0.0f : 80.0f, temp_v1->rangeSides, temp_v1->rangeY);

        if (fabsf(temp_f2) < 50.0f) {
            s16 phi_v0 = player->actor.shape.rot.y - this->dyna.actor.shape.rot.y;

            if (temp_f2 > 0.0f) {
                phi_v0 = 0x8000 - phi_v0;
            }
            if (ABS(phi_v0) < 0x3000) {
                return (temp_f2 >= 0.0f) ? 1.0f : -1.0f;
            }
        }
    }
    return 0.0f;
}

void func_80996A54(DoorShutter* this, PlayState* play) {
    if (Flags_GetClear(play, this->dyna.actor.room) || Flags_GetTempClear(play, this->dyna.actor.room)) {
        Flags_SetClear(play, this->dyna.actor.room);
        DoorShutter_SetupAction(this, func_80997150);
        if (GameInteractor_Should(VB_PLAY_ONEPOINT_ACTOR_CS, true, this)) {
            OnePointCutscene_Attention(play, &this->dyna.actor);
            OnePointCutscene_Attention(play, &GET_PLAYER(play)->actor);
            this->actionTimer = -100;
        }
    } else if (func_809968D4(this, play) != 0) {
        Player* player = GET_PLAYER(play);

        player->naviTextId = -0x202;
    }
}

void func_80996B00(DoorShutter* this, PlayState* play) {
}

void func_80996B0C(DoorShutter* this, PlayState* play) {
    if (this->isActive != 0) {
        DoorShutter_SetupAction(this, func_80997004);
        this->dyna.actor.velocity.y = 0.0f;
        if (this->unlockTimer != 0) {
            Flags_SetSwitch(play, this->dyna.actor.params & 0x3F);
            if (this->doorType != SHUTTER_BOSS) {
                gSaveContext.inventory.dungeonKeys[gSaveContext.mapIndex]--;
                Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_CHAIN_KEY_UNLOCK);
            } else {
                Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_CHAIN_KEY_UNLOCK_B);
            }
        }
    } else {
        s32 doorDirection = func_809968D4(this, play);

        if (doorDirection != 0) {
            Player* player = GET_PLAYER(play);

            if (this->unlockTimer != 0) {
                if (this->doorType == SHUTTER_BOSS) {
                    if (!CHECK_DUNGEON_ITEM(DUNGEON_KEY_BOSS, gSaveContext.mapIndex)) {
                        player->naviTextId = -0x204;
                        return;
                    }
                } else if (gSaveContext.inventory.dungeonKeys[gSaveContext.mapIndex] <= 0) {
                    player->naviTextId = -0x203;
                    return;
                }
                player->doorTimer = 10;
            }
            player->doorType = PLAYER_DOORTYPE_SLIDING;
            player->doorDirection = doorDirection;
            player->doorActor = &this->dyna.actor;
        }
    }
}

void func_80996C60(DoorShutter* this, PlayState* play) {
    if (this->dyna.actor.category == ACTORCAT_DOOR) {
        Player* player = GET_PLAYER(play);
        s32 sp38 = this->gfxType;
        s32 sp34 = 0xF;

        if (DoorShutter_SetupDoor(this, play)) {
            sp34 = 0x20;
        }
        DoorShutter_SetupAction(this, func_80997004);
        this->gfxType = sp38;
        this->barsClosedAmount = 0.0f;
        Camera_ChangeDoorCam(play->cameraPtrs[MAIN_CAM], &this->dyna.actor, player->cv.slidingDoorBgCamIndex, 0.0f, 12,
                             sp34, 10);
    }
}

s32 func_80996D14(DoorShutter* this, PlayState* play) {
    if (this->gfxType != 3) {
        if (this->dyna.actor.velocity.y == 0.0f) {
            Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_SLIDE_DOOR_OPEN);
            func_80996C60(this, play);
        }
        Math_StepToF(&this->dyna.actor.velocity.y, 15.0f, 3.0f);
        if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y + 200.0f,
                         this->dyna.actor.velocity.y)) {
            return true;
        }
    } else {
        if (this->jabuDoorClosedAmount == 100) {
            Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_BUYODOOR_OPEN);
            func_80996C60(this, play);
        }
        if (Math_StepToS(&this->jabuDoorClosedAmount, 0, 10)) {
            return true;
        }
    }
    return false;
}

s32 func_80996E08(DoorShutter* this, PlayState* play, f32 arg2) {
    if (this->barsClosedAmount == 1.0f - arg2) {
        if (this->gfxType != 3) {
            if (arg2 == 1.0f) {
                Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_METALDOOR_CLOSE);
            } else {
                Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_METALDOOR_OPEN);
            }
        } else {
            if (arg2 == 1.0f) {
                Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_BUYOSHUTTER_CLOSE);
            } else {
                Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_BUYOSHUTTER_OPEN);
            }
        }
    }
    if (Math_StepToF(&this->barsClosedAmount, arg2, 0.2f)) {
        return true;
    }
    return false;
}

void func_80996EE8(DoorShutter* this, PlayState* play) {
    if (func_80996E08(this, play, 1.0f)) {
        if (Flags_GetSwitch(play, this->dyna.actor.params & 0x3F)) {
            DoorShutter_SetupAction(this, func_80997150);
            if (GameInteractor_Should(VB_PLAY_ONEPOINT_ACTOR_CS, true, this)) {
                OnePointCutscene_Attention(play, &this->dyna.actor);
                this->actionTimer = -100;
            }
        } else if (func_809968D4(this, play)) {
            Player* player = GET_PLAYER(play);
            // Jabu navi text for switch doors is different
            player->naviTextId = (play->sceneNum == SCENE_JABU_JABU) ? -0x20B : -0x202;
        }
    }
}

void func_80996F98(DoorShutter* this, PlayState* play) {
    if (this->isActive == 0 && !Flags_GetSwitch(play, this->dyna.actor.params & 0x3F)) {
        DoorShutter_SetupAction(this, func_80996EE8);
    } else {
        func_80996B0C(this, play);
    }
}

void func_80997004(DoorShutter* this, PlayState* play) {
    if (DECR(this->unlockTimer) == 0 && play->roomCtx.status == 0 && func_80996D14(this, play) != 0) {
        if (((this->doorType == SHUTTER_BOSS) ? 20.0f : 50.0f) < this->dyna.actor.xzDistToPlayer) {
            if (DoorShutter_SetupDoor(this, play)) {
                this->dyna.actor.velocity.y = 30.0f;
            }
            if (this->gfxType != 3) {
                Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_SLIDE_DOOR_CLOSE);
                DoorShutter_SetupAction(this, func_809973E8);
            } else {
                Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_BUYODOOR_CLOSE);
                if ((this->doorType == SHUTTER_FRONT_SWITCH || this->doorType == SHUTTER_FRONT_SWITCH_BACK_CLEAR) &&
                    !Flags_GetSwitch(play, this->dyna.actor.params & 0x3F)) {
                    Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_BUYOSHUTTER_CLOSE);
                }
                DoorShutter_SetupAction(this, func_80997528);
            }
        }
    }
}

void func_80997150(DoorShutter* this, PlayState* play) {
    if (this->actionTimer != 0) {
        if (this->actionTimer < 0) {
            if (play->state.frames % 2 != 0) {
                this->actionTimer++;
            }
            if (this->dyna.actor.category == func_8005B198() || this->actionTimer == 0) {
                this->actionTimer = 5;
            }
        } else {
            this->actionTimer--;
        }
    } else if (func_80996E08(this, play, 0.0f)) {
        if (!(this->doorType == SHUTTER || this->doorType == SHUTTER_FRONT_CLEAR)) {
            DoorShutter_SetupAction(this, func_80996F98);
        } else {
            DoorShutter_SetupAction(this, func_80996B0C);
        }
        func_800F5B58();
    }
}

void func_80997220(DoorShutter* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s8 room = this->dyna.actor.room;

    if (this->dyna.actor.room >= 0) {
        Vec3f vec;

        Actor_WorldToActorCoords(&this->dyna.actor, &vec, &player->actor.world.pos);
        this->dyna.actor.room =
            play->transiActorCtx.list[(u16)this->dyna.actor.params >> 0xA].sides[(vec.z < 0.0f) ? 0 : 1].room;
        if (room != this->dyna.actor.room) {
            Room tempRoom = play->roomCtx.curRoom;

            play->roomCtx.curRoom = play->roomCtx.prevRoom;
            play->roomCtx.prevRoom = tempRoom;
            play->roomCtx.unk_30 ^= 1;
        }
        func_80097534(play, &play->roomCtx);
        Play_SetupRespawnPoint(play, RESPAWN_MODE_DOWN, 0x0EFF);
    }
    this->isActive = 0;
    this->dyna.actor.velocity.y = 0.0f;
    if (DoorShutter_SetupDoor(this, play) && !(player->stateFlags1 & PLAYER_STATE1_CARRYING_ACTOR)) {
        DoorShutter_SetupAction(this, func_80997568);
        Player_SetCsActionWithHaltedActors(play, NULL, 2);
    }
}

void func_809973E8(DoorShutter* this, PlayState* play) {
    s32 quakeId;

    if (this->dyna.actor.velocity.y < 20.0f) {
        Math_StepToF(&this->dyna.actor.velocity.y, 20.0f, 8.0f);
    }
    if (Math_StepToF(&this->dyna.actor.world.pos.y, this->dyna.actor.home.pos.y, this->dyna.actor.velocity.y)) {
        if (this->dyna.actor.velocity.y > 20.0f) {
            this->dyna.actor.floorHeight = this->dyna.actor.home.pos.y;
            Actor_SpawnFloorDustRing(play, &this->dyna.actor, &this->dyna.actor.world.pos, 45.0f, 10, 8.0f, 500, 10,
                                     false);
        }
        Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_STONE_BOUND);
        quakeId = Quake_Add(Play_GetCamera(play, 0), 3);
        Quake_SetSpeed(quakeId, -32536);
        Quake_SetQuakeValues(quakeId, 2, 0, 0, 0);
        Quake_SetCountdown(quakeId, 10);
        func_800AA000(this->dyna.actor.xyzDistToPlayerSq, 0xB4, 0x14, 0x64);
        func_80997220(this, play);
    }
}

void func_80997528(DoorShutter* this, PlayState* play) {
    if (Math_StepToS(&this->jabuDoorClosedAmount, 0x64, 0xA)) {
        func_80997220(this, play);
    }
}

void func_80997568(DoorShutter* this, PlayState* play) {
    if (this->actionTimer++ > 30) {
        Player_SetCsActionWithHaltedActors(play, NULL, 7);
        DoorShutter_SetupDoor(this, play);
    }
}

void func_809975C0(DoorShutter* this, PlayState* play) {
    Actor_MoveXZGravity(&this->dyna.actor);
    Actor_UpdateBgCheckInfo(play, &this->dyna.actor, 0.0f, 0.0f, 0.0f, 4);
    if (this->dyna.actor.bgCheckFlags & 1) {
        DoorShutter_SetupAction(this, func_809976B8);
        if (!Flags_GetEventChkInf(EVENTCHKINF_BEGAN_GOHMA_BATTLE)) {
            BossGoma* parent = (BossGoma*)this->dyna.actor.parent;

            this->isActive = 10;
            Audio_PlayActorSound2(&this->dyna.actor, NA_SE_EV_STONE_BOUND);
            func_8099803C(play, 2, 10, parent->subCameraId);
            Actor_SpawnFloorDustRing(play, &this->dyna.actor, &this->dyna.actor.world.pos, 70.0f, 20, 8.0f, 500, 10,
                                     true);
        }
    }
}

void func_809976B8(DoorShutter* this, PlayState* play) {
    f32 mult;

    if (this->isActive != 0) {
        this->isActive--;
        mult = sinf(this->isActive * 250.0f / 100.0f);
        this->dyna.actor.shape.yOffset = this->isActive * 3.0f / 10.0f * mult;
    }
}

void func_80997744(DoorShutter* this, PlayState* play) {
    f32 phi_f0;

    osSyncPrintf("FHG SAKU START !!\n");
    if (this->isActive != 0) {
        this->isActive--;
    }
    phi_f0 = (this->isActive % 2 != 0) ? -3.0f : 0.0f;
    Math_SmoothStepToF(&this->dyna.actor.world.pos.y, -34.0f + phi_f0, 1.0f, 20.0f, 0.0f);
    osSyncPrintf("FHG SAKU END !!\n");
}

void DoorShutter_Update(Actor* thisx, PlayState* play) {
    DoorShutter* this = (DoorShutter*)thisx;
    Player* player = GET_PLAYER(play);

    if (!(player->stateFlags1 &
          (PLAYER_STATE1_TALKING | PLAYER_STATE1_DEAD | PLAYER_STATE1_GETTING_ITEM | PLAYER_STATE1_IN_ITEM_CS)) ||
        (this->actionFunc == DoorShutter_SetupType)) {
        this->actionFunc(this, play);
    }
}

Gfx* func_80997838(PlayState* play, DoorShutter* this, Gfx* p) {
    MtxF mtx;
    f32 angle = 0.0f;
    f32 yScale = this->jabuDoorClosedAmount * 0.01f;
    s32 i;

    Matrix_Get(&mtx);
    for (i = 0; i < ARRAY_COUNT(sJabuDoorDLists); i++) {
        Matrix_RotateZ(angle, MTXMODE_APPLY);
        if (i % 2 == 0) {
            Matrix_Translate(0.0f, 800.0f, 0.0f, MTXMODE_APPLY);
        } else if (i == 1 || i == 7) {
            Matrix_Translate(0.0f, 848.52f, 0.0f, MTXMODE_APPLY);
        } else {
            Matrix_Translate(0.0f, 989.94f, 0.0f, MTXMODE_APPLY);
        }
        if (this->jabuDoorClosedAmount != 100) {
            Matrix_Scale(1.0f, yScale, 1.0f, MTXMODE_APPLY);
        }
        gSPMatrix(p++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
        gSPDisplayList(p++, sJabuDoorDLists[i]);
        angle -= M_PI / 4;
        Matrix_Put(&mtx);
    }
    return p;
}

s32 func_80997A34(DoorShutter* this, PlayState* play) {
    s32 phi_a1;
    s32 phi_a0;

    if (Player_InCsMode(play)) {
        return true;
    }
    phi_a0 = (s16)(Actor_WorldYawTowardPoint(&this->dyna.actor, &play->view.eye) - this->dyna.actor.shape.rot.y);
    phi_a1 = (s16)(this->dyna.actor.yawTowardsPlayer - this->dyna.actor.shape.rot.y);
    phi_a0 = ABS(phi_a0);
    phi_a1 = ABS(phi_a1);
    if ((phi_a1 < 0x4000 && phi_a0 > 0x4000) || (phi_a1 > 0x4000 && phi_a0 < 0x4000)) {
        return false;
    }
    return true;
}

void DoorShutter_Draw(Actor* thisx, PlayState* play) {
    DoorShutter* this = (DoorShutter*)thisx;

    //! @bug This actor is not fully initialized until the required object dependency is loaded.
    //! In most cases, the check for objBankIndex to equal requiredObjBankIndex prevents the actor
    //! from drawing until initialization is complete. However if the required object is the same as the
    //! object dependency listed in init vars (gameplay_keep in this case), the check will pass even though
    //! initialization has not completed. When this happens, it will try to draw the display list of the
    //! first entry in `sShutterInfo`, which will likely crash the game.
    //! This only matters in very specific scenarios, when the door is unculled on the first possible frame
    //! after spawning. It will try to draw without having run update yet.
    //!
    //! The best way to fix this issue (and what was done in Majora's Mask) is to null out the draw function in
    //! the init vars for the actor, and only set draw after initialization is complete.

    if (this->dyna.actor.objBankIndex == this->requiredObjBankIndex &&
        (this->styleType == 0 || func_80997A34(this, play) != 0)) {
        s32 pad[2];
        DoorShutterGfxInfo* sp70 = &sGfxInfo[this->gfxType];

        OPEN_DISPS(play->state.gfxCtx);

        Gfx_SetupDL_25Opa(play->state.gfxCtx);

        if (this->gfxType == 3) {
            POLY_OPA_DISP = func_80997838(play, this, POLY_OPA_DISP);
            if (this->barsClosedAmount != 0.0f) {
                f32 sp58 = (this->jabuDoorClosedAmount * 0.01f) * this->barsClosedAmount;

                Gfx_SetupDL_25Opa(play->state.gfxCtx);
                gDPSetEnvColor(POLY_OPA_DISP++, 0, 0, 0, 255.0f * sp58);
                Matrix_Translate(0, 0, sp70->barsOffsetZ, MTXMODE_APPLY);
                Matrix_Scale(sp58, sp58, sp58, MTXMODE_APPLY);
                gSPMatrix(POLY_OPA_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_OPA_DISP++, sp70->barsDL);
            }
        } else {
            if (sp70->barsDL != NULL) {
                TransitionActorEntry* transitionEntry = &play->transiActorCtx.list[(u16)this->dyna.actor.params >> 0xA];

                if (play->roomCtx.prevRoom.num >= 0 ||
                    transitionEntry->sides[0].room == transitionEntry->sides[1].room) {
                    s32 yaw = Math_Vec3f_Yaw(&play->view.eye, &this->dyna.actor.world.pos);

                    if (ABS((s16)(this->dyna.actor.shape.rot.y - yaw)) < 0x4000) {
                        Matrix_RotateY(M_PI, MTXMODE_APPLY);
                    }
                } else if (this->dyna.actor.room == transitionEntry->sides[0].room) {
                    Matrix_RotateY(M_PI, MTXMODE_APPLY);
                }
            } else if (this->doorType == SHUTTER_BOSS) {
                gSPSegment(POLY_OPA_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(sBossDoorTextures[this->bossDoorTexIndex]));
            }
            gSPMatrix(POLY_OPA_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_OPA_DISP++, sp70->doorDL);
            if (this->barsClosedAmount != 0.0f && sp70->barsDL != NULL) {
                Matrix_Translate(0, sp70->barsOpenOffsetY * (1.0f - this->barsClosedAmount), sp70->barsOffsetZ, MTXMODE_APPLY);
                gSPMatrix(POLY_OPA_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_OPA_DISP++, sp70->barsDL);
            }
        }

        if (this->unlockTimer != 0) {
            Matrix_Scale(0.01f, 0.01f, 0.025f, MTXMODE_APPLY);
            Actor_DrawDoorLock(play, this->unlockTimer,
                               (this->doorType == SHUTTER_BOSS)
                                   ? DOORLOCK_BOSS
                                   : ((this->gfxType == 6) ? DOORLOCK_NORMAL_SPIRIT : DOORLOCK_NORMAL));
        }

        CLOSE_DISPS(play->state.gfxCtx);
    }
}

void func_8099803C(PlayState* play, s16 y, s16 countdown, s16 camId) {
    s16 quakeId = Quake_Add(Play_GetCamera(play, camId), 3);

    func_800A9F6C(0.0f, 180, 20, 100);
    Quake_SetSpeed(quakeId, 20000);
    Quake_SetQuakeValues(quakeId, y, 0, 0, 0);
    Quake_SetCountdown(quakeId, countdown);
}
