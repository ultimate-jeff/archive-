
import string
import re
import random
import math
import json
import csv

data_paths = ["C:\\Users\\matt\\.c\\llms\\data1.txt"]

class Tokenizer:
    def __init__(self):
        self.tokens = [" "]
        self.full_tokens = {}

    def tokens_in1(self,text:str,min_len=1,max_len=2):
        for token in self.tokens:
            tlen = len(token)
            if not (tlen >= min_len and tlen <= max_len):
                continue
            if token in text:
                return True
        return False

    def tokenize(self,full_text:str):
        split_text = full_text.split()
        st_len = len(split_text)
        for i,text in enumerate(split_text):
            token_cantadate = ""
            for char in text:
                token_cantadate += char
                if token_cantadate not in self.tokens and not self.tokens_in1(token_cantadate,2,3):
                    self.tokens.append(token_cantadate)
                    break

            if i % 10000 == 0:
                print(f"tokenized to {((i/st_len)*100):.2f}% ")

    def init_full_tokens(self,tokens):
        for token in tokens:
            self.full_tokens[token] = 0

    def all_tokens_in(self,text:str,tokens:list[str]):
        tokens_in_text = []
        for token in tokens:
            if token in text:
                tokens_in_text.append(token)
        return tokens_in_text

    def find_probabilaties(self,tokens:list[str],full_text:str):
        split_text = full_text.split()
        st_len = len(split_text)
        for i,text in enumerate(split_text):
            tokens_in_text = self.all_tokens_in(text,tokens)
            for token in tokens_in_text:
                self.full_tokens[token] += 1

            if i % 10000 == 0:
                print(f"tokenized to {((i/st_len)*100):.2f}% ")
        
        for token,value in self.full_tokens.items():
            self.full_tokens[token] = value / st_len


        


t = Tokenizer()

with open(data_paths[0],"r",encoding="UTF-8") as f:
    text : str = f.read()

t.tokenize(text.lower())



for i in range(100):
    try:
        print(t.tokens[i])
    except IndexError:
        pass

t.init_full_tokens(t.tokens)
t.find_probabilaties(t.tokens,text)

with open("tokens.json","w") as f:
    json.dump(t.full_tokens,f,indent=4)





