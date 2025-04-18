#include "OTRGlobals.h"
#include "ResourceManagerHelpers.h"
#include <libultraship/libultraship.h>
#include "soh/resource/type/Scene.h"
#include <utils/StringHelper.h>
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "global.h"
#include "vt.h"
#include <Vertex.h>
#include "resource/type/scenecommand/SetStartPositionList.h"
#include "resource/type/scenecommand/SetLightingSettings.h"
#include "resource/type/scenecommand/SetSkyboxSettings.h"
#include "resource/type/scenecommand/SetActorList.h"
#include "resource/type/scenecommand/SetRoomList.h"
#include "resource/type/scenecommand/SetObjectList.h"
#include "resource/type/scenecommand/SetPathways.h"
#include "resource/type/scenecommand/SetSoundSettings.h"

extern "C" void Play_InitScene(PlayState* play, s32 spawn);
extern "C" void Play_InitEnvironment(PlayState* play, s16 skyboxId);
void OTRPlay_InitScene(PlayState* play, s32 spawn);
s32 OTRScene_ExecuteCommands(PlayState* play, SOH::Scene* scene);
void OTRPlay_HookSceneCommands(PlayState* play, s32 sceneId, SOH::Scene* scene);

// LUS::OTRResource* OTRPlay_LoadFile(PlayState* play, RomFile* file) {
Ship::IResource* OTRPlay_LoadFile(PlayState* play, const char* fileName) {
    auto res = Ship::Context::GetInstance()->GetResourceManager()->LoadResource(fileName);
    return res.get();
}

extern "C" void OTRPlay_SpawnScene(PlayState* play, s32 sceneId, s32 spawn) {
    SceneTableEntry* scene = &gSceneTable[sceneId];

    scene->unk_13 = 0;
    play->loadedScene = scene;
    play->sceneNum = sceneId;
    play->sceneConfig = scene->config;

    // osSyncPrintf("\nSCENE SIZE %fK\n", (scene->sceneFile.vromEnd - scene->sceneFile.vromStart) / 1024.0f);

    // Scenes considered "dungeon" with a MQ variant
    int16_t inNonSharedScene = (sceneId >= SCENE_DEKU_TREE && sceneId <= SCENE_ICE_CAVERN) ||
                               sceneId == SCENE_GERUDO_TRAINING_GROUND || sceneId == SCENE_INSIDE_GANONS_CASTLE;

    std::string sceneVersion = "shared";
    if (inNonSharedScene) {
        sceneVersion = ResourceMgr_IsGameMasterQuest() ? "mq" : "nonmq";
    }
    std::string scenePath = StringHelper::Sprintf("scenes/%s/%s/%s", sceneVersion.c_str(), scene->sceneFile.fileName,
                                                  scene->sceneFile.fileName);

    play->sceneSegment = OTRPlay_LoadFile(play, scenePath.c_str());

    // Failed to load scene... default to doodongs cavern
    if (play->sceneSegment == nullptr) {
        lusprintf(__FILE__, __LINE__, 2, "Unable to load scene %s... Defaulting to Doodong's Cavern!\n",
                  scenePath.c_str());
        OTRPlay_SpawnScene(play, 0x01, 0);
        return;
    }

    scene->unk_13 = 0;

    // gSegments[2] = VIRTUAL_TO_PHYSICAL(play->sceneSegment);

    OTRPlay_HookSceneCommands(play, sceneId, (SOH::Scene*)play->sceneSegment);

    OTRPlay_InitScene(play, spawn);
    auto roomSize = func_80096FE8(play, &play->roomCtx);

    osSyncPrintf("ROOM SIZE=%fK\n", roomSize / 1024.0f);

    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSceneInit>(play->sceneNum);
    SPDLOG_INFO("Scene Init - sceneNum: {0:#x}, entranceIndex: {1:#x}", play->sceneNum, gSaveContext.entranceIndex);
}

void OTRPlay_InitScene(PlayState* play, s32 spawn) {
    play->curSpawn = spawn;
    play->linkActorEntry = nullptr;
    play->unk_11DFC = nullptr;
    play->setupEntranceList = nullptr;
    play->setupExitList = nullptr;
    play->cUpElfMsgs = nullptr;
    play->setupPathList = nullptr;
    play->numSetupActors = 0;
    Object_InitBank(play, &play->objectCtx);
    LightContext_Init(play, &play->lightCtx);
    TransitionActor_InitContext(&play->state, &play->transiActorCtx);
    func_80096FD4(play, &play->roomCtx.curRoom);
    YREG(15) = 0;
    gSaveContext.worldMapArea = 0;
    OTRScene_ExecuteCommands(play, (SOH::Scene*)play->sceneSegment);
    Play_InitEnvironment(play, play->skyboxId);
    /* auto data = static_cast<LUS::Vertex*>(Ship::Context::GetInstance()
                                               ->GetResourceManager()
                                               ->ResourceLoad("object_link_child\\object_link_childVtx_01FE08")
                                               .get());

    auto data2 = ResourceMgr_LoadVtxByCRC(0x68d4ea06044e228f);*/

    volatile int a = 0;
}

void OTRPlay_HookSceneCommands(PlayState* play, s32 sceneId, SOH::Scene* scene) {
    if (sceneId != SCENE_BATTLEHALL) {
        return;
    }

    for (auto& cmd : scene->commands) {
        if (cmd->cmdId == SOH::SceneCommandID::SetStartPositionList) {
            SOH::SetStartPositionList* cmdActList = (SOH::SetStartPositionList*)cmd.get();
            auto& playerEntry = cmdActList->startPositions[0];

            playerEntry.pos.x = 0.0f;
            playerEntry.pos.y = 0.0f;
            playerEntry.pos.z = 0.0f;

        } else if (cmd->cmdId == SOH::SceneCommandID::SetSkyboxSettings) {
            SOH::SetSkyboxSettings* cmdSettings = (SOH::SetSkyboxSettings*)cmd.get();

            cmdSettings->settings.indoors = 1; // LightMode::LIGHT_MODE_SETTINGS
        } else if (cmd->cmdId == SOH::SceneCommandID::SetLightingSettings) {
            SOH::SetLightingSettings* cmdSettings = (SOH::SetLightingSettings*)cmd.get();
            SOH::EnvLightSettings lightSetting = {
                { 70, 70, 70 },    // Ambient Color
                { 73, -73, 73 },   // Diffuse0 Direction
                { 35, 35, 35 },    // Diffuse0 Color
                { -73, 73, -73 },  // Diffuse1 Direction
                { 100, 100, 100 }, // Diffuse1 Color
                { 30, 30, 30 },    // Fog Color
                ((1 << 10) | 900), // Blend Rate & Fog Near
                2500,              // Clipping Plane
            };

            cmdSettings->settings.clear();
            cmdSettings->settings.push_back(lightSetting);
        } else if (cmd->cmdId == SOH::SceneCommandID::SetSoundSettings) {
            SOH::SetSoundSettings* cmdSound = (SOH::SetSoundSettings*)cmd.get();
            cmdSound->settings.seqId = NA_BGM_FOREST_TEMPLE;
        }
    }

    // remove end marker from commands so we can add more

    auto endCmd = scene->commands.back();
    scene->commands.pop_back();

    // scene command additions
    {
        // add object bank entries
        std::shared_ptr<SOH::SetObjectList> cmdObjList = std::make_shared<SOH::SetObjectList>();
        cmdObjList->cmdId = SOH::SceneCommandID::SetObjectList;

        cmdObjList->objects.push_back(OBJECT_MIZU_OBJECTS);
        cmdObjList->numObjects = cmdObjList->objects.size();

        scene->commands.push_back(cmdObjList);

        // add SetActorList command to scene

        std::shared_ptr<SOH::SetActorList> cmdActorList = std::make_shared<SOH::SetActorList>();
        cmdActorList->cmdId = SOH::SceneCommandID::SetActorList;

        // maybe move these to 857.0f to "hide" visible seam in ground
        cmdActorList->actorList.push_back({ ACTOR_BG_MIZU_SHUTTER,    { 0, 0, 1000 }, { 0, 0, 0 }, 0x1FCA }); // center
        cmdActorList->actorList.push_back({ ACTOR_BG_MIZU_SHUTTER,  { 160, 0, 1000 }, { 0, 0, 0 }, 0x1FCA }); // left
        cmdActorList->actorList.push_back({ ACTOR_BG_MIZU_SHUTTER, { -160, 0, 1000 }, { 0, 0, 0 }, 0x1FCA }); // right

        cmdActorList->numActors = cmdActorList->actorList.size();

        scene->commands.push_back(cmdActorList);

        // create some paths for enemies that need them

        std::shared_ptr<SOH::SetPathways> cmdPathways = std::make_shared<SOH::SetPathways>();
        cmdPathways->cmdId = SOH::SceneCommandID::SetPathways;
        // doesnt seem like we need to include file paths along with these custom paths, so dont worry about it for now

        // Path 1 (Green Bubbles)
        SOH::Path* path1 = new SOH::Path();
        std::vector<Vec3s> path1Points;

        path1Points.push_back(Vec3s(-150.0f, 60.0f, 134.0f));
        path1Points.push_back(Vec3s(-150.0f, 60.0f, -902.0f));
        path1Points.push_back(Vec3s(150.0f, 60.0f, -902.0f));
        path1Points.push_back(Vec3s(150.0f, 60.0f, 134.0f));

        path1->paths.push_back(path1Points);

        SOH::PathData path1Data;
        path1Data.count = path1Points.size();
        path1Data.points = path1->paths.back().data();

        path1->pathData.push_back(path1Data);

        cmdPathways->paths.push_back(path1->GetPointer());

        // Path 2 (White Bubbles)
        /*{
            SOH::Path* path2 = new SOH::Path();
            std::vector<Vec3s> path2Points;

            path2Points.push_back(Vec3s(0.0f, 60.0f, -300.0f));
            path2Points.push_back(Vec3s(0.0f, 60.0f, -900.0f));

            path2->paths.push_back(path2Points);

            SOH::PathData path2Data;
            path2Data.count = path2Points.size();
            path2Data.points = path2->paths.back().data();

            path2->pathData.push_back(path2Data);

            cmdPathways->paths.push_back(path2->GetPointer());
        }*/

        cmdPathways->numPaths = cmdPathways->paths.size();

        scene->commands.push_back(cmdPathways);
    }

    // add end command back
    scene->commands.push_back(endCmd);
}