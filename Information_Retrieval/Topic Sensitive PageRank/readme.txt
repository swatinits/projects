==================================================================================================
README

Author: Swati Singh
==================================================================================================
Given, a dataset, a medium collection of tweets. json module is required which has been built in Python 2.7 to load data.
The tweet corpus in json format must be present in the same directory as these python files.
These programs have been tested on a corpus of about 100000 tweets

TOPICS:

	1. Brief Description of python files
	2. Dsicussion of three ranking systems
	3. Usage and Sample Run

-------------------------------------------------------------------------------------------	
	
1. Description of python files and how to run them
		
a. part1.py: It returns the top 50 tweets as per the query dependent tweet rank using tf-idf cosine similarity and vector space model. It is a vector space retrieval system ONLY based on tweets' texts. Each tweet's text is tokenized using whitespaces and punctuations as delimiters and stop words are not removed. The program returns results ordered by the cosine similarity between the tf-idf of the tweet text and the query. Tweets with higher similarity to the query should be first. If a query returns more than 50 results, only the first 50 are returned. If any of the tokens in the query do not occur in the tweet corpus, then the idf will be undefined and no results will be returned. 

b. part2.py: It returns the top 50 users based on the linear method for calculating PageRank. The classic PageRank algorithm is implemented on a tweet corpus. Rather than scoring each tweet, the PageRank score of users is calculated. First I build a graph structure based on @mentions: The edges are binary and directed. If Bob mentions Alice once, in 10 tweets, or 10 times in one tweet, there is an edge from Bob to Alice, but there is not an edge from Alice to Bob. The edges are unweighted. If Bob mentions Alice once, in 10 tweets, or 10 times in one tweet, there is only one edge from Bob to Alice. If a user mentions herself, ignore it. If a user is never mentioned and does not mention anyone, their pagerank should be zero. Do not include the user in the calculation. Assume all nodes start out with equal probability and the probability of the random surfer teleporting is 0.1 . 
		
c. part3.py: It returns the top 50 tweets based on query dependent tweet rank as well as the user rank who posted the tweet, an integrated tweet ranking system by integrating the cosine similarity (per tweet) and PageRank score (per user)
		
d. part4.py: It returns the top 10 tweets of each category based on query dependent tweet rank as well as the user rank who posted the tweet.
				
-------------------------------------------------------------------------------------------				
				
2. Dsicussion of three ranking systems with sample queries and sample run results
	
a. part1.py: Vector Space Model
The vector space model returns tweets based o the cosine similarity calculated for both tweets and the query and returns the matching pairs in decreasing order of similarity in the coine values. The models are based on "bag of words" model and do not take into account the order of the words.	For example, if sample query is "white bag with green grapes", then a document containing "green bag with white grapes" would be returned as highly relevant while it is not what the user is looking for. 					
			
b. part2.py: PageRank Algorithm
It ranks the users based on the link structure among the users, i.e, the number of incoming and outgoing links. It uses the linear method for caculating pagerank.	
pagerank[i] = d + Sum over all j ((1-d)*pagerank[j]/number of outlinks for j)
where j is a user who points to i
		
c: part3.py: Comnbination of VSM and PageRank Algorithm
It combines the Vector Space Model and the UserRank to rank the tweets. 
The tweets are first ranked based on the cosine simlarity value and then tweets with similiar cosine values are ranked based on the user ranks. BThis is to ensure that relevant results are given higher priority over the userranks. The userranks matters in case of results which are equally relevant to the query.
		
d. part4.py: Topic Sensitive PageRank Algorithm
For this assignment, since all tweets were concerning Mars, it was difficult to categorize the tweets and the users. So I have randomly divided the users into 4 categories. A user can belong to more than one category. While calculating pagerank, only those users in the same group who mention a user have considerable weightage while calculating userrank and users outside the group are ignored. 
		
-------------------------------------------------------------------------------------------					
								
3. Usage and Sample Run
	
a. part1.py: 
To run the program:  python part1.py
To Exit: Ctrl + C
Approximate Run Time: 14 seconds

Returns top 50 results that matched the query:
				
Enter search query 
mars rover
Tweet Rankings 

rank:  1 tweet id:  232308572124504064 tweet:  Mars Rover
rank:  2 tweet id:  232309643370717184 tweet:  Mars rover??
rank:  3 tweet id:  232316463300620288 tweet:  Mars Rover!!!
rank:  4 tweet id:  232316564811165697 tweet:  #Rover #Mars
rank:  5 tweet id:  232320614017413122 tweet:  Mars rover :)
rank:  6 tweet id:  232326618092290048 tweet:  Mars Rover
rank:  7 tweet id:  232331024892321793 tweet:  Mars rover!!!!
rank:  8 tweet id:  232331336210337792 tweet:  Mars Rover
rank:  9 tweet id:  232332221149745152 tweet:  mars rover '
rank:  10 tweet id:  232320669415776256 tweet:  Mars Curiosity Rover!!!
rank:  11 tweet id:  232331957239959553 tweet:  Mars rover Curiosity!
rank:  12 tweet id:  232333579210854400 tweet:  MARS CURIOSITY ROVER!
rank:  13 tweet id:  232311879807139840 tweet:  Rover on Mars
rank:  14 tweet id:  232318175323238401 tweet:  #rover curiosity
rank:  15 tweet id:  232329552477687809 tweet:  curiosity rover
rank:  16 tweet id:  232315252367323136 tweet:  mars rover landing!
rank:  17 tweet id:  232315686683303937 tweet:  Mars Rover landing
rank:  18 tweet id:  232319938973552640 tweet:  Mars rover landing!!!
rank:  19 tweet id:  232327603091017730 tweet:  Mars rover landing!
....
....
rank:  46 tweet id:  232324634782081024 tweet:  Watching the mars rover landing
rank:  47 tweet id:  232325686931640321 tweet:  watching the mars rover landing
rank:  48 tweet id:  232326041157394432 tweet:  Watching the Mars Rover Landing.
rank:  49 tweet id:  232313692539191296 tweet:  Mars rover Curiosity landing tonight :)
rank:  50 tweet id:  232324382612156417 tweet:  Curiosity! Mars rover landing! TONIGHT.
		
Enter search query 
planet red
Tweet Rankings 
rank:  1 tweet id:  232303837799665664 tweet:  Planet Mars =)
rank:  2 tweet id:  232321793803829249 tweet:  Is Mars red?
rank:  3 tweet id:  232322366053691392 tweet:  Mars landing soon!! #Ready #Red planet
rank:  4 tweet id:  232332665796313088 tweet:  @MarsCuriosity
Mars rover Curiosity closing in on red planet at 8,000 mph
rank:  5 tweet id:  232315008057495552 tweet:  Red planet rover, red planet rover, let curiosity come over #fb  ( @marscuriosity live at http://t.co/jIq6o1Ln)
rank:  6 tweet id:  232326083746349056 tweet:  90 minutes to #MSL and #Curiosity lands on the Red Planet.
rank:  7 tweet id:  232332702194487296 tweet:  @MarsCuriosity Hope you land safely on the Red Planet. #MSL
rank:  8 tweet id:  232314148615233536 tweet:  I am so excited to see Curiosity land on the red planet!
rank:  9 tweet id:  232313336421834752 tweet:  Getting ready to watch @MarsCuriosity land on the Red Planet!  Go NASA!
rank:  10 tweet id:  232322062075719680 tweet:  When @MarsCuriosity lands, we'll get to see more of how the Red Planet isn't all that red
rank:  11 tweet id:  232327071781756929 tweet:  #Curiosity #NASA ...we are getting closer to landing on the Red planet!!!
rank:  12 tweet id:  232321823155568640 tweet:  The red one #mars
rank:  13 tweet id:  232305183886020608 tweet:  Red wine &amp; red lights for the landing on the red planet! #Mars @MarsCuriosity http://t.co/Pl6mmzUG
rank:  14 tweet id:  232333023989870592 tweet:  Godspeed, @MarsCuriosity -Good luck on safe landing on the red planet!
rank:  15 tweet id:  232318235947716609 tweet:  Cant wait for Curiosity to land on that red planet. MARS
....
....
rank:  49 tweet id:  232333256236888064 tweet:  RT @NASA: #MSL: One hour until the landing of @MarsCuriosity on the red planet. Are you watching? http://t.co/qMeFERLo #MSL
rank:  50 tweet id:  232333256815693825 tweet:  RT @NASA: #MSL: One hour until the landing of @MarsCuriosity on the red planet. Are you watching? http://t.co/qMeFERLo #MSL
Enter search query 
	
**For the above example, even though the first result is not a perfect match, it has a higher cosine value because of smaller tweet size.
	
Enter search query 
geo
Tweet Rankings 
rank:  1 tweet id:  232331173743972352 tweet:  I have the Mars Rover feed going, but is it on TV anywhere? Nat Geo maybe?
rank:  2 tweet id:  232320131341103104 tweet:  Why the shit is http://t.co/7LJuJR5K on Nat Geo for the Mars mission?
rank:  3 tweet id:  232318328344039424 tweet:  Anybody want to see Mars? Nat Geo has started their live mars landing coverage. http://t.co/6S491NGA
rank:  4 tweet id:  232327500599001088 tweet:  Watching @NASAJPL attempt to crash land a pimped out Geo Tracker on Mars. T-minus 2 hours and falling. http://t.co/EghDxK2o
rank:  5 tweet id:  232324468490518528 tweet:  Nat Geo &amp; NASA are prepping for a Mars landing, while on AdultSwim BlackDynamite is going to the moon to stop OJ Simpson. Space is the place
		
Enter search query 
edkeodked
no match

b. part2.py: 
To run the program:  python part2.py		
Approximate Run Time: 18 seconds
	
Sample Run:
		
User Rankings 
rank:  1  user id:  61497333 user name:  ASTRO_SAL
rank:  2  user id:  90227660 user name:  24HorasTVN
rank:  3  user id:  24978242 user name:  rmaza2008
rank:  4  user id:  57748411 user name:  pamela_araya
rank:  5  user id:  300000982 user name:  1D_trackers
rank:  6  user id:  532975158 user name:  M_A_Larson
rank:  7  user id:  271129391 user name:  GuilleLobo
rank:  8  user id:  14476063 user name:  Nelsormensch
rank:  9  user id:  391944542 user name:  Ron_nn
rank:  10  user id:  91053979 user name:  js100radio
rank:  11  user id:  5623612 user name:  Greenskull
rank:  12  user id:  610545216 user name:  FaktaAstronomy
rank:  13  user id:  379232451 user name:  LovesLovato5
rank:  14  user id:  99357823 user name:  Toni_daTIGER
rank:  15  user id:  354209795 user name:  BreakdownCalvin
rank:  16  user id:  13623602 user name:  tekanet
rank:  17  user id:  756278 user name:  ChrisPirillo
rank:  18  user id:  15387870 user name:  ja2ke
rank:  19  user id:  198732594 user name:  MichaelSteeber
rank:  20  user id:  17049666 user name:  mtholyoke
rank:  21  user id:  335161606 user name:  KilamAll
rank:  22  user id:  52779910 user name:  ipeeintheWoods
rank:  23  user id:  210541257 user name:  pepiTOCH
rank:  24  user id:  429905502 user name:  AttivissimoLIVE
rank:  25  user id:  239909926 user name:  spacelivecast
rank:  26  user id:  104248138 user name:  mattderienzo
rank:  27  user id:  288038830 user name:  CTTechjunkie
rank:  28  user id:  193976920 user name:  rdbrewer4
rank:  29  user id:  362331386 user name:  Zingbot_3000
rank:  30  user id:  545720865 user name:  DianaPirillo
rank:  31  user id:  74441829 user name:  keinzantezuken
rank:  32  user id:  324798949 user name:  pittssuda
rank:  33  user id:  411679867 user name:  MaxHardcore100
rank:  34  user id:  352116388 user name:  AlianzaPorChile
rank:  35  user id:  174776928 user name:  elABC1
rank:  36  user id:  397646268 user name:  x_MrTwenty3
rank:  37  user id:  58715842 user name:  akuning
rank:  38  user id:  19024627 user name:  joshgreenman
rank:  39  user id:  104710814 user name:  bmkatz
rank:  40  user id:  441083353 user name:  TOPMUNDIALMUSI2
rank:  41  user id:  630890475 user name:  xiloveyoudemi
rank:  42  user id:  25943008 user name:  Radio2UE
rank:  43  user id:  223590246 user name:  WELinde
rank:  44  user id:  173669648 user name:  SteveUmstead
rank:  45  user id:  392473344 user name:  DavidJCobb
rank:  46  user id:  252953851 user name:  indrawankevin
rank:  47  user id:  256921389 user name:  xxMohannadxx
rank:  48  user id:  344349234 user name:  HighOff_Life_
rank:  49  user id:  58080086 user name:  ConceTV
rank:  50  user id:  171881026 user name:  mehtapkaraben
END
		
c. part3.py: 
To run the program:  python part3.py
To Exit: Ctrl + C
Approximate Run Time: 24 seconds

Enter search query 
mars rover					
rank:  1  tweetid:  232316564811165697  tweet:  #Rover #Mars
rank:  2  tweetid:  232326618092290048  tweet:  Mars Rover
rank:  3  tweetid:  232331024892321793  tweet:  Mars rover!!!!
rank:  4  tweetid:  232308572124504064  tweet:  Mars Rover
rank:  5  tweetid:  232332221149745152  tweet:  mars rover '
rank:  6  tweetid:  232331336210337792  tweet:  Mars Rover
rank:  7  tweetid: 232320614017413122  tweet:  Mars rover :)
rank:  8  tweetid:  232316463300620288  tweet:  Mars Rover!!!
rank:  9  tweetid:  232309643370717184  tweet:  Mars rover??
..
..
rank:  49  tweetid  232324382612156417  tweet:  Curiosity! Mars rover landing! TONIGHT.
rank:  50  tweetid  232313692539191296  tweet:  Mars rover Curiosity landing tonight :)
		
**The order of the results returned are different from the part1 results. iT consider both tweet similarity and userrank foe ranking tweets.				
		
d. part4.py: 
To run the program:  python part4.py		

 Group:  0 
rank:  0 name:  MarsCuriosity node:  15473958 val:  632.726026655
rank:  1 name:  NASA node:  11348282 val:  161.403327581
rank:  2 name:  iamwill node:  49573859 val:  75.2145076805
rank:  3 name:  davelavery node:  20612844 val:  70.1138353211
rank:  4 name:  NASA_ESPANOL node:  347420129 val:  42.5142109406
rank:  5 name:  1CatfishKnight1 node:  448269139 val:  36.4532487056
rank:  6 name:  BadAstronomer node:  4620451 val:  33.7058889268
rank:  7 name:  SethMacFarlane node:  18948541 val:  29.4193273964
rank:  8 name:  Astro_Flow node:  86336234 val:  20.5000932836
rank:  9 name:  MSL_101 node:  376949805 val:  16.8989572984

Group:  1 

rank:  0 name:  MarsCuriosity node:  15473958 val:  648.921669846
rank:  1 name:  NASA node:  11348282 val:  124.361095636
rank:  2 name:  iamwill node:  49573859 val:  88.2596045577
rank:  3 name:  davelavery node:  20612844 val:  82.1021348135
rank:  4 name:  BadAstronomer node:  4620451 val:  44.0156956509
rank:  5 name:  NASA_ESPANOL node:  347420129 val:  33.67702045
rank:  6 name:  nasahqphoto node:  18164420 val:  31.667414078
rank:  7 name:  NASAJPL node:  19802879 val:  28.1644037455
rank:  8 name:  marsroverdriver node:  15947277 val:  21.8631602802
rank:  9 name:  carsonmyers node:  72420267 val:  15.3906360295

Group:  2 

rank:  0 name:  NASA node:  11348282 val:  1252.17033552
rank:  1 name:  SekerekGerg node:  15536091 val:  1127.05328192
rank:  2 name:  starstryder node:  12702822 val:  54.8299887678
rank:  3 name:  BadAstronomer node:  4620451 val:  50.3792662149
rank:  4 name:  SciShow node:  397655438 val:  44.325198586
rank:  5 name:  vondellswain node:  40997767 val:  40.2626782671
rank:  6 name:  UrbanAstroNYC node:  733322844 val:  32.9571315104
rank:  7 name:  kelly_heather node:  66476989 val:  24.3081273016
rank:  8 name:  MSL_101 node:  376949805 val:  22.8476798752
rank:  9 name:  marsroverdriver node:  15947277 val:  21.7473726895

Group:  3 

rank:  0 name:  NASAJPL node:  19802879 val:  63.9639708287
rank:  1 name:  starstryder node:  12702822 val:  54.233320366
rank:  2 name:  jovemnerd node:  8515492 val:  53.689952798
rank:  3 name:  BadAstronomer node:  4620451 val:  50.9306356382
rank:  4 name:  FellipeC node:  34777829 val:  48.9909418487
rank:  5 name:  marsroverdriver node:  15947277 val:  29.7173275543
rank:  6 name:  UrbanAstroNYC node:  733322844 val:  28.9915730382
rank:  7 name:  MSL_101 node:  376949805 val:  25.0895506137
rank:  8 name:  BoingBoing node:  5971922 val:  17.2203901334
rank:  9 name:  milesobrien node:  11180212 val:  16.7460282449
END
			
	
