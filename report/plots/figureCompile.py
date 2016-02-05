import os
import subprocess

for filename in os.listdir(os.path.dirname(os.path.realpath(__file__))):
  if (filename.endswith(".tex")) and not filename == "plotData.tex":
    print filename
    return_code = subprocess.call("pdflatex %s" % filename, shell=True)  
  
  
  
  
for filename in os.listdir(os.path.dirname(os.path.realpath(__file__))):
  if (filename.endswith(".aux") or filename.endswith(".log")):
    os.remove(filename)
    