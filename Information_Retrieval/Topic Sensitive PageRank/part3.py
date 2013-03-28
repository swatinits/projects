#!/usr/bin/python -tt
#The json file must be present in the same directory as this python script
#To Exit, use Ctrl+C

import re
import json
import math
import signal
from sets import Set
from collections import defaultdict
from collections import OrderedDict

#Global Dictionaries 
tf = defaultdict(lambda: defaultdict(float))
idf = defaultdict(float)
logidf = defaultdict(float)
tfidf = defaultdict(lambda: defaultdict(float))
tweets=defaultdict()
outmatr=defaultdict(set)
inmatr=defaultdict(set)
uniqueIds=defaultdict()
ranklst=defaultdict()
tweetid=defaultdict()
tweetuserid=defaultdict()
tweetuserscore=defaultdict(float)

#Read the json file, parse it line by line and calculate tf-idf values to populate dictionary.
#It also gets the pageRank/userRank for the users.
def readFl(jsonfile):  
    data=[]      
    index=0
    with open(jsonfile,'r') as json_file:    
        for line in json_file:
            data.append(json.loads(line))    
    for field in data:
        uniqueIds[field["user"]["id"]]=field["user"]["screen_name"]        
        if 'user_mentions' in field["entities"]:
                if field["entities"]["user_mentions"]:                                    
                    for dt in field["entities"]["user_mentions"]:                                                                                              
                        if(field["user"]["id"]!=dt["id"]):
                            uniqueIds[dt["id"]]=dt["screen_name"]                            
                            inmatr[dt["id"]].add(field["user"]["id"])
                            outmatr[field["user"]["id"]].add(dt["id"])
                            if dt["id"] not in outmatr:
                                outmatr[dt["id"]]=Set()
                            if field["user"]["id"] not in inmatr:
                                inmatr[field["user"]["id"]]=Set()

        tweets[index]=field["text"]
        tweetid[index]=field["id"]
        tweetuserid[index]=field["user"]["id"]        
        processData(field["text"],index)
        index=index+1
    ranklst=calcPgRank()      
    newdict=OrderedDict(sorted(ranklst.items(), key=lambda t: t[1],reverse=True))
    for item,val in tweetid.iteritems():
        tweetuserscore[val]=ranklst[tweetuserid[item]]    
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

#Calculates the tfidf value of the query
#Get the top 50 tweets that match the quesry based on the tf-idf value
def processQuery(str):    
    qtf = defaultdict(float)
    qtfidf = defaultdict(float)    
    qwords=re.findall('\w+', str.lower(),re.UNICODE)
    sum=0
    for word in qwords:
        qtf[word]=qtf[word]+1;
    for k,v in qtf.iteritems():
        qtf[k]=(1+math.log(qtf[k],2))
        qtfidf[k]=qtf[k]*idf[k]
        sum+=qtfidf[k]*qtfidf[k]
        if(qtfidf[k]==0):
            print "no match"
            return
    norm=math.sqrt(sum)            
    for i,j in qtfidf.iteritems():
        qtfidf[i]=qtfidf[i]/norm
    pgrank=buildRank(qtfidf)                    
    newdict=OrderedDict(sorted(pgrank.iteritems(), key=lambda t: t[1],reverse=True)[:50])    
    topdict=OrderedDict()
    for key,val in newdict.iteritems():
        topdict[key]=val                                                 
    combdict=defaultdict(list)        
    for key,val in topdict.iteritems():
        if(topdict[key]!=0):
            score=tweetuserscore[tweetid[key]]
            combdict[key].append(val)
            combdict[key].append(score)
    finaldict=OrderedDict(sorted(combdict.iteritems(), key=lambda t: (t[1][0],t[1][1]),reverse=True)[:50])
    rank=1    
    for key,val in finaldict.iteritems():
        print "rank: ",rank," tweetid: ",tweetid[key]," tweet: ",tweets[key].encode('utf-8')
        rank+=1         
    return

#Function that calculates the tf-idf value from each the tf and idf dictionaries
def buildRank(qtidf):    
    rank=defaultdict(float)
    for k,v in tfidf.iteritems():          
        for i,j in qtidf.iteritems():                        
            rank[k]+=qtidf[i]*v[i]                   
    return rank

#calculate rank value of every user. If dangling user, rank should be zero. 
#Uses the linear method for calculating pagerank
def calcPgRank():
    alpha=0.1
    ranklist=defaultdict(lambda: 0.0000)
    currlist=defaultdict(lambda: 0.0000)
    
    for node,name in uniqueIds.iteritems():
          if node in inmatr:            
              ranklist[node]=1.0000/len(inmatr)                            
                       
    while(1):                         
        flag=0
        for node,name in uniqueIds.iteritems():            
            currlist[node]            
            if node in inmatr:                                   
                currlist[node]=alpha/len(inmatr)     
                for innode in inmatr[node]:                                                                                
                    currlist[node]+=((1-alpha)*(ranklist[innode]/len(outmatr[innode])))
                if (math.fabs(currlist[node]-ranklist[node])>0.0001):                       
                    flag=1                                                                  
        ranklist=currlist.copy()                
        if(flag==0):
            break        
    return currlist                                           

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

#Main function that processes the json file, inputs and processes the query repeatedly
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
