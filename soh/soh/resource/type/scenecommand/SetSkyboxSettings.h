#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "Resource.h"
#include "SceneCommand.h"
#include <libultraship/libultra/types.h>

namespace SOH {
typedef struct {
    uint8_t unk;
    uint8_t skyboxId; // skyboxId
    uint8_t weather; // skyboxConfig
    uint8_t indoors; // envLightMode
} SkyboxSettings;

class SetSkyboxSettings : public SceneCommand<SkyboxSettings> {
  public:
    using SceneCommand::SceneCommand;

    SkyboxSettings* GetPointer();
    size_t GetPointerSize();

    SkyboxSettings settings;
};
}; // namespace SOH
