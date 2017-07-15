import sys
import re
from operator import add

fileName = sys.argv[1]
filePointer = open(fileName,'r')
fileData = filePointer.read()

fileName2 = sys.argv[2]
filePointer2 = open(fileName2,'r')
fileData2 = filePointer2.read()

fileData2 = fileData2.replace('&&','and')
fileData2 = fileData2.replace('||','or')
fileData2 = fileData2.replace('==',' =')
fileData2 = fileData2.replace('\n',' ')

fileData = fileData.replace('}',':'+fileData2+'}')

f = file('.set','w+')
f.writelines(fileData)
f.close
