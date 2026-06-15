#    KPI Parser file for mem logger
#    Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
#    SPDX-License-Identifier: BSD-3-Clause-Clear
from struct import unpack, iter_unpack, calcsize
from datetime import datetime
import sys
import memLoggerUtils
import os

util = memLoggerUtils.MemLoggerUtil()

def parseKPIBin(inFile, display, outputDir):

    FORMAT = util.getFormat("KPI_QUEUE")
    kpiDic = {}
    enterTS =""
    output_file_name = os.path.splitext(os.path.basename(inFile))
    file = open(inFile, 'rb').read()
    original_stdout = sys.stdout # Save a reference to the original standard output
    openStreams = []

    if not display:
        path = os.path.join(outputDir, output_file_name[0]+".txt")
        output = open(path, 'w')
        sys.stdout = output

    print(f'-----------------------------------------')
    print(f'------------KPI QUEUE-STATS--------------')
    for qItem in iter_unpack(FORMAT, file):
        #writeItem(qItem)

        #logic to get kpi cumulative info
        func_name = qItem[2].decode("utf-8")
        if func_name in kpiDic.keys():
            #store entry ts
            if qItem[3]:
                enterTS = qItem[0]
            else:
                diffs = kpiDic[func_name]
                diffs.append(qItem[0] - enterTS)
                kpiDic[func_name] = diffs
        else:
           kpiDic[func_name] = ([])
           enterTS = qItem[0]
    print(f'-----------------------------------------')
    for key in kpiDic.keys():
        clean_key = key.replace("\00", "")
        print(f'-----------------------------------------')
        print(f'{clean_key}')
        print(f'\tnumber of occurences {len(kpiDic[key])} ')
        if (len(kpiDic[key]) > 0):
            print(f'\taverage time in milliseconds  {sum(kpiDic[key])/len(kpiDic[key])} ')

    if not display:
        output.close()
        sys.stdout = original_stdout
        print('generating file '+ outputDir + "\\" + output_file_name[0] +'.txt')

def writeItem(item):
    timeStamp, pid, func_name, type = item
    date = util.getTimeStamp(timeStamp)
    entry = "enter"
    if(type):
        entry="exit"
    if "pal_set_param" not in str(func_name):
        print(f'-----------------------------------------')
        print(f'Timestamp:....{date}')
        print(f'Function.:....{func_name.decode("utf-8")}')
        print(f'PID:..........{pid}')
        print(f'type:.........{entry}')
        print(f'-----------------------------------------')

def main():
    args = util.argparser()
    # Get the current working directory
    cwd = os.getcwd()
    outputDir = os.path.join(cwd, util.createLogDir())
    #get file path
    full_path = os.path.realpath(__file__)
    path, filename = os.path.split(full_path)
    os.chdir(path)
    parseKPIBin(args.fileName, args.display, outputDir)


if __name__ == "__main__":
   main()