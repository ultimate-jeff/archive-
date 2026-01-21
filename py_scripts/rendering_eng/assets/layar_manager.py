
import pygame
import math

# engine imports
from skyport.core.paths import PathUtil as pu
from global_utils import *

class Camera:
    instances = 0
    def __init__(self, display_surface, chunk_size,world_size,bg_fill_color=None,pryoraty=None,chunk_genorator_script=None):
        self.tiles = {}
        self.all_game_objs = []
        self.display_surface = display_surface
        self.chunk_size = chunk_size
        self.offset_x = 0
        self.offset_y = 0
        self.zoom = 1.0
        self.old_zoom = self.zoom
        self.WORLD_HIGHT = world_size[0]
        self.WORLD_WIDTH = world_size[1]
        self.CHUNKS_ON_X = self.WORLD_WIDTH // self.chunk_size
        self.CHUNKS_ON_Y = self.WORLD_HIGHT // self.chunk_size
        self.print_queue = ""
        self.genorator = chunk_genorator_script
        Camera.instances += 1
        self.pryoraty = util.pryoraty(pryoraty,Camera.instances)
        self.bg_fill_color = bg_fill_color

    def render_offset_objs(self,disp=None):
        sw, sh = self.display_surface.get_size() # sw and sh stand for screen width and screen height
        for obj in self.all_game_objs: # r_objs need a .x ,.y .og_x .og_y ,.get_surf
            screen_x = obj.x * self.zoom + self.offset_x
            screen_y = obj.y * self.zoom + self.offset_y 
            self.display_surface.blit(obj.get_surf(self.zoom),(screen_x,screen_y))   

    def chunkify(self,obj):
        c_pos = self.get_chunk_cords(obj.x,obj.y)
        chunk = self.get_chunk(c_pos[0], c_pos[1])
        chunk.all_objs.append(obj)

    def re_chunk(self,all_objs,obj):
        all_objs.remove(obj)
        self.chunkify(obj)

    def get_chunk_cords(self,x,y):
        cx = int(x) // self.chunk_size
        cy = int(y) // self.chunk_size
        return (cx,cy)

    def get_chunk(self,cx, cy):
        #return chunk at (cx, cy), create if it doesnt exist yet
        if (cx, cy) not in self.tiles:
            self.tiles[(cx, cy)] = Chunk(cx, cy, self.chunk_size,genorator=self.genorator,bg_fill_color=self.bg_fill_color)
            print(f"{prin_GREEN}Created chunk {cx},{cy}{prin_RESET}")
        return self.tiles[(cx, cy)]

    def tick_ops(self):
        if self.print_queue != "":
            print(self.print_queue)
        self.print_queue = ""

    def set_zoom(self, level):
        self.zoom = max(0.1, level)

    def get_min_max_c_pos(self,z,W,H):
        world_left   = -self.offset_x / z
        world_top    = -self.offset_y / z
        world_right  = world_left + W / z
        world_bottom = world_top + H / z

        min_cx = max(0, int(math.floor(world_left / self.chunk_size)))
        max_cx = min(self.CHUNKS_ON_X - 1, int(math.floor(world_right / self.chunk_size)))
        min_cy = max(0, int(math.floor(world_top / self.chunk_size)))
        max_cy = min(self.CHUNKS_ON_Y - 1, int(math.floor(world_bottom / self.chunk_size)))
        return min_cx,min_cy,max_cx,max_cy

    def render(self, target_x, target_y):
        W, H = self.display_surface.get_size()
        z = self.zoom

        self.offset_x = -target_x * z + W // 2
        self.offset_y = -target_y * z + H // 2

        min_cx,min_cy,max_cx,max_cy = self.get_min_max_c_pos(z,W,H)

        for cx in range(min_cx, max_cx + 1):
            for cy in range(min_cy, max_cy + 1):
                chunk = self.get_chunk(cx, cy)
                surf = chunk.scaled_surface(z)
                chunk.update(self.zoom,self)
                screen_x = cx * self.chunk_size * z + self.offset_x
                screen_y = cy * self.chunk_size * z + self.offset_y
                self.display_surface.blit(surf, (screen_x, screen_y))
        self.tick_ops()

    def get_surf(self):
        return self.display_surface

    def remove_chunk_obj(self,obj):
        obj_chunk = self.get_chunk_cords(obj.x,obj.y)
        for y in range(-1,1):
            for x in range(-1,1):
                chunk = self.get_chunk(obj_chunk[0]+x,obj_chunk[1]+y)
                if obj in chunk.all_objs:
                    chunk.all_objs.remove(obj)

class Chunk:
    def __init__(self, cx, cy, chunk_size ,bg_fill_color=None,genorator=None):
        self.cx = cx
        self.cy = cy
        self.size = chunk_size
        self.surf = pygame.Surface((chunk_size, chunk_size), flags=pygame.SRCALPHA)
        self.surf.fill((0, 0, 0))
        self._scaled = None
        self._last_zoom = None
        self.tags = {}
        self.all_objs = []
        self.bg_surf = None
        self.generate_terrain(genorator)
        self.bg_fill_color = bg_fill_color

    def scale__(self,zoom):
        w = max(1, int(self.size * zoom))
        h = max(1, int(self.size * zoom))
        self._scaled = pygame.transform.scale(self.surf, (w, h))
        self._last_zoom = zoom

    def scaled_surface(self, zoom):
        if self._last_zoom != zoom or self._scaled is None:
            self.scale__(zoom)
        return self._scaled

    def generate_terrain(self,genorator=None):
        if genorator != None:
            exec(genorator)

    def blit_objs_v2(self,zoom,cam):
        for obj in self.all_objs: # requierd atributes of rendering obj is .get_surf, .x ,.og_x , .y ,.og_y
            cam.display_surface.blit(obj.get_surf(zoom),((obj.x + self.cx) * zoom, (obj.y + self.cy) * zoom))
            if abs(obj.x - obj.og_x) >= self.size or abs(obj.y - obj.og_y) >= self.size:
                cam.print_queue += f"obj {obj.id} is at {obj.x},{obj.y} and beeing rechunked\n"
                cam.re_chunk(self.all_objs,obj)

    def update(self,zoom,cam):
        if self.bg_fill_color != None:
            self.surf.fill(self.bg_fill_color)
            self._scaled.fill(self.bg_fill_color)
        self.blit_objs_v2(zoom,cam)

class Layar_manager:
    def __init__(self,surf):
        self.surf = surf
        self.layars = []
        self.stashed_layars = []

    def render(self):
        for l in self.layars:
            l.render(l.offset_x,l.offset_y)
            self.surf.blit(l.get_surf(),(0,0))
        return self.surf

    def get_surf(self):
        return self.surf

    def sort(self):
        self.layars = util.sort_objects_by_attr(self.layars,"pryoraty")

    def add_layar(self,layar):
        self.layars.append(layar)
        self.sort()
    def del_layar(self,pryoraty):
        if pryoraty in self.layars:
            self.layars.pop(pryoraty)
            return True
        print(f"layar {pryoraty} duse not exsis")
        return False
    def remove_layar(self,pryoraty):
        if pryoraty in self.layars:
            self.stashed_layars.append(self.layars[pryoraty])
            return True
        print(f"layar {pryoraty} duse not exsis")
        return False

    def get_layar(self,pryoraty):
        if pryoraty in self.layars:
            return self.layars[pryoraty]
        return None

    def get_layar_objs(self,layar):
        offset_objs = layar.all_game_objs
        chunk_objs = {}
        for pos in layar.tiles.key():
            chunk = layar.tiles[pos]
            c_obj = chunk.all_objs
            chunk_objs[pos] = c_obj
        return chunk_objs,offset_objs

    def set_layar_objs(self,chunk_objs,offset_objs,layar):
        layar.all_game_objs + offset_objs
        for pos in layar.tiles.key():
            chunk = layar.tiles[pos]
            chunk.all_objs + chunk_objs[pos]
        return layar

    def add_obj(self,obj,layar_pryoraty):
        layar = self.get_layar(layar_pryoraty)
        if obj.render_type == "chunk" or obj.render_type == "CHUNK":
            layar.chunkify(obj)
        else:
            layar.all_game_objs.append(obj)
    def remove_obj(self,obj,layar_pryoraty):
        layar = self.get_layar(layar_pryoraty)
        if obj.render_type == "chunk" or obj.render_type == "CHUNK":
            layar.remove_chunk_obj(obj)
        else:
            if obj in layar.all_game_objs:
                layar.all_game_objs.remove(obj)


