#include <libultraship/bridge.h>
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/ShipInit.hpp"
#include "functions.h"
#include "macros.h"

extern "C" {
#include <z64.h>
#include "soh/cvar_prefixes.h"
#include "variables.h"

extern SaveContext gSaveContext;
extern PlayState* gPlayState;
}

static bool isNextHitProtected = false;
static bool isRespawnFromHit = false;
static bool isVoidDamage = false;
static const int invulnTimer = -60;

#define CVAR_GLOOM_MODE_NAME CVAR_ENHANCEMENT("GloomMode")
#define CVAR_GLOOM_MODE_DH_NAME CVAR_ENHANCEMENT("GloomModeDoubleHits")

#define CVAR_GLOOM_MODE_VALUE CVarGetInteger(CVAR_GLOOM_MODE_NAME, 0)
#define CVAR_GLOOM_MODE_DH_VALUE CVarGetInteger(CVAR_GLOOM_MODE_DH_NAME, 0)

void GloomModeVoidOut() {
    if (!CVarGetInteger(CVAR_ENHANCEMENT("GloomModeDoVoidOut"), 0)) {
        return;
    }

    Audio_PlaySoundGeneral(NA_SE_OC_ABYSS, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultReverb);
    Play_TriggerRespawn(gPlayState);

    // give player some i frames while voiding out
    auto* player = GET_PLAYER(gPlayState);
    if (player->invincibilityTimer > invulnTimer) {
        player->invincibilityTimer = invulnTimer;
    }
    player->damageFlickerAnimCounter = 0;

    // increment hit counter
    gSaveContext.ship.stats.count[COUNT_HITS_TAKEN] += 1;

    // prevent void out damage when reloading
    isRespawnFromHit = true;
}

void VoidDamageBehaviour(GIVanillaBehavior _, bool* should, va_list _originalArgs) {
    va_list args;
    va_copy(args, _originalArgs);

    if (isRespawnFromHit) {
        isRespawnFromHit = false;
        *should = false;
    } else {
        isVoidDamage = *should;
    }

    va_end(args);
}

void RegisterGloomMode() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(
        [](int16_t fileNum) { CVarSetInteger(CVAR_ENHANCEMENT("GloomModeDefenseProtection"), isNextHitProtected); });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnPlayerHealthChange>([](int amount) {
        if (!GameInteractor::IsSaveLoaded())
            return;

        if (!CVAR_GLOOM_MODE_VALUE) {
            // increment hit counter here if gloom mode isnt enabled
            if (amount < 0)
                gSaveContext.ship.stats.count[COUNT_HITS_TAKEN] += 1;

            return;
        }

        if (amount < 0) {
            if (gSaveContext.isDoubleDefenseAcquired && CVAR_GLOOM_MODE_DH_VALUE) {
                isNextHitProtected = !isNextHitProtected;
                CVarSetInteger(CVAR_ENHANCEMENT("GloomModeDefenseProtection"), isNextHitProtected);

                if (isNextHitProtected) {
                    gSaveContext.health = gSaveContext.healthCapacity;

                    if (!isVoidDamage)
                        GloomModeVoidOut();
                    else
                        isVoidDamage = false;
                    return;
                }
            }

            gSaveContext.healthCapacity -= 16;
            gSaveContext.health = gSaveContext.healthCapacity;

            if (!isVoidDamage)
                GloomModeVoidOut();
            else
                isVoidDamage = false;
        }
    });

    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnVanillaBehavior>(VB_INFLICT_VOID_DAMAGE,
                                                                                       VoidDamageBehaviour);
}

static RegisterShipInitFunc initFunc(RegisterGloomMode);