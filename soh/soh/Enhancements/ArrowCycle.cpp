#include "public/bridge/consolevariablebridge.h"
#include "game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"

extern "C" {
#include "macros.h"
#include "variables.h"
#include "functions.h"
#include "overlays/actors/ovl_En_Arrow/z_en_arrow.h"
void Player_InitItemAction(PlayState* play, Player* thisx, s8 itemAction);
s32 func_808351D4(Player* thisx, PlayState* play); // Arrow nocked
//s32 Player_UpperAction_8(Player* thisx, PlayState* play);
}

#define CVAR_NAME "gEnhancements.ArrowCycle"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

#define BUTTON_ITEM_EQUIP(button) (gSaveContext.equips.buttonItems[(button) + 1])
#define DPAD_BUTTON_ITEM_EQUIP(button) (gSaveContext.equips.buttonItems[(button) + 4]) 

#define C_SLOT_EQUIP(button) (gSaveContext.equips.cButtonSlots[(button) + 1])
#define DPAD_SLOT_EQUIP(button) (gSaveContext.equips.cButtonSlots[(button) + 4])

#define ARROW_IS_MAGICAL(arrowType) (((arrowType) >= ArrowType::ARROW_FIRE) && ((arrowType) <= ArrowType::ARROW_LIGHT))
#define ARROW_GET_MAGIC_FROM_TYPE(arrowType) (s32)((arrowType) - ArrowType::ARROW_FIRE)

// Magic arrow costs based on z_player.c
static const s16 sMagicArrowCosts[] = { 4, 4, 8 };

// Button Flash Effect Configuration
static const s16 BUTTON_FLASH_DURATION = 3;
static const s16 BUTTON_FLASH_COUNT = 3;
static const s16 BUTTON_HIGHLIGHT_ALPHA = 128;

// State Variables
static s16 sButtonFlashTimer = 0;
static s16 sButtonFlashCount = 0;
static s8 sJustCycledFrames = 0;

// Arrow Type Definitions
static const PlayerItemAction ACTION_ARROW_NORMAL = PLAYER_IA_BOW;
static const PlayerItemAction ACTION_ARROW_FIRE = PLAYER_IA_BOW_FIRE;
static const PlayerItemAction ACTION_ARROW_ICE = PLAYER_IA_BOW_ICE;
static const PlayerItemAction ACTION_ARROW_LIGHT = PLAYER_IA_BOW_LIGHT;

static const int EQUIP_SLOT_C_LEFT = 0;
static const int EQUIP_SLOT_C_DOWN = 1;
static const int EQUIP_SLOT_C_RIGHT = 2;

static const int EQUIP_SLOT_D_RIGHT = 0;
static const int EQUIP_SLOT_D_LEFT = 1;
static const int EQUIP_SLOT_D_DOWN = 2;
static const int EQUIP_SLOT_D_UP = 3;

static const PlayerItemAction sArrowCycleOrder[] = {
    ACTION_ARROW_NORMAL,
    ACTION_ARROW_FIRE,
    ACTION_ARROW_ICE,
    ACTION_ARROW_LIGHT,
};

// Utility Functions
static bool IsHoldingBow(Player* player) {
    return player->heldItemAction >= ACTION_ARROW_NORMAL && player->heldItemAction <= ACTION_ARROW_LIGHT;
}

static bool IsHoldingMagicBow(Player* player) {
    return player->heldItemAction >= ACTION_ARROW_FIRE && player->heldItemAction <= ACTION_ARROW_LIGHT;
}

static bool IsAimingBow(Player* player) {
    return IsHoldingBow(player) && ((player->unk_6AD == 2) || /* Aiming box in first person */
                                    (player->upperActionFunc == func_808351D4) /* Arrow pulled back on bow */);
}

static bool HasArrowType(PlayerItemAction arrowType) {
    switch (arrowType) {
        case ACTION_ARROW_NORMAL:
            return true;
        case ACTION_ARROW_FIRE:
            return (INV_CONTENT(ITEM_ARROW_FIRE) == ITEM_ARROW_FIRE);
        case ACTION_ARROW_ICE:
            return (INV_CONTENT(ITEM_ARROW_ICE) == ITEM_ARROW_ICE);
        case ACTION_ARROW_LIGHT:
            return (INV_CONTENT(ITEM_ARROW_LIGHT) == ITEM_ARROW_LIGHT);
        default:
            return false;
    }
}

static s32 GetBowItemForArrow(PlayerItemAction arrowType) {
    switch (arrowType) {
        case ACTION_ARROW_FIRE:
            return ITEM_BOW_ARROW_FIRE;
        case ACTION_ARROW_ICE:
            return ITEM_BOW_ARROW_ICE;
        case ACTION_ARROW_LIGHT:
            return ITEM_BOW_ARROW_LIGHT;
        default:
            return ITEM_BOW;
    }
}

static bool CanCycleArrows() {
    Player* player = GET_PLAYER(gPlayState);

    // Don't allow cycling during bow minigames in specific scenes
    if (gSaveContext.minigameState != 0 && (gPlayState->sceneNum == SCENE_SHOOTING_GALLERY)) // Kakariko Shooting Gallery
        return false;

    return !(player->stateFlags1 & PLAYER_STATE1_ON_HORSE) && player->rideActor == NULL &&
           INV_CONTENT(SLOT_BOW) == ITEM_BOW &&
           (INV_CONTENT(ITEM_ARROW_FIRE) == ITEM_ARROW_FIRE || INV_CONTENT(ITEM_ARROW_ICE) == ITEM_ARROW_ICE ||
            INV_CONTENT(ITEM_ARROW_LIGHT) == ITEM_ARROW_LIGHT);
}

// Arrow Cycling Logic
static s8 GetNextArrowType(s8 currentArrowType) {
    int currentIndex = 0;
    for (int i = 0; i < (int)ARRAY_COUNT(sArrowCycleOrder); i++) {
        if (sArrowCycleOrder[i] == currentArrowType) {
            currentIndex = i;
            break;
        }
    }

    for (int offset = 1; offset <= (int)ARRAY_COUNT(sArrowCycleOrder); offset++) {
        int nextIndex = (currentIndex + offset) % ARRAY_COUNT(sArrowCycleOrder);
        if (HasArrowType(sArrowCycleOrder[nextIndex])) {
            return sArrowCycleOrder[nextIndex];
        }
    }

    return ACTION_ARROW_NORMAL;
}

// UI Update Functions
static void UpdateButtonAlpha(s16 flashAlpha, bool isButtonBow, u16* buttonAlpha) {
    if (isButtonBow) {
        *buttonAlpha = flashAlpha;
        if (sButtonFlashTimer == 0) {
            *buttonAlpha = 255;
        }
    }
}

static void UpdateFlashEffect(PlayState* play) {
    if (sButtonFlashTimer <= 0) {
        return;
    }

    sButtonFlashTimer--;
    s16 flashAlpha = (sButtonFlashTimer % 3) ? BUTTON_HIGHLIGHT_ALPHA : 255;

    if (sButtonFlashTimer == 0 && sButtonFlashCount < BUTTON_FLASH_COUNT - 1) {
        sButtonFlashTimer = BUTTON_FLASH_DURATION;
        sButtonFlashCount++;
    }

    // Update C-buttons
    UpdateButtonAlpha(flashAlpha,
                      (C_BTN_ITEM(EQUIP_SLOT_C_LEFT) == ITEM_BOW) ||
                          (C_BTN_ITEM(EQUIP_SLOT_C_LEFT) >= ITEM_BOW_ARROW_FIRE &&
                           C_BTN_ITEM(EQUIP_SLOT_C_LEFT) <= ITEM_BOW_ARROW_LIGHT),
                      &play->interfaceCtx.cLeftAlpha);

    UpdateButtonAlpha(flashAlpha,
                      (C_BTN_ITEM(EQUIP_SLOT_C_DOWN) == ITEM_BOW) ||
                          (C_BTN_ITEM(EQUIP_SLOT_C_DOWN) >= ITEM_BOW_ARROW_FIRE &&
                           C_BTN_ITEM(EQUIP_SLOT_C_DOWN) <= ITEM_BOW_ARROW_LIGHT),
                      &play->interfaceCtx.cDownAlpha);

    UpdateButtonAlpha(flashAlpha,
                      (C_BTN_ITEM(EQUIP_SLOT_C_RIGHT) == ITEM_BOW) ||
                          (C_BTN_ITEM(EQUIP_SLOT_C_RIGHT) >= ITEM_BOW_ARROW_FIRE &&
                           C_BTN_ITEM(EQUIP_SLOT_C_RIGHT) <= ITEM_BOW_ARROW_LIGHT),
                      &play->interfaceCtx.cRightAlpha);

    // Update D-pad
    UpdateButtonAlpha(flashAlpha,
                      (DPAD_ITEM(EQUIP_SLOT_D_RIGHT) == ITEM_BOW) ||
                          (DPAD_ITEM(EQUIP_SLOT_D_RIGHT) >= ITEM_BOW_ARROW_FIRE &&
                           DPAD_ITEM(EQUIP_SLOT_D_RIGHT) <= ITEM_BOW_ARROW_LIGHT),
                      &play->interfaceCtx.dpadRightAlpha);

    UpdateButtonAlpha(flashAlpha,
                      (DPAD_ITEM(EQUIP_SLOT_D_LEFT) == ITEM_BOW) ||
                          (DPAD_ITEM(EQUIP_SLOT_D_LEFT) >= ITEM_BOW_ARROW_FIRE &&
                           DPAD_ITEM(EQUIP_SLOT_D_LEFT) <= ITEM_BOW_ARROW_LIGHT),
                      &play->interfaceCtx.dpadLeftAlpha);

    UpdateButtonAlpha(flashAlpha,
                      (DPAD_ITEM(EQUIP_SLOT_D_DOWN) == ITEM_BOW) ||
                          (DPAD_ITEM(EQUIP_SLOT_D_DOWN) >= ITEM_BOW_ARROW_FIRE &&
                           DPAD_ITEM(EQUIP_SLOT_D_DOWN) <= ITEM_BOW_ARROW_LIGHT),
                      &play->interfaceCtx.dpadDownAlpha);

    UpdateButtonAlpha(flashAlpha,
                      (DPAD_ITEM(EQUIP_SLOT_D_UP) == ITEM_BOW) ||
                          (DPAD_ITEM(EQUIP_SLOT_D_UP) >= ITEM_BOW_ARROW_FIRE &&
                           DPAD_ITEM(EQUIP_SLOT_D_UP) <= ITEM_BOW_ARROW_LIGHT),
                      &play->interfaceCtx.dpadUpAlpha);
}

static void UpdateEquippedBow(PlayState* play, s8 arrowType) {
    s32 bowItem = GetBowItemForArrow(static_cast<PlayerItemAction>(arrowType));

    // Update C-buttons
    for (s32 i = EQUIP_SLOT_C_LEFT; i <= EQUIP_SLOT_C_RIGHT; i++) {
        if ((C_BTN_ITEM(i) == ITEM_BOW) ||
            (C_BTN_ITEM(i) >= ITEM_BOW_ARROW_FIRE && C_BTN_ITEM(i) <= ITEM_BOW_ARROW_LIGHT)) {
            BUTTON_ITEM_EQUIP(i) = bowItem;
            C_SLOT_EQUIP(i) = SLOT_BOW;
            Interface_LoadItemIcon1(play, i);
            gSaveContext.buttonStatus[i] = BTN_ENABLED;
            sButtonFlashTimer = BUTTON_FLASH_DURATION;
            sButtonFlashCount = 0;
        }
    }

    // Update D-pad
    for (s32 i = EQUIP_SLOT_D_RIGHT; i <= EQUIP_SLOT_D_UP; i++) {
        if ((DPAD_ITEM(i) == ITEM_BOW) ||
            (DPAD_ITEM(i) >= ITEM_BOW_ARROW_FIRE && DPAD_ITEM(i) <= ITEM_BOW_ARROW_LIGHT)) {
            DPAD_BUTTON_ITEM_EQUIP(i) = bowItem;
            DPAD_SLOT_EQUIP(i) = SLOT_BOW;
            Interface_LoadItemIcon1(play, i + 4); // offset iterator by 4 to account for actual dpad index
            gSaveContext.buttonStatus[i + 5] = BTN_ENABLED; // offset by 5 for same reasons as above
            sButtonFlashTimer = BUTTON_FLASH_DURATION;
            sButtonFlashCount = 0;
        }
    }

    UpdateFlashEffect(play);
}

// Core Arrow Cycling Function
static void CycleToNextArrow(PlayState* play, Player* player) {
    s8 nextArrow = GetNextArrowType(player->heldItemAction);

    if (player->heldActor != NULL && player->heldActor->id == ACTOR_EN_ARROW) {
        EnArrow* arrow = (EnArrow*)player->heldActor;

        if (arrow->actor.child != NULL) {
            Actor_Kill(arrow->actor.child);
        }

        Actor_Kill(&arrow->actor);
    }

    Player_InitItemAction(play, player, static_cast<PlayerItemAction>(nextArrow));
    UpdateEquippedBow(play, nextArrow);
    Sfx_PlaySfxCentered(NA_SE_PL_CHANGE_ARMS);
}

void ArrowCycleMain() {
    if (sJustCycledFrames > 0) {
        sJustCycledFrames--;
    }

    if (gPlayState == nullptr || !CanCycleArrows()) {
        return;
    }

    UpdateFlashEffect(gPlayState);

    Player* player = GET_PLAYER(gPlayState);
    Input* input = &gPlayState->state.input[0];

    // Block camera changes when cycling arrows while drawing the bow
    if ((player->stateFlags1 & PLAYER_STATE1_READY_TO_FIRE) && player->unk_836 == 0) {
        return;
    }

    if (IsAimingBow(player) && CHECK_BTN_ANY(input->press.button, BTN_R)) {
        if (IsHoldingMagicBow(player) && gSaveContext.magicState != MAGIC_STATE_IDLE && player->heldActor == NULL) {
            Sfx_PlaySfxCentered(NA_SE_SY_ERROR);
            return;
        }

        if (player->heldActor != NULL && player->heldActor->id == ACTOR_EN_ARROW) {
            EnArrow* heldArrow = (EnArrow*)player->heldActor;

            // If the held arrow itself is magical, then we should "restore" the consumed magic upon cycling
            if (ARROW_IS_MAGICAL(heldArrow->actor.params)) {
                Magic_Reset(gPlayState); // reset magic state before adding back magic
                GameInteractor::RawAction::AddOrRemoveMagic(
                    sMagicArrowCosts[ARROW_GET_MAGIC_FROM_TYPE(heldArrow->actor.params)]);
            }
        }

        CycleToNextArrow(gPlayState, player);
        // Track that we just cycled for 2 frames to prevent held R input from triggering the shield action when in
        // Z-Target mode as the arrow is respawned (Player_UpperAction_8)
        sJustCycledFrames = 2;
    }
}

// Registration and Hooks
void RegisterArrowCycle() {
    COND_VB_SHOULD(VB_SHIELD_FROM_BUTTON_HOLD, CVAR, {
        if (CanCycleArrows()) {
            Player* player = GET_PLAYER(gPlayState);
            Input* input = &gPlayState->state.input[0];

            // Suppress Shield input when holding an arrow in Z-Target mode
            if (IsHoldingBow(player) && sJustCycledFrames > 0 && CHECK_BTN_ANY(input->cur.button, BTN_R)) {
                *should = false;
            }
        }
    });

    COND_VB_SHOULD(VB_EXIT_FIRST_PERSON_MODE_FROM_BUTTON, CVAR, {
        if (CanCycleArrows()) {
            Player* player = GET_PLAYER(gPlayState);
            Input* input = &gPlayState->state.input[0];

            // Suppress Shield input first person cancel when aiming the bow
            if (IsAimingBow(player) && CHECK_BTN_ANY(input->cur.button, BTN_R)) {
                *should = false;
            }
        }
    });

    COND_ID_HOOK(OnActorUpdate, ACTOR_PLAYER, CVAR, [](void* actor) { ArrowCycleMain(); });
}

static RegisterShipInitFunc initFunc(RegisterArrowCycle, { CVAR_NAME });