import string
import re
import json

pattern = r'\w+|[^\w\s]'
flag_map = {
# [flags,invert_mask]  flags are set by (a-b)
    "==":[2,0],
    "!=":[0,2],
    "<":[4,4],
    ">=":[4,0],
    "true":[1,0] 
}
class Util:
    def __init__(self):
        self.pryoraty_defalt_value = None
    def pryoraty(self,a=None,b=None):
        if a == None:
            a = self.pryoraty_defalt_value
        if b == None:
            b = self.pryoraty_defalt_value
        if a != self.pryoraty_defalt_value:
            return a
        return b
    def tokenize(self,text):
        return re.findall(pattern, text)
util = Util()
class TYPE:
    requierd_regesters = 1
    instances = 0
    def __init__(self,name,addr,value=None):
        INT.instances += 1
        self.name = name
        self.value = util.pryoraty(value,0)
        self.is_const = value == None
        if self.is_const:
            self.addr = None
        else:
            self.addr = addr+TYPE.requierd_regesters
class INT(TYPE):
    def asine(self,value):
        pass
class NULL(TYPE):
    pass

class Tree_parce:
    def __init__(self):
        self.inst = []
    def tokenize(self,text):
        return re.findall(pattern, text)
    def comp(self,text:"str"):
        lines = text.splitlines()
        for line in lines:
            self.inst.append(self.tokenize(line))

t = Tree_parce()
idk = """
#include <iostream>

int main(){
    cout << "this is meee" << endl;
    int a = 4;
    int b = a+5;
}

"""
t.comp(idk)
print(t.inst)






