import ThreadRunner
import time
import json
import string

DURATION = 100000000


# Example: run SorterHunter on two machines, one with 16 threads and one with 8.

# "ssh -t" option: used to allow SIGHUP to reach remote process

joblist = [['ssh','-t','192.168.0.123',"cd","/home/username/SortingNetworks/SorterHunter64;","./SorterHunter","sample.txt"]] * 16
joblist+= [['ssh','-t','192.168.0.246',"cd","/home/username/SortingNetworks/SorterHunter64;","./SorterHunter","sample.txt"]] * 8



bestfinds=[]

def tryImproved(r):
	global bestfinds
	Nr, Lr, Dr = r['N'],r['L'],r['D']
	ok=True
	for b in bestfinds:
		Nb, Lb, Db = b['N'],b['L'],b['D']
		if Nb>=Nr and Lb<=Lr and Db<=Dr:
			ok=False
			break
	if ok:
		nbf=[r]
		for b in bestfinds:
			Nb, Lb, Db = b['N'],b['L'],b['D']
			ok=True
			for r in nbf:
				Nr, Lr, Dr = r['N'],r['L'],r['D']
				if Nb<=Nr and Lb>=Lr and Db>=Dr:
					ok=False
					break
			if ok:
				nbf.append(b)
		bestfinds = nbf
		print("\nBest so far:")
		for b in bestfinds:
			print(b)
				


def processResult(l):
	if len(l)>1 and l[0]=='{' and l[-1]=='}':
		# Input line reporting a sorting network, let's try to use something existing for once instead of writing our own parser
		translation = l.maketrans("'()",'"[]')  # Input is close to JSON, but not exactly: single quotes and "tuple" syntax need to be replaced
		s=l.translate(translation)
		r=json.loads(s)
		if 'nw' in r:
			r['nw'] = [tuple(p) for p in r['nw']] # After parsing, restore tuples
		tryImproved(r)




# Create thread objects
threadlist=[]
for j in joblist:
	tr=ThreadRunner.ThreadRunner()
	tr.start(j, logfile=None)
	threadlist.append(tr)
lc = [0]*len(threadlist)


# Poll results

t1 = time.time()

while time.time() < t1+DURATION:
	for i in range(len(threadlist)):
		tr = threadlist[i]
		lines = tr.pollOutputLines()
		for l in lines:
			lc[i]+=1
			processResult(l)
	time.sleep(0.1)

# Kill them all
for tr in threadlist:
	tr.stop()
