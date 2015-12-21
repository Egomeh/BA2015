import threading
import Queue
import os
import sys
import re
from random import randint
from time import sleep

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
    sleep(randint(1,10))
    os.system( cmd )

def main():

  if len(sys.argv) < 3:
    print "Usage: python runner.py {experiment file} {number of concurrent processes}"
    exit()

  if not os.path.isfile(sys.argv[1]):
    print "First argument is not a filepath"
    exit()

  if not sys.argv[2].isdigit():
    print "Second argument must be an integer"
    exit()

  lineIt = 0
  reg = re.compile("^(\(.*\)) x (\d+)")
  hasIterator = re.compile(".*(\$i).*")
  with open(sys.argv[1], 'r') as f:
    for line in f:
      lineIt += 1
      m = reg.match(line)
      repNum = m.group(2)
      expression = m.group(1)
      if not m:
        print "Expression on line %d did not compile." % lineIt
        exit()
      if not hasIterator.match(expression):
        print "Expression on line %d did not have an iterator." % lineIt
      if not repNum.isdigit():
        print "Expression on line %d has ill formatted repetition number: %s" % (lineIt, repNum)

      for i in range(0, int(repNum)):
        proc = expression.replace("$i", str(i))
        print proc
        tasks.put( proc  )
      
        

  nThreads = int(sys.argv[2])
  threads = []
  for i in range(0, nThreads):
    t = threading.Thread(target=work, args=[])
    t.daemon = True
    threads.append( t )
    t.start()
    sleep(randint(1,10))
  

  for i in range(0, nThreads):
    threads[i].join()


if __name__ == '__main__':
  main();


