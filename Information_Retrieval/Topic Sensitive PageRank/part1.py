#!/usr/bin/python -tt
#The json file must be present in the same directory as this python script
#To Exit, use Ctrl+C

import re
import sys
import json
import math
import signal
from sets import Set
from collections import defaultdict
from collections import OrderedDict

#Global Dictionaries to store tf, idf, tf-idf values
tf = defaultdict(lambda: defaultdict(float))
idf = defaultdict(float)
logidf = defaultdict(float)
tfidf = defaultdict(lambda: defaultdict(float))
#Dictionary to store tweets with line number/json object number as the key
tweets=defaultdict()
#Dictionary to store tweet id with line number/json object number as the key
id=defaultdict()

#Read the json file, parse it line by line and calculate tf-idf values to populate dictionary 
def readFl(jsonfile):  
    data=[]      
    index=0
    with open(jsonfile,'r') as json_file:    
        for line in json_file:
            data.append(json.loads(line))    
    for field in data:
        tweets[index]=field["text"]
        id[index]=field["id"]        
        processData(field["text"],index)
        index=index+1               
    for k,v in idf.iteritems():
        n=index/v
        idf[k]=math.log(n,2)    
    for k,v in tf.iteritems(): 
        sum=0                                
        for m,n in v.iteritems():                                    
            tfidf[k][m]=(1+math.log(tf[k][m],2))*idf[m]
            sum+=(tfidf[k][m])*(tfidf[k][m])
        norm=math.sqrt(sum)            
        for i,j in v.iteritems():
            tfidf[k][i]=tfidf[k][i]/norm                                                                                                    
    return

#Calculates the tfidf value of the query and get the top 50 tweets that match the quesry based on the tf-idf value 
def processQuery(str):
    qtf = defaultdict(float)
    qtfidf = defaultdict(float)
    sum=0    
    qwords=re.findall('\w+', str.lower(),re.UNICODE)
    for word in qwords:
        qtf[word]=qtf[word]+1;
    for k,v in qtf.iteritems():
        qtf[k]=(1+math.log(qtf[k],2))
        qtfidf[k]=qtf[k]*idf[k]
        if(qtfidf[k]==0):
            print "no match"
            return
        sum+=qtfidf[k]*qtfidf[k]    
    norm=math.sqrt(sum)            
    for i,j in qtfidf.iteritems():
        qtfidf[i]=qtfidf[i]/norm
    pgrank=buildRank(qtfidf)
    newdict=OrderedDict()              
    newdict=OrderedDict(sorted(pgrank.iteritems(), key=lambda t: t[1],reverse=True)[:50])
    print "Tweet Rankings \n"
    rank=1    
    for key,val in newdict.iteritems():
        if(newdict[key]!=0):
            print "rank: ",rank, "tweet id: ", id[key],"tweet: ",tweets[key].encode('utf-8')
            rank+=1                                                    
    return

#Function that calculates the tf-idf value from each the tf and idf dictionaries
def buildRank(qtidf):    
    rank=defaultdict(float)
    for k,v in tfidf.iteritems():          
        for i,j in qtidf.iteritems():                        
            rank[k]+=qtidf[i]*v[i]                   
    return rank

#Splits each tweet text into a set of unique words and updates the global tf and idf dictionary based on the word count. 
def processData(strng,index):
    uniqueWords=Set()
    parwords=re.findall('\w+', strng.lower(),re.UNICODE)
    for word in parwords:
        tf[index][word]=tf[index][word]+1;
        uniqueWords.add(word)  
    for word in uniqueWords:
        idf[word] = idf[word]+1   
    return

#handling ctrl+C interrupt to exit
def signal_handler(signal, frame):
 print '\nExiting. Bye'
 sys.exit(0)

#Main function that calls the function that processes the json file, inputs and processes the query repeatedly  
def main():
    signal.signal(signal.SIGINT, signal_handler)                
    readFl('mars_tweets_medium.json')
    str1 = raw_input("Enter search query \n")
    while(1):        
        processQuery(str1)
        str1 = raw_input("Enter search query \n")
    print "END"
    return

if __name__ == '__main__':
    main()
