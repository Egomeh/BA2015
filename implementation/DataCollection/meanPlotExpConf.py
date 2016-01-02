import threading
import Queue
import os
import sys
import re
from random import randint
from time import sleep
import csv
import collections
import math


def usageGuide():
  print "Usage: python %s inputFiles outputFile refCol meanCol" % os.path.basename(__file__)
  print "inputFiles: A regext that will match all files for input"
  print "outputFile: A filename for the output file"
  print "refCol    : The name of the column that is kept for reference"
  print "meanCol   : The name of the column that is aggregated to a mean"


def main():
  if len(sys.argv) < 5:
    usageGuide()
    exit("Not enough arguments")
  
  inputFileExp = sys.argv[1]
  outputFile = sys.argv[2]
  refCol = sys.argv[3]
  meanCol = sys.argv[4]

  inputFileRe = re.compile(inputFileExp)

  """
  Get all the files that match the regEx
  """
  files = [f for f in os.listdir('.') if os.path.isfile(f)]
  input_files = []
  for f in files:
    if inputFileRe.match(f):
      input_files.append( f )

  """
  Make sure that we have input files
  """
  if len(input_files) == 0:
    exit("No input files given")
  else:
    print "Input is:"
    for f in input_files:
      print f

  
  sumData = {}
  """
  Check taht all files have the reference columns
  """
  for f in input_files:
    with open(f, 'rb') as csvfile:
      reader = csv.DictReader(csvfile)
      for row in reader:
        if not (refCol in row and meanCol in row):
          exit("The file %s does not have the required columns" % f)
        else:
          #print "%s = %s, %s = %s" % (refCol, row[refCol], meanCol, row[meanCol])
          if int(row[refCol]) not in sumData:
            sumData[ int(row[refCol]) ] = [ float(row[meanCol]) ]
          else:
            sumData[ int(row[refCol]) ].append( float(row[meanCol]) )



  # Get the mean of the data
  meanData = {}
  observations = -1
  for i in range(0, 80000):
    if i in sumData:
      if observations == -1:
        observations = len(sumData[i])
      else:
        if len(sumData[i]) != observations:
          print "index: %d does not have inconsistent observation count" % i

      meanData[i] = sum(sumData[i]) / float(len(sumData[i]))


  # Check if the output file exists
  if os.path.isfile(outputFile):
    exit("output file already exists")

  # write the CSV data to the file
  with open(outputFile, 'w') as csvfile:
    fieldnames = [refCol, meanCol, 'clow', 'chigh']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()
    for i in range(0, 80000):
      if i in meanData:
        confSpan = 1.96 / (math.sqrt(30))
        clow  = meanData[i] * (1 - (1.96 / math.sqrt(30)))
        chigh = meanData[i] * (1 + (1.96 / math.sqrt(30)))
        writer.writerow( { refCol:i , meanCol:meanData[i], 'clow':clow, 'chigh':chigh } )



if __name__ == "__main__":
  main()
