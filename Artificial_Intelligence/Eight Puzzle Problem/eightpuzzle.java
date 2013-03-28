package asgn;
import java.util.*;
import java.io.*;

class Node {
	String state;
	int depth;
	Node(String state, int depth){
		this.state=state;
		this.depth=depth;
	}
}

public class Homework1 {
	static String goal="1234.5678";
	static String puzzle;
	static Queue<Node> frontier = new LinkedList<Node>();
	static Map<Node,Node> stateHistory = new HashMap<Node,Node>();
	static List<String> stateVisited  = new ArrayList<String>();
	static Stack<Node> frontierSt = new Stack<Node>();
	static int maxqueue=0;
	static int goaltest=0;
	
		public static void main(String args[]) {
		String str="";
		int distance=0;
		int option=0;
		System.out.println("In how many moves do you want to scramble to form the puzzle");
		try {
		      BufferedReader is = new BufferedReader(new InputStreamReader(System.in));
		      str = is.readLine();
		      distance = Integer.parseInt(str);
		    } catch (NumberFormatException ex) {
		      System.err.println("Not a valid number: " + str);
		    } catch (IOException e) {
		      System.err.println("Unexpected IO ERROR: " + e);
		    }		      
		puzzle=scramble(goal,distance);
		System.out.println("goal: ");
		printState(goal);
		System.out.println("puzzle: ");
		printState(puzzle);
						
        while(true)
        {			
			System.out.println("\n\nSelect a number to run the algorithm to unscramble the puzzle\n");
			System.out.println("1. Breadth First Search");
			System.out.println("2. Breadth First Search Visited");
			System.out.println("3. Depth First Search");
			System.out.println("4. Depth Limited Search");
			System.out.println("5. Iterative Deepenin");
			System.out.println("6. Enter any other key to Exit");
			
		try {
		      BufferedReader opt = new BufferedReader(new InputStreamReader(System.in));
		      str = opt.readLine();
		      option = Integer.parseInt(str);
		    } catch (NumberFormatException ex) {
		      System.err.println("Not a valid number: " + str);
		    } catch (IOException e) {
		      System.err.println("Unexpected IO ERROR: " + e);
		    }
				    
		switch(option)
		{							   
		case 1: {
		clearData();
		Node resultBFS = BFS(goal,puzzle);
		if((resultBFS.state).equals(goal)) {			
			printSolution(stateHistory, resultBFS);
			System.out.println("BFS Complete and successful\n");			
			System.out.println("Maximum Queue Size: "+maxqueue);
			System.out.println("Number of Goal Test: "+goaltest);
		} break;
		}
		
		case 2: {
		clearData();
		Node resultBFSv = BFSvisited(goal,puzzle);		
		if((resultBFSv.state).equals(goal)) {				
			printSolution(stateHistory, resultBFSv);
			System.out.println("BFS Visited Complete and successful");
			System.out.println("Maximum Queue Size: "+maxqueue);
			System.out.println("Number of Goal Test: "+goaltest);
		} break;
		}
				
		case 3: {
		clearData();			
		Node resultDFSv = DFSvisited(goal,puzzle);		
		if((resultDFSv.state).equals(goal)) {				
			printSolution(stateHistory, resultDFSv);
			System.out.println("DFS Visited Complete and successful");
			System.out.println("Maximum Queue Size: "+maxqueue);
			System.out.println("Number of Goal Test: "+goaltest);
        } break;
		}
		
		case 4: {
        clearData();
		Node resultDLS = DLS(goal,puzzle,30);		
		if((resultDLS.state).equals(goal)) {				
			printSolution(stateHistory, resultDLS);
			System.out.println("Depth-Limited DFS Complete and successful");
			System.out.println("Maximum Queue Size: "+maxqueue);
			System.out.println("Number of Goal Test: "+goaltest);
	    } break;
		}	    
		
		case 5: {
		clearData();
		Node resultIter = IterDeepening(goal,puzzle);		
		if((resultIter.state).equals(goal)) {				
			printSolution(stateHistory, resultIter);
			System.out.println("Iterative Deepening Complete and successful");
			System.out.println("Maximum Queue Size: "+maxqueue);
		} break;
		}	
		default:			
			System.exit(1);
		}
     }  
	}

	static Node BFS(String goal,String puzzle){	
		int iteration=1;		
        String newState;
        Node init = new Node(puzzle,0);
		frontier.add(init);
		maxqueue=1;


		
		while (!frontier.isEmpty())  
		{   
			init=frontier.remove();
			goaltest=iteration;
			System.out.println("iteration: "+iteration+", queue: "+frontier.size()+", depth: "+(init.depth)+"\n");
			if(goalTest(init.state,goal)) {
				System.out.println("Solution Found");				
			    return init;
			}
							
			newState= moveUp(init.state);
			createNodeBFS(newState,init);			                       
			newState= moveDown(init.state);			
			createNodeBFS(newState,init);			
			newState= moveLeft(init.state);
			createNodeBFS(newState,init);					
			newState= moveRight(init.state);
			createNodeBFS(newState,init);			
			iteration++;
		}	
		System.out.println("No Solution found by BFS. All moves exhausted");		
		return init;
	}

	static Node BFSvisited(String goal,String puzzle){	
		int iteration=1;		
        String newState;
        Node init = new Node(puzzle,0);
		frontier.add(init); 	
		stateVisited.add(init.state);
		frontier.add(init);		
		maxqueue=1;
			
		while (!frontier.isEmpty())
		{   
			init=frontier.remove();
			goaltest=iteration;
			System.out.println("iteration: "+iteration+", queue: "+frontier.size()+", depth: "+(init.depth)+"\n");
			if(goalTest(init.state,goal)) {
				System.out.println("Solution Found");				
			    return init;
			}

							
			newState= moveUp(init.state);
			createNodeBFSv(newState,init);			                       
			newState= moveDown(init.state);			
			createNodeBFSv(newState,init);			
			newState= moveLeft(init.state);
			createNodeBFSv(newState,init);					
			newState= moveRight(init.state);
			createNodeBFSv(newState,init);			
			iteration++;
		}			
		System.out.println("No Solution found by BFS Visited. All moves exhausted");		
		return init;
	}

	static Node DFSvisited(String goal,String puzzle){	
		int iteration=1;		
        String newState;
        Node init = new Node(puzzle,0);
		frontierSt.add(init); 	
		maxqueue=1;
		stateVisited.add(init.state);			    
		
		while (!frontierSt.isEmpty()) {   
			init=frontierSt.pop();
			goaltest=iteration;
			System.out.println("iteration: "+iteration+", stack: "+frontierSt.size()+", depth: "+(init.depth)+"\n");			
			if(goalTest(init.state,goal)) {
				System.out.println("Solution Found");				
			    return init;
			}			
							
			newState= moveUp(init.state);
			createNodeDFSv(newState,init);			
			newState= moveDown(init.state);			
			createNodeDFSv(newState,init);			
			newState= moveLeft(init.state);
			createNodeDFSv(newState,init);					
			newState= moveRight(init.state);
			createNodeDFSv(newState,init);			
			iteration++;			
	}
		System.out.println("No Solution found by DFS Visited. All moves exhausted");		
		return init;
	}
	
        static Node DLS(String goal,String puzzle,int limit){	
		int iteration=1;		
        String newState;
        Node init = new Node(puzzle,0);		                
        frontierSt.add(init); 	
        maxqueue=1;
		stateVisited.add(init.state);			    
		
		while (!frontierSt.isEmpty()){			
			init=frontierSt.pop();
			goaltest=iteration;
			if (init.depth>=limit)
				continue;
			System.out.println("iteration: "+iteration+", stack: "+frontierSt.size()+", depth: "+(init.depth)+"\n");			
			if(goalTest(init.state,goal)) {
				System.out.println("Solution Found");
			    return init;
			}			
							
			newState= moveUp(init.state);
			createNodeDFSv(newState,init);			
			newState= moveDown(init.state);			
			createNodeDFSv(newState,init);			
			newState= moveLeft(init.state);
			createNodeDFSv(newState,init);					
			newState= moveRight(init.state);
			createNodeDFSv(newState,init);			
			iteration++;			
	}
		System.out.println("No Solution found by DLS. Maximum Depth "+limit+" reached");		
		return init;
	}

	static Node IterDeepening(String goal,String puzzle){					    					
		boolean fl=false;
		Node n=null;
		int depth=0;
		int goalTestIt=0;
		while(!fl) {		 
			n = DLS(goal,puzzle,depth);
			goalTestIt=goalTestIt+goaltest;
			if((n.state).equals(goal)) {
			  fl=true;
			  System.out.println("Number of goal Test: "+goalTestIt);
			  return n;
			}
			clearData();
			depth++;
		}		
		return n;
	}	
	
	static String scramble(String goal, int number) {
		String puzzle=goal;
		Random randomGenerator = new Random();
		for(int i=0;i<number;i++) {
			int randomInt = randomGenerator.nextInt(4);			
			switch(randomInt){			
			case 0:
				puzzle=moveUp(puzzle);
				break;
			case 1:
				puzzle=moveDown(puzzle);
				break;
			case 2:
				puzzle=moveLeft(puzzle);
				break;
			case 3:
				puzzle=moveRight(puzzle);
				break;				
			default:
				System.out.println("Faulty scramble function");
				System.exit(1);
		 }	
		}
		return puzzle;		
	}
		
	static boolean goalTest(String node, String goal) {	  
	  if(node.equals(goal))	 
		  return true;	 
	  else 		  
		  return false;	  
	}
	
	static String moveUp(String currState) {
		int posn=currState.indexOf(".");
		if(posn>2) {
			String newState = currState.substring(0,posn-3)+"."+currState.substring(posn-2,posn)+currState.charAt(posn-3)+currState.substring(posn+1);
			return newState;
		} else
		return currState;		
	}
	
	static String moveDown(String currState) {
		int posn=currState.indexOf(".");
		if(posn<6) {
			String newState = currState.substring(0,posn)+currState.substring(posn+3,posn+4)+currState.substring(posn+1,posn+3)+"."+currState.substring(posn+4);
			return newState;
		} else
			return currState;
	}
		
	static String moveLeft(String currState) {
			int posn=currState.indexOf(".");
			if(posn!=0 && posn!=3 && posn!=6) {
				String newState=currState.substring(0,posn-1)+"."+currState.charAt(posn-1)+currState.substring(posn+1);
				return newState;
			} else
				return currState;		
	}
	
	static String moveRight(String currState) {
		int posn=currState.indexOf(".");
		if(posn!=2 && posn!=5 && posn!=8) {
			String newState=currState.substring(0,posn)+currState.charAt(posn+1)+"."+currState.substring(posn+2);
			return newState;
		} else
		return currState;
	}
	
	static void printSolution(Map<Node,Node> hist, Node start) {		
		Node temp=start;
		System.out.println();
		List<String> vstore = new ArrayList<String>();
		vstore.add(temp.state);	
		while(!((temp.state).equals(puzzle)))
		{ 						
			temp=hist.get(temp);				
			vstore.add(temp.state);			
		}		
		for(int i=((vstore.size())-1);i>=0;i--)	
		{
			String pr = vstore.get(i);
			printState(pr);			
		}
	}
	
	static void printState(String pr) {
		for(int j=0;j<3;j++)				
	    	System.out.print(pr.charAt(j)+" ");
		System.out.println();
		for(int j=3;j<6;j++)
			System.out.print(pr.charAt(j)+" ");
		System.out.println();
		for(int j=6;j<9;j++)
			System.out.print(pr.charAt(j)+" ");
		System.out.println("\n");			
	}

	static void createNodeBFS(String newState,Node init) {
		Node child = new Node(newState,init.depth+1);
		frontier.add(child);
        stateHistory.put(child,init);		        
        if(maxqueue<frontier.size())
        	maxqueue=frontier.size();
	}
	
	static void createNodeBFSv(String newState, Node init) {
    		Node child=new Node(newState,init.depth+1);
    		if(!(stateVisited.contains(child.state))){
		    frontier.add(child);	       			  		   			   
		    stateHistory.put(child,init);
		    stateVisited.add(child.state);
		    if(maxqueue<frontier.size())
	        	maxqueue=frontier.size();
    		}	    	
	}
	
	static void createNodeDFSv(String newState, Node init)	{
		Node child=new Node(newState,init.depth+1);
		if(!(stateVisited.contains(child.state))){			
	    frontierSt.push(child);	       			  		   			   
	    stateHistory.put(child,init);
	    stateVisited.add(child.state);
	    if(maxqueue<frontierSt.size())
        	maxqueue=frontierSt.size();
     }
	}
	
	static void clearData() 	{
		frontier.clear();
		stateHistory.clear();
		stateVisited.clear();
		frontierSt.clear();
		maxqueue=0;
		goaltest=0;
	}		
}
