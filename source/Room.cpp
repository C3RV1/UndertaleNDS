//
// Created by cervi on 27/08/2022.
//

#include "Room.hpp"

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

    f = fopen(roomData.roomBg, "rb");
    if (f) {
        int bgLoad = bg.loadCBGF(f);
        if (bgLoad != 0) {
            sprintf(buffer, "Error loading room %d bg %s: %d", roomId,
                    roomData.roomBg, bgLoad);
            nocashMessage(buffer);
            fclose(f);
        }
    } else {
        sprintf(buffer, "Error opening room %d bg %s", roomId, roomData.roomBg);
        nocashMessage(buffer);
    }
    fclose(f);

    int bgLoad = Engine::loadBgExtendedMain(bg, 512 / 8);
    if (bgLoad != 0) {
        sprintf(buffer, "Error loading room bg: %d", bgLoad);
        nocashMessage(buffer);
    }

    if (roomData.musicBg[0] != 0) {
        if (strcmp(roomData.musicBg, BGM::globalWAV.getFilename()) != 0) {
            BGM::globalWAV.loadWAV(roomData.musicBg);
            BGM::globalWAV.setLoop(true);
            BGM::playWAV(BGM::globalWAV);
        }
    } else {
        BGM::stopWAV();
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
    uint32_t size = ftell(f);
    fseek(f, pos, SEEK_SET);

    if (roomFile.header.fileSize != size) {
        return 2;
    }

    fread(&roomFile.header.version, 4, 1, f);
    if (roomFile.header.version != 3) {
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

    int bgPathLen = strlen_file(f, 0);
    if (bgPathLen == -1)
        return 5;

    roomData.roomBg = (char*) malloc(bgPathLen + 1);
    fread(roomData.roomBg, bgPathLen + 1, 1, f);

    int musicPathLen = strlen_file(f, 0);
    if (musicPathLen == -1)
        return 5;

    roomData.musicBg = (char*) malloc(musicPathLen + 1);
    fread(roomData.musicBg, musicPathLen + 1, 1, f);

    fread(&roomData.roomExits.exitCount, 1, 1, f);
    roomData.roomExits.roomExits = (ROOMExit*) malloc(sizeof(ROOMExit) * roomData.roomExits.exitCount);
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
                nocashMessage("rect exit");
                fread(&roomExits[i].x, 2, 1, f);
                fread(&roomExits[i].y, 2, 1, f);
                fread(&roomExits[i].w, 2, 1, f);
                fread(&roomExits[i].h, 2, 1, f);
                break;
            default:
                break;
        }
    }

    rectExits = (ROOMExit**) malloc(sizeof(ROOMExit*) * rectExitCount);
    for (int i = 0, j = 0; i < roomData.roomExits.exitCount; i++) {
        if (roomExits[i].exitType != 1)
            continue;
        nocashMessage("rect exit copy");
        rectExits[j++] = &roomExits[i];
    }

    fread(&roomData.roomSprites.spriteCount, 1, 1, f);
    roomData.roomSprites.roomSprites = (ROOMSprite*) malloc(sizeof(ROOMSprite) * roomData.roomSprites.spriteCount);
    ROOMSprite* roomSprites = roomData.roomSprites.roomSprites;

    for (int i = 0; i < roomData.roomSprites.spriteCount; i++) {
        int sprPathLen = strlen_file(f, 0);
        if (sprPathLen == -1)
            return 5;
        roomSprites[i].spritePath = (char*) malloc(sprPathLen + 1);
        fread(roomSprites[i].spritePath, sprPathLen + 1, 1, f);
        fread(&roomSprites[i].x, 2, 1, f);
        fread(&roomSprites[i].y, 2, 1, f);
        fread(&roomSprites[i].layer, 2, 1, f);
        int animLen = strlen_file(f, 0);
        if (animLen == -1)
            return 5;
        roomSprites[i].animation = (char*) malloc(animLen + 1);
        fread(roomSprites[i].animation, animLen + 1, 1, f);
        fread(&roomSprites[i].canInteract, 1, 1, f);
        fread(&roomSprites[i].interactAction, 1, 1, f);
        if (roomSprites[i].interactAction == 1) {
            fread(&roomSprites[i].cutsceneId, 2, 1, f);
        }
    }

    fread(&roomData.roomColliders.colliderCount, 2, 1, f);
    roomData.roomColliders.roomColliders = (ROOMCollider*) malloc(sizeof(ROOMCollider) * roomData.roomColliders.colliderCount);
    ROOMCollider* roomColliders = roomData.roomColliders.roomColliders;

    for (int i = 0; i < roomData.roomColliders.colliderCount; i++) {
        fread(&roomColliders[i].x, 2, 1, f);
        fread(&roomColliders[i].y, 2, 1, f);
        fread(&roomColliders[i].w, 2, 1, f);
        fread(&roomColliders[i].h, 2, 1, f);
        fread(&roomColliders[i].colliderAction, 1, 1, f);
        if (roomColliders[i].colliderAction == 1) {
            fread(&roomColliders[i].cutsceneId, 2, 1, f);
        }
    }

    return 0;
}

void Room::free_() {
    free(roomData.roomBg);
    free(roomData.roomExits.roomExits);
    for (int i = 0; i < roomData.roomSprites.spriteCount; i++) {
        sprites[i].free_();
        free(roomData.roomSprites.roomSprites[i].spritePath);
        free(roomData.roomSprites.roomSprites[i].animation);
    }
    free(roomData.roomSprites.roomSprites);
    free(sprites);
    bg.free_();
}

void Room::loadSprites() {
    sprites = (RoomSprite*) malloc(sizeof(RoomSprite) * roomData.roomSprites.spriteCount);
    for (int i = 0; i < roomData.roomSprites.spriteCount; i++) {
        sprites[i].load(&roomData.roomSprites.roomSprites[i]);
    }
}

bool Room::evaluateCondition(FILE *f) {
    return true;
}

void Room::draw(Camera &cam) const {
    for (int i = 0; i < roomData.roomSprites.spriteCount; i++) {
        sprites[i].draw(cam);
    }
}

void loadNewRoom(Room*& room, Camera& cam, Player& player, int roomId) {
    int timer = ROOM_CHANGE_FADE_FRAMES;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(1, (-16 * (ROOM_CHANGE_FADE_FRAMES - timer)) / ROOM_CHANGE_FADE_FRAMES);
        timer--;
    }

    Room* newRoom = new Room(roomId);
    room->free_();
    delete room;
    room = newRoom;

    cam.updatePosition(*room, player, true);
    player.draw(cam);
    room->draw(cam);

    timer = ROOM_CHANGE_FADE_FRAMES;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(1, (-16 * timer) / ROOM_CHANGE_FADE_FRAMES);
        timer--;
    }
}
