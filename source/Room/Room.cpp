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
#include "Room/InGameMenu.hpp"

Room::Room(int roomId) : _roomId(roomId) {
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

    _bg.loadPath(_roomData.roomBg);

    int bgLoad = _bg.loadBgExtendedMain(512 / 8);
    if (bgLoad != 0) {
        sprintf(buffer, "Error loading room bg: %d", bgLoad);
        nocashMessage(buffer);
    }

    if (_roomData.musicBg[0] != 0) {
        bool musicChange = Audio::cBGMusic.getFilename() == nullptr;
        if (!musicChange)
            musicChange = strcmp(_roomData.musicBg, Audio::cBGMusic.getFilename()) != 0;
        if (musicChange) {
            Audio::playBGMusic(_roomData.musicBg, true);
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
        fread(&_roomData.lengthBytes, 4, 1, f);
        long endPos = ftell(f) + _roomData.lengthBytes;
        fread(&_roomData.conditionCount, 1, 1, f);

        valid = true;
        for (int j = 0; j < _roomData.conditionCount && valid; j++) {
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

    fread(_roomData.roomBg, bgPathLen + 1, 1, f);
    _roomData.roomBg[bgPathLen] = 0;

    int musicPathLen = str_len_file(f, 0);
    if (musicPathLen == -1)
        return 5;

    fread(_roomData.musicBg, musicPathLen + 1, 1, f);
    _roomData.musicBg[musicPathLen] = 0;

    fread(&_spawnX, 2, 1, f);
    fread(&_spawnY, 2, 1, f);

    fread(&_roomData.roomExits.exitCount, 1, 1, f);
    _roomData.roomExits.roomExits = new ROOMExit[_roomData.roomExits.exitCount];
    ROOMExit* roomExits = _roomData.roomExits.roomExits;

    _rectExitCount = 0;
    for (int i = 0; i < _roomData.roomExits.exitCount; i++) {
        fread(&roomExits[i].exitType, 1, 1, f);
        fread(&roomExits[i].roomId, 2, 1, f);
        fread(&roomExits[i].spawnX, 1, 2, f);
        fread(&roomExits[i].spawnY, 1, 2, f);
        switch (roomExits[i].exitType) {
            case 0:
                fread(&roomExits[i].side, 1, 1, f);
                switch (roomExits[i].side) {
                    case 0:
                        _exitTop = &roomExits[i];
                        break;
                    case 1:
                        _exitBtm = &roomExits[i];
                        break;
                    case 2:
                        _exitLeft = &roomExits[i];
                        break;
                    case 3:
                        _exitRight = &roomExits[i];
                        break;
                    default:
                        break;
                }
                break;
            case 1:
                _rectExitCount++;
                fread(&roomExits[i].x, 2, 1, f);
                fread(&roomExits[i].y, 2, 1, f);
                fread(&roomExits[i].w, 2, 1, f);
                fread(&roomExits[i].h, 2, 1, f);
                break;
            default:
                break;
        }
    }

    _rectExits = new ROOMExit*[_rectExitCount];
    for (int i = 0, j = 0; i < _roomData.roomExits.exitCount; i++) {
        if (roomExits[i].exitType != 1)
            continue;
        _rectExits[j++] = &roomExits[i];
    }

    fread(&_textureCount, 1, 1, f);
    _textures = new Engine::Texture*[_textureCount];
    char path[100];
    for (int i = 0; i < _textureCount; i++){
        _textures[i] = new Engine::Texture;

        int sprPathLen = str_len_file(f, 0);
        if (sprPathLen == -1)
            return 5;
        fread(path, sprPathLen + 1, 1, f);

        _textures[i]->loadPath(path);
    }

    fread(&_spriteCount, 1, 1, f);
    _roomData.roomSprites.roomSprites = new ROOMSprite[_spriteCount];
    ROOMSprite* roomSprites = _roomData.roomSprites.roomSprites;

    for (int i = 0; i < _spriteCount; i++) {
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

    fread(&_roomData.roomColliders.colliderCount, 2, 1, f);
    _roomData.roomColliders.roomColliders = new ROOMCollider[_roomData.roomColliders.colliderCount];
    ROOMCollider* roomColliders = _roomData.roomColliders.roomColliders;

    for (int i = 0; i < _roomData.roomColliders.colliderCount; i++) {
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
    _bg.free_();
    delete[] _roomData.roomExits.roomExits;
    _roomData.roomExits.roomExits = nullptr;
    for (int i = 0; i < _spriteCount; i++) {
        _sprites[i]->free_();
        delete _sprites[i];
    }
    for (int i = 0; i < _textureCount; i++) {
        delete _textures[i];
    }
    delete[] _textures;
    for (int i = 0; i < _roomData.roomSprites.spriteCount; i++) {
        delete[] _roomData.roomSprites.roomSprites[i].animation;
        _roomData.roomSprites.roomSprites[i].animation = nullptr;
        delete[] _roomData.roomSprites.roomSprites[i].closeAnim;
        _roomData.roomSprites.roomSprites[i].closeAnim = nullptr;
    }
    delete[] _roomData.roomSprites.roomSprites;
    _roomData.roomSprites.roomSprites = nullptr;
    delete[] _sprites;
    _sprites = nullptr;

    delete[] _roomData.roomColliders.roomColliders;
    _roomData.roomColliders.roomColliders = nullptr;
    _bg.free_();
}

void Room::loadSprites() {
    _sprites = new ManagedSprite*[_spriteCount];
    for (int i = 0; i < _spriteCount; i++) {
        _sprites[i] = new ManagedSprite(Engine::Allocated3D);
        _sprites[i]->load(&_roomData.roomSprites.roomSprites[i], _textureCount, _textures);
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
    for (int i = 0; i < _spriteCount; i++) {
        _sprites[i]->draw(true);
    }
}

void loadNewRoom(int roomId, s32 spawnX, s32 spawnY) {
    int timer = kRoomChangeFadeFrames;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(1, (-16 * (kRoomChangeFadeFrames - timer)) / kRoomChangeFadeFrames);
        timer--;
    }

    globalRoom->free_();

    for (int i = 210; i <= 219; i++) {
        globalSave.flags[i] = 0; // clear room specific flags
    }

    delete globalRoom;
    globalRoom = new Room(roomId);
    globalPlayer->_playerSpr._wx = spawnX << 8;
    globalPlayer->_playerSpr._wy = spawnY << 8;

    if (globalCutscene != nullptr) {
        // Cutscenes are confined to rooms
        delete globalCutscene;
        globalCutscene = nullptr;
        globalInGameMenu.show(false);
        globalPlayer->_playerControl = true;
        globalCamera._manual = false;
    }
    globalCamera.updatePosition(true);
    globalPlayer->draw();
    globalRoom->draw();

    timer = kRoomChangeFadeFrames;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(1, (-16 * timer) / kRoomChangeFadeFrames);
        timer--;
    }
}

void Room::update() {
    _nav.update();
    for (int i = 0; i < _spriteCount; i++) {
        _sprites[i]->update(true);
    }
}

void Room::push() {
    globalPlayer->_playerSpr.push();
    _bg.free_();
    for (int i = 0; i < _spriteCount; i++) {
        _sprites[i]->_spr.push();
    }
}

void Room::pop() {
    char buffer[100];
    _bg.loadPath(_roomData.roomBg);
    globalPlayer->_playerSpr.pop();

    int bgLoad = _bg.loadBgExtendedMain(512 / 8);
    if (bgLoad != 0) {
        sprintf(buffer, "Error loading room bg: %d", bgLoad);
        nocashMessage(buffer);
    }
    for (int i = 0; i < _spriteCount; i++) {
        _sprites[i]->_spr.pop();
    }
}

Room* globalRoom = nullptr;
