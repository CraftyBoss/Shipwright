/**
 * This file handles the custom messages for Gossip Stone
 * hints.
 */
#include <soh/OTRGlobals.h>
#include <spdlog/spdlog.h>

#include "soh/Enhancements/randomizer/randomizer.h"

extern "C" {
extern PlayState* gPlayState;
#include <macros.h>
#include <functions.h>
#include <variables.h>
#include <overlays/actors/ovl_En_Gs/z_en_gs.h>
}
constexpr int maxHintIdx = RH_DODONGOS_CAVERN_GOSSIP_STONE; // RH_MAX

int curViewHintIdx = RH_NONE; // RH_DODONGOS_CAVERN_GOSSIP_STONE - 1;
bool isSawNoHintsLeft = false;
bool isRestartViewHints = false;

void BuildHintStoneMessage(uint16_t* textId, bool* loadFromMessageTable) {
    if ((RAND_GET_OPTION(RSK_GOSSIP_STONE_HINTS).Is(RO_GOSSIP_STONES_NEED_TRUTH) &&
         Player_GetMask(gPlayState) != PLAYER_MASK_TRUTH) ||
        (RAND_GET_OPTION(RSK_GOSSIP_STONE_HINTS).Is(RO_GOSSIP_STONES_NEED_STONE) &&
         CHECK_QUEST_ITEM(QUEST_STONE_OF_AGONY) == 0)) {
        return;
    }
    CustomMessage msg;
    Actor* stone = GET_PLAYER(gPlayState)->talkActor;
    RandomizerHint stoneHint = RH_NONE;
    int16_t hintParams = stone->params & 0xFF;

    if (Rando::StaticData::stoneParamsToHint.contains(hintParams)) {
        stoneHint = Rando::StaticData::stoneParamsToHint[hintParams];
    } else if (hintParams == 0x18) {
        for (size_t i = 0; i < ACTORCAT_MAX; i++) {
            if (gPlayState->actorCtx.actorLists[i].length) {
                if (gPlayState->actorCtx.actorLists[i].head->id == 10 &&
                    Rando::StaticData::grottoChestParamsToHint.contains(
                        gPlayState->actorCtx.actorLists[i].head->params)) {
                    stoneHint =
                        Rando::StaticData::grottoChestParamsToHint[gPlayState->actorCtx.actorLists[i].head->params];
                }
            }
        }
    }
    if (stoneHint == RH_NONE) {
        msg = CustomMessage("INVALID STONE. PARAMS: " + std::to_string(hintParams));
    } else {
        msg = OTRGlobals::Instance->gRandoContext->GetHint(stoneHint)->GetHintMessage(MF_AUTO_FORMAT);

        auto hintName = Rando::StaticData::hintNames[stoneHint];
        SPDLOG_DEBUG("Gossip Stone's Hint Name: " + hintName.GetEnglish()); 
    }
    // Remove "Buy " if present.
    msg.Replace("Buy ", "");
    msg.Replace("Acheter: ", "");
    msg.Replace(" kaufen ", "");
    msg.Replace(" kaufen", "");
    msg.LoadIntoFont();
    *loadFromMessageTable = false;
}

void BuildHintBuyMessage(uint16_t* textId, bool* loadFromMessageTable) {
    CustomMessage msg;
    if (isSawNoHintsLeft) {
        msg = "All hints read. Would you like to start over?" + CustomMessage::TWO_WAY_CHOICE() + "%gYes&No%w";
        isRestartViewHints = true;
    } else {
        msg = "Read another hint?" + CustomMessage::TWO_WAY_CHOICE() + "%gYes&No%w";
    }

    msg.AutoFormat();
    msg.LoadIntoFont();

    *loadFromMessageTable = false;
}

void BuildBoughtHintMessage(uint16_t* textId, bool* loadFromMessageTable) {
    CustomMessage msg;
    Actor* stone = GET_PLAYER(gPlayState)->talkActor;

    if (isSawNoHintsLeft && isRestartViewHints) {
        isSawNoHintsLeft = false;
        isRestartViewHints = false;
        curViewHintIdx = RH_NONE;
    }

    if (curViewHintIdx > maxHintIdx) {
        msg = CustomMessage("No more hints available.");
        msg.AutoFormat();
        isSawNoHintsLeft = true;
    } else {
        auto hint = OTRGlobals::Instance->gRandoContext->GetHint((RandomizerHint)++curViewHintIdx);

        while (!hint->IsEnabled() && curViewHintIdx <= maxHintIdx) {
            auto hintName = Rando::StaticData::hintNames[curViewHintIdx];
            SPDLOG_DEBUG("Skipped Hint: " + hintName.GetEnglish()); 
            hint = OTRGlobals::Instance->gRandoContext->GetHint((RandomizerHint)++curViewHintIdx);
        }

        if (curViewHintIdx > maxHintIdx || !hint->IsEnabled()) {
            msg = CustomMessage("No more hints available.");
            msg.AutoFormat();
            isSawNoHintsLeft = true;
        } else {
            auto hintName = Rando::StaticData::hintNames[curViewHintIdx];
            msg = CustomMessage("%y") + hintName + "%w";
            msg += CustomMessage::NEWLINE();
            msg += hint->GetHintMessage(MF_RAW);

            msg.AutoFormat();

            SPDLOG_DEBUG("Cur View Hint: " + hintName.GetEnglish()); 

            msg.Replace("Buy ", "");
            msg.Replace("Acheter: ", "");
            msg.Replace(" kaufen ", "");
            msg.Replace(" kaufen", "");
        }
    }

    msg.LoadIntoFont();
    *loadFromMessageTable = false;
}

bool HandleStoneContinueTextbox(EnGs* stone) {
    switch (stone->actor.textId) {
        case TEXT_RANDOMIZER_GOSSIP_STONE_HINTS:
            stone->actor.textId = TEXT_RANDOMIZER_BUY_GOSSIP_HINT;
            return true;
        case TEXT_RANDOMIZER_BUY_GOSSIP_HINT: {
            if (gPlayState->msgCtx.choiceIndex == 0) {
                stone->actor.textId = TEXT_RANDOMIZER_SHOW_BUY_GOSSIP_HINT;
                return true;
            }
            return false;
        }
        case TEXT_RANDOMIZER_SHOW_BUY_GOSSIP_HINT: {
            if (isSawNoHintsLeft) {
                return false;
            }
            stone->actor.textId = TEXT_RANDOMIZER_BUY_GOSSIP_HINT;
            return true;
        }
    }
    return false;
}

void RegisterGossipStoneHints() {
    COND_ID_HOOK(OnOpenText, TEXT_RANDOMIZER_GOSSIP_STONE_HINTS,
                 RAND_GET_OPTION(RSK_GOSSIP_STONE_HINTS).IsNot(RO_GOSSIP_STONES_NONE), BuildHintStoneMessage);

    COND_ID_HOOK(OnOpenText, TEXT_RANDOMIZER_BUY_GOSSIP_HINT,
                RAND_GET_OPTION(RSK_GOSSIP_STONE_HINTS).IsNot(RO_GOSSIP_STONES_NONE), BuildHintBuyMessage);

    COND_ID_HOOK(OnOpenText, TEXT_RANDOMIZER_SHOW_BUY_GOSSIP_HINT,
                RAND_GET_OPTION(RSK_GOSSIP_STONE_HINTS).IsNot(RO_GOSSIP_STONES_NONE), BuildBoughtHintMessage);

    // use COND here with an always true statement because REGISTER_VB_SHOULD does not unregister previous hook ids when re-ran.
    COND_VB_SHOULD(VB_GS_CONTINUE_TEXTBOX, true, {
        if (!CVarGetInteger(CVAR_ENHANCEMENT("TimeSavers.ShowHintsWithAgony"), 0) ||
            CHECK_QUEST_ITEM(QUEST_STONE_OF_AGONY) == 0)
            return;

        auto stone = static_cast<EnGs*>(va_arg(args, EnGs*));
        *should = HandleStoneContinueTextbox(stone);
    });
}

static RegisterShipInitFunc initFunc(RegisterGossipStoneHints, { "IS_RANDO" });