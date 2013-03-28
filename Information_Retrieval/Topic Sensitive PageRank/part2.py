#!/usr/bin/python -tt
#The json file must be present in the same directory as this python script

import json
import math
from sets import Set
from collections import defaultdict
from collections import OrderedDict

#Global Dictionaries to store inlinks,outlinks for every user and the unique user ids
outmatr=defaultdict(set)
inmatr=defaultdict(set)
uniqueIds=defaultdict()

#Read the json file, parse it line by line and calculate inlinks and outlinks for each user, 
#call the function that calculates pagerank and outputs the top 50 results 
def readFl(jsonfile):    
    data=[]
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
    ranklst=calcPgRank()
    newdict=OrderedDict(sorted(ranklst.items(), key=lambda t: t[1],reverse=True)[:50])
    print "User Rankings \n"
    rank=1            
    for node,val in newdict.iteritems():
        print "rank: ",rank, " user id: ",node, "user name: ",uniqueIds[node].encode('utf-8')
        rank+=1                            
    return

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
                currlist[node]=alpha//len(inmatr)       
                for innode in inmatr[node]:                                                                                
                    currlist[node]+=((1-alpha)*(ranklist[innode]/len(outmatr[innode])))
                if (math.fabs(currlist[node]-ranklist[node])>0.00001):                       
                    flag=1                                                                  
        ranklist=currlist.copy()                
        if(flag==0):
            break        
    return currlist                                           

#Main function that calls the function to process the json file
def main():
    readFl('mars_tweets_medium.json')
    print "END"
    return

if __name__ == '__main__':
    main()
