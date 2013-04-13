#!/usr/bin/python
# -*- coding: utf-8 -*-

#Modified k-nearestneighbour classifier
#Author: Swati Singh
#UIN: 321007275

#Import statements
import os
import re
import sys
import math
import json
import string
import signal
import requests
import random
from random import randint
from collections import defaultdict

#Global variables
uniqueWords=set()
pages=defaultdict()
categorydict=defaultdict(set)  
category_words=defaultdict(list)
category_total_words=defaultdict()
cf = defaultdict(lambda: defaultdict(float))
original_clusters=defaultdict(set)
train_tfidf = defaultdict(lambda: defaultdict(float))
train_class=defaultdict()
test_tfidf = defaultdict(lambda: defaultdict(float))
test_class=defaultdict()
idf = defaultdict(float)
classof=defaultdict()

#Function to extract the dataset from the API calls; qr and category are the array of query terms and categories
def get_dataset(filenm,qr,category):
  filenm = open(filenm, 'a')
  k=1
  for i in range (0,len(category)):
    for j in range (0,len(qr)):
     count=0
     while(count<2):
      if count==0:
        num='0'
      elif (count==1):
        num='15'
      query = 'https://api.datamarket.azure.com/Bing/Search/News?Query=%27'+qr[j]+'%27&$format=json&$skip='+num+'&NewsCategory=%27rt_'+category[i]+'%27'
      r = requests.get(query, auth=('xpressionz.89@gmail.com', '5gO1hIj0YO4IWjRm9z3iOfYXjSqJI81Jy/B41khKQM0='))
      count+=1
      for obj in json.loads(r.text)['d']['results']:        
        k+=1
        json.dump(obj,filenm)
        filenm.write('\n')
  filenm.close()
  return

#Train the Classifier based on training dataset, calculate tfidf value of each document
def trainClassifier(filenm):
  tf = defaultdict(lambda: defaultdict(float))  
  data=readfile(filenm)
  pgIndex=0
  uset=set()
  for page in data:
    uri = page['__metadata']['uri']
    category = findCategory(uri)
    train_class[pgIndex]=category
    pgIndex+=1
    uset.add(category)
    categorydict[category].add(pgIndex)
  pgIndex=0
  for page in data:
    title = page['Title'].lower()
    description = page['Description'].lower()
    words=re.findall('\w+', title + ' ' + description, re.UNICODE)
    for word in words:   
      tf[pgIndex][word]+=1  
      uniqueWords.add(word)    
    uwords=set(words)
    for word in uwords:
      idf[word] = idf[word]+1  
    pages[pgIndex] = page['Title']
    pgIndex+=1
  for k,v in idf.iteritems():
    n=pgIndex/v
    idf[k]=math.log(n,2)
  findTfidf(tf,idf)  
  return 

#Function to get a list of query results from the json file
def readfile(filenm):
  data=[]
  with open(filenm,'r') as json_file:
    for line in json_file:
      data.append(json.loads(line))  
  return data

#Find actual category from the URI field mentioned in the returned json result
def findCategory(mystr):
  pos = mystr.find('&NewsCategory=\'rt_')
  start_pos=pos+18
  end_pos=string.find(mystr,'\'',start_pos,len(mystr))  
  category = mystr[start_pos:end_pos]
  return category

#Function to calculate tfidf of pages
def findTfidf(tf,idf):  
  for k,v in tf.iteritems(): 
    sum=0                                
    for m,n in v.iteritems():                                    
      train_tfidf[k][m]=(1+math.log(tf[k][m],2))*idf[m]
      sum+=(train_tfidf[k][m])*(train_tfidf[k][m])
    norm=math.sqrt(sum)            
    for i,j in v.iteritems():
      train_tfidf[k][i]=train_tfidf[k][i]/norm
  return

#Classify the documents test dataset into one of the classes based on training data
def testClassifier(filenm):
  doc_category=defaultdict()
  data=readfile(filenm)
  pgIndex=0
  for page in data:
    title = page['Title'].lower()
    description = page['Description'].lower()
    page_str=title + ' ' + description
    doc_category[pgIndex]=evaluateCategory(page_str)
    pgIndex+=1
  return doc_category

#Find an appropriate category for a page/document based on cosine similarity of the top k similiar pages
def evaluateCategory(page_content):
    assigned_catg='nil'
    sum=0
    num=475
    qtf = defaultdict(float)
    qtfidf = defaultdict(float)
    words=re.findall('\w+', page_content, re.UNICODE)
    for word in words:   
      qtf[word]+=1
    for k,v in qtf.iteritems():
        qtf[k]=(1+math.log(qtf[k],2))
        qtfidf[k]=qtf[k]*idf[k]
        sum+=qtfidf[k]*qtfidf[k]
    norm=math.sqrt(sum)            
    for i,j in qtfidf.iteritems():
        qtfidf[i]=qtfidf[i]/norm    
    similiar_pages_stat=find_ksimiliar_pages(qtfidf,num)
    similiar_pages=[]
    for entry in similiar_pages_stat:
      similiar_pages.append(entry[0])
    category_count=defaultdict(float)
    for ct in categorydict.keys():
      category_count[ct]=0
    for pg in similiar_pages_stat:
      ct=train_class[pg[0]]
      category_count[ct]+=(pg[1])
    maxm=0        
    for ct in category_count:
      if(category_count[ct])>maxm:    
        maxm=category_count[ct]
        assigned_catg=ct
    return assigned_catg

#Get num most similiar pages for doc with qtfidf vector 
def find_ksimiliar_pages(qtfidf,num):
  rank=defaultdict(float)
  for k,v in train_tfidf.iteritems():          
    for i,j in qtfidf.iteritems():                            
      rank[k]+=qtfidf[i]*v[i]      
  knearest=sorted(rank.iteritems(), key=lambda t: t[1],reverse=True)[:num]  
  return knearest

#Extract Actual Class from the json data result
def analyseClassifier(filenm,doc_category):
  data=[]
  original_clusters.clear()
  data=readfile(filenm)
  pgIndex=0
  for page in data:
    uri = page['__metadata']['uri']
    category = findCategory(uri)   
    original_clusters[category].add(pgIndex)
    classof[pgIndex]=category
    pgIndex+=1
  return

#Calculate Confusion Matrix and F-Measure
def compute_fmeasure(clusters,original_clusters):
  tp=0
  fp=0
  tn=0
  fn=0
  d_tp=defaultdict()
  d_fp=defaultdict()
  d_tn=defaultdict()
  d_fn=defaultdict()
  d_recall=defaultdict()
  d_precision=defaultdict()
  other_pages=defaultdict(set)
  fmeasure=defaultdict()
  for category in original_clusters:
    d_tp[category]=len(original_clusters[category].intersection(clusters[category]))
    d_fn[category]=len(original_clusters[category].difference(clusters[category]))   
    d_fp[category]=len(clusters[category].difference(original_clusters[category]))
    tp+=len(original_clusters[category].intersection(clusters[category]))
    fn+=len(original_clusters[category].difference(clusters[category]))   
    fp+=len(clusters[category].difference(original_clusters[category]))
    for catg in original_clusters:
      if(catg==category):
        continue
      other_pages[category]=other_pages[category].union(original_clusters[catg])    
  for category in original_clusters: 
    tn+=len(other_pages[category].difference(clusters[category]))
    d_tn[category]=len(other_pages[category].difference(clusters[category]))
    d_precision[category]=float(d_tp[category])/(d_tp[category]+d_fp[category])
    d_recall[category]=float(d_tp[category])/(d_tp[category]+d_fn[category])
  precision=float(tp)/(tp+fp)
  recall=float(tp)/(tp+fn)
  fmeasure=float(2*precision*recall)/(precision+recall)
#  for category in original_clusters:
#    print " tp:", d_tp[category], " fn:",d_fn[category], " fp:", d_fp[category], " tn:", d_tn[category] 
#    print d_precision[category], d_recall[category]
  return
  
#Function to print the classified documents in required format
def print_classes(classes):  
  for catg in classes:
    print "\n",catg, ": "
    for doc in classes[catg]:
      print classof[doc],": ",pages[doc]
  return

#handling ctrl+C interrupt to exit
def signal_handler(signal, frame):
    print '\nExiting. Bye'
    sys.exit(0)

#Main function
def main():  

  # Ctrl+C Interrupt handling
  signal.signal(signal.SIGINT, signal_handler)

#Get the dataset. Commented so as not to run the API call.
#  training_qr=['bing', 'amazon', 'twitter', 'yahoo', 'google', 'beyonce', 'bieber', 'television', 'movies', 'music', 'obama', 'america', 'congress', 'senate', 'lawmakers']
#  test_qr=['apple', 'facebook', 'westeros', 'gonzaga', 'banana']
#  category=['entertainment', 'business', 'politics']
#  get_dataset('trainingset.json',training_qr,category)
#  get_dataset('test_set2.json',test_qr,category)
  
  classified_docs=defaultdict(set)

  #Train the classifier
  trainClassifier('trainingset.json')

  #Perform classification on Test data
  doc_category=testClassifier('test_set.json')

  #Extracts and creates a mapping of actual categories from the json data results
  analyseClassifier('test_set.json',doc_category)

  #Creating mapping of category and its respective set of classfied documents
  for doc in doc_category:
    classified_docs[doc_category[doc]].add(doc)

#  Print results
  print_classes(classified_docs)

  #Compute f-measure and confusion matrix
  compute_fmeasure(classified_docs,original_clusters)
  return
  
if __name__ == '__main__':
  main()

