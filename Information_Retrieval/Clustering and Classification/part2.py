#!/usr/bin/python
# -*- coding: utf-8 -*-

#Naive Baye's classifier
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
pages=defaultdict()
uniqueWords=set()
prior_prob=defaultdict(float)
categorydict=defaultdict(set)  
category_words=defaultdict(list)
category_total_words=defaultdict()
cf = defaultdict(lambda: defaultdict(float))
cond_prob = defaultdict(lambda: defaultdict(float))
original_clusters=defaultdict(set)
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

#Train the Classifier based on training dataset   
def trainClassifier(filenm):
  data=readfile(filenm)
  total_pages=len(data)
  pgIndex=0
  for page in data:
    uri = page['__metadata']['uri']
    category = findCategory(uri)
    categorydict[category].add(page['Title'].lower()+' '+page['Description'].lower())
  for category in categorydict:
    catg_pages=len(categorydict[category])
    x=float(catg_pages)/float(total_pages)
    prior_prob[category]=math.log(x,2)       
    for doc in categorydict[category]:
      words=re.findall('\w+', doc, re.UNICODE)
      for word in words:   
        cf[category][word]+=1  
        category_words[category].append(word)
        uniqueWords.add(word)  
  return

#Find actual category from the URI field mentioned in the returned json result
def findCategory(mystr):
  pos = mystr.find('&NewsCategory=\'rt_')
  start_pos=pos+18
  end_pos=string.find(mystr,'\'',start_pos,len(mystr))  
  category = mystr[start_pos:end_pos]
  return category

#Function to get a list of query results from the json file
def readfile(filenm):
  data=[]
  with open(filenm,'r') as json_file:
    for line in json_file:
      data.append(json.loads(line))  
  return data

#Classify the documents test dataset into one of the classes based on training data
def testClassifier(filenm):
  doc_prob=defaultdict(lambda: defaultdict(float))
  doc_category=defaultdict()
  data=readfile(filenm)
  total_words=len(uniqueWords)
  for category in categorydict.keys():
    category_total_words[category]=len(category_words[category])
  pgIndex=0
  for page in data:
    doc=page['Title'].lower()+' '+page['Description'].lower()
    pages[pgIndex] = page['Title']
    words=re.findall('\w+', doc, re.UNICODE)
    for word in set(words):
      for category in categorydict.keys():
        x=(1+cf[category][word])/(category_total_words[category]+total_words)
        cond_prob[category][word]=math.log(x,2)
    for word in words:  
      for category in categorydict:
        doc_prob[category][pgIndex]+=cond_prob[category][word]
    for category in categorydict:
      doc_prob[category][pgIndex]+=prior_prob[category]
    max=-100000
    catg='null'
    for category in categorydict.keys():
      if doc_prob[category][pgIndex]>max:
        max=doc_prob[category][pgIndex]
        catg=category
    doc_category[pgIndex]=catg          
    pgIndex+=1
  return doc_category

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
#  print fmeasure
#  print " tp:", tp, " fn:",fn, " fp:", fp, " tn:", tn 
#  print precision,recall
#  for category in original_clusters:
#    print category
#    print " tp:", d_tp[category], " fn:",d_fn[category], " fp:", d_fp[category], " tn:", d_tn[category] 
#    print d_precision[category], d_recall[category]
  return

#Function to print the classified documents in required format
def print_classes(classes):  
  for catg in classes:
    print "\n",catg, ":"   
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

  #Print results
  print_classes(classified_docs)

  #Compute f-measure and confusion matrix
  compute_fmeasure(classified_docs,original_clusters)
  return
  
if __name__ == '__main__':
  main()

