import numpy as np
from nn import TFProcess
from nn import batch_size
import settings
import sys
BSIZE=settings.BSIZE

from Dataloader import GmkData

def main():
    datapath=[]
    for line in open("datapath.txt"):
        line=line.strip('\n')
        datapath.append(line)
    data=GmkData(datapath)

    if (len(sys.argv)==3):
        trainer=TFProcess(sys.argv[2], sys.argv[1] + "/model")
    else:
        trainer=TFProcess(None, sys.argv[1] + "/model")
    while trainer.process(data.next_batch(batch_size), data.testdata):
        pass

    trainer.save_weights(sys.argv[1] + ".txt")

main()
