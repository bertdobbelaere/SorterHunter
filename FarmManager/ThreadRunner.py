import subprocess, os


class ThreadRunner:
	def __init__(self):
		self.proc = None
		self.buffer = []
		self.logfile = None
	def start(self, args, logfile=None):
		if self.proc==None:
			""" What about bufsize ?  0 misses some beginnings of lines, -1 sometimes breaks off before line end, same with 1 
			    Sadly, 'unbuffer' seems needed to avoid line buffering problems (installation: "sudo apt install expect"). Would be nice to solve this without intermediate.
			""" 
			if type(args)==list:
				ab = ['unbuffer']+args
			elif type(args)==str:
				ab = "unbuffer "+args
			self.proc = subprocess.Popen(ab, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True, bufsize=0)
			os.set_blocking(self.proc.stdout.fileno(), False)
			if logfile!=None:
				self.logfile = open(logfile,'a')

	def pollOutputLines(self):
		rv=[]
		if self.proc!=None: 
			try:
				while True:
					out = self.proc.stdout.readline()
					if out==None or out=='':
						break
					if self.logfile!=None:
						self.logfile.write(out)
					self.buffer.append(out.strip())
			except:
				pass
			rv = self.buffer
			self.buffer= []
		return rv
	def stop(self):
		if self.proc!=None:
			self.proc.kill()
			self.proc = None
		self.buffer=[]
		if self.logfile!=None:
			self.logfile.close()
		
