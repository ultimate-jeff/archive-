import numpy as np
import pygame
import math
import time
import json
import os
import sys
import inspect
from skyport.core.paths import PathUtil as pu
from pathlib import Path

class Util:
    instanses = 0
    def __init__(self):
        Util.instanses += 1
        #print(Util.instanses)
        #self.ROOT_PATH = Path(__file__).resolve().parent.parent
        if Util.instanses <= 3:
            self.fp = self.e_fp
        else:
            self.fp = self.m_fp

    def m_fp(self, relative_fp):
        return relative_fp
    def e_fp(self, relative_fp):
        return pu.fp(relative_fp)

    def get_angle_and_dist(self,x1,y1,x,y):
        dx = x1 - x
        dy = y1 - y
        dist = math.hypot(dx, dy)
        angle = (math.degrees(math.atan2(dy, dx)) + 180) % 360
        return angle,dist

    def color_swap(self,surface: pygame.Surface, old_color: tuple, new_color: tuple) -> pygame.Surface:
        arr_rgb = pygame.surfarray.array3d(surface)
        arr_alpha = pygame.surfarray.array_alpha(surface)
        mask = np.all(arr_rgb == old_color, axis=-1)
        arr_rgb[mask] = new_color
        new_surface = pygame.Surface(surface.get_size(), pygame.SRCALPHA, 32)
        new_surface = new_surface.convert_alpha()
        pygame.surfarray.blit_array(new_surface, arr_rgb)
        pygame.surfarray.pixels_alpha(new_surface)[:] = arr_alpha
        return new_surface

    def pryoraty(self,a=None,b=None):
        if a != None:
            return a
        else:
            return b

    def sort_objects_by_attr(self,obj_list : list, attr_name : str, reverse=False):
        return sorted(obj_list, key=lambda x: getattr(x, attr_name), reverse=reverse)

class Loader:
    loader_instanses = 0
    lutil = Util()
    error_img=pygame.image.load(lutil.fp("assets/images/error.png")),
    error_sound=pygame.mixer.Sound(lutil.fp("assets/sounds/error.mp3"))
    def __init__(self,texture_map_path=None,GF_map_path=None,sound_map_path=None,loader_name=None,error_img=None,error_sound=None):
        self.lutil = Util()
        self.texture_map = {}
        self.file_map = {}
        self.sound_map = {}
        if texture_map_path != None:
            self.load_texture_map(texture_map_path)
        if GF_map_path != None:
            self.load_file_map(GF_map_path)
        if sound_map_path != None:
            self.load_sound_map(sound_map_path)
        # loader naming for debuging
        self.error_assets = {"img":self.lutil.pryoraty(error_img,Loader.error_img),"sound":self.lutil.pryoraty(error_sound,Loader.error_sound)}
        Loader.loader_instanses += 1
        self.loader_name = util.pryoraty(loader_name,str(Loader.loader_instanses))
        print(f"{prin_BLUE}++| inisalized Loader instans '{self.loader_name}' |++{prin_RESET}")

    def _error(self,Type,error,path):
        print(f"{prin_RED}!!-error loading {Type} : {path} ->> in loader {self.loader_name} with error >> {error} -!!{prin_RESET}")

    def _loade_img_catagory(self,val):
        paths = val["value"]
        images = []
        for path in paths:
            images.append(pygame.image.load(self.lutil.fp(path)).convert_alpha())
        val["value"] = images
        return val
    def init_comon_textures(self,texture_map):
        all_keys = texture_map.keys()
        TM = texture_map
        for key in all_keys:
            val = TM[key]
            Type = val["type"].lower()
            if Type == "d" or Type == "defalt":
                val["value"] = pygame.image.load(self.lutil.fp(key)).convert_alpha()
                TM[key] = val
            elif Type == "r" or Type == "replace":
                val["value"] = pygame.image.load(val["value"]).convert_alpha()
                TM[key] = val
            elif Type == "c" or Type == "catagory":
                val = self._loade_img_catagory(self.lutil.fp(val))
            else:
                val["value"] = pygame.image.load(self.lutil.fp(key)).convert_alpha()
                TM[key] = val
        return TM
    def load_texture_map(self,map_path):
        map_path = self.lutil.fp(map_path)
        with open(map_path,"r") as file:
            texture_map = json.load(file)
        self.texture_map = self.init_comon_textures(texture_map)

    def _loade_file_catagory(self,val):
        paths = val["value"]
        files = []
        for path in paths:
            with open(self.lutil.fp(path),"r") as file:
                files.append(json.load(file))
        val["value"] = files
        return val
    def init_game_files(self,file_map):
        all_keys = file_map.keys()
        TM = file_map
        for key in all_keys:
            val = TM[key]
            Type = val["type"].lower()
            if Type == "d" or Type == "defalt":
                with open(self.lutil.fp(key),"r") as file:
                    val["value"] = json.load(file)
                TM[key] = val
            elif Type == "r" or Type == "replace":
                with open(self.lutil.fp(val["value"]),"r") as file:
                    val["value"] = json.load(file)
                TM[key] = val
            elif Type == "c" or Type == "catagory":
                val = self._loade_file_catagory(val)
            else:
                with open(self.lutil.fp(key),"r") as file:
                    val["value"] = json.load(file)
                TM[key] = val
        return TM
    def load_file_map(self,map_path):
        map_path = self.lutil.fp(map_path)
        with open(map_path,"r") as file:
            file_map = json.load(file)
        self.file_map = self.init_game_files(file_map)

    def _loade_sound_catagory(self,val):
        file_paths = val["value"]
        sounds = []
        for file_path in file_paths:
            sounds.append(pygame.mixer.Sound(self.lutil.fp(file_path)))
        val["value"] = sounds
        return val
    def init_sound_map(self,sound_map):
        all_keys = sound_map.keys()
        TM = sound_map
        for key in all_keys:
            val = TM[key]
            Type = val["type"].lower()
            if Type == "d" or Type == "defalt":
                val["value"] = pygame.mixer.Sound(self.lutil.fp(key))
                TM[key] = val
            elif Type == "r" or Type == "replace":
                val["value"] = pygame.mixer.Sound(self.lutil.fp(val["value"]))
                TM[key] = val
            elif Type == "c" or Type == "catagory":
                val = self._loade_sound_catagory(val)
            else:
                val["value"] = pygame.mixer.Sound(self.lutil.fp(key))
                TM[key] = val
        return TM
    def load_sound_map(self,map_path):
        map_path = self.lutil.fp(map_path)
        with open(map_path,"r") as file:
            sound_map = json.load(file)
        self.sound_map = self.init_sound_map(sound_map)

    def image(self,path):
        try:
            return self.texture_map[path]["value"]
        except KeyError:
            try:
                return pygame.image.load(self.lutil.fp(path)).convert_alpha()
            except Exception as e:
                self._error("image",e,path)
                return self.error_assets["img"]
    def data(self,file_path):
        try:
            return self.file_map[file_path]["value"]
        except KeyError:
            try:
                with open(self.lutil.fp(file_path),"r") as file:
                    return json.load(file)
            except Exception as e:
                self._error("data",e,file_path)
                return -1
    def sound(self,file_path):
        try:
            return self.sound_map[file_path]["value"]
        except KeyError:
            try:
                return pygame.mixer.Sound(self.lutil.fp(file_path))
            except Exception as e:
                self._error("sound",e,file_path)
                return self.error_assets["sound"]

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

    def play_sound(self,file_path, volume=0.5,loops=0):
        try:
            sound = self.sound(file_path)
            sound.set_volume(volume)
            sound.play(loops)
            return 1
        except Exception as e:
            print(f" error >> {e} from loader >> {self.loader_name} ")

prin_RED = '\033[91m'
prin_GREEN = '\033[92m'
prin_BLUE = '\033[94m'
prin_RESET = '\033[0m'
util = Util()
loader = Loader(
    "assets/loader/texture_maps/engine_textures.json",
    "assets/loader/game_file_maps/engine_assets.json",
    "assets/loader/sound_maps/engine_sounds.json",
    loader_name="engine_loader"
)
class Delta_timer:
    def __init__(self):
        # Using perf_counter for highest precision
        self.prev_time = time.perf_counter()
        self.dt = 0.0

    def update(self):
        now = time.perf_counter()
        self.dt = now - self.prev_time
        self.prev_time = now
        return self.dt

class Sprite:
    instanses = 0
    ld = loader
    def __init__(self,file_path,game_fps):
        self.dt = Delta_timer()
        Sprite.instanses += 1
        self.bace_path = file_path # bace path would be something like assets/sprites/test1.sptite
        self.data = Sprite.ld.data(f"{self.bace_path}/settings.json")
        self.game_fps = game_fps
        self.fps = self.data["fps"]
        self.total_frames = self.data["total_frames"]
        self.img_dt = self.data["img_dt"]
        self._imgs = []
        self.scaled_imgs = []
        self.elapsed_time = 0.0
        self._last_zoom = None
        self.init_imgs(file_path)

    def get_frame_index(self, dt):
        self.elapsed_time += dt
        raw_frame = int(self.elapsed_time * self.fps)
        index = raw_frame % self.total_frames
        return index

    def get_surf(self,zoom,angle):
        if self._last_zoom != zoom:
            self.scale_all(zoom)
        ind = self.get_frame_index(self.dt.update())
        surf = self.scaled_imgs[ind]
        surf = pygame.transform.rotate(surf,angle)

    def scale_all(self,zoom):
        surf = pygame.surface.Surface((10,10))
        surf.get_size()
        self.scaled_imgs.clear()
        for i in self._imgs:
            surf = i
            surf_size = i.get_size()
            surf_size[0] *= zoom
            surf_size[1] *= zoom
            surf = pygame.transform.scale(surf,surf_size)
            self.scaled_imgs.append(surf)

    def init_imgs(self,FP):
        self._imgs = Sprite.ld.image(FP)

    def manual_init_imgs(self,FP):
        for i in range(self.total_frames):
            fp = f"img_{i}{self.img_dt}"
            full_fp = f"{self.bace_path}/images/{fp}"
            self._imgs.append(Sprite.ld.image(full_fp))

class r_obj:
    instanses = 0
    loader = loader
    def __init__(self,x,y,sx,sy,angle,zoom,texture_fp=None,hitbox_rect=None,render_type="chunk"):
        self.x,self.y,self.sx,self.sy = x,y,sx,sy
        self.angle = angle
        self.last_zoom = None
        self.last_angle = None
        self.texture_path = texture_fp
        self.render_type = render_type
        self.init_render_type(texture_fp)
        self.OG_IMAGE = self.get_df_img(texture_fp)
        self.surf = self.get_surf(zoom)
        r_obj.instanses += 1
        self.id = r_obj.instanses
        if hitbox_rect != None:
            self.hitbox_rect = hitbox_rect
        else:
            self.hitbox_rect = pygame.Rect(x,y,sx,sy)

    def init_render_type(self,fp):
        file_extension = os.path.splitext(fp)[1]
        if file_extension == "sprite":
            self.render_method = self.sprite_render
        self.render_method = self.render

    def get_df_img(self,fp):
        if fp == None:
            return loader.image("assets/images/None.png")
        return self.image(fp)

    def image(self,fp):
        return r_obj.loader.image(fp)
    def file(self,fp):
        return r_obj.loader.data(fp)

    def get_img(self,zoom):
        return self.OG_IMAGE
    def get_scaled_img(self,zoom,angle):
        return self.scaled_surf

    def should_scale(self,zoom):
        if self.last_zoom != zoom:
            self.last_zoom = zoom
            return True
        return False

    def should_rotate(self,angle):
        if angle != self.last_angle:
            self.last_angle = angle
            return True
        return False

    def sprite_render(self,zoom):
        img = self.OG_IMAGE.get_surf(zoom,self.angle)
        return img

    def render(self,zoom):
        if self.should_scale(zoom):
            self.scaled_surf = pygame.transform.scale(self.OG_IMAGE,(self.sx * zoom , self.sy * zoom))
            self.surf = pygame.transform.rotate(self.scaled_surf,self.angle)
        if self.should_rotate(self.angle):
            self.surf = pygame.transform.rotate(self.scaled_surf(zoom,self.angle),self.angle)
        return self.surf

    def get_surf(self,zoom):
        return self.render_method(zoom)

