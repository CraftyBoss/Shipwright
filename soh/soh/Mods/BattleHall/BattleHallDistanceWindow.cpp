#include "BattleHallDistanceWindow.h"
#include "battle-hall.h"
#include <StringHelper.h>

extern "C" {
#include "functions.h"
#include "macros.h"
#include "variables.h"
extern PlayState* gPlayState;
}

void BattleHallDistWindow::Draw() {
    if (!gPlayState || gPlayState->sceneNum != SCENE_BATTLEHALL)
        return;

    auto vp = ImGui::GetMainViewport();
    const float margin = 100.0f;
    const float padding = 10.0f;
    const float fontScale = 3.0f;
    const ImVec2 shadowOffset = ImVec2(2.0f, 2.0f);
    const ImVec4 shadowColor = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);
    const ImVec4 textColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

    std::string displayText = StringHelper::Sprintf("%dft", (int)floor(mCurDist / DISTANCE_SCALE));

    auto txtSize = ImGui::CalcTextSize(displayText.c_str()) * fontScale;

    auto pos = ImVec2(vp->Pos.x + (vp->Size.x / 2) - (txtSize.x / 2), vp->Pos.y + vp->Size.y - margin);
    
    ImGui::SetNextWindowPos(pos);

    ImGui::Begin("distwindow", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDocking |
                     ImGuiWindowFlags_NoBackground |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove
                     );

    ImGui::SetWindowFontScale(fontScale);

    ImVec2 cursorPos = ImGui::GetCursorPos();

    // draw shadow text first
    ImGui::SetCursorPos(ImVec2(cursorPos.x + shadowOffset.x, cursorPos.y + shadowOffset.y));
    ImGui::TextColored(shadowColor, displayText.c_str());

    // draw normal text next
    ImGui::SetCursorPos(cursorPos);
    ImGui::TextColored(textColor, displayText.c_str());

    ImGui::End();
}

void BattleHallDistWindow::UpdateElement() {

}
