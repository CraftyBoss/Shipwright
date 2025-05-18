#include "BattleHallDebugWindow.h"
#include "battle-hall.h"


extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
#include <soh/Enhancements/nametag.h>
#include <soh/ActorDB.h>
#include <StringHelper.h>
extern PlayState* gPlayState;
}

const char* actorNames[] = {
    "DekuBaba",     "Keese",    "MadScrub",    "GohmaLarva",  "Stalchild",   "Stinger",     "Shabom",      "SkullKid",
    "BigSkulltula", "Armos",    "Beamos",      "Spike",       "Bari",        "TailPasaran", "FireKeese",   "Tektite",
    "BabyDodongo",  "Anubis",   "TorchSlug",   "Wolfos",      "BigDekuBaba", "ClubMoblin",  "GreenBubble", "RedBubble",
    "BlueBubble",   "Poe",      "Guay",        "IceKeese",    "ReDead",      "ShellBlade",  "WhiteWolfos", "Leever",
    "Dodongo",      "Lizalfos", "WhiteBubble", "Floormaster", "Gibdo",       "Peahat",      "Dinolfos",    "Sharp",
    "Freezard",     "Stalfos",  "IronKnuckle", "DarkLink",
};

void drawSpawnList(const char* name, BattleHallActorTypes start, BattleHallActorTypes end) {
    if (ImGui::TreeNodeEx(name)) {
        for (size_t i = start; i < end; i++) {
            auto type = (BattleHallValidActors)i;

            ImGui::Text(actorNames[i]);

            ImGui::SameLine();
            if (ImGui::SmallButton(StringHelper::Sprintf("Spawn##HallSpawnButton%d", i).c_str())) {
                BattleHall_SpawnActorWithName(type, BattleHall_GetRandomTestName());
            }

            ImGui::SameLine();
            if (ImGui::SmallButton(StringHelper::Sprintf("Queue##HallQueueButton%d", i).c_str())) {
                BattleHall_RegisterActor(type, BattleHall_GetRandomTestName(), StringHelper::Sprintf("TestQueue%d", i).c_str());
            }
        }
        ImGui::TreePop();
    }
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
    auto& playerVel = player->actor.velocity;
    auto& camEyePos = camera->eye;
    auto& camAtPos = camera->at;
    Vec3f relPos = Vec3f(camEyePos.x - playerPos.x, camEyePos.y - playerPos.y, camEyePos.z - playerPos.z);

    ImGui::SeparatorText("Battle Hall Info");

    ImGui::Text("Loop Count: %d", mHallData->loopCount);
    ImGui::Text("Loop Offset: %f %f %f", mHallData->curLoopOffset.x, mHallData->curLoopOffset.y, mHallData->curLoopOffset.z);
    ImGui::Text("Current Alive Actors: %d", mHallData->curAliveActors.size());
    ImGui::Text("Gate State: %s", Flags_GetSwitch(gPlayState, FLAG_OPEN_GATE) ? "Open" : "Closed");
    ImGui::Text("Actor Queue Count: %d", mHallData->actorQueue.size());

    if (ImGui::TreeNodeEx("Spawned Actors")) {
        int idx = 0;
        for (const auto& actor : mHallData->curAliveActors) {
            auto& dbEntry = ActorDB::Instance->RetrieveEntry(actor->id);

            ImGui::Text("%s Pos: %f %f %f", dbEntry.desc.c_str(), actor->world.pos.x, actor->world.pos.y,
                        actor->world.pos.z);
            ImGui::SameLine();
            if (ImGui::SmallButton(StringHelper::Sprintf("Teleport##ActorTPButton%d", idx).c_str())) {
                BattleHall_WarpPlayer(&actor->world.pos);
            }

            ImGui::SameLine();
            if (ImGui::SmallButton(StringHelper::Sprintf("Kill##ActorKillButton%d", idx).c_str())) {
                Actor_Kill(actor);
            }

            idx++;
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Queued Actors")) {
        int i = 0;
        for (const auto& actor : mHallData->actorQueue) {
            ImGui::Text("[%s] %s's %s", actor.donoId.c_str(), actor.name.c_str(), actorNames[actor.type]);

            ImGui::SameLine();
            if (ImGui::SmallButton(StringHelper::Sprintf("Spawn##QueueSpawnButton%d", i).c_str())) {
                if (BattleHall_SpawnActorWithName(actor.type, actor.name.c_str()) != nullptr)
                {
                    auto it = mHallData->actorQueue.begin();
                    std::advance(it, i);
                    mHallData->actorQueue.erase(it);
                }
            }
            i++;
        }
        ImGui::TreePop();
    }


    drawSpawnList("Spawn Very Easy Actors", BH_ACTORS_VERY_EASY, BH_ACTORS_EASY);
    drawSpawnList("Spawn Easy Actors", BH_ACTORS_EASY, BH_ACTORS_MEDIUM);
    drawSpawnList("Spawn Medium Actors", BH_ACTORS_MEDIUM, BH_ACTORS_HARD);
    drawSpawnList("Spawn Hard Actors", BH_ACTORS_HARD, BH_ACTORS_VERY_HARD);
    drawSpawnList("Spawn Very Hard Actors", BH_ACTORS_VERY_HARD, BH_ACTORS_MAX);

    ImGui::SeparatorText("Game Info");

    ImGui::Text("Player Pos: %f %f %f", playerPos.x, playerPos.y, playerPos.z);
    ImGui::Text("Player Vel: %f %f %f", playerVel.x, playerVel.y, playerVel.z);
    ImGui::Text("Player Yaw: %f", player->yaw);
    ImGui::Text("Player Pushed Yaw: %f", player->pushedYaw);
    ImGui::Text("Player Pushed Speed: %f", player->pushedSpeed);
    ImGui::Text("Camera Eye Pos: %f %f %f", camEyePos.x, camEyePos.y, camEyePos.z);
    ImGui::Text("Camera At Pos: %f %f %f", camAtPos.x, camAtPos.y, camAtPos.z);

    ImGui::Text("Relative Camera Pos: %f %f %f", relPos.x, relPos.y, relPos.z);

    ImGui::SeparatorText("Game Interaction");

    ImGui::InputInt("Current Spawn Limit", &mHallData->spawnLimit);

    if (ImGui::Button("Kill all Registered Actors")) {
        for (const auto& actor : mHallData->curAliveActors) {
            Actor_Kill(actor);
        }
    }

    if (ImGui::Button("Clear Actor Queue")) {
        mHallData->actorQueue.clear();
    }

    if (ImGui::Button("Teleport to Start Point")) {
        Vec3f movePos = Vec3f(playerPos.x, playerPos.y, -LOOP_POINT_Z);
        BattleHall_WarpPlayer(&movePos);
    }

    if (ImGui::Button("Teleport to End Point")) {
        Vec3f movePos = Vec3f(playerPos.x, playerPos.y, LOOP_POINT_Z);
        BattleHall_WarpPlayer(&movePos);
    }

    if (ImGui::Button("Teleport to Random Position")) {
        Vec3f movePos = Vec3f();
        BattleHall_GetRandomVec3f(&movePos, Vec3f(-ROOM_BOUND_X, 0, -LOOP_POINT_Z),
                                  Vec3f(ROOM_BOUND_X, 0, LOOP_POINT_Z));
        BattleHall_WarpPlayer(&movePos);
    }

    if (ImGui::Button("Queue All Available Actors")) {
        BattleHall_QueueAllAvailableActors();
    }

    if (ImGui::Button("Reset Distance Offset")) {
        mHallData->curLoopOffset = Vec3f();
        mHallData->loopCount = 0;
    }

    if (ImGui::Button("Activate Gate Flag")) {
        Flags_SetSwitch(gPlayState, FLAG_OPEN_GATE);
    }

    if (ImGui::Button("Deactivate Gate Flag")) {
        Flags_UnsetSwitch(gPlayState, FLAG_OPEN_GATE);
    }

    ImGui::End();
}