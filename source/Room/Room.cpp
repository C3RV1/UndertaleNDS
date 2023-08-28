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
    _roomId = roomId;
    std::string buffer = "nitro:/data/rooms/room" + std::to_string(roomId) + ".room";
    FILE* f = fopen(buffer.c_str(), "rb");
    if (f == nullptr) {
        buffer = "Error opening room " + std::to_string(roomId);
        Engine::throw_(buffer);
    }
    loadRoom(f);
    fclose(f);

    _bg.loadPath(_roomData.roomBg);

    int bgLoad = _bg.loadBgExtendedMain(512 / 8);
    if (bgLoad != 0) {
        buffer = "Error loading room bg. Error Code: " + std::to_string(bgLoad);
        nocashMessage(buffer.c_str());
    }

    if (!_roomData.musicBg.empty()) {
        bool musicChange = _roomData.musicBg != Audio::cBGMusic.getFilename();
        if (musicChange) {
            Audio::playBGMusic(_roomData.musicBg, true);
        }
    } else {
        Audio::stopBGMusic();
    }

    loadSprites();
}

void Room::loadRoom(FILE *f) {
    ROOMFile roomFile;

    fread(roomFile.header.header, 4, 1, f);
    char expectedHeader[4] = {'R', 'O', 'O', 'M'};

    if (memcmp(expectedHeader, roomFile.header.header, 4) != 0) {
        std::string buffer = "Error loading room #r" + std::to_string(_roomId) +
                "#x: Invalid header.";
        Engine::throw_(buffer);
    }

    fread(&roomFile.header.fileSize, 4, 1, f);
    long pos = ftell(f);
    fseek(f, 0, SEEK_END);
    u32 size = ftell(f);
    fseek(f, pos, SEEK_SET);

    if (roomFile.header.fileSize != size) {
        std::string buffer = "Error loading spr #r" + std::to_string(_roomId) +
                "#x: File size doesn't match (expected: " + std::to_string(roomFile.header.fileSize) +
                ", actual: " + std::to_string(size) + ")";
        Engine::throw_(buffer);
    }

    fread(&roomFile.header.version, 4, 1, f);
    if (roomFile.header.version != 8) {
        std::string buffer = "Error loading room #r" + std::to_string(_roomId) +
                "#x: Invalid version (expected: 8, actual: "
                + std::to_string(roomFile.header.version) + ")";
        Engine::throw_(buffer);
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
    if (!valid) {  // no valid room part found
        std::string buffer = "Error loading room #r" + std::to_string(_roomId) +
                             "#x: No valid room part found.";
        Engine::throw_(buffer);
    }

    int bgPathLen = str_len_file(f, 0);

    _roomData.roomBg.resize(bgPathLen);
    fread(&_roomData.roomBg[0], bgPathLen, 1, f);
    fseek(f, 1, SEEK_CUR);

    int musicPathLen = str_len_file(f, 0);

    _roomData.musicBg.resize(musicPathLen);
    fread(&_roomData.musicBg[0], musicPathLen, 1, f);
    fseek(f, 1, SEEK_CUR);

    fread(&_spawnX, 2, 1, f);
    fread(&_spawnY, 2, 1, f);

    u8 exitCount;
    fread(&exitCount, 1, 1, f);
    _roomData.roomExits.roomExits.resize(exitCount);
    auto & roomExits = _roomData.roomExits.roomExits;

    _rectExitCount = 0;
    for (int i = 0; i < exitCount; i++) {
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

    _rectExits.resize(_rectExitCount);
    for (int i = 0, j = 0; i < exitCount; i++) {
        if (roomExits[i].exitType != 1)
            continue;
        _rectExits[j++] = &roomExits[i];
    }

    u8 textureCount;
    fread(&textureCount, 1, 1, f);
    _textures.resize(textureCount);
    std::string path;
    path.reserve(50);
    for (int i = 0; i < textureCount; i++){
        _textures[i] = std::make_shared<Engine::Texture>();

        int sprPathLen = str_len_file(f, 0);
        path.resize(sprPathLen);
        fread(&path[0], sprPathLen, 1, f);
        fseek(f, 1, SEEK_CUR);

        _textures[i]->loadPath(path);
    }

    u8 spriteCount;
    fread(&spriteCount, 1, 1, f);
    _roomData.roomSprites.roomSprites.resize(spriteCount);
    auto & roomSprites = _roomData.roomSprites.roomSprites;

    for (int i = 0; i < spriteCount; i++) {
        fread(&roomSprites[i].textureId, 1, 1, f);
        fread(&roomSprites[i].x, 2, 1, f);
        fread(&roomSprites[i].y, 2, 1, f);
        int animLen = str_len_file(f, 0);
        roomSprites[i].animation.resize(animLen);
        fread(&roomSprites[i].animation[0], animLen, 1, f);
        fseek(f, 1, SEEK_CUR);
        fread(&roomSprites[i].interactAction, 1, 1, f);
        if (roomSprites[i].interactAction == 1) {
            fread(&roomSprites[i].cutsceneId, 2, 1, f);
        } else if (roomSprites[i].interactAction == 2) {
            fread(&roomSprites[i].distance, 2, 1, f);
            animLen = str_len_file(f, 0);
            roomSprites[i].closeAnim.resize(animLen);
            fread(&roomSprites[i].closeAnim[0], animLen, 1, f);
            fseek(f, 1, SEEK_CUR);
        }
    }

    u16 colliderCount;
    fread(&colliderCount, 2, 1, f);
    _roomData.roomColliders.roomColliders.resize(colliderCount);
    auto & roomColliders = _roomData.roomColliders.roomColliders;

    for (int i = 0; i < colliderCount; i++) {
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
}

void Room::free_() {

}

void Room::loadSprites() {
    _sprites.reserve(_roomData.roomSprites.roomSprites.size());
    for (auto const & roomSprite : _roomData.roomSprites.roomSprites) {
        auto sprite = std::make_unique<ManagedSprite>(Engine::Allocated3D);
        sprite->load(roomSprite, _textures);
        _sprites.push_back(std::move(sprite));
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
    for (const auto & _sprite : _sprites) {
        _sprite->draw(true);
    }
}

void loadNewRoom(int roomId, s32 spawnX, s32 spawnY) {
    int timer = kRoomChangeFadeFrames;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(1, (-16 * (kRoomChangeFadeFrames - timer)) / kRoomChangeFadeFrames);
        timer--;
    }

    for (int i = 210; i <= 219; i++) {
        globalSave.flags[i] = 0; // clear room specific flags
    }

    globalRoom = std::make_unique<Room>(roomId);
    globalPlayer->_playerSpr._wx = spawnX << 8;
    globalPlayer->_playerSpr._wy = spawnY << 8;

    if (globalCutscene != nullptr) {
        // Cutscenes are confined to rooms
        globalCutscene = nullptr;
        globalInGameMenu.show(false);
        globalPlayer->setPlayerControl(true);
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
    for (const auto & _sprite : _sprites) {
        _sprite->update(true);
    }
}

void Room::push() {
    globalPlayer->_playerSpr.push();
    for (const auto & _sprite : _sprites) {
        _sprite->_spr.push();
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
    for (const auto & _sprite : _sprites) {
        _sprite->_spr.pop();
    }
}

std::unique_ptr<Room> globalRoom = nullptr;
