import math
import random
import struct
import sys
import numpy as np

from settings import RANDOM_ROTATE
from settings import BSIZE
BLSIZE=BSIZE*BSIZE

transform_table=np.ndarray([16,BLSIZE],dtype=int)
def boardTransform(mode, board):
    return board[transform_table[mode]]
    
for i in range(BLSIZE):
    transform_table[0][i]=i
for i in range(BSIZE):
    for j in range(BSIZE):
        transform_table[1][i*BSIZE + j] = j*BSIZE + BSIZE - i - 1
for i in range(BSIZE):
    for j in range(BSIZE):
        transform_table[4][i*BSIZE + j] = i*BSIZE + BSIZE - j - 1
transform_table[2] = boardTransform(1, transform_table[1]);
transform_table[3] = boardTransform(1, transform_table[2]);
transform_table[5] = boardTransform(1, transform_table[4]);
transform_table[6] = boardTransform(1, transform_table[5]);
transform_table[7] = boardTransform(1, transform_table[6]);

for i in range(8):
    for j in range(BLSIZE):
        transform_table[i + 8][transform_table[i][j]] = j;

def swap3(board):
    t=np.ndarray([BLSIZE],dtype=int);
    for i in range(BLSIZE):
        if board[i]==1:
            t[i]=2
        elif board[i]==2:
            t[i]=1
        else:
            t[i]=0
    return t
#-------------------------
def loadRawData(filename):
    print("[Info] Start loading data from " + filename)
    with open(filename,"rb") as file:
        st_int=struct.Struct('i')
        st_float=struct.Struct('f')
        st_policy=struct.Struct('%df'%BLSIZE)
        # (step,z,mov[step],rate[step],policy[step][BLSIZE])
        gamedata=[]
        sum_board, bwin=0,0.0
        while True:
            s=file.read(4)
            if s==b'' :   #EOF
                break
            steps=st_int.unpack(s)[0]
            sum_board+=steps
            z=st_int.unpack(file.read(4))[0]
            bwin+=(z==1)+(z==0)*0.5
            moves=np.ndarray([steps],dtype=int)
            winrate=np.ndarray([steps],dtype=float)
            policys=np.ndarray([steps,BLSIZE],dtype=float)
            for i in range(steps):
                tup=struct.Struct('if').unpack(file.read(8))
                moves[i], winrate[i]=tup[0],tup[1]
                if tup[0]<0 or tup[0]>BLSIZE: #BLSIZE intend swap
                    raise Exception("bad binary file")
                policy_=st_policy.unpack(file.read(BLSIZE*4))
                policys[i]=np.array(policy_,dtype=float)
            gamedata.append((steps,z,moves,winrate,policys))
    print("[Info] Binary data loaded, %d games, %f black wins, %d boards"%(len(gamedata),bwin,sum_board))
    return gamedata, sum_board

def calcVal(nums, val0, svals):
    first_z=0.6
    decay=math.pow(first_z, 1.0 / nums)
    alpha=0.4
    mcdecay=0.65
    vals=np.zeros([nums],dtype=float)
    vals[nums-1]=val0
    for i in range(nums-2,-1,-1):
        vals[i]=vals[i+1]*decay
        svals[i]=(1-mcdecay)*svals[i]-svals[i+1]*mcdecay
    for i in range(nums-1,-1,-1):
        if i%2==1:
            vals[i]=-vals[i]
    for i in range(nums-2,-1,-1):
        vals[i]=alpha*vals[i]+(1-alpha)*svals[i]
    return vals

BUFFER_SIZE=131072
TEST_SIZE=1024

class GmkData:
    def destruct_test(self,data):
        steps=data[0]
        board=np.zeros([BLSIZE],dtype=int)
        nowcol=1
        val0=data[1]
        if val0==2:
            val0=-1
        values=calcVal(steps,val0,data[3].copy())
        moves=data[2]
        for i in range(steps):
            self.testdata[0][self.testidx][0]=(board==nowcol)
            self.testdata[0][self.testidx][1]=(board==nowcol%2+1)
            self.testdata[1][self.testidx]=data[4][i]
            self.testdata[2][self.testidx]=values[i]
            self.testidx+=1
            if (moves[i]==BLSIZE):
                board=swap3(board)
            else:
                board[moves[i]]=nowcol
            nowcol=nowcol%2+1

    def destruct(self, data):
        steps=data[0]
        board=np.zeros([BLSIZE],dtype=int)
        nowcol=1
        val0=data[1]
        if val0==2:
            val0=-1
        values=calcVal(steps,val0,data[3].copy())
        moves=data[2]
        for i in range(steps):
            if RANDOM_ROTATE:
                r=random.randint(0,7)
                board_r=boardTransform(r,board)
                policy_r=boardTransform(r,data[4][i])
            else:
                board_r=board
                policy_r=data[4][i]
            self.data[0][self.bufferidx][0]=(board_r==nowcol)
            self.data[0][self.bufferidx][1]=(board_r==nowcol%2+1)
            self.data[1][self.bufferidx]=policy_r
            self.data[2][self.bufferidx]=values[i]
            self.bufferidx+=1
            if (moves[i]==BLSIZE):
                board=swap3(board)
            else:
                board[moves[i]]=nowcol
            nowcol=nowcol%2+1

    def loadtestData(self):
        self.testdata=[np.ndarray([TEST_SIZE+BLSIZE,2,BLSIZE]), \
                    np.ndarray([TEST_SIZE+BLSIZE,BLSIZE]),np.ndarray([TEST_SIZE+BLSIZE,1])]
        self.testidx=0;
        while self.testidx<TEST_SIZE:
            self.destruct_test(self.rawdata[self.gameindex])
            self.gameindex+=1
        self.rawdata=self.rawdata[self.gameindex:]
        self.gameindex=0

    def loadtoBuffer(self):
        print("[Info] Load raw data to buffer")
        self.bufferidx=0;
        while self.bufferidx<BUFFER_SIZE:
            self.destruct(self.rawdata[self.gameindex])
            self.gameindex+=1
            if self.gameindex==len(self.rawdata):
                self.gameindex=0
        sf=np.array(range(BUFFER_SIZE+BLSIZE))
        np.random.shuffle(sf)
        self.data=(self.data[0][sf],self.data[1][sf],self.data[2][sf])
        print("[Info] Loaded to buffer")
    
    def __init__(self,files):
        self.nowindex=0
        self.gameindex=0
        self.rawdata=[]
        self.sum_board=0
        for filename in files:
            data, sum_board=loadRawData(filename)
            self.rawdata+=data
            self.sum_board+=sum_board
        random.shuffle(self.rawdata)
        print("[Info] Total %d games, %d boards"%(len(self.rawdata),self.sum_board))
        self.loadtestData()

        self.data=[np.ndarray([BUFFER_SIZE+BLSIZE,2,BLSIZE]), \
                    np.ndarray([BUFFER_SIZE+BLSIZE,BLSIZE]),np.ndarray([BUFFER_SIZE+BLSIZE,1])]
        self.loadtoBuffer()

    def next_batch(self, batchsize):
        r=min(self.nowindex+batchsize,BUFFER_SIZE)
        s1=[x[self.nowindex:r] for x in self.data]
        self.nowindex+=batchsize
        if self.nowindex>BUFFER_SIZE:
            self.nowindex%=BUFFER_SIZE
            self.loadtoBuffer()
            return [np.vstack((s1[i],self.data[i][:self.nowindex])) for i in range(3)]
        else:
            return s1
