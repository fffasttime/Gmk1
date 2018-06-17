from multiprocessing import Process
import sys
import os
import random
import time

it_num=int(sys.argv[1])
game_num=2400
proc_num=4
path=os.getcwd()

def run_proc(count):
    print("starting process %d"%(count))
    string_cmd=\
        "p1\\cppGmk.exe -c %d --puct 1.6 --seed %d -p 400 -o selfplaydata%d.bin "%(game_num//proc_num, random.randint(0,100000), count)
    print(string_cmd)
    os.system(string_cmd + " >p1\\splog%d.txt 2>nul"%(count))

def multi_selfplay():
    
    if not os.path.exists("data/I%d.txt"%(it_num-1)):
        raise FileNotFoundError
    os.system("copy %s\\data\\I%d.txt %s\\p1\\weight.txt"%(path,it_num-1,path))
    for i in range(proc_num):
        if os.path.exists("p1/selfplaydata%d.bin"%(i)):
            pass
            #raise FileExistsError
    
    pool = [Process(target=run_proc, args=(i,)) for i in range(proc_num)]
    for p in pool:
        p.start()
    for p in pool:
        p.join()
    
    if not os.path.exists("spdata/I%d"%it_num):
        os.mkdir("spdata/I%d"%it_num)
    for i in range(proc_num):
        os.system("move %s\\p1\\selfplaydata%d.bin %s\\spdata\\I%d\\selfplaydata%d.bin"%(path,i,path,it_num,i))

'''   
def dataremake():
    if os.path.exists("data/gmkdata.txt"):
        os.remove("data/gmkdata.txt")
    for p in os.listdir("spdata/I%d"%(it_num)):
        os.system("dataremake_s.exe spdata/I%d/"%(it_num) + p)
        os.system("dataremake_s.exe spdata/I%d/"%(it_num) + p)
    for i,p in enumerate(os.listdir("spdata/I%d"%(it_num-1))):
        os.system("dataremake_s.exe spdata/I%d/"%(it_num-1) + p)
        os.system("dataremake_s.exe spdata/I%d/"%(it_num-1) + p)
        if i==len(os.listdir("spdata/I%d"%(it_num-1)))-2:
            break
    for i,p in enumerate(os.listdir("spdata/I%d"%(it_num-2))):
        os.system("dataremake_s.exe spdata/I%d/"%(it_num-2) + p)
        if i==len(os.listdir("spdata/I%d"%(it_num-2)))-3:
            break
'''
def train():
    with open("data/datapath.txt","w") as file:
        for p in os.listdir("spdata/I%d"%(it_num)):
            file.write('../spdata/I%d/'%(it_num)+p+'\n')
        for p in os.listdir("spdata/I%d"%(it_num-1)):
            file.write('../spdata/I%d/'%(it_num-1)+p+'\n')
        for p in os.listdir("spdata/I%d"%(it_num-2)):
            file.write('../spdata/I%d/'%(it_num-2)+p+'\n')
    os.chdir("data/")
    os.system("python Trainer.py I%d I%d >>trainer.log"%(it_num, it_num-1))
    os.chdir("..")

def run_proc2(count):
    if count==1:
        time.sleep(5)
    os.system("match.exe -p \"p2/cppGmk.exe\" -P \"p3/cppGmk.exe\" -c 25 -d c --name1 I%d --name2 I%d >>match%d.log"%(it_num,it_num-1,count))

def match():
    os.system("copy %s\\data\\I%d.txt %s\\p2\\weight.txt"%(path,it_num,path))
    os.system("copy %s\\data\\I%d.txt %s\\p3\\weight.txt"%(path,it_num-1,path))
    print("matching, I%d vs I%d"%(it_num, it_num-1))
    pool = [Process(target=run_proc2, args=(i,)) for i in range(2)]
    for p in pool:
        p.start()
    for p in pool:
        p.join()

def main():
    multi_selfplay()
    train()
    match()

if __name__=="__main__":
    main()
