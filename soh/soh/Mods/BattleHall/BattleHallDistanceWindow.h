#ifdef __cplusplus

#include <string>
#include <libultraship/libultraship.h>

class BattleHallDistWindow : public Ship::GuiWindow {
  private:
    int mCurDist = 0;
  public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override;

    inline void SetDistance(int dist) {
        mCurDist = dist;
    }
};

#endif