//
// Created by cervi on 02/09/2022.
//

#include "Battle/Battle.hpp"

Battle* globalBattle = nullptr;

Battle::Battle() : playerManager(Engine::Allocated3D) {
    char buffer[100];
    FILE* f = fopen("nitro:/spr/battle/spr_heart.cspr", "rb");
    if (f) {
        int sprLoad = player.loadCSPR(f);
        if (sprLoad != 0) {
            sprintf(buffer, "Error loading battle heart spr: %d", sprLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening battle heart spr");
    }
    fclose(f);
}

void Battle::loadFromStream(FILE *stream) {
    fread(&enemyCount, 1, 1, stream);
    enemies = new Enemy[enemyCount];
    char buffer[100];
    for (int i = 0; i < enemyCount; i++) {
        fread(&enemies[i].enemyId, 2, 1, stream);
        fread(&enemies[i].maxHp, 2, 1, stream);
        enemies[i].hp = enemies[i].maxHp;

        sprintf(buffer, "nitro:/data/enemies/name%d.txt", enemies[i].enemyId);
        FILE* enemyNameFile = fopen(buffer, "rb");
        int len = strlen_file(enemyNameFile, '\0');
        fread(&enemies[i].enemyName, len + 1, 1, stream);
        enemies[i].enemyName[len] = '\0';
        fclose(enemyNameFile);
    }
}

void Battle::draw() {
    for (int i = 0; i < spriteCount; i++) {
        sprites[i]->draw(false);
    }
    if (currentBattleAttack != nullptr)
        currentBattleAttack->draw();
}

void Battle::update() {
    if (currentBattleAttack != nullptr) {
        if (currentBattleAttack->update()) {
            currentBattleAttack->free_();
            delete currentBattleAttack;
            currentBattleAttack = nullptr;
        }
    }
}

void Battle::free_() {
    delete[] enemies;
    enemies = nullptr;
    enemyCount = 0;
    for (int i = 0; i < spriteCount; i++) {
        sprites[i]->free_();
        delete sprites[i];
        sprites[i] = nullptr;
    }
    delete[] sprites;
    sprites = nullptr;
}

void runBattle(FILE* stream) {
    int timer = ROOM_CHANGE_FADE_FRAMES;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(3, (-16 * (ROOM_CHANGE_FADE_FRAMES - timer)) / ROOM_CHANGE_FADE_FRAMES);
        timer--;
    }

    globalRoom->push();
    Engine::clearMain();
    globalInGameMenu.unload();

    lcdMainOnBottom();

    globalBattle = new Battle();
    globalBattle->loadFromStream(stream);

    if (globalCutscene != nullptr) {
        globalCutscene->update(LOAD_BATTLE);
        globalCutscene->runCommands(LOAD_BATTLE);
    }

    timer = ROOM_CHANGE_FADE_FRAMES;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(3, (-16 * timer) / ROOM_CHANGE_FADE_FRAMES);
        timer--;
    }

    while (globalBattle->running) {
        Engine::tick();
        if (globalCutscene != nullptr) {
            if (currentDialogue != nullptr) {
                if (currentDialogue->update()) {
                    currentDialogue->free_();
                    delete currentDialogue;
                    currentDialogue = nullptr;
                }
            }
            globalCutscene->update(BATTLE);
            if (globalCutscene->runCommands(BATTLE)) {
                delete globalCutscene;
                globalCutscene = nullptr;
                globalInGameMenu.show();
                globalPlayer->playerControl = true;
                globalCamera.manual = false;
            }
        }
        globalBattle->update();
        globalBattle->draw();
    }

    timer = ROOM_CHANGE_FADE_FRAMES;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(3, (-16 * (ROOM_CHANGE_FADE_FRAMES - timer)) / ROOM_CHANGE_FADE_FRAMES);
        timer--;
    }

    globalBattle->free_();
    delete globalBattle;

    lcdMainOnTop();

    globalRoom->pop();
    globalInGameMenu.load();

    if (globalCutscene != nullptr) {
        globalCutscene->update(LOAD_ROOM);
        globalCutscene->runCommands(LOAD_ROOM);
    }
    globalCamera.updatePosition(true);
    globalPlayer->draw();
    globalRoom->draw();

    timer = ROOM_CHANGE_FADE_FRAMES;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(3, (-16 * timer) / ROOM_CHANGE_FADE_FRAMES);
        timer--;
    }
}
