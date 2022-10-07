//
// Created by cervi on 27/08/2022.
//

#include "Room/Room.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Audio.hpp"
#include "Save.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Room/Player.hpp"
#include "Formats/utils.hpp"
#include "Room/Camera.hpp"

Room::Room(int roomId) : roomId(roomId) {
    char buffer[100];
    sprintf(buffer, "nitro:/data/rooms/room%d.room", roomId);
    FILE* f = fopen(buffer, "rb");
    if (f) {
        int roomLoad = loadRoom(f);
        if (roomLoad != 0) {
            sprintf(buffer, "Error loading room %d: %d", roomId, roomLoad);
            nocashMessage(buffer);
            fclose(f);
            return;
        }
    } else {
        sprintf(buffer, "Error opening room %d", roomId);
        nocashMessage(buffer);
        return;
    }
    fclose(f);

    bg.loadPath(roomData.roomBg);

    int bgLoad = Engine::loadBgExtendedMain(bg, 512 / 8);
    if (bgLoad != 0) {
        sprintf(buffer, "Error loading room bg: %d", bgLoad);
        nocashMessage(buffer);
    }

    if (roomData.musicBg[0] != 0) {
        bool musicChange = Audio::currentBGMusic.getFilename() == nullptr;
        if (!musicChange)
            musicChange = strcmp(roomData.musicBg, Audio::currentBGMusic.getFilename()) != 0;
        if (musicChange) {
            Audio::playBGMusic(roomData.musicBg, true);
        }
    } else {
        Audio::stopBGMusic();
    }

    loadSprites();
}

int Room::loadRoom(FILE *f) {
    ROOMFile roomFile;

    fread(roomFile.header.header, 4, 1, f);
    char expectedHeader[4] = {'R', 'O', 'O', 'M'};

    if (memcmp(expectedHeader, roomFile.header.header, 4) != 0) {
        return 1;
    }

    fread(&roomFile.header.fileSize, 4, 1, f);
    long pos = ftell(f);
    fseek(f, 0, SEEK_END);
    u32 size = ftell(f);
    fseek(f, pos, SEEK_SET);

    if (roomFile.header.fileSize != size) {
        return 2;
    }

    fread(&roomFile.header.version, 4, 1, f);
    if (roomFile.header.version != 8) {
        return 3;
    }

    fread(&roomFile.partCount, 1, 1, f);

    bool valid = false;
    for (int i = 0; i < roomFile.partCount && !valid; i++) {
        fread(&roomData.lengthBytes, 4, 1, f);
        long endPos = ftell(f) + roomData.lengthBytes;
        fread(&roomData.conditionCount, 1, 1, f);

        valid = true;
        for (int j = 0; j < roomData.conditionCount && valid; j++) {
            if (!evaluateCondition(f))
                valid = false;
        }

        if (!valid)
            fseek(f, endPos, SEEK_SET);
    }
    if (!valid)  // no valid room part found
        return 4;

    int bgPathLen = str_len_file(f, 0);
    if (bgPathLen == -1)
        return 5;

    fread(roomData.roomBg, bgPathLen + 1, 1, f);
    roomData.roomBg[bgPathLen] = 0;

    int musicPathLen = str_len_file(f, 0);
    if (musicPathLen == -1)
        return 5;

    fread(roomData.musicBg, musicPathLen + 1, 1, f);
    roomData.musicBg[musicPathLen] = 0;

    fread(&spawnX, 2, 1, f);
    fread(&spawnY, 2, 1, f);

    fread(&roomData.roomExits.exitCount, 1, 1, f);
    roomData.roomExits.roomExits = new ROOMExit[roomData.roomExits.exitCount];
    ROOMExit* roomExits = roomData.roomExits.roomExits;

    rectExitCount = 0;
    for (int i = 0; i < roomData.roomExits.exitCount; i++) {
        fread(&roomExits[i].exitType, 1, 1, f);
        fread(&roomExits[i].roomId, 2, 1, f);
        fread(&roomExits[i].spawnX, 1, 2, f);
        fread(&roomExits[i].spawnY, 1, 2, f);
        switch (roomExits[i].exitType) {
            case 0:
                fread(&roomExits[i].side, 1, 1, f);
                switch (roomExits[i].side) {
                    case 0:
                        exitTop = &roomExits[i];
                        break;
                    case 1:
                        exitBtm = &roomExits[i];
                        break;
                    case 2:
                        exitLeft = &roomExits[i];
                        break;
                    case 3:
                        exitRight = &roomExits[i];
                        break;
                    default:
                        break;
                }
                break;
            case 1:
                rectExitCount++;
                fread(&roomExits[i].x, 2, 1, f);
                fread(&roomExits[i].y, 2, 1, f);
                fread(&roomExits[i].w, 2, 1, f);
                fread(&roomExits[i].h, 2, 1, f);
                break;
            default:
                break;
        }
    }

    rectExits = new ROOMExit*[rectExitCount];
    for (int i = 0, j = 0; i < roomData.roomExits.exitCount; i++) {
        if (roomExits[i].exitType != 1)
            continue;
        rectExits[j++] = &roomExits[i];
    }

    fread(&textureCount, 1, 1, f);
    textures = new Engine::Texture*[textureCount];
    char path[100];
    for (int i = 0; i < textureCount; i++){
        textures[i] = new Engine::Texture;

        int sprPathLen = str_len_file(f, 0);
        if (sprPathLen == -1)
            return 5;
        fread(path, sprPathLen + 1, 1, f);

        textures[i]->loadPath(path);
    }

    fread(&spriteCount, 1, 1, f);
    roomData.roomSprites.roomSprites = new ROOMSprite[spriteCount];
    ROOMSprite* roomSprites = roomData.roomSprites.roomSprites;

    for (int i = 0; i < spriteCount; i++) {
        fread(&roomSprites[i].textureId, 1, 1, f);
        fread(&roomSprites[i].x, 2, 1, f);
        fread(&roomSprites[i].y, 2, 1, f);
        int animLen = str_len_file(f, 0);
        if (animLen == -1)
            return 5;
        roomSprites[i].animation = new char[animLen + 1];
        fread(roomSprites[i].animation, animLen + 1, 1, f);
        fread(&roomSprites[i].interactAction, 1, 1, f);
        if (roomSprites[i].interactAction == 1) {
            fread(&roomSprites[i].cutsceneId, 2, 1, f);
        } else if (roomSprites[i].interactAction == 2) {
            fread(&roomSprites[i].distance, 2, 1, f);
            animLen = str_len_file(f, 0);
            roomSprites[i].closeAnim = new char[animLen + 1];
            fread(roomSprites[i].closeAnim, animLen + 1, 1, f);
        }
    }

    fread(&roomData.roomColliders.colliderCount, 2, 1, f);
    roomData.roomColliders.roomColliders = new ROOMCollider[roomData.roomColliders.colliderCount];
    ROOMCollider* roomColliders = roomData.roomColliders.roomColliders;

    for (int i = 0; i < roomData.roomColliders.colliderCount; i++) {
        fread(&roomColliders[i].x, 2, 1, f);
        fread(&roomColliders[i].y, 2, 1, f);
        fread(&roomColliders[i].w, 2, 1, f);
        fread(&roomColliders[i].h, 2, 1, f);
        fread(&roomColliders[i].colliderAction, 1, 1, f);
        fread(&roomColliders[i].enabled, 1, 1, f);
        if (roomColliders[i].colliderAction == 1) {
            fread(&roomColliders[i].cutsceneId, 2, 1, f);
        }
    }

    return 0;
}

void Room::free_() {
    bg.free_();
    delete[] roomData.roomExits.roomExits;
    roomData.roomExits.roomExits = nullptr;
    for (int i = 0; i < spriteCount; i++) {
        sprites[i]->free_();
        delete sprites[i];
    }
    for (int i = 0; i < textureCount; i++) {
        delete textures[i];
    }
    delete[] textures;
    for (int i = 0; i < roomData.roomSprites.spriteCount; i++) {
        delete[] roomData.roomSprites.roomSprites[i].animation;
        roomData.roomSprites.roomSprites[i].animation = nullptr;
        delete[] roomData.roomSprites.roomSprites[i].closeAnim;
        roomData.roomSprites.roomSprites[i].closeAnim = nullptr;
    }
    delete[] roomData.roomSprites.roomSprites;
    roomData.roomSprites.roomSprites = nullptr;
    delete[] sprites;
    sprites = nullptr;

    delete[] roomData.roomColliders.roomColliders;
    roomData.roomColliders.roomColliders = nullptr;
    bg.free_();
}

void Room::loadSprites() {
    sprites = new ManagedSprite*[spriteCount];
    for (int i = 0; i < spriteCount; i++) {
        sprites[i] = new ManagedSprite(Engine::Allocated3D);
        sprites[i]->load(&roomData.roomSprites.roomSprites[i], textureCount, textures);
    }
}

bool Room::evaluateCondition(FILE *f) {
    ROOMPartCondition cond;
    fread(&cond.flagId, 2, 1, f);
    fread(&cond.cmpOperator, 1, 1, f);
    bool flip = cond.cmpOperator & 4;
    cond.cmpOperator = cond.cmpOperator & 3;
    fread(&cond.cmpValue, 2, 1, f);

    u16 flagValue = globalSave.flags[cond.flagId];
    bool flag = false;
    if (cond.cmpOperator == ComparisonOperator::EQUALS)
        flag = (flagValue == cond.cmpValue);
    else if (cond.cmpOperator == ComparisonOperator::GREATER_THAN)
        flag = (flagValue > cond.cmpValue);
    else if (cond.cmpOperator == ComparisonOperator::LESS_THAN)
        flag = (flagValue < cond.cmpValue);
    if (flip)
        flag = !flag;
    return flag;
}

void Room::draw() const {
    for (int i = 0; i < spriteCount; i++) {
        sprites[i]->draw(true);
    }
}

void loadNewRoom(int roomId, s32 spawnX, s32 spawnY) {
    int timer = ROOM_CHANGE_FADE_FRAMES;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(1, (-16 * (ROOM_CHANGE_FADE_FRAMES - timer)) / ROOM_CHANGE_FADE_FRAMES);
        timer--;
    }

    globalRoom->free_();

    for (int i = 210; i <= 219; i++) {
        globalSave.flags[i] = 0; // clear room specific flags
    }

    delete globalRoom;
    globalRoom = new Room(roomId);
    globalPlayer->spriteManager.wx = spawnX << 8;
    globalPlayer->spriteManager.wy = spawnY << 8;

    if (globalCutscene != nullptr) {
        globalCutscene->runCommands(LOAD_ROOM);
    }
    globalCamera.updatePosition(true);
    globalPlayer->draw();
    globalRoom->draw();

    timer = ROOM_CHANGE_FADE_FRAMES;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(1, (-16 * timer) / ROOM_CHANGE_FADE_FRAMES);
        timer--;
    }
}

void Room::update() {
    nav.update();
    for (int i = 0; i < spriteCount; i++) {
        sprites[i]->update(true);
    }
}

void Room::push() {
    globalPlayer->spriteManager.push();
    bg.free_();
    for (int i = 0; i < spriteCount; i++) {
        sprites[i]->spriteManager.push();
    }
}

void Room::pop() {
    char buffer[100];
    bg.loadPath(roomData.roomBg);
    globalPlayer->spriteManager.pop();

    int bgLoad = Engine::loadBgExtendedMain(bg, 512 / 8);
    if (bgLoad != 0) {
        sprintf(buffer, "Error loading room bg: %d", bgLoad);
        nocashMessage(buffer);
    }
    for (int i = 0; i < spriteCount; i++) {
        sprites[i]->spriteManager.pop();
    }
}

Room* globalRoom = nullptr;
