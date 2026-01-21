








import sys
import math
import random
import time
import json
import numpy as np
import os
import pyperclip
import copy
from datetime import datetime
import pygame 

prin_RED = '\033[91m'
prin_GREEN = '\033[92m'
prin_BLUE = '\033[94m'
prin_RESET = '\033[0m'

text_font = pygame.font.SysFont("Arial", 20)
text_font_big = pygame.font.SysFont("Arial", 50)

display_size = None
window_size = None
tps = None
fps = None
vol = 0
running = True
CHUNK_SIZE = 1
WORLD_WIDTH = 1
WORLD_HIGHT = 1
CHUNKS_ON_X = WORLD_WIDTH // CHUNK_SIZE
CHUNKS_ON_Y = WORLD_HIGHT // CHUNK_SIZE
text_fonts = (text_font,text_font_big)
loader = None


class Loader:
    def __init__(self,texture_map_path,GF_map_path):
        self.texture_map = {}
        self.file_map = {}
        self.seed_obj = {}
        self.load_texture_map(texture_map_path)
        self.load_file_map(GF_map_path)
        
    def init_comon_textures(self,texture_map):
        all_keys = texture_map.keys()
        TM = texture_map
        for key in all_keys:
            val = TM[key]
            if val["type"] == "d" or val["type"] == "D":
                val = pygame.image.load(key["content"]).convert_alpha()
                TM[key] = val
            elif val["type"] == "r" or val["type"] == "R":
                val = pygame.image.load(val["content"]).convert_alpha()
                TM[key] = val
            elif val["type"] == "s" or val["type"] == "S":
                val = self._loade_type_i(val,key)
                TM[key] = val
            else:
                val = pygame.image.load(key).convert_alpha()
                TM[key] = val
        return TM

    def init_game_files(self,file_map):
        all_keys = file_map.keys()
        TM = file_map
        for key in all_keys:
            val = TM[key]
            if val["type"] == "d" or val["type"] == "D":
                with open(key,"r") as file:
                    val = json.load(file)
                TM[key] = val
            elif val["type"] == "r" or val["type"] == "R":
                with open(key,"r") as file:
                    val = json.load(file)
                TM[key] = val
            elif val["type"] == "s" or val["type"] == "S":
                TM[key] = self._loade_type(val,key)
            else:
                with open(key,"r") as file:
                    val = json.load(file)
                TM[key] = val
        return TM

    def _loade_type_i(self,val,og_dir):
        all_objs = []
        for i in range(len(val["content"])):
            all_objs.append(pygame.image.load(og_dir+val["content"][i]).convert_alpha())
            #print(f"loading image in a catagory {og_dir+val["content"][i]}")
        val["content"] = all_objs
        return val

    def _loade_type(self,val,og_dir):
        all_objs = []
        for i in range(len(val["content"])):
            with open(og_dir+val["content"][i],"r") as f:
                data = json.load(f)
            all_objs.append(data)
        val["content"] = all_objs
        return val

    def load_texture_map(self,map_path):
        with open(map_path,"r") as file:
            texture_map = json.load(file)
        self.texture_map = self.init_comon_textures(texture_map)

    def load_file_map(self,map_path):
        with open(map_path,"r") as file:
            file_map = json.load(file)
        self.file_map = self.init_game_files(file_map)

    def image(self,path):
        try:
            return self.texture_map[path]["content"]
        except KeyError:
            return pygame.image.load(path).convert_alpha()

    def data(self,file_path):
        try:
            return self.file_map[file_path]["content"]
        except KeyError:
            with open(file_path,"r") as file:
                return json.load(file)

    def warp_image(self,image,sizex,sizey,angle):
        image1 = pygame.transform.scale(image,(sizex,sizey))
        image2 = pygame.transform.rotate(image1,angle)
        return image2
    def rotate_image(self,image,angle):
        image2 = pygame.transform.rotate(image,angle)
        return image2
    def scale_image(self,image,sizex,sizey):
        image1 = pygame.transform.scale(image,(sizex,sizey))
        return image1

    def play_sound(self,file_path, volume=0.5,loops=1):
        try:
            sound = pygame.Sound(file_path)
            sound.set_volume(volume)
            sound.play(loops)
            return 1
        except Exception as e:
            print(f"{prin_RED}!!-error loading sound ->> {e} -!!{prin_RESET}")
            return -1

class Sprite:
    def __init__(self,bace_path,name,tps,L=None,use_loader=True):
        # note these classes requier some varyable to exsist such as prin_RESET,prin_RED .....
        self.bace_path = bace_path
        self.name = name
        self.images = []
        self.og_images = []
        self.use_loader = use_loader
        # L stands for loader
        if use_loader and L != None: # this inits the self.data
            self.data = L.data(f"{bace_path}/sprite_data.json")  
        else:
            with open(f"{bace_path}/sprite_data.json","r") as f:
                self.data = json.load(f)

        self.curent_frame = -1
        self.itaration_offset = 0
        self.alive = True
        self.set_speed(tps,self.data["frames_per_second"])
        self.rand_stager = random.randint(0,tps)
        self._loade_all_images(L)

    def set_speed(self,tps,fps):
        self.fps = fps
        self.tpf = self.fps * tps

    def load_img(self,path,L=None):
        try:
            return pygame.image.load(path).convert_alpha()
        except Exception as e:
            print(f"{prin_RED}!!-- error loading {path} with the error of {e} reverting to: error image --!!{prin_RESET}")
            return pygame.image.load(f"{self.bace_path}/error.png").convert_alpha()

    def _loade_all_images(self,L=None):
        self.images = []
        if L == None:
            for i in range(self.data["all_frames"]):
                load_dir = f"{self.bace_path}/images/img_{i}.{self.data["extention_type"]}"
                img = self.load_img(load_dir,L)
                self.images.append(img)
        else:
            self.images = L.image(self.bace_path)
        self.og_images = self.images

    def scale_all(self,size):
        for img in self.images:
            index = self.images.index(img)
            img = pygame.transform.scale(img,size)
            self.images[index] = img

    def rotate_all(self,angle):
        for img in self.images:
            index = self.images.index(img)
            img = pygame.transform.rotate(img,angle)
            self.images[index] = img

    def get_image(self,iteration):
        if self.curent_frame >= self.data["all_frames"]-1:
            self.alive = self.data["is_repete"]
        if self.alive:                               
            self.curent_frame = (self.curent_frame + self.tpf) % self.data["all_frames"]
        #print(f"{self.curent_frame}  fps>> {self.fps} loops >> {iteration}")
        return self.images[int(self.curent_frame)]


class GameCamera:
    def __init__(self, display_surface, chunk_size,WORLD_WIDTH,WORLD_HIGHT):
        self.display_surface = display_surface
        self.chunk_size = chunk_size
        self.WORLD_WIDTH = WORLD_WIDTH
        self.WORLD_HIGHT = WORLD_HIGHT
        self.HUNKS_ON_X = WORLD_WIDTH // self.chunk_size
        self.CHUNKS_ON_Y = WORLD_HIGHT // self.chunk_size
        self.tiles = {}
        self.offset_x = 0
        self.offset_y = 0
        self.zoom = 1.0

    def get_chunk(self,cx, cy):
        #return chunk at (cx, cy), create if it doesnt exist yet
        if (cx, cy) not in self.tiles:
            self.tiles[(cx, cy)] = Chunk(cx, cy, self.chunk_size)
            print(f"{prin_GREEN}Created chunk {cx},{cy}{prin_RESET}")
        return self.tiles[(cx, cy)]

    def set_zoom(self, level):
        self.zoom = max(0.1, level)

    def camera_render(self, target_x, target_y, zoom=1.0):
        self.set_zoom(zoom)
        W, H = self.display_surface.get_size()
        z = self.zoom

        self.offset_x = -target_x * z + W // 2
        self.offset_y = -target_y * z + H // 2

        world_left   = -self.offset_x / z
        world_top    = -self.offset_y / z
        world_right  = world_left + W / z
        world_bottom = world_top + H / z

        min_cx = max(0, int(math.floor(world_left / self.chunk_size)))
        max_cx = min(self.CHUNKS_ON_X - 1, int(math.floor(world_right / self.chunk_size)))
        min_cy = max(0, int(math.floor(world_top / self.chunk_size)))
        max_cy = min(self.CHUNKS_ON_Y - 1, int(math.floor(world_bottom / self.chunk_size)))

        for cx in range(min_cx, max_cx + 1):
            for cy in range(min_cy, max_cy + 1):
                chunk = self.get_chunk(cx, cy)
                surf = chunk.scaled_surface(z)
                screen_x = cx * self.chunk_size * z + self.offset_x
                screen_y = cy * self.chunk_size * z + self.offset_y
                self.display_surface.blit(surf, (screen_x, screen_y))
                chunk.update_objs()
                
class Chunk:
    def __init__(self, cx, cy, chunk_size):
        self.cx = cx
        self.cy = cy
        self.size = chunk_size
        self.surf = pygame.Surface((chunk_size, chunk_size), flags=pygame.SRCALPHA)
        self.surf.fill((0,0,0))
        self._scaled = None
        self._last_zoom = None
        self.all_objs = []
        self.generate_terrain()

    def scaled_surface(self, zoom):
        if self._last_zoom != zoom or self._scaled is None:
            w = max(1, int(self.size * zoom))
            h = max(1, int(self.size * zoom))
            self._scaled = pygame.transform.scale(self.surf, (w, h))
            self._last_zoom = zoom
        return self._scaled

    def update_objs(self):
        for obj in self.all_objs:
            pass

    def generate_terrain(self):
        self.surf.blit(defalt_image, (0,0))

class UI_manager:
    def __init__(self,is_full_screen):
        self.layers = {}
        self.window = pygame.display.set_mode(window_size)
        self.display = pygame.Surface(display_size)
        if not pygame.display.is_fullscreen() and not is_full_screen:
            pygame.display.toggle_fullscreen()

        self.display_rect = self.display.get_rect(center=(self.window.get_width() // 2, self.window.get_height() // 2))
        self.window_width = self.window.get_width()
        self.window_height = self.window.get_height()
        self.scale = min(self.window_width / self.display.get_width(), self.window_height / self.display.get_height())
        self.new_size = (int(self.display.get_width() * self.scale), int(self.display.get_height() * self.scale))
        self.W_pos = ((self.window_width - self.new_size[0]) // 2, (self.window_height - self.new_size[1]) // 2)

        self.center_x = self.display.get_width() / 2
        self.center_y = self.display.get_height() / 2

    def get_mouse_pos(self):
        mx, my = pygame.mouse.get_pos()
        mouse_pos = (((mx - W_pos[0]) / scale),((my - W_pos[1]) / scale))



def init(self,texture_map_path,game_file_map_path,window_size,display_size,chunk_size,world_width,world_hight,TPS,FPS):
    pygame.init()
    display_size = display_size
    window_size = window_size
    tps = TPS
    fps = FPS
    vol = 0
    running = True
    CHUNK_SIZE = chunk_size
    WORLD_WIDTH = world_width
    WORLD_HIGHT = world_hight
    CHUNKS_ON_X = self._WORLD_WIDTH // self._CHUNK_SIZE
    CHUNKS_ON_Y = self._WORLD_HIGHT // self._CHUNK_SIZE
    text_fonts = (text_font,text_font_big)
    loader = Loader(texture_map_path,game_file_map_path)

if __name__ == "__main__":
    pass
