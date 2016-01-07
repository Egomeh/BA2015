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
  largestRefPoint = 0
  for i in range(0, 800000):
    if i in sumData:
      largestRefPoint = i
      if observations == -1:
        observations = len(sumData[i])
      else:
        if len(sumData[i]) != observations:
          print "index: %d does not have inconsistent observation count" % i

      meanData[i] = sum(sumData[i]) / float(len(sumData[i]))


  # Check if the output file exists
  if os.path.isfile(outputFile):
    exit("output file already exists")
    
  generationData = {}

  # write the CSV data to the file
  with open(outputFile, 'w') as csvfile:
    fieldnames = [refCol, meanCol, 'clow', 'chigh', 'Q1', 'Q2', 'Q3', 'means']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()
    for i in range(0, 80000):
      if i in meanData:
        rateHat = 1.0 / meanData[i]
        rateLow = rateHat * (1 - ( 1.96 / math.sqrt(30) ) ) 
        rateUpp = rateHat * (1 + ( 1.96 / math.sqrt(30) ) )
        clow  = 1.0 / rateLow
        chigh = 1.0 / rateUpp
        
        sortedMeans = sorted(sumData[i])
        
        # Define quantiles and calculate
        Q1 = 0
        Q2 = 0
        Q3 = 0
        n = len(sortedMeans)
        Q1 = (sortedMeans[int(math.floor(n   / 4.0) ) -1 ] + sortedMeans[int(math.ceil(n   / 4.0) ) - 1  ]) / 2
        Q3 = (sortedMeans[int(math.floor(n*3.0 / 4.0) )  ] + sortedMeans[int(math.ceil(n*3.0 / 4.0) )  ]) / 2
        
        if (n % 2) == 0:
            Q2 = (sortedMeans[int(n / 2)-1  ] + sortedMeans[int(n / 2)  ]) / 2
        else:
            Q2 = sortedMeans[int(math.floor(n/2))]
        
        writer.writerow( { refCol:i , meanCol:meanData[i], 'clow':clow, 'chigh':chigh, 'Q1':Q1, 'Q2':Q2, 'Q3':Q3, 'means':sortedMeans } )
        
        generationData[refCol] = i
        generationData[meanCol] = meanData[i]
        generationData['clow'] = clow
        generationData['chigh'] = chigh
        generationData['Q1'] = Q1
        generationData['Q2'] = Q2
        generationData['Q3'] = Q3
        
    # Report data
    print "Last x-axis point: %s" % str(largestRefPoint)
    
    # Prting last iteration values
    print generationData
    
   
    


if __name__ == "__main__":
  main()
