
It is a functioning text-based mini search engine which supports both single term queries and phrase queries, as well as wild-card queries. 
First, I build an inverted index over documents from the data set by tokenizing each document using whitespaces and punctuations as delimiters and then case-folding to build an index. I then build a positional index to support phrase queries. Quotes are used in a query to tell the search engine that this is a phrase query. Do not explicitly type AND in queries and never use OR, NOT or parentheses. Finally, I build a k-gram index to support wild-card queries.


1) To run the program:
   >>python hw1_Swati_Singh.py ./books/books
   python <programname>  <path>
   Edit the pathname argument if a different path is used. If the files are in same directory, use the same path.

2) The command line would show the following:
   >>"Enter any char to search for a query or 0 to exit

3) Enter a query after the following prompt:
   >>"Enter the query"
   >>quixo* "mark twain" try
   >>245,76,416,543

4) It creates 3 index files named: boolindex, posindex, kgramindex

5) In order to exit the program, enter 0
   >>"Enter any char to search for a query or 0 to exit
   >>0
   >>Search Engine Exitting
   >>

6) Example:

   swati@ubuntu:~/IR/hw1$ python hw1_Swati_Singh.py ./books/books/
   Enter any char to search for a query or 0 to exit 
   d
   Enter the query
   "mark twain"
   142,76,86,102,245,543,416,130,70,91,269,119,93,74
   Enter any char to search for a query or 0 to exit
   0
   Search Engine Exitting


