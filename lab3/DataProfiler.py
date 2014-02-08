#-----------------------------------------------------------------------------------------------
# Profiler script for reading information about user processes and to display graph for the same
# Information will be as follows:
# FSSP: TS:25, UID:504, PID:1234
#-----------------------------------------------------------------------------------------------
# 1. Open file
# 2. Get UID, PID and TS into Maps with UID as the Key and another Map as the value. 2nd map has PID as key and TS as value.

import matplotlib.pyplot as plt
import itertools
import re

# 0. Define uniq. order preserving
def uniq(inlist): 
	uniques = []
	for item in inlist:
		if item not in uniques:
			uniques.append(item)
	return uniques
	
# 1. Find all UIDs:
with open("C:\Ananth\OSU\Spring-2013\OS-lab\cse5433\lab3\log.txt", 'r') as f:
	data = f.read()
	UID = re.findall("UID:(\d+)", data)
	PID = re.findall("PID:(\d+)", data)

	
# 2. Find unique UIDs and PIDs
UID=uniq(UID)
PID=uniq(PID)

# 3. Create separate dictionaries for PID and TS
UID_PID={}
UID_TS={}

# 4. For each of the UIDs, assign an empty list. 
for u in UID:
	UID_PID[u] = {}
	UID_TS[u] = []
	
	
# 5. Now read each line from text file; Get TS, PID and UID. Assign TS and PID to the dictionary
totTs = 0
with open("C:\Ananth\OSU\Spring-2013\OS-lab\cse5433\lab3\log.txt", 'r') as f:
	data = f.readlines()
	for d in data:
		UIDTemp = re.findall("UID:(\d+)", d)
		UIDTemp = " ".join(UIDTemp)
		TS = re.findall("TS:(\d+)", d)
		TS = " ".join(TS)
		totTs += int(TS)
		PID = re.findall("PID:(\d+)", d)
		PID = " ".join(PID)
		if UIDTemp in UID_PID:
			if PID not in UID_PID[UIDTemp]:
				UID_PID[UIDTemp][PID] = 0
			UID_PID[UIDTemp][PID] += int(TS)
			UID_TS[UIDTemp].append(TS)
		else:
			UID_PID[UIDTemp] = PID
			UID_TS[UIDTemp] = TS

print(UID_PID)
for UID,PID in UID_PID.items():
	print("UID: ", UID)
	for PID_temp,TS in PID.items():
		print("PID: " ,PID_temp)
		print("TS:" ,TS)
	

# 6. Get the PID and TS values into a list
pidVal = []
tsVal = []
for key,value in UID_PID.items():
	pidVal.append(value)
for key,value in UID_TS.items():
	tsVal.append(value)
	
print ("totTS: ", totTs)
for uid,ts in UID_TS.items():
	print(uid,sum([float(x) for x in ts])/totTs)
	
	
	
# Find sum of TS for each UID:
sumTS = 0
sumList = []
for v in tsVal:
	sumTS = sum([int(x) for x in v])
	sumList.append(sumTS)
	print ("sumTS: " ,sumTS)
	sumTS = 0

# Plot sum of TS for each UID.
# plt.figure(1)
# plt.ylim([0,10000])
# plt.plot(UID, sumList)


# 7. Plot the PID (x-axis) against the TS (y-axis)
# c = 2	
# for x,y in itertools.izip(pidVal,tsVal):
	# plt.figure(c)
	# plt.plot(x,y)
	# print x
	# plt.plot(x)
	# print y
	# plt.plot(y)
	# c += 1
	
# plt.show()
	
# ====================================================================================	
# term = 'FSSP:'
# with open("C:\Ananth\OSU\Spring-2013\OS-lab\lab3\ProfileData.txt", 'r') as f:
	# data = f.read()
	# words = data.split()
	# if term in words:
		# print data
		
		
# re.search(r"(?<=UID:).*?(?=,)", words_new).group(0)

# for i in words:
	# type(i) -- string
	# re.split('\W+', i)
	

# with open("C:\Ananth\OSU\Spring-2013\OS-lab\lab3\ProfileData.txt", 'r') as f:
	# data = f.read()
	# TS = re.findall("TS:(\d+)", data)
	# UID = re.findall("UID:(\d+)", data)
	# PID = re.findall("PID:(\d+)", data)
	
# with open("C:\Ananth\OSU\Spring-2013\OS-lab\lab3\ProfileData.txt", 'r') as f:
	# data = f.readlines()
	# for d in data:
		# UID = re.findall("UID:(\d+)", d)
		# TS = re.findall("TS:(\d+)", d)
		# PID = re.findall("PID:(\d+)", d)
		# UID_PID[UID].append(PID)
		# UID_TS[UID].append(TS)
	
# def uniq(inlist): 
    # order preserving
    # uniques = []
    # for item in inlist:
        # if item not in uniques:
            # uniques.append(item)
    # return uniques
	
# import matplotlib.pyplot as plt
# plt.plot(TS)
# plt.show()


# for c in range(1,len(UID_PID)+1):
	# for key,value in UID_PID.items():
		# val1 = value
		# print val1
	# for key,value in UID_TS.items():
		# val2 = value
		# print val2
	# plt.figure(c)
	# plt.plot(val1,val2)
	
	