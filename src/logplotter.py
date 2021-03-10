#!/usr/bin/env python3
import sys
import matplotlib.pyplot as plt
import numpy as np
import dateutil.parser
from dateutil.parser import parse
import pytz

#2021-03-08T17:15:06 10.253.98.201 send: 0/49560 recv: 118970/0 prot: 0/0/0 tot: 405162 sync: 4/68293/0/0/0 skew: 68293/68292 bcast: 0/0 autoq: 105.1/0


class MemberInfo:
  def __init__(self):
    self.time=[]
    self.send_underruns=[]
    self.send_overflows=[]
    self.recv_underruns=[]
    self.recv_overflows=[]
    self.lost=[]
    self.tot=[]

  def processString(self, sts):
    self.time.append(parse(sts[0]).astimezone(pytz.timezone('US/Eastern')))
    self.send_underruns.append(int((sts[3].split("/"))[0]))
    self.send_overflows.append(int((sts[3].split("/"))[1]))
    self.recv_underruns.append(int((sts[5].split("/"))[0]))
    self.recv_overflows.append(int((sts[5].split("/"))[1]))
    self.lost.append(int((sts[7].split("/"))[0]))
    self.tot.append(int(sts[9]))

def main():
  print("Main")

  if (len(sys.argv)<2):
    print("Please choose a file", file=sys.stderr)
    sys.exit(1)
  
  print("File chosen")
  data = {}
  with open(sys.argv[1], "r") as fi:
    st = fi.readline()
    
    while(st):
      sts = st.split(" ")
      #print(st)
      #print(sts[1])
      if(not sts[1] in data):
        data[sts[1]] = MemberInfo()
      data[sts[1]].processString(sts)
      st = fi.readline()
  
  #sidelength = int(np.sqrt(len(data))+1)
  #print(f"{sidelength}")
  #fig, axs  = plt.subplots(5, 4)
  #fig.tight_layout()
  #axs = axs.reshape(20)


  fig, axs = plt.subplots(1, 2)

  p = 0
  for m in data:
    #axs[p].set_title(f"Recv Underruns {m}")
    #axs[p].plot(data[m].time, data[m].recv_underruns)
    
    axs[0 if p<10 else 1].plot(data[m].time, data[m].send_overflows, label=m)
    p += 1

  axs[0].set_title(f"Send Overflows")
  axs[1].set_title(f"Send Overflows")
  axs[0].legend()
  axs[1].legend()
  plt.show()

  """
  for mem in data:
    print(mem)
    print(data[mem].time)
    """


if (__name__=="__main__"):
  main()