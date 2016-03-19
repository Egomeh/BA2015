import os
import subprocess
import hashlib

hashTableFileName = ".plothtab"
lHashTable = {}
hashTable = {}

def saveHashTable( table, filename ):
	with open( filename, "wb" ) as f:
		lines = []
		for key in table:
			lines.append("%s,%s\n" % (key, table[key]))
		f.writelines(lines) 

def loadHashTable( filename ):
	ret = {}
	if not os.path.exists( filename ):
		print "No such file %s" % filename
	else:
		with open( filename, "r" ) as f:
			content = f.readlines()
			counter = 1
			for line in content:
				tokens = line.split(",")
				if not len(tokens) == 2:
					print "Line %d invalid!" % counter
				else:
					ret [tokens[0].strip()] = tokens[1].strip()
				counter += 1
	return ret
	
def hashFile (  filename, blocksize=65536):
	hasher = hashlib.sha256()
	with open( filename, "rb" ) as f:
		buf = f.read(blocksize)
		while len(buf) > 0:
			hasher.update(buf)
			buf = f.read(blocksize)
		return hasher.hexdigest()


def compilePlot( plotname ):

	texFile = "%s.tex" % plotname
	pdfFile = "%s.pdf" % plotname
	auxFile = "%s.aux" % plotname
	logFile = "%s.log" % plotname

	devnull = open(os.devnull, 'w')
	return_code = subprocess.call("pdflatex -interaction=nonstopmode %s" % texFile, shell=True, stdout=devnull)
	
	if (return_code == 0):
		os.remove(auxFile)
		os.remove(logFile)
		
	return return_code == 0

def storePlotHashes( plotname ):
	source = "%s.tex" % plotname
	output = "%s.pdf" % plotname
	hashTable[hashFile(source)] = hashFile(output)

def checkNeedCompile( plotname ):
	source = "%s.tex" % plotname
	output = "%s.pdf" % plotname
	if not os.path.exists(output):
		return True
	sourceHash = hashFile(source)
	outputHash = hashFile(output)
	if sourceHash in lHashTable:
		return not outputHash == lHashTable[sourceHash]

	
	return True

if not os.path.exists(hashTableFileName):
    print "No hashtable found, recompiling all!"
else:
	print "Found file loading hashes..."
	lHashTable = loadHashTable(hashTableFileName)

for filename in os.listdir(os.path.dirname(os.path.realpath(__file__))):
	if (filename.endswith(".tex")) and not filename == "plotData.tex":
	
		plotName = os.path.splitext(os.path.join(os.path.dirname(os.path.realpath(__file__)),filename))[0]

		sourceHash = hashFile (filename)
		needReCompile = checkNeedCompile(plotName)
		
		if needReCompile:
			if not compilePlot(plotName):
				print "Error: %s" % os.path.basename(plotName)
			else:
				storePlotHashes(plotName)
				print "Success: %s" % os.path.basename(plotName)
		else:
			storePlotHashes(plotName)
			print "No change in %s" % os.path.basename(plotName)
				
				
saveHashTable( hashTable, hashTableFileName )

