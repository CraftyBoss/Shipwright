#ifdef __cplusplus

#include <string>
#include <libultraship/libultraship.h>
#include "battle-hall.h"

class BattleHallDebugWindow : public Ship::GuiWindow {
  private:
    BattleHallData* mHallData;
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override{};

    void SetData(BattleHallData* data) {
        mHallData = data;
    }
};

#endif