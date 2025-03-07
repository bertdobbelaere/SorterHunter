"""
	nw_tool.py
	Utility script for sorting network verification.
	
	Copyright (c) 2025 Bert Dobbelaere
	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
"""

def linearToLayers(input_lst):
	"""
	Converts a linear network representation as pairs into a layer representation.
	"""
	outlst=[]
	lblstate={}
	for (i,j) in input_lst:
		if not i in lblstate:
			lblstate[i]=0
		if not j in lblstate:
			lblstate[j]=0
		l=max(lblstate[i],lblstate[j])
		if l>=len(outlst):
			outlst.append([])
		outlst[l].append( (i,j) )
		lblstate[i]=lblstate[j]=l+1
	for i in range(len(outlst)):
		outlst[i]=sorted(outlst[i])
	return outlst

def layersToLinear(input_layers):
	"""
	Converts a layer representation into a linear network representation
	"""
	ol=[]
	for l in input_layers:
		ol += l
	return ol
	

class SorterException(Exception):
	def __init__(self, msg):
		self.msg=msg
		super().__init__(msg)

class SortingNetwork:
	"""
	Sorting network class supporting verification of a sorter using cluster merging, layered printing and symmetry checking.
	"""
	DEFAULTMAXPATTERNS=100 * 10**6
	maxpatterns=DEFAULTMAXPATTERNS
	
	def setMaxVerifierPatterns(n):
		SortingNetwork.maxpatterns=n
	
	def __init__(self, N, pairs=None):
		if not type(N)==int or N<=0:
			raise SorterException("Size (N) must be positive integer")
		self._N=N
		self._pairs=[] if pairs==None else pairs
		self._verifyStatus=None
	def _combineclusters(self,i,j,verbose=False):
		li,lj=self._patternlists[self._listidx[i]],self._patternlists[self._listidx[j]]
		if len(li)*len(lj) > SortingNetwork.maxpatterns:
			raise SorterException("Maximum amount of patterns exceeded. Try increasing maximum using SortingNetwork.setMaxVerifierPatterns")
		if verbose:
			print("Combining clusters containing inputs %d and %d, resulting in %d patterns"%(i,j,len(li)*len(lj)))
		lz=[]
		for p in li:
			for q in lj:
				lz.append( p|q)
		lz.sort()
		self._patternlists[self._listidx[i]]=lz
		self._patternlists[self._listidx[j]]=None
		oldidx=self._listidx[j]
		for k in range(self._N):
			if self._listidx[k]==oldidx:
				self._listidx[k]=self._listidx[i]
	def _applyCE(self,i,j,verbose=False):
		p=1<<i
		q=1<<j
		mask=p|q
		res=[]
		idxp=0
		idxnp=0
		patterns=self._patternlists[self._listidx[i]]
		l=len(patterns)
		last=-1
		while ((idxp<l) and ((patterns[idxp] &mask)!=p)):
			idxp+=1
		while ((idxnp<l) and ((patterns[idxnp]&mask)==p)):
			idxnp+=1
	
		while (idxnp<l) and (idxp<l):
			a=patterns[idxp]^mask
			b=patterns[idxnp]
			if a<b:
				if a!=last:
					res.append(a);
					last=a
				idxp+=1
				while ((idxp<l) and ((patterns[idxp]&mask)!=p)):
					idxp+=1
			else:
				if a!=last:
					res.append(b);
					last=b
				idxnp+=1
				while ((idxnp<l) and ((patterns[idxnp]&mask)==p)):
					idxnp+=1
		while idxnp<l:
			res.append(patterns[idxnp])
			idxnp+=1
		while idxp<l:
			res.append(patterns[idxp]^mask)
			idxp+=1
		self._patternlists[self._listidx[i]]=res;
		if verbose:
			print("Applied CE for %d and %d: %d patterns remaining in cluster"%(i,j,len(res)))
	def _outputPatternSize(self):
		p=1
		for c in self._patternlists:
			if c:
				p*=len(c)
		return p	
	def verifySorter(self, verbose=False):
		if verbose:
			print("Verifying network with %d inputs, %d CE's. Starting with %d clusters of 2 patterns, %d binary patterns combined"%(self._N,len(self._pairs),self._N,2**self._N))
		self._patternlists=[[0,1<<k] for k in range(self._N)]
		self._listidx=[k for k in range(self._N)]
		cnt=0
		worklist=list(self._pairs)
		while len(worklist)>0:
			(i,j) = worklist[0]
			if self._listidx[i]!=self._listidx[j]:
				self._combineclusters(i,j,verbose)
				# Move remaining accessible pairs within existing clusters to front of list: attempt to reduce working memory requirement
				l1=[]
				l2=[]
				visited=set()
				for (p,q) in worklist:
					if not (p in visited) and not (q in visited) and (self._listidx[p]==self._listidx[q]):
						l1.append( (p,q) )
					else:
						l2.append( (p,q) )
					visited.add(p)
					visited.add(q)
				worklist = l1+l2
			self._applyCE(i,j,verbose)
			if verbose:
				cnt+=1
				print("%d total patterns remaining after applying %d CE's"%(self._outputPatternSize(),cnt))
			worklist=worklist[1:]
		self._verifyStatus=self._outputPatternSize() == (self._N+1)
		if verbose:
			print("A valid sorter requires exactly %d remaining patterns at this point: %s"%(1+self._N, "PASS" if self._verifyStatus else "FAIL"))
		return self._verifyStatus
	def printLayered(self):
		layers=linearToLayers(self._pairs)
		print("Network with %d inputs, %d CE's and %d layers."%(self._N,len(self._pairs),len(layers)))
		print()
		for l in layers:
			print(l)
		print()
		if self._verifyStatus==True:
			print("Verified as valid sorting network.")
		if self.isSymmetric():
			print("Network is symmetric.")
	def isSymmetric(self):
		if self._N%2:
			return False
		layers=linearToLayers(self._pairs)
		for l in layers:
			for (i,j) in l:
				if not (self._N-1-j,self._N-1-i) in l:
					return False
		return True
	def getPairs(self):
		return list(self._pairs)
	def __str__(self):
		return "Network of %d inputs, %d elements %s"%(self._N,len(self._pairs),str(self._pairs))		
			
