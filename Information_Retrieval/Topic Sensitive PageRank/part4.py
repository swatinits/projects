#!/usr/bin/python -tt
#The json file must be present in the same directory as this python script

import json
import math
from sets import Set
from collections import defaultdict
from collections import OrderedDict
outmatr=defaultdict(set)
inmatr=defaultdict(set)
uniqueIds=defaultdict()
groupmap = defaultdict(set)

#Read the json file, parse it line by line and calculate tf-idf values to populate dictionary
def readFl(jsonfile):    
    data=[]
    with open(jsonfile,'r') as json_file:    
        for line in json_file:
            data.append(json.loads(line))
    grpcnt=0;
    for field in data:                                
            uniqueIds[field["user"]["id"]]=field["user"]["screen_name"]    
            groupmap[grpcnt%4].add(field["user"]["id"])
            grpcnt+=1                   
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

    for key,val in groupmap.iteritems():                                                
        ranklst=calcPgRank(key)
        newdict=OrderedDict(sorted(ranklst.items(), key=lambda t: t[1],reverse=True)[:10])
        print "\n Group: ", key, "\n"          
        rank=0
        for node,val in newdict.iteritems():
            print "rank: ",rank,"name: ",uniqueIds[node].encode('utf-8'),"node: ",node,"val: ",val
            rank+=1                                  
    return

#calculate rank value of every user. If dangling user, rank should be zero. 
#Uses the linear method for calculating pagerank
def calcPgRank(grp):
    alpha=0.1
    ranklist=defaultdict(lambda: 0.0000)
    currlist=defaultdict(lambda: 0.0000)
    
    for node,name in uniqueIds.iteritems():
        if node in inmatr and node in groupmap[grp]:                      
            ranklist[node]=1.0000/len(inmatr)
                                   
    while(1):                         
        flag=0                
        for node,name in uniqueIds.iteritems():
          if node in groupmap[grp]:            
            currlist[node]            
            if node in inmatr:                                   
                currlist[node]=alpha                
                for innode in inmatr[node]:
                    if innode in groupmap[grp]:
                        outcnt=0                                          
                        for nd in outmatr[innode]:
                            if nd in groupmap[grp]:
                                outcnt+=1
                        currlist[node]+=((1-alpha)*(ranklist[innode]/outcnt))            
                if (math.fabs(currlist[node]-ranklist[node])>0.0001):                       
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
    