import json
import pygame
import os
import sys

def draw_image_with_cam(screen: pygame.Surface, image: pygame.Surface, pos: list, cam_pos: list, cam_scale: float):
    if cam_scale != 1:
        size = list(image.get_size())
        size[0] = int(size[0] * cam_scale)
        size[1] = int(size[1] * cam_scale)
        image = pygame.transform.scale(image, size)

    new_pos = pos.copy()
    new_pos[0] -= cam_pos[0]
    new_pos[1] -= cam_pos[1]
    new_pos[0] *= cam_scale
    new_pos[1] *= cam_scale

    screen.blit(image, tuple(new_pos))


def rect_to_positive_rect(rect: list):
    if rect[2] < 0:
        rect[0] += rect[2]
        rect[2] = -rect[2]
    if rect[3] < 0:
        rect[1] += rect[3]
        rect[3] = -rect[3]
    return rect


def draw_rect_with_cam(screen: pygame.Surface, rect: list, color: str | list, cam_pos: list, cam_scale: float):
    rect = rect.copy()
    rect = rect_to_positive_rect(rect)
    rect[0] -= cam_pos[0]
    rect[1] -= cam_pos[1]
    rect[0] *= cam_scale
    rect[1] *= cam_scale
    rect[2] *= cam_scale
    rect[3] *= cam_scale

    width = 1 if cam_scale <= 1 else cam_scale
    pygame.draw.rect(screen, rect=tuple(rect), color=color, width=int(width))


def draw_collider_with_cam(screen: pygame.Surface, collider: dict, color: str | list, cam_pos: list, cam_scale: float):
    collider_pos = [collider["x"], collider["y"], collider["w"], collider["h"]]
    draw_rect_with_cam(screen, collider_pos, color, cam_pos, cam_scale)


def screen_crd_to_world(pos: list, cam_pos: list, cam_scale: float, max_size: list | tuple):
    pos[0] /= cam_scale
    pos[1] /= cam_scale
    pos[0] += cam_pos[0]
    pos[1] += cam_pos[1]
    pos[0] = min(max(int(pos[0]), 0), max_size[0] - 1)
    pos[1] = min(max(int(pos[1]), 0), max_size[1] - 1)
    return pos
    

def main():
    room_id = int(sys.argv[-1])

    room_img_path = f"bg/rooms/room{room_id}.png" 
    room_json_path = f"rooms/room{room_id}.json" 

    if not os.path.isfile(room_img_path):
        print(f"Couldn't find image of room {room_id}")
        return

    if os.path.isfile(room_json_path):
        with open(room_json_path, "r") as f:
            room_json: dict = json.load(f)
    else:
        room_json: dict = {"parts": [{"colliders": []}]}

    colliders: list = room_json["parts"][0]["colliders"]

    pygame.init()
    screen = pygame.display.set_mode((1280, 720))

    bg_img = pygame.image.load(room_img_path).convert()
    bg_img_size = bg_img.get_size()

    clock = pygame.time.Clock()
    running = True

    selected_collider = 0
    changed_selected = True
    moved_mouse = False

    drawing_collider = False
    pos_start = [0, 0]
    pos_mouse = [0, 0]
    camera_position = [0.0, 0.0]
    camera_zoom = 4
    cam_speed = 300
    dt = 0

    while running:
        screen.fill("black")

        for event in pygame.event.get():
            if event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:
                    pos = screen_crd_to_world(list(event.pos), camera_position, camera_zoom, bg_img_size)
                    if not drawing_collider:
                        drawing_collider = True
                        pos_start = pos
                    else:
                        rect = rect_to_positive_rect([pos_start[0], pos_start[1], pos[0] - pos_start[0], pos[1] - pos_start[1]])
                        collider = {
                                "x": rect[0],
                                "y": rect[1],
                                "w": rect[2] + 1,
                                "h": rect[3] + 1
                                }
                        colliders.append(collider)
                        drawing_collider = False
                elif event.button == 3:
                    drawing_collider = False
            elif event.type == pygame.MOUSEWHEEL:
                if event.y > 0:
                    camera_zoom *= 2
                elif event.y < 0:
                    camera_zoom /= 2
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    if drawing_collider:
                        drawing_collider = False
                elif event.key == pygame.K_p:
                    if selected_collider < len(colliders):
                        colliders.pop(selected_collider)
                        selected_collider -= 1
                        changed_selected = True
                elif event.key == pygame.K_n:
                    selected_collider += 1
                    if len(colliders) > 0:
                        selected_collider %= len(colliders)
                    changed_selected = True
                elif event.key == pygame.K_m:
                    selected_collider -= 1
                    changed_selected = True
                    if len(colliders) > 0:
                        selected_collider %= len(colliders)
                elif event.key == pygame.K_g:
                    print(f"Saving to {room_json_path}")
                    with open(room_json_path, "w") as f:
                        json.dump(room_json, f, indent=2)
            elif event.type == pygame.QUIT:
                running = False

        pos = screen_crd_to_world(list(pygame.mouse.get_pos()), camera_position, camera_zoom, bg_img_size)
        if pos != pos_mouse:
            moved_mouse = True
        pos_mouse = pos

        if selected_collider >= len(colliders):
            selected_collider = len(colliders) - 1
        if selected_collider < 0:
            selected_collider = 0
        if changed_selected:
            print(f"Selected collider: {selected_collider}")

        keys = pygame.key.get_pressed()
        if keys[pygame.K_d]:
            camera_position[0] += (cam_speed / camera_zoom) * dt
        if keys[pygame.K_a]:
            camera_position[0] -= (cam_speed / camera_zoom) * dt
        if keys[pygame.K_s]:
            camera_position[1] += (cam_speed / camera_zoom) * dt
        if keys[pygame.K_w]:
            camera_position[1] -= (cam_speed / camera_zoom) * dt

        draw_image_with_cam(screen, bg_img, [0, 0], camera_position, camera_zoom)
        draw_rect_with_cam(screen, [-1, -1, bg_img_size[0] + 2, bg_img_size[1] + 2], "grey", camera_position, camera_zoom)
        
        for i, collider in enumerate(colliders):
            color = "green" if i != selected_collider else "red"
            draw_collider_with_cam(screen, collider, color, camera_position, camera_zoom)

        if drawing_collider:
            coll_rect = [
                    pos_start[0], pos_start[1],
                    pos_mouse[0] - pos_start[0],
                    pos_mouse[1] - pos_start[1]
                    ]
            coll_rect = rect_to_positive_rect(coll_rect)
            coll_rect[2] += 1
            coll_rect[3] += 1
            if moved_mouse:
                print(f"Creating collider: {coll_rect}")
            draw_rect_with_cam(screen, coll_rect, "blue", camera_position, camera_zoom)
        else:
            if moved_mouse:
                print(f"Mouse position: {pos_mouse}")
            coll_rect = [pos_mouse[0], pos_mouse[1], 1, 1]
            draw_rect_with_cam(screen, coll_rect, "blue", camera_position, camera_zoom)

        pygame.display.flip()
        dt = clock.tick(60) / 1000

        moved_mouse = False
        changed_selected = False

if __name__ == "__main__":
    main()
