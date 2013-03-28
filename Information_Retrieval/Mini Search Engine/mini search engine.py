#!/usr/bin/python -tt
#Author: Swati Singh
#UIN: 321007275
#Course: Information Storage and Retrieval
#Homework 1
#Before running the program, update the path variable
#To run the program: python hw1_Swati_Singh.py ./books/books

import os
import re
import sys
import copy
import string
import itertools
from array import array
from collections import defaultdict
dic=defaultdict(set)
posdic=defaultdict(list)
bigramdic = defaultdict(set)

def processfiles(path):
  filepaths=[]
  for root, dirs, files in os.walk(path):
    for tfile in files:
      filepaths.append(os.path.abspath(os.path.join(root,tfile)))
  for fl in filepaths:
    nmfile=os.path.basename(fl).split('.')[0]
    afile=open(fl)
    line=afile.read()
    line=line.lower()
    line=re.sub(r'_',"",line)
    parwords=re.findall('\w+', line) 
    buildIndex(nmfile,parwords)
    buildPosIndex(nmfile,parwords)
    buildBigramIndex(parwords)
    afile.close()
  return

def buildIndex(filename,wordlist):
  for word in wordlist:
    dic[word].add(filename)
  return

def buildPosIndex(filename,wordlist):
  pagedict={}
  for pos, word in enumerate(wordlist):
    try:
      pagedict[word][1].append(pos)
    except:
      pagedict[word]=[filename,array('i',[pos])]
  for k,v in pagedict.iteritems():
    posdic[k].append(v) 
  return

def buildBigramIndex(wordlist):
  for word in wordlist:
    bigramlist=set()
    mword='$'+word+'$$'
    length=len(mword)
    for i in range(0,length-2):
      bigramlist.add(mword[i:i+2])
    for bigram in bigramlist:
      bigramdic[bigram].add(word)
  return

def writeBoolIndex():
   f=open("boolindex", 'w')
   for word in dic.iterkeys():
     printword=', '.join(dic[word])
     print >> f, word, ' | ', printword
   f.close()
   return

def writeposIndex():
   f=open("posindex", 'w')
   for word in posdic.iterkeys():
     postings=[]
     for posting in posdic[word]:
       pageID=posting[0]
       locations=posting[1]
       postings.append(' : '.join([str(pageID) ,', '.join(map(str,locations))]))
     print >> f, ''.join((word,' | ','; '.join(postings)))            
   f.close()
   return

def writeBigramIndex():
   f=open("kgramindex", 'w')
   for word in bigramdic.iterkeys():
     printword1=', '.join(bigramdic[word])
     printword=word+' | '+printword1
     print >> f, printword
   f.close()
   return

def postresult(rset):
  if not rset:
   print "sorry no match :("
  else:
   myString = ",".join([str(i) for i in rset])
   print myString
  return

def searchWord(qwords):
  qset=set()
  if(len(qwords)==1):
    qset=dic.get(qwords[0].lower())
  elif(len(qwords)>1):
    rset=set
    for i in range(0,len(qwords)):
      if qwords[i].lower() in dic:
        rset= rset.intersection(dic.get(qwords[i].lower()))
      else:
        rset=()
        break
    qset=copy.deepcopy(rset)
  elif(len(qwords)==0):
    return(set())
  return qset

def searchPhrase(query):
   result=set()
   postings=[]
   pageposts=[]
   commonpages=set()
   wordq=query.split()
   for word in wordq:
     if word not in posdic:
       return result
   for word in wordq:
     postings.append(posdic[word])
   pages=[]
   for posting in postings:
     docs=set()
     for elem in posting:
       docs.add(elem[0])
     pages.append(docs)
   commonpages=set.intersection(*pages)
   pageposts=copy.deepcopy(postings)
   for i in range(0,len(postings)):
     for x in postings[i]:
       if x[0] not in commonpages:
           pageposts[i].remove(x)
 
   for i in range(0,len(pageposts)):
     for j in range(0,len(pageposts[i])):
       pageposts[i][j][1]=[num-i for num in pageposts[i][j][1]]
  
   for i in range(0,len(pageposts[0])):
     list1=[]
     for eachword in pageposts:       
       list1.append(eachword[i][1])
     if (len(list1)==0):
        return set()
     list1.sort(key=len)
     res=list(reduce(lambda x,y: set(x)&set(y),list1))
     if not res:
       continue
     else:
       result.add(pageposts[0][i][0]) 
   return result

def searchWildcard(qwords):
  finalset=set()
  finallist=[]
  for i in range(0,len(qwords)):
    initialset=set()
    rset=set()
    posn=qwords[i].find('*')
    if(posn==0):
      lqword=qwords[i].lower().replace("*","")+'$$'
      for j in range(0,len(lqword)-2):
        rset.add(lqword[j:j+2])
    elif(posn==len(qwords[i])-1):
      lqword='$'+qwords[i].lower().replace("*","")+'$'
      for l in range(0,len(lqword)-2):
        rset.add(lqword[l:l+2])
    else:
     lqword='$'+qwords[i]+'$$'   
     for j in range(0,posn):
       rset.add(lqword[j:j+2])     
     for k in range(posn+2,len(lqword)-2):
       rset.add(lqword[k:k+2])
    setlist=[]
    for bigr in rset:     
      setlist.append(bigramdic[bigr])
    initialset=set.intersection(*setlist)
    copyset=initialset.copy()
    breakword=qwords[i].lower().split('*');
    flag=0
    if(posn>0 and posn<len(qwords[i])-1):
      lenstar2=0-len(breakword[1])
      flag=1
    elif(posn==0):
      flag=2
    else:
      flag=3
    for word in initialset:
      if(flag==3):
        if(word[0:len(breakword[0])]!=breakword[0]):
          copyset.remove(word)
          continue
      if(flag==2):
        if(word[0-len(breakword[1]):]!=breakword[1]):
          copyset.remove(word)
          continue
      if(flag==1):        
        print word, len(word)
        if(word[0:len(breakword[0])]!=breakword[0] or len(word)<len(qwords[i])):
          copyset.remove(word)          
          continue
        if(word[lenstar2:]!=breakword[1] or len(word)<len(qwords[i])):
          copyset.remove(word)
          continue
    finallist.append(copyset)
  totalset=[]
  for list1 in finallist:    
    set1=set()
    for elemn in list1:
      if elemn in dic:
        set1=set1.union(dic[elemn])        
    totalset.append(set1) 
  if len(totalset)>0:
    final_set=set.intersection(*totalset)
  return final_set

def searchquery(query):
  phrases=[]
  wildcards=[]
  words=[]
  flag=[0,0,0]
  answerlist=[set(),set(),set()]
  finallist=[]
  finalset=set()
  phrases=re.findall(r'\"(.+?)\"',query)
  if(len(phrases)>0):
    for i in xrange(len(phrases)):
      remstr="\""+phrases[i]+"\""
      query=query.replace(remstr,"")
  splitwords=query.split()
  for aword in splitwords:
    aword=aword.replace("\"","")
    if '*' in aword:
      wildcards.append(aword)
    else:
      words.append(aword)
  if len(words)>0:   
    flag[0]=1
    answerlist[0]=searchWord(words)    
  if len(phrases)>0:
    flag[1]=1
    phlist=[]
    for word in phrases:
      phlist.append(searchPhrase(word))
    answerlist[1]=set.intersection(*phlist) 
  if len(wildcards)>0:
    flag[2]=1
    answerlist[2]=searchWildcard(wildcards)
  for i in range(0,3):
    if (flag[i]==1):
      finallist.append(set(answerlist[i]))
  if (len(finallist)>0):
    finalset=set.intersection(*finallist)
  postresult(finalset)
  return
  
def main():
  if(len(sys.argv)==2):
    path=sys.argv[1]
  else:
    print "Enter file path name as Argument"
    return
  processfiles(path)
  writeBoolIndex()
  writeBigramIndex()
  writeposIndex()
  opt = raw_input("Enter any char to search for a query or 0 to exit \n")
  while(opt!=str(0)):
    query=raw_input("Enter the query\n")
    searchquery(query)
    opt = raw_input("Enter any char to search for a query or 0 to exit\n")
  print "Search Engine Exitting"
  return

if __name__ == '__main__':
  main()
