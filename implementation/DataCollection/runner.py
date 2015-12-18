import threading
import Queue
import os

tasks = Queue.Queue()

class Worker (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(seelf)
    def run (self):
          print tasks.get()

def work():
  while not tasks.empty():
    cmd = tasks.get()
    print cmd
    os.system( cmd )

def main():
  
  
  for i in range(0, 10):
    tasks.put( "./ExampleProject -optimizer=ce -noiseType=0 -noise=0 -maxiter=81 -sigma=100 -output=ce_NoNoise_%d" % i )

  for i in range(0, 10):
    tasks.put( "./ExampleProject -optimizer=ce -noiseType=1 -noise=4 -maxiter=81 -sigma=100 -output=ce_ConstantNoise_%d" % i )

  for i in range(0, 10):
    tasks.put( "./ExampleProject -optimizer=ce -noiseType=2 -noise=4 -maxiter=81 -sigma=100 -output=ce_LinearNoise_%d" % i )

  nThreads = 4
  threads = []
  for i in range(0, nThreads):
    t = threading.Thread(target=work, args=[])
    threads.append( t )
    t.start()
  

  for i in range(0, nThreads):
    threads[i].join()


if __name__ == '__main__':
  main();


