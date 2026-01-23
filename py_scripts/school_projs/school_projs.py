"""
made by Matthew Robins for 4th hour computer science
----------------------------------
proj = console interactive program / Tetres clone

Note the game loop is at the botom of the file
"""
from threading import Thread
import time
import random
import json

# console color map / texture map 
# Note this i used a google serch for the color codes and the texture charicter (but i knew i wanted the black box char )
prin_RED = '\033[91m'
prin_GREEN = '\033[92m'
prin_BLUE = '\033[94m'
prin_RESET = '\033[0m'
prin_orange = '\033[33m'
prin_black = '\033[30m'
prin_gray = '\033[37m'
colors = [prin_RESET,prin_RED,prin_GREEN,prin_BLUE,prin_orange,prin_black,prin_gray]
orientation_map = ["north","east","south","west"]
textures = ["\u2588","֍","▓","▒"]
error_queue = []

# get requierd files for game
with open("data/blocks.json","r") as f:
  shapes = json.load(f)
with open("data/settings.json","r") as f:
   settings_data = json.load(f)

class Tile:
   """
   this class is a texture compiler and when caled it rets the propor texture with the propor color
   """
   def __init__(self):
       self.texture_ind = 0
       self.combos = {}
       for texture in textures:
           for color in colors:
               self.combos[f"{color}{texture}{texture}{prin_RESET}"] = textures.index(texture)
   def __call__(self, color_ind, **kwds):
       temp_texture_ind = kwds.get('texture_ind', self.texture_ind)
       tex_char = textures[temp_texture_ind]
       return f"{colors[color_ind]}{tex_char}{tex_char}{prin_RESET}"

   def get(self,color):
       # tile textures are (color texture texture color_reset)
       return self.combos[color]

class Bord:
    """
    this class is responsable for all rendering and pixel and row management

    """
    def __init__(self,width,hight,BG_color_ind=5):
        self.width = width
        self.hight = hight
        self.BG_color_ind = BG_color_ind
        self.bord = {}
        self.curent_frame = 0

    def snap_cords_in_bounds(self,x,y):
        new_x = min(max(x,0),self.width-1)
        new_y = min(max(y,0),self.hight-1)
        return new_x,new_y

    def set_tile(self,pos,color_ind):
        self.bord[pos] = tile(color_ind)
    def get_tile(self,pos):
        try:
            return self.bord[pos]
        except KeyError:
            x,y = self.snap_cords_in_bounds(pos[0],pos[1])
            print(f"{prin_RED}!!-- error tryed to go out of bounds --!!{prin_RESET}")
            return self.bord[(x,y)]

    def fill(self,color_ind):
        # fills the display with a color
        for y in range(self.hight):
            for x in range(self.width):
                self.set_tile((x,y),color_ind)

    def fill_row(self,row,color_ind):
        for x in range(self.width):
            self.set_tile((x,row),color_ind)

    def check_row(self,row):
        #checks a row for how meny colord pixels there are
        num_of_colors = 0
        for x in range(self.width):
            t = self.get_tile((x,row))
            if t != tile(self.BG_color_ind):
                num_of_colors += 1
        return num_of_colors

    def flip(self):
        text_buffer = ""
        # prints all the bord data on to the consle
        print("---"*self.width)
        for y in range(self.hight):
            for x in range(self.width):
                text_buffer += (self.get_tile((x,y)))
            text_buffer += "\n"
        print(text_buffer)

    def tile_is_empty(self,pos):
        return self.get_tile(pos) == tile(self.BG_color_ind)

    def place_shape(self,pos,color_ind,formation):
        # places a list of pixels for a shape
        for p in formation:
            x = p[0] + pos[0]
            y = p[1] + pos[1]
            self.set_tile((x,y),color_ind)

    def get_row_data(self,y):
        data = []
        for x in range(self.width):
            data.append(self.get_tile((x,y)))
        return data

    def fill_row_from_data(self,data,new_y=None,old_y=None):
        #instead of filling a row with 1 color this will fill it with what ever data is pased in
        for x in range(self.width):
            self.bord[x,new_y] = data[x]

    def shift_down(self,start_y):
        for y in range(start_y,0,-1):
            data = self.get_row_data(y).copy()
            if y != start_y:
                self.fill_row_from_data(data,y+1,y)

class Block:
   """
   this class is responsable for managing all the pixels that are part of the block
   """
   def __init__(self,x,y,last_color):
       self.can_move = True
       self.x = x
       self.y = y
       self.rotation = "north"
       self.shape = random.choice(shapes["blocks"])
       self.formation = self.shape["formation"]  # self.formation[self.rotation] is [(x1,y1),(x2,y2),...]
       self.color_ind = random.choice([1,2,3,4])

   def check_if_out_of_bounds(self,t,width_hight):
       if t >= width_hight or t < 0:
           return False
       return True

   def check_can_move(self,disp,dx=0,dy=1):
       # this method loops over evry pixel that the shape owns and then determans if the target ops of that pixel is open and if all the pixels can move then it will ret the cmox,cmoy
       CMOX = True # cmoy/cmoy means  Can Move On X/Y
       CMOY = True
       cmox,cmoy = True,True
       for pos in self.formation[self.rotation]:
           x = pos[0] + self.x
           y = pos[1] + self.y
           fx = x + dx
           fy = y + dy

           cmox = self.check_if_out_of_bounds(fx,disp.width)
           cmoy = self.check_if_out_of_bounds(fy,disp.hight)

           if cmox:
               check_x = fx
           else:
               check_x = x       # check for colisions wiith the target tile
           cmoy = disp.tile_is_empty((check_x,fy))

           if not (CMOX == cmox):
               CMOX = False
           if not (CMOY == cmoy):
               CMOY = False
       return CMOX,CMOY

   def move(self,disp,dx,dy):
       # moves the block then rets true if it moved
       cmox,cmoy = self.check_can_move(disp,dx,dy)
       ret_choice = True
       if not cmoy:
           return True
       if cmoy:
           self.y += dy
           ret_choice =  False
       if cmox:
           self.x += dx
           ret_choice =  False
       return ret_choice

   def get_data(self):
       x = self.x
       y = self.y
       formation = self.formation[self.rotation]
       color_ind = self.color_ind
       return x,y,color_ind,formation

class Game:
   """
   this class is incharge of actualy runing the game and holds maby all game memory assets

   the names of the methods should be descriptev enugh to get the idea of what thay do
   """
   def __init__(self,width,hight,bg_color_ind=5,gravaty=1,time_between_frames=1):
       self.alive = True
       self.points = 0
       self.leval = 1
       self.gravaty = gravaty
       self.curent_block = None
       self.last_color = 1
       self.TBF = time_between_frames
       self.display = Bord(width,hight,bg_color_ind)
       self.change_curent_block = False
       self.move = None
       self.orientation_map = ["north","east","south","west"]
       self.loops = 0
       with open("data/settings.json","r") as f:
           self.settings_data = json.load(f)

   def start(self):
       # starts the input thread and initiates the game
       self.in_thread = Thread(target=self.manage_inputs,daemon=True)
       confermation = input("move; a : left / b : rite  rotate : space   press enter to play :")
       self.get_new_curent_block()
       self.in_thread.start()

   def QUIT(self):
       # quits the game
       print(f"your points are {self.points} and you made it to leval {self.leval}")
       if self.points > self.settings_data["record"]:
           record_lev = self.settings_data["record"]
           print(f"{prin_GREEN}new record of {self.points} and your old record was {record_lev} points {prin_RESET}")
           self.settings_data["record"] = self.points
       if self.leval > self.settings_data["record_lev"]:
           print(f"{prin_GREEN}new record you got to {self.leval} and your last record was {record_lev}{prin_RESET}")
           self.settings_data["record_lev"] = self.leval
       with open("data/settings.json","w") as f:
           json.dump(self.settings_data,f,indent=4)

   def manage_inputs(self):
       # gets inputs and then choses the next move
       global running
       while running:
           move_choice = input("")
           if move_choice == "a" or move_choice == "aa":
               self.move = "left"
           elif move_choice == "d" or move_choice == "dd":
               self.move = "rite"
           elif move_choice == " " or move_choice == "  ":
               self.move = "rotate"
           elif move_choice == "q":
               running = False
           else:
               self.move = None
       time.sleep(0.1)

   def get_x_movement(self):
       if self.move == "left":
           return -1
       elif self.move == "rite":
           return 1
       else:
           return 0

   def run(self):
       # this method is caled evry frame to run the game
       global running
       print(f"--- points >> {self.points} --- leval >> {self.leval} --- loops >> {self.loops} --- TBF >> {self.TBF}")
       self.loops += 1
       self.TBF = self.get_TBF()
       time.sleep(self.TBF)
       self.update_curent_block()

       self.display.flip()
       if not self.alive:
           running = False
       if self.loops % 64 == 0:
           self.leval += 1

   def get_TBF(self):
       #TBF stands for time btween frames and this gets the TBF for the curent leval
       if not self.leval-1 > len(self.settings_data["leval_speed_chart"]):
           return self.settings_data["leval_speed_chart"][self.leval-1]
       return self.settings_data["leval_speed_chart"][-1]

   def update_curent_block(self):
       self.move_curent_block()
       if self.change_curent_block:
           self.get_new_curent_block()

   def rotate_block(self):
       if self.move == "rotate":
           curent_rotation = self.orientation_map.index(self.curent_block.rotation) # rotation stuffs
           new_rotation = (curent_rotation + 1) % 4
           self.curent_block.rotation = self.orientation_map[new_rotation]

   def shift_rows_dows(self,row_cleard,rows_to_clear):
       print(f"caled for {row_cleard}")
       self.display.shift_down(row_cleard)
       #self.display.fill_row(row_cleard, self.display.BG_color_ind)

   def check_row_cancle(self):
       # this method is responsable for checking all the rows to del and then dels them but also initiates the shifting of rows
       rows_to_clear = []
       rows_to_move_down = []
       ticks_till_kill = 5
       kill = False
       for y in range(self.display.hight):
           num_of_colors = self.display.check_row(y)
           if num_of_colors >= self.display.width:
               rows_to_clear.append(y)
           elif num_of_colors > 0 and self.display.check_row(y+1) <= 0:
               rows_to_move_down.append(y)
       for row in rows_to_clear:
           self.shift_rows_dows(row, rows_to_clear)
           #self.display.fill_row(row,self.display.BG_color_ind)

       #print(self.display.check_row(1))
       if self.display.check_row(1) > 0:  # kill management
           kill = True

       self.points += len(rows_to_clear) * self.display.width
       return not kill

   def move_curent_block(self):
       x_move = self.get_x_movement()

       x,y,color_ind,formation = self.curent_block.get_data()
       self.display.place_shape((x,y),self.display.BG_color_ind,formation) # remove block from bord

       self.rotate_block()
       self.alive = self.check_row_cancle()

       self.change_curent_block = self.curent_block.move(self.display,x_move,self.gravaty) # put it back on in new pos
       x,y,color_ind,formation = self.curent_block.get_data()            
       self.display.place_shape((x,y),self.curent_block.color_ind,formation)
       self.move = None

   def get_new_curent_block(self):
       self.curent_block = Block(int(self.display.width/2),0,self.last_color)

while True:
    # initglobal varyables
    tile = Tile()
    running = True
    game = Game(width=10,hight=20,bg_color_ind=6,gravaty=1,time_between_frames=0.5)

    #initing settings
    if game.settings_data["override_texture"] != None:
       textures.append(game.settings_data["override_texture"])
    tile.texture_ind = game.settings_data["defalt_texture_ind"]
    game.display.fill(game.display.BG_color_ind)

    # first time instruction prints
    if game.settings_data["first_time"]: # first time stuffs
       print("this is your first time playing consles tetres by Matthew Robins for 4th hour pc programing")
       print()
       print("controlls:")
       print("when trying to put input into the game you will type your input then press enter even thow you can't see what you are typing this is bc inputs are on a sepres thread as rendering")
       print("type 'a' then press enter to move your peace to the left")
       print("type 'd' then press enter to move your peace to the rite ")
       print("type ' ' or space then press enter to rotate your peace")
       print()
       print("enjoy this game of tetres")
       print()
       confermation = input("press enter to continue :")
       game.settings_data["first_time"] = False
    # start the game and clear the screen
    game.start()
    game.display.fill_row(game.display.hight,5)

    # game loop
    while running:
      game.run()

    print("nice run")
    game.QUIT()
    if input("would u like to play again? (y/n) :").lower() == "n":
        break



