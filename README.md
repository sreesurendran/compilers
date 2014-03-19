RA
==

vecruntime.cpp
1. generate intel vectorization reports for loops which are (a) not vectorized and (b) partially vectorized
2. get hot spots of the program using perfexpert
3. get the intersection of 1 and 2
4. run macpo for the intersection set generated in 3
5. store information from intel, macpo and hotspots into perfexpert
6. run the perfexpert framework to generate recommendations and change code
7. 
