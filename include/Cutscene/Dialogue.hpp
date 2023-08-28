//
// Created by cervi on 31/08/2022.
//

#ifndef UNDERTALE_DIALOGUE_HPP
#define UNDERTALE_DIALOGUE_HPP

#include <nds.h>
#include "Engine/Audio.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Font.hpp"
#include <cstdio>

class Dialogue {
public:
    Dialogue(u16 textId, Engine::Sprite* target,
             const std::string& targetIdle, const std::string& targetTalk, const std::string& typeSndPath,
             const std::string& fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager,
             Engine::AllocationMode heartAlloc);

    Dialogue(const std::string& text_, const std::string& typeSndPath, const std::string& fontTxt,
             u16 framesPerLetter, Engine::TextBGManager& txtManager, Engine::AllocationMode heartAlloc);
    bool update();
    virtual ~Dialogue() = default;

    virtual void doRedraw();

protected:
    virtual void setTalk();
    virtual void setNoTalk();
    void progressText(bool clear_, bool draw_);

    virtual void handleInline(std::string::iterator& pos, bool doEffect);
    virtual int sizeInline(std::string::iterator& posToProgress);
    virtual void onLineBreak();
    virtual void onClear();
    virtual void onOptionChoose();
    virtual void onPause();

    virtual void draw(bool draw_, bool clear_) = 0;
    bool _paused = false;

    constexpr static u16 _lineSpacing = 20;
    int _x = 0, _y = 0;

    u16 _cTimer;
    u16 _letterFrames = 20;

    Engine::Sprite* _target = nullptr;
    Engine::TextBGManager* _textManager;
    int _targetIdle = -1, _targetTalk = -1;
    std::string _text;
    std::string::iterator _textPos;
    // Maybe will be used to repaint the dialogue
    // For example, in FIGHTs the flavor text must be redrawn without
    // going over all the Dialogue animation.
    std::string::iterator _lastClearPos;
    u8 _lastClearColor = 15;

    Audio::WAV _typeSnd;

    Engine::Font _fnt;

    Engine::Texture _heartTexture;
    Engine::Sprite _heartSprite;

    bool _choosingOption = false;
    int _optionCount = 0;
    int _optionPositions[4][2]{0};
    int _currentOption = 0;
    void updateChoosingOption();
};

class DialogueCentered : public Dialogue {
public:
    DialogueCentered(u16 textId, const std::string& speaker, s32 speakerX, s32 speakerY,
                     const std::string& speakerIdle, const std::string& speakerTalk, Engine::Sprite* target,
                     const std::string& targetIdle, const std::string& targetTalk, const std::string& typeSndPath,
                     const std::string& fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager,
                     Engine::AllocationMode heartAlloc);
    DialogueCentered(const std::string& text_, const std::string& typeSndPath,
                     const std::string& fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager,
                     Engine::AllocationMode heartAlloc);
    ~DialogueCentered() override;

protected:
    void clearCentered();
    void draw(bool draw_, bool clear_) override;
    u16 getLineWidth(std::string::iterator pos);
    void setTalk() override;
    void setNoTalk() override;
    int sizeInline(std::string::iterator &posToProgress) override;
    void handleInline(std::string::iterator& pos, bool doEffect) override;
    void onLineBreak() override;
    void onClear() override;
    void onPause() override;
    void onOptionChoose() override;

    Engine::Texture _speakerTex;
    Engine::Sprite _speakerSpr{Engine::AllocatedOAM};
    int _speakerIdle = -1, _speakerTalk = -1;
    int _startingY = 0;
    std::string::iterator _lineStart, _lastPrintedPos;
    u8 _lineStartColor = 15;
    int _lineOptionStart = 0;
};

class DialogueLeftAligned : public Dialogue {
public:
    DialogueLeftAligned(u16 textId, s32 startingX, s32 startingY, Engine::Sprite* target,
                        const std::string& targetIdle, const std::string& targetTalk,
                        const std::string& typeSndPath, const std::string& fontTxt,
                        u16 framesPerLetter, Engine::TextBGManager& txtManager,
                        Engine::AllocationMode heartAlloc);
    DialogueLeftAligned(int startingX, int startingY, const std::string& text_, const std::string& typeSndPath,
                        const std::string& fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager,
                        Engine::AllocationMode heartAlloc);

protected:
    void draw(bool draw_, bool clear_) override;
    void onClear() override;
    void onLineBreak() override;
    void doRedraw() override;
    void handleInline(std::string::iterator &pos, bool doEffect) override;

    int _startingX = 0, _startingY = 0;
    int _lastClearStartingX = 0;
    int _originalStartingX = 0;
};

#endif //UNDERTALE_DIALOGUE_HPP
