import threading
import Queue
import subprocess

tasks = Queue.Queue()

class Worker (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(seelf)
    def run (self):
          print tasks.get()

def work():
  while not tasks.empty():
    (cmd, arg) = tasks.get()
    subprocess.call([cmd, arg])

def main():
  
  
  for i in range(0, 1):
    tasks.put( ("echo \"Hehe\" > %d.txt" % i, "") )

  nThreads = 1
  threads = []
  for i in range(0, nThreads):
    t = threading.Thread(target=work, args=[])
    threads.append( t )
    t.start()
  

  for i in range(0, nThreads):
    threads[i].join()


if __name__ == '__main__':
  main();


