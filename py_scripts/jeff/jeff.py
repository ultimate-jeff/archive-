

import random
import threading
import time
import skyport as sp
pygame = sp.pygame
util = sp.Util()

lock = threading.Lock()
dm = sp.Display_Manager(
    window_size=sp.REZ_360p,
    display_size=sp.REZ_1080p,
    window_name="conways game of life"
    )
display = dm.get_display()


alive_color = (50,50,128)
dead_color = (50,50,50)
chunk_size = 20


WORLD_MIN_X, WORLD_MIN_Y = 0, 0
WORLD_MAX_X = (display.get_width()  // chunk_size) - 1
WORLD_MAX_Y = (display.get_height() // chunk_size) - 1

rules = {
    "requierd_neighbors":1,
    "max_neighbors":4,
    "back_to_life":3
}

def in_range(value,max_value,min_value):
    if value < max_value and value >= min_value:
        return True
    return False

def get_adjecent(self:"sp.Chunk"):
    adjasent = []
    for y in range(self.cy -1 , self.cy + 2):
        for x in range(self.cx -1 , self.cx + 2):
            if (x,y) != (self.cx,self.cy):
                adjasent.append((x,y))
    return adjasent
def get_alive(self:"sp.Chunk",adjasent:"list[(int,int)]"):
    global bord
    alive = 0
    for cord in adjasent:
        if in_range(cord[0],WORLD_MAX_X,0) and in_range(cord[1],WORLD_MAX_Y,0):
            tile = bord.get_tile(cord[0],cord[1])
            if tile.tags["alive"]:
                alive += 1
    return alive

def apply_rules(self:"sp.Chunk",alive):

    salive = self.tags["alive"]

    if alive >= rules["back_to_life"]:
        salive = True
    if alive > rules["max_neighbors"]:
        salive = False
    elif alive < rules["requierd_neighbors"]:
        salive = False

    return salive
            
def tick_update(self:"sp.Chunk"):
    global rules,tick

    adjasent = get_adjecent(self)
    alive = get_alive(self,adjasent)
    return apply_rules(self,alive)

def update(self:"sp.Chunk"):
    with lock:
        alive = self.tags["alive"]
    if alive:
        self.fill_color = alive_color
        self.fill(alive_color)
    else:
        self.fill_color = dead_color
        self.fill(dead_color)

def genorator(self:"sp.Chunked_Layer"):
    self.tags["alive"] = random.choice([True,False,False,False,False])



bord = sp.Chunked_Layer(
    0,
    0,
    display.get_width(),
    display.get_height(),
    chunk_updateor=update,
    chunk_genorator=genorator,
    chunk_size=100
)
bord._render_visable_chunks()

tick = True
def idk():
    global tick
    tick = not tick
dm.add_keydown_bind(pygame.K_SPACE,idk)

dm.root_layer.add_obj(bord)
dm.START_RENDERING_THREAD(60)
time.sleep(1)
while dm.running:
    if tick:
        with lock:
            tiles = list(bord._tiles.values())
            next_states = {t: tick_update(t) for t in tiles}
            for t,alive in next_states.items():
                t.tags["alive"] = alive

    sp.loger.print()
    dm.event_handler()

    dm.tick(10)

