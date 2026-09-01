import enum


class SpriteIDs(enum.IntEnum):
    FLOWEY_ROOM = 100
    TORIEL_ROOM = 101
    DUMMY_ROOM = 102
    SPR_EXC_ROOM = 103
    TORIEL_HANDHOLD_ROOM = 104


ROOM_SPRITE_NAMES_TO_SPRITE_IDS = {
    "flowey_room": SpriteIDs.FLOWEY_ROOM,
    "toriel_room": SpriteIDs.TORIEL_ROOM,
    "dummy_room": SpriteIDs.DUMMY_ROOM
}
