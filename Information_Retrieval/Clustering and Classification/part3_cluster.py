#!/usr/bin/python
# -*- coding: utf-8 -*-

#Modified K-Means Clustering
#Author: Swati Singh
#UIN: 321007275

#Import statements
import os
import re
import sys
import math
import json
import signal
import random
import string
import requests
from random import randint
from collections import defaultdict

#Global variables
final_rss = 0.0
uniqueWords=set()
pages=defaultdict()
tf = defaultdict(lambda: defaultdict(float))
idf = defaultdict(float)
tfidf = defaultdict(lambda: defaultdict(float))
#Actual Page to Category mapping 
clust = defaultdict()
#Identified Page to Category mapping 
new_clust = defaultdict()

#Function to extract the dataset from the API calls, qr is the array of query terms, filenm is the name of file 
def get_dataset(filenm,qr):
  filenm = open(filenm, 'a')  
  k=1
  for j in range (0,len(qr)):
    count=0
    while(count<2):
      if count==0:
        num='0'
      elif (count==1):
        num='15'
      query = 'https://api.datamarket.azure.com/Bing/Search/News?Query=%27'+qr[j]+'%27&$format=json&$skip='+num
      r = requests.get(query, auth=('xpressionz.89@gmail.com', '5gO1hIj0YO4IWjRm9z3iOfYXjSqJI81Jy/B41khKQM0='))
      count+=1
      for obj in json.loads(r.text)['d']['results']:
        k+=1
        json.dump(obj,filenm)
        filenm.write('\n')
  filenm.close()
  return

#Function to process file and calculate term frequence and inverse document frequency
def processfile(filenm):
  original_clusters=defaultdict(set)
  data = readfile(filenm)
  pgIndex=0
  #For every result extracted from the json file
  for page in data:
    title = page['Title'].lower()
    description = page['Description'].lower()
    uri = page['__metadata']['uri']
    #Extract the query/category type from the URI field of json result and create a page to category mapping
    query = findQuery(uri)
    original_clusters[query].add(pgIndex)    
    clust[pgIndex]=query
    #Get a list of all the words in title and description field of result and calculate the term-frequency 
    words=re.findall('\w+', title + ' ' + description, re.UNICODE)
    for word in words:   
      tf[pgIndex][word]+=1  
      uniqueWords.add(word)    
    uwords=set(words)
    #Add 1 to inverse docuent frequency on occurence of a word in the document
    for word in uwords:
      idf[word] = idf[word]+1  
    pages[pgIndex] = title
    pgIndex+=1
  #Compute logarithmic idf 
  for k,v in idf.iteritems():
    n=pgIndex/v
    idf[k]=math.log(n,2)
  return original_clusters

#Function to get a list of query results from the json file
def readfile(filenm):
  data=[]
  with open(filenm,'r') as json_file:
    for line in json_file:
      data.append(json.loads(line))
  return data

#Find the query term from the  uri of returned json page
def findQuery(mystr):
  pos = mystr.find('Query=\'')
  start_pos=pos+7  
  end_pos=string.find(mystr,'\'',start_pos,len(mystr))  
  query = mystr[start_pos:end_pos]
  return query

#Function to calculate tfidf of documents/pages
def findTfidf():
  for k,v in tf.iteritems(): 
    sum=0                                
    for m,n in v.iteritems():                                    
      tfidf[k][m]=(1+math.log(tf[k][m],2))*idf[m]
      sum+=(tfidf[k][m])*(tfidf[k][m])
    norm=math.sqrt(sum)            
    for i,j in v.iteritems():
      tfidf[k][i]=tfidf[k][i]/norm
  return

#Function to make k clusters out of the pages based on the centroid provided
def makeCluster(k,centroid):
  global final_rss   
  clusters = defaultdict(tuple)
  new_rss=0
  ctr = 1
  #Initialize k clusters with centroid as passed from argument
  for i in xrange(0, k): 
    clusters[i] = (centroid[i], set())
  #Loop until the RSS value converges
  while(1):
    rss=new_rss
    new_clust.clear()
    for page in pages:
      similiar_cluster=-1
      max_similarity=-1
      #Find the most appropriate cluster for the document/page based on cosine similarity
      for i in xrange(0, k): 
        similarity_value = get_similarity(centroid[i], page)
        if similarity_value >= max_similarity:
          max_similarity = similarity_value
          similiar_cluster = i
      clusters[similiar_cluster][1].add(page)
      new_clust[page]=similiar_cluster
    #Compute RSS of the new cluster formed
    new_rss=get_rss(clusters,k)     
    #Break if the RSS converges
    if math.fabs(new_rss-rss)<0.01:
      final_rss=new_rss
      break
    #If RSS does not converge, find the centroid of newly formed cluster and continue
    for i in xrange(0, k):       
      centroid[i] = get_centroid(clusters[i])
      clusters[i] = (centroid[i], set())
    ctr+=1   
  return clusters

#Function to calculate centroid of clusters
def get_centroid(cluster):
  centroid = defaultdict()
  num_elements = len(cluster[1])      
  #Assume every unique word present in the vocab as a dimension and compute centroid for a particular cluster      
  for word in uniqueWords:
    sum = 0
    for page in cluster[1]:  
      sum += tfidf[page][word]
    if num_elements != 0:
      centroid[word] = sum/num_elements
    else:
      centroid[word] = num_elements
  return centroid
  
#Function to calculate RSS of clusters
def get_rss(clusters,k):
  rss=0
  for i in xrange(0,k):
    for page in clusters[i][1]:
      for word in clusters[i][0]:
        diff= clusters[i][0][word]-tfidf[page][word]      
        difsqr = diff*diff  
        rss += difsqr 
  return rss

#Function to calculate cosine similarity between two pages 
def get_similarity(page_tfidf_1, page2):
  s_value = 0
  for k in page_tfidf_1:
    s_value += page_tfidf_1[k]*tfidf[page2][k]
  return s_value

#Function to calculate purity of clusters in percentage: percentage of corrrectly clustered docs in total number of docs
def get_purity(clusters,original_clusters):
  purity=0
  total_pages=0
  #Calculate total number of documents
  for orig_cluster in original_clusters:
    total_pages+=len(original_clusters[orig_cluster])
  #Calculate correctly clustered docs
  for key in clusters:
    max_common=0
    for orig_cluster in original_clusters:
      common_pages=original_clusters[orig_cluster].intersection(clusters[key][1])
      cardn=len(common_pages)
      if cardn>max_common:
        max_common=cardn          
    purity +=max_common
  purity = (float(purity)/total_pages)*100
  return purity

#Function to print the clusters
def print_clusters(clusters):  
  for i in clusters:
    print "cluster: ",i
    for pg in clusters[i][1]:
      print clust[pg],": ",pages[pg]
  return

#Function to calculate RI of clusters
def get_rand_index():
  ri = 0.0
  tp=0.0
  fp=0.0
  tn=0.0
  fn=0.0
  n=len(pages)
  for i in xrange(0, n-1):
    for j in xrange(i+1, n):
      if clust[i] == clust[j] and new_clust[i] == new_clust[j]:
        tp += 1
      elif clust[i] != clust[j] and new_clust[i] == new_clust[j]:
        fp += 1
      elif clust[i] == clust[j] and new_clust[i] != new_clust[j]:
        fn += 1
      elif clust[i] != clust[j] and new_clust[i] != new_clust[j]:
        tn += 1
  ri = float(tp+tn)/(tp+tn+fn+fp)
  return ri

#handling ctrl+C interrupt to exit
def signal_handler(signal, frame):
    print '\nExiting. Bye'
    sys.exit(0)

#Main function
def main():  
  # Ctrl+C Interrupt handling
  signal.signal(signal.SIGINT, signal_handler)

#Get the dataset. Commented so as not to run the API call.
#  qr=['texas aggies', 'texas longhorns', 'duke blue devils', 'dallas cowboys', 'dallas mavericks']
#  get_dataset('texasaggies.json',qr)

  #Fix a value of k
  k=10
  clusters = defaultdict(tuple)
  ini_clusters = defaultdict()
  centroid = defaultdict() 
  original_clusters = processfile('texasaggies.json')
  findTfidf()
  purity=0
  count=0
  range_index = range(len(pages))
  random.shuffle(range_index)
  cnt = 0
  for i in range_index[:k]:    
    ini_clusters[cnt] = tfidf[i]
    cnt+=1
  for i in xrange(0, k):
    clusters[i] = (ini_clusters[i], set())
    centroid[i]=ini_clusters[i]
  #Repeat ( 10 times ) and pick the cluster with maximum purity
  while(count<10):
    new_clusters = makeCluster(k,centroid)
    new_purity = get_purity(new_clusters,original_clusters)
    if(new_purity>purity):
      purity=new_purity   
      clusters=new_clusters
    count+=1 
    #Get the initial k points from each of the k different clusters rather than random restart
    for i in new_clusters: 
      rand_num=random.sample(new_clusters[i][1],1)
      for m in rand_num:
        num=m
      centroid[i]=tfidf[num]  
  ri = get_rand_index()
  print_clusters(clusters)
#  print "RSS: ", final_rss
#  print "Purity: ", purity
#  print "Rand Index: ", ri
  return

if __name__ == '__main__':
  main()

