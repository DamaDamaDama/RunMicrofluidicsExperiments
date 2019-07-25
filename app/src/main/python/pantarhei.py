"""
Created on 3/27/2019

@author: Alan, Chaitanya, and David
"""

import numpy as np
import scipy as sp
import pandas as pd
from io import StringIO
from os.path import dirname, join
#import matplotlib.pyplot as plt
import sys
import experiments

# Builds a list of pump heights using variables Xc (Volume Fraction), lp (laminar position), Q0, and R (resistance)
# Returns: an array containing heights and Q
def getHeights(Xc, lp, Q0, R, motorcount):

    # code for basic 3 inlet chip only
    if(motorcount is 4):
        Q = np.column_stack((np.multiply(np.multiply(Xc,Q0),(1-lp)), np.multiply(np.multiply((1-Xc),Q0),(1-lp)), np.multiply(lp,Q0)))

        P = Q*R;
        heights = np.around(P/9.8,2)
        heights = np.delete(heights,list(range(0, heights.shape[0], 2)), axis=0)
        Q = np.delete(Q,list(range(0, Q.shape[0], 2)), axis=0)
    return np.append(heights, np.zeros((len(heights),1)), axis=1), Q

# Corrects pump heights
# Unsure why the corrections need to be made (as fluid drops, pressure changes, corrections are an attempt to account for this)
# Returns: a list containing the change in height
def getHeightCorrections(Q, delays):
    area = 3.14*np.square(0.006)
    delH = 1000*Q*delays[:, np.newaxis]/area
    delH = np.cumsum(delH, axis=0)
    return delH

# Packs up values into a single string to return back to app
# Returns: a single string
def packHeightsAndDelays(correctedHeights, delays, motorCount=4):
    total_len = len(correctedHeights) # Calculate how many times to iterate to encompass everything
    #This may be wrong...
    build_string = "" #Initialize it to empty string

    print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")
    print("First delay: " + str(delays[0]))
    print("Second delay: " + str(delays[1]))
    print("Third delay: " + str(delays[2]))
    print("First corrected Height for first motor: " + str(correctedHeights[0,0]))
    print("Second corrected Height for first motor: " + str(correctedHeights[1,0]))
    print("First corrected Height for 3rd motor: " + str(correctedHeights[0,2]))
    print("Second corrected Height for 3rd motor: " + str(correctedHeights[1,2]))

    print("The entire body of corrected heights data looks like this: ")
    print(correctedHeights)

    print("Delays is supposed to look like this: ")
    print(delays)

    print("Total length is purported to be: ")
    print(total_len)
    print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")

    #There is no switch statement in python. There's some wonky shit you can do with dictionaries...
    #but nah. if statements will cover 3-8 motors... Why would you ever use 1 or 2?
    for i in range(0, total_len):
        if(motorCount is 2):
            build_string += "1" + str(correctedHeights[i,0])[:6] + "_2" + str(correctedHeights[i,1])[:6] + "_9" + str(delays[i]) + "_"
        if(motorCount is 3):
            build_string += "1" + str(correctedHeights[i,0])[:6] + "_2" + str(correctedHeights[i,1])[:6] + "_3" + str(correctedHeights[i,2])[:6] + "_9" + str(delays[i]) + "_"
        if(motorCount is 4):
            build_string += "1" + str(correctedHeights[i,0])[:6] + "_2" + str(correctedHeights[i,1])[:6] + "_3" + str(correctedHeights[i,2])[:6] + "_4" + str(correctedHeights[i,3])[:6] + "_9" + str(delays[i]) + "_"
        if(motorCount is 5):
            build_string += "1" + str(correctedHeights[i,0])[:6] + "_2" + str(correctedHeights[i,1])[:6] + "_3" + str(correctedHeights[i,2])[:6] + "_4" + str(correctedHeights[i,3])[:6] + "_5" + str(correctedHeights[i,4])[:6] + "_9" + str(delays[i]) + "_"
        if(motorCount is 6):
            build_string += "1" + str(correctedHeights[i,0])[:6] + "_2" + str(correctedHeights[i,1])[:6] + "_3" + str(correctedHeights[i,2])[:6] + "_4" + str(correctedHeights[i,3])[:6] + "_5" + str(correctedHeights[i,4])[:6] + "_6" + str(correctedHeights[i,5])[:6] + "_9" + str(delays[i]) + "_"
        if(motorCount is 7):
            build_string += "1" + str(correctedHeights[i,0])[:6] + "_2" + str(correctedHeights[i,1])[:6] + "_3" + str(correctedHeights[i,2])[:6] + "_4" + str(correctedHeights[i,3])[:6] + "_5" + str(correctedHeights[i,4])[:6] + "_6" + str(correctedHeights[i,5])[:6] + "_7" + str(correctedHeights[i,6])[:6] + "_9" + str(delays[i]) + "_"
        if(motorCount is 8):
            build_string += "1" + str(correctedHeights[i,0])[:6] + "_2" + str(correctedHeights[i,1])[:6] + "_3" + str(correctedHeights[i,2])[:6] + "_4" + str(correctedHeights[i,3])[:6] + "_5" + str(correctedHeights[i,4])[:6] + "_6" + str(correctedHeights[i,5])[:6] + "_7" + str(correctedHeights[i,6])[:6] + "_8" + str(correctedHeights[i,7])[:6] + "_9" + str(delays[i]) + "_"
    return build_string[:-1] #Excise out the last _ added

# __main__ (if __name__ == '__main__': is not used because it may not work with android apps and chaquopy)
# Takes a experiment in .csv format, converts to a matrix, whose values are extracted and used to determine correct pump heights
# Returns: a single string in the format of "var_var_var_var" where the vars are various pump heights and delays.
def main(expfile, motorcount=4):

    # Scaling for 0.9 y = (m9)x + (b9) scaledXc[i,] = m9*rawXc[i,] + b9
    m9 = 1
    b9 = 0
    #Scaling for 0.3 y = (m3)x + (b3) scaledXc[i,] = m3*rawXc[i,] + b3
    m3 = 1
    b3 = 0

    # Define Resistance of capilary tubes
    # resistance for 4cm length of .000076mm cap tube is 6.533E12 (based on experimental measurment)
    # 1.633E12 resistance/cm of tube length
    #R = 6.533E12 #for 4cm tube
    R = 1.633E12 * 4

    #warnings.filterwarnings("ignore", category=FutureWarning) #Suppress warning related to .as_matrix() being deprecated soon. It will 100% not affect our usage
    # This script supports .csv, .xlsx, and .xls files only

    #For hardcodeded csv and xlsx files
    if expfile[-3:] == "csv":
        data = pd.read_csv("/data/user/0/com.LeeLab.runmicrofluidicsexperiments/app_experiments/" + expfile, sep=',', engine='python').as_matrix() # Removed from read_csv call: header=None, engine='Python'
    elif expfile[-4:] == "xlsx" or expfile[-3:] == "xls":
        data = pd.read_excel("/data/user/0/com.LeeLab.runmicrofluidicsexperiments/app_experiments/" + expfile, header=None).as_matrix()
    else:
        print("File type \"" + expfile[-4:] + "\" not supported")
        exit()

    if(motorcount is 4):
        time = data[:,0]
        Xc = data[:,1]
        lp = data[:,2]
        Q0 = data[:,3]
    #if(motorcount is ?):
        #rawTW = data[:,4]

    print(data)
    np.nan_to_num(data, copy=False)
    print("After NaN filter")
    print(data)

    #Xc = rawXc
    #lp = rawLP
    #TW = rawTW
    #lp9 = 0.9*np.ones((np.size(lp),))
    #lp = scaleXc(rawLP,1.5)

    heights, Q = getHeights(Xc, 1-lp, Q0, R, motorcount)

    delays = 60*np.diff(time)

    delays = delays[delays != 0] #What is the purpose of this line??? delays != 0 should evaluate to True, always.
    #That means you have delays[True] which is equivalent to delays[1]. The second
    #Element in the delays list will be the only delay set. If you had delays = [6, 3, 4, 9] in seconds
    #This line would convert it to delays = 3, every time

    corrections = np.append(getHeightCorrections(Q, delays), np.zeros((len(heights), 1)), axis=1)

    correctedHeights = heights + corrections

    np.nan_to_num(correctedHeights, copy=False) #Call this to convert any NaNs into 0s. This can happen if
                                                #Excel file etc had blank spaces instead of 0s, I think!
    if np.any(heights > 80):
        #print(heights) Some heights are easily over 80. Is this ok? Flaw in my refactoring? We also have NaNs, but I suspect it is due to xlsx file
        print('heights must be less than 80 mm! Reduce flow rate (Q0) or change LP!')

    experiment_data = packHeightsAndDelays(correctedHeights, delays)

    return experiment_data

#Commenting out matplotlib stuff for now to simplify things. Not going to worry about it right now.

#print("Experiment data packed is: " + main(sys.argv[1]))