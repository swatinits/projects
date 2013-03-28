Eight Puzzle Problem: Implementation of BFS, BFSv,DFS, DLS, ID DFS
Author: Swati Singh


Table of Statistics:
The table below lists the mean number of goal tests and maximum number of queue size for each algorithm, averaged over 10 trials for same scrambled state:
	Goal    Test	Maximum	 Queue Size
BFS	9545	28632
BFSv	68	44
DFSv	44940	10804
DLS	8874	25
Iterative Deepening	160	  7

Example Run for for BFS:
In how many moves do you want to scramble to form the puzzle
15
goal: 
1 2 3 
4 . 5 
6 7 8 

puzzle: 
1 2 3 
4 5 8 
6 . 7 

iteration: 1, queue: 0, depth: 0

iteration: 2, queue: 3, depth: 1

iteration: 3, queue: 6, depth: 1

……
iteration: 71, queue: 210, depth: 3

iteration: 72, queue: 213, depth: 3

Solution Found

1 2 3 
4 5 8 
6 . 7 

1 2 3 
4 5 8 
6 7 . 

1 2 3 
4 5 . 
6 7 8 

1 2 3 
4 . 5 
6 7 8 

BFS Complete and successful

Example Run for for BFSv:
In how many moves do you want to scramble to form the puzzle
30
goal: 
1 2 3 
4 . 5 
6 7 8 

puzzle: 
1 . 3 
6 2 5 
7 4 8 

iteration: 1, queue: 0, depth: 0

iteration: 2, queue: 2, depth: 1

…………

iteration: 34, queue: 27, depth: 5

iteration: 35, queue: 27, depth: 5

Solution Found

1 . 3 
6 2 5 
7 4 8 

1 2 3 
6 . 5 
7 4 8 

1 2 3 
6 4 5 
7 . 8 

1 2 3 
6 4 5 
. 7 8 

1 2 3 
. 4 5 
6 7 8 

1 2 3 
4 . 5 
6 7 8 

BFS Visited Complete and successful



