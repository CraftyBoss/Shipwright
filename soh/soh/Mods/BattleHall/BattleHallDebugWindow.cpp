#include "BattleHallDebugWindow.h"
#include "battle-hall.h"


extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
#include <soh/Enhancements/nametag.h>
#include <soh/ActorDB.h>
extern PlayState* gPlayState;
}

void BattleHallDebugWindow::Draw() {
    if (!gPlayState || !mHallData || gPlayState->sceneNum != SCENE_BATTLEHALL)
        return;
    Player* player = GET_PLAYER(gPlayState);
    Camera* camera = GET_ACTIVE_CAM(gPlayState);
    if (!player || !camera)
        return;

    ImGui::Begin("Battle Hall Debug Window");

    auto& playerPos = player->actor.world.pos;
    auto& camEyePos = camera->eye;
    auto& camAtPos = camera->at;
    Vec3f relPos = Vec3f(camEyePos.x - playerPos.x, camEyePos.y - playerPos.y, camEyePos.z - playerPos.z);

    ImGui::Text("Player Pos: %f %f %f", playerPos.x, playerPos.y, playerPos.z);
    ImGui::Text("Camera Eye Pos: %f %f %f", camEyePos.x, camEyePos.y, camEyePos.z);
    ImGui::Text("Camera At Pos: %f %f %f", camAtPos.x, camAtPos.y, camAtPos.z);

    ImGui::Text("Relative Camera Pos: %f %f %f", relPos.x, relPos.y, relPos.z);
    ImGui::Text("Loop Offset: %f %f %f", mHallData->curLoopOffset.x, mHallData->curLoopOffset.y, mHallData->curLoopOffset.z);

    if (ImGui::Button("Teleport to LoopPoint1")) {
        Vec3f movePos = Vec3f(playerPos.x, playerPos.y, -LOOP_POINT_Z);
        BattleHall_WarpPlayer(&movePos);
    }

    if (ImGui::Button("Teleport to LoopPoint2")) {
        Vec3f movePos = Vec3f(playerPos.x, playerPos.y, LOOP_POINT_Z);
        BattleHall_WarpPlayer(&movePos);
    }

    if (ImGui::Button("Spawn Test Keese")) {
        Vec3f spawnPos = Vec3f(playerPos.x, playerPos.y + 80.0f, playerPos.z);  
        BattleHall_SpawnActorWithName(ACTOR_EN_FIREFLY, 2, &spawnPos, "CraftyBoss");
    }

    ImGui::End();
}