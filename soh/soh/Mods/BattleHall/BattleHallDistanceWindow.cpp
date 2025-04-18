#include "BattleHallDistanceWindow.h"
#include "battle-hall.h"
#include <StringHelper.h>

extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
extern PlayState* gPlayState;
}

const float margin = 100.0f;
const float padding = 10.0f;
const float fontScale = 3.0f;
const ImVec2 shadowOffset = ImVec2(2.0f, 2.0f);
const ImVec4 shadowColor = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);
const ImVec4 textColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

void drawTextAtPos(const char* windowName, const ImVec2& pos, float scale, const char* displayText) {
    ImGui::SetNextWindowPos(pos);

    ImGui::Begin(windowName, nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove);

    ImGui::SetWindowFontScale(scale);

    ImVec2 cursorPos = ImGui::GetCursorPos();

    // draw shadow text first
    ImGui::SetCursorPos(ImVec2(cursorPos.x + shadowOffset.x, cursorPos.y + shadowOffset.y));
    ImGui::TextColored(shadowColor, displayText);

    // draw normal text next
    ImGui::SetCursorPos(cursorPos);
    ImGui::TextColored(textColor, displayText);

    ImGui::End();
}

void BattleHallDistWindow::Draw() {
    if (!gPlayState || gPlayState->sceneNum != SCENE_BATTLEHALL)
        return;

    auto vp = ImGui::GetMainViewport();
    
    std::string displayText = StringHelper::Sprintf("%dft", (int)floor(mHallData->totalRunDist / DISTANCE_SCALE));

    auto txtSize = ImGui::CalcTextSize(displayText.c_str()) * fontScale;
    
    ImGui::SetNextWindowViewport(vp->ID);
    drawTextAtPos("DistWindow", ImVec2(vp->Pos.x + (vp->Size.x / 2) - (txtSize.x / 2), vp->Pos.y + vp->Size.y - margin),
                  fontScale,
                  displayText.c_str());\

    displayText = StringHelper::Sprintf("Enemies Remaining: %d", mHallData->curAliveActors.size());

    ImGui::SetNextWindowViewport(vp->ID);
    drawTextAtPos("RemainWindow", ImVec2(vp->Pos.x + margin, vp->Pos.y + vp->Size.y - 100.0f), 2.0f, displayText.c_str());
}

void BattleHallDistWindow::UpdateElement() {

}
