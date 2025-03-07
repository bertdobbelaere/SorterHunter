""" Example file for nw_tool """

from nw_tool import *

# Known good 10 input sorter:
print("Case 1")

lst=[(0,8),(1,9),(2,7),(3,5),(4,6),(0,2),(1,4),(5,8),(7,9),(0,3),(2,4),(5,7),(6,9),(0,1),(3,6),(8,9),(1,5),(2,3),(4,8),(6,7),(1,2),(3,5),(4,6),(7,8),(2,3),(4,5),(6,7),(3,4),(5,6)]
s=SortingNetwork(10,lst)

rv=s.verifySorter(verbose=True)
print("Check result : ",rv)

s.printLayered()

print('-'*80)

# Broken sorter:
print("Case 2")


lst=[(0,8),(1,9),(2,7),(3,5),(4,5),(0,2),(1,4),(5,8),(7,9),(0,3),(2,4),(5,7),(6,9),(0,1),(3,6),(8,9),(1,5),(2,3),(4,8),(6,7),(1,2),(3,5),(4,6),(7,8),(2,3),(4,5),(6,7),(3,4),(5,6)]
s=SortingNetwork(10,lst)

rv=s.verifySorter(verbose=True)
print("Check result : ",rv)

s.printLayered()

