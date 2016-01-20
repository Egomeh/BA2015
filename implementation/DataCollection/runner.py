import threading
import Queue
import os
import sys
import re
from random import randint
from time import sleep

tasks = Queue.Queue()
outFolder = None


outputFileRe = re.compile(".*-output=(.*).*")

class Worker (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(seelf)
    def run (self):
        print tasks.get()

def work():
  while not tasks.empty():
    # Get the task
    cmd = tasks.get()
    
    # print the task
    print cmd
    
    # If there is an output folder, exctract the output
    # file from the task
    outputFinder = outputFileRe.match(cmd)
    outFile = "%s.txt" % outputFinder.group(1)
    
    # Wait a while, rng is time based
    sleep(randint(10,30))
    
    # execute the task
    os.system( cmd )
    
    # Check if the output file is made and 
    # that the output folder still exists
    global outFolder
    print outFolder
    if outFolder:
    
        print "Moving file"
    
        if os.path.isfile( outFile ) and os.path.exists( outFolder ):
        
            dstName = outFile
            
            # Check if a file in the output folder has the same name
            while os.path.exists( os.path.join( outFolder, dstName ) ):
                dstName = "_%s" % dstName
                print "Renaming output file to %s" % outFile
                
            # Now move the file to the folder
            os.rename( outFile , os.path.join( outFolder, dstName ) )
        

def main():
  global outFolder

  if len(sys.argv) < 3:
    print "Usage: python runner.py {experiment file} {number of concurrent processes}"
    exit()

  if not os.path.isfile(sys.argv[1]):
    print "First argument is not a filepath"
    exit()

  if not sys.argv[2].isdigit():
    print "Second argument must be an integer"
    exit()
   
  # The user wants to put everything in a final folder 
  if "-finalFolder" in sys.argv:
    # Get indexof this argument
    ffid = sys.argv.index("-finalFolder")

    # Check for next argument
    if len(sys.argv) < ffid+2:
        exit("-finalFolder set without value")
        
    # Set the output folder to the absolute path
    outFolder = os.path.abspath(sys.argv[ffid+1])
    
    # Check if the output path exists
    if not os.path.exists(outFolder):
        exit("The output folder '%s' does not exist" % outFolder)
        
    print "Output is sent to %s" % outFolder


  lineIt = 0
  reg = re.compile("^\((.*)\) x (\d+)")
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
    sleep(randint(10,20))
  

  for i in range(0, nThreads):
    threads[i].join()


if __name__ == '__main__':
  main();


