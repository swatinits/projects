package asgn;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.*;

class StateComparator implements Comparator<State>
{
	public int compare(State x, State y)
    {
        if (x.fScore < y.fScore){
            return -1;
        }
        if (x.fScore > y.fScore){
            return 1;
        }
        return 0;
    }	
}

class State {				
	ArrayList<ArrayList<Integer>> sr;
	int gScore=0;
	int hScore=0;
	int fScore=0;
    
	public State(){
		sr=new ArrayList<ArrayList<Integer>>();
	}
		
	State copyState(){
		State s=new State();		
		for(int i=0;i<this.sr.size();i++){
			s.sr.add(new ArrayList<Integer>());
			for(int j=0;j<this.sr.get(i).size();j++)
				s.sr.get(i).add(this.sr.get(i).get(j));
		}
		s.gScore=this.gScore;
		s.hScore=this.hScore;
		s.fScore=this.fScore;
		return s;
	}
		
	void setGScore(int gScore) {
	this.gScore=gScore;
	}
	void setHScore(int hScore) {
		this.hScore=hScore;
	}
	void setFScore() {
		fScore=gScore+hScore;
	}
}

public class Homework2 {
	static int nStack=0,nElement=0;	;
	
	public static void main(String args[]) {
		String str="";			
		int option=0;
		
		try {
		      System.out.println("Input the number of stacks required");				
		      BufferedReader is = new BufferedReader(new InputStreamReader(System.in));
		      str = is.readLine();
		      nStack = Integer.parseInt(str);
		      System.out.println("Input the total number of elements required");
		      is = new BufferedReader(new InputStreamReader(System.in));
		      str = is.readLine();
		      nElement = Integer.parseInt(str);
		    } catch (NumberFormatException ex) {
		      System.err.println("Not a valid number: " + str);
		    } catch (IOException e) {
		      System.err.println("Unexpected IO ERROR: " + e);
		    }		
		
		if(nStack<=1) {
			System.out.println("Number of stacks should be greater than 1");
			return;
		}		
		if(nElement<=0) {
			System.out.println("Number of elements should be greater than 0");
			return;
		}	
				
		State initial=generatePuzzle(nStack,nElement);
		State goal = generateGoal(nStack,nElement);
		System.out.println("\nInitial state:");
		printState(initial);
		System.out.println("\nGoal state:");
		printState(goal);						
		
        while(true) {			
			System.out.println("\nSelect a number to select the heuristics to reach the goal state\n");
			System.out.println("1. Number of blocks out of place h(0)");			
			System.out.println("2. New heuristics h(1)");			
			System.out.println("3. Enter any other key to Exit");
			try {
			      BufferedReader opt = new BufferedReader(new InputStreamReader(System.in));
			      str = opt.readLine();
			      option = Integer.parseInt(str);
			    } catch (NumberFormatException ex) {
			      System.err.println("Not a valid number: " + str);
			    } catch (IOException e) {
			      System.err.println("Unexpected IO ERROR: " + e);
			    }
					   
			switch(option) {							   
			case 1: {		
				aStarHeur(initial,goal,1);
				break;
			}			
			case 2: {				
				aStarHeur(initial,goal,2);
				break;			
			}			
			default:
				return;
			}
    }		
	}
	
	static void aStarHeur(State initial, State goal,int heur) {
		int depth=0,maxQSize=0;		
		int goaltest=0;
	    State init=initial.copyState();						
		Comparator<State> comparator = new StateComparator();
	    PriorityQueue<State> openSet = new PriorityQueue<State>(1,comparator);
	    ArrayList<State> visited = new ArrayList<State>();  
		Map<State,State> cameFrom = new HashMap<State,State>();		
		init.setGScore(0);
		init.setHScore(calcBlock(initial));
		init.setFScore();
		openSet.add(init);		
		visited.add(init);
 		while (!openSet.isEmpty()){ 	
 			if(maxQSize<openSet.size())
 				maxQSize=openSet.size();
			State current=openSet.poll();					
			goaltest++;
			if(goaltest>5000)
				break;
			System.out.println("goaltest :"+goaltest);
			if(compareStates(current,goal)){			
			 System.out.println("Solution found\n");			 
			 depth=retrace(current,initial,cameFrom);
			 System.out.println("Solution at depth: "+depth);
			 System.out.println("No of goal tests: "+goaltest);
			 System.out.println("Maximum Queue Size "+maxQSize);
			 return;
			}										
			State st=current.copyState();				
			for(int i=0;i<nStack;i++){
				int size=st.sr.get(i).size();
				if(size==0)
					continue;
				int top=st.sr.get(i).get(size-1);
				State s1=st.copyState();
				s1.sr.get(i).remove(size-1);
										
				for(int j=0;j<nStack;j++){
					boolean visit=false;
					State s=s1.copyState();
					if(i!=j){														   
						s.sr.get(j).add(top);										
						for(int l=0;l<visited.size();l++){
							if(compareStates(visited.get(l),s)){
									visit=true;								
									break;
							}
						}
						if(visit)														
							continue;				
					    s.setGScore(current.gScore+1);
					    if(heur==1)
					       s.setHScore(calcBlock(s));
					    else if(heur==2)
					       s.setHScore(newHeur(s));					    					    
					    s.setFScore();				    
					    openSet.add(s);
					    visited.add(s);
					    cameFrom.put(s,current);					    						
				}
			}			
			}					
		}				
		System.out.println("Solution not found\n");
		return;
	}			
    
	static State generatePuzzle(int nStack, int nElement) {	    
		State newst = new State();
		for(int i=0;i<nStack;i++)
			newst.sr.add(new ArrayList<Integer>());		    			       		    
	    	Random randomGenerator = new Random();
			int randomInt=0;				
			for(int i=1;i<=nElement;i++){			
			    randomInt = randomGenerator.nextInt(nStack);		    		    
			    (newst.sr.get(randomInt)).add(i);			
			}																     
			return newst;
	}
	
	static State generateGoal(int nStack, int nElement) {
		State goal=new State();
		ArrayList<Integer> first=new ArrayList<Integer>();
		for(int i=1;i<=nElement;i++)
			first.add(i);
		goal.sr.add(first);
		for(int i=1;i<nStack;i++)
			goal.sr.add(new ArrayList<Integer>());		
		return goal;
	}
	
	static void printState(State state){
		for(int i=0;i<nStack;i++){
			System.out.print("Stack "+(i+1)+" : ");
			for(int j=0;j<((state.sr.get(i)).size());j++)					
				System.out.print((state.sr.get(i)).get(j)+ ", ");			
		    System.out.println();
		}		
		System.out.println();
	}
		
	static int calcBlock(State st){
		int count=0;
		for(int i=1;i<=st.sr.get(0).size();i++){
	    	if(!(i==st.sr.get(0).get(i-1)))
	    		count++;	            	    
		}
		count=count+nElement-st.sr.get(0).size();
		return count;
	}
			
	static int newHeur(State st){
		int count=0,sum=0;
		for(int i=1;i<=st.sr.get(0).size();i++){
	    	if(!(i==st.sr.get(0).get(i-1)))	    		    		
	    		break;	    	
	    	count++;
		}		
		sum=(st.sr.get(0).size()-count)+(nStack-1)*(nElement-count);
		for(int i=1;i<nStack;i++){	
			    if(st.sr.get(i).size()==0)
			    	continue;		
				int min=Collections.min(st.sr.get(i));				
				int pos=st.sr.get(i).indexOf(min);
		        count=st.sr.get(i).size()-pos-1;
		        sum=sum+count;
	    }
		return sum;	
	}
	
	static int retrace(State st, State init, Map<State,State> cameFrom){
		State temp=st;
		int dep=0;
		State initial=init.copyState();		
		ArrayList<State> vstore = new ArrayList<State>();		
		vstore.add(temp);			
		while(!compareStates(temp,initial)) { 						
			temp=cameFrom.get(temp);				
			vstore.add(temp);
			dep++;
		}		
		for(int i=((vstore.size())-1);i>=0;i--)			
			printState(vstore.get(i));			
		return dep;
	}
	
	static boolean compareStates(State a,State b){
		boolean flag=true;
		if(a.sr.size()!=b.sr.size()){
			flag=false;
		    return flag;
		}
		for(int i=0;i<a.sr.size();i++){
			if(a.sr.get(i).size()!=b.sr.get(i).size()){
				flag=false;
			    return flag;
			}
			for(int j=0;j<a.sr.get(i).size();j++){
				if(a.sr.get(i).get(j)!=b.sr.get(i).get(j)){
					flag=false;
				    return flag;	
				}									
			}
		}
		return flag;
	}	
}
