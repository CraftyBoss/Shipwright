#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/ShipInit.hpp"

#define CVAR_PREVENT_CONTAINER_NAME CVAR_ENHANCEMENT("PreventNewHeartContainer")
#define CVAR_PREVENT_CONTAINER_VALUE CVarGetInteger(CVAR_PREVENT_CONTAINER_NAME, 0)

static void RegisterPreventContainer() {
    COND_VB_SHOULD(VB_HEARTS_INCREASE_WITH_CONTAINERS, CVAR_PREVENT_CONTAINER_VALUE, {
        *should = false;
    });
}

static RegisterShipInitFunc initFunc(RegisterPreventContainer, { CVAR_PREVENT_CONTAINER_NAME });