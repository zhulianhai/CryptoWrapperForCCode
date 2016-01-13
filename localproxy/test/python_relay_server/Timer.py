import threading
import os
#import win32api

def sayhello():
	print "create  process"
#	handle = win32process.CreateProcess('E:\\workspace\\LocalProxy\\build\\dbin\\Test.exe', '', None , None , 0 , win32process. CREATE_NO_WINDOW , None , None , win32process.STARTUPINFO())

	os.system('E:\\workspace\\bak\\LocalProxy8-11\\build\\dbin\\Test.exe')
#	win32process.TerminateProcess(handle, 0)
	global t        #Notice: use global variable!
	t = threading.Timer(5.0, sayhello)
	t.start()

t = threading.Timer(5.0, sayhello)
t.start()