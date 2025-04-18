#ifdef __cplusplus

#include <string>
#include <libultraship/libultraship.h>
#include "battle-hall.h"

class BattleHallDistWindow : public Ship::GuiWindow {
  private:
    BattleHallData* mHallData = nullptr;
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override;

    inline void SetHallData(BattleHallData* data) {
        mHallData = data;
    }
};

#endif