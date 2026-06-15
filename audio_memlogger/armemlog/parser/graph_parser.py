#    Graph Parser file for mem logger
#    Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
#    SPDX-License-Identifier: BSD-3-Clause-Clear
from struct import iter_unpack
import sys
import os
import memLoggerUtils
import ctypes

util = memLoggerUtils.MemLoggerUtil()

def parseGraphBin(inFile, display, outputDir):
    if "queue" in inFile:
        parseGraphQueueBin(inFile, display, outputDir)
    elif "statbuf" in inFile:
        parseGraphStatbufBin(inFile, display, outputDir)

def parseGraphStatbufBin(inFile, display, outputDir):
    STATBUF_FORMAT = util.getFormat("GRAPH_STATBUF")
    output_file_name = os.path.splitext(os.path.basename(inFile))
    file = open(inFile, 'rb').read()
    original_stdout = sys.stdout # Save a reference to the original standard output

    if not display:
        path = os.path.join(outputDir, output_file_name[0]+".txt")
        output = open(path, 'w')
        sys.stdout = output

    print(f'-----------------------------------------')
    print(f'-----------GRAPH-STATBUF-STATS-----------')
    for idx, qItem in enumerate(iter_unpack(STATBUF_FORMAT, file)):
        writeStatbufItem(idx, qItem)

    if not display:
        output.close()
        sys.stdout = original_stdout

def parseGraphQueueBin(inFile, display, outputDir):
    QUEUE_FORMAT = util.getFormat("GRAPH_QUEUE")
    output_file_name = os.path.splitext(os.path.basename(inFile))
    file = open(inFile, 'rb').read()
    original_stdout = sys.stdout # Save a reference to the original standard output

    if not display:
        path = os.path.join(outputDir, output_file_name[0]+".txt")
        output = open(path, 'w')
        sys.stdout = output

    print(f'-----------------------------------------')
    print(f'------------GRAPH-QUEUE-STATS------------')
    for qItem in iter_unpack(QUEUE_FORMAT, file):
        writeQueueItem(qItem)
        
    if not display:
        output.close()
        sys.stdout = original_stdout

def writeQueueItem(item):
    timeStamp, state, result, streamHandle = item
    date = util.getTimeStamp(timeStamp)
    print(f'-----------------------------------------')
    print(f'Timestamp:....{date}')
    print(f'State:........{util.getEnumToString("graph_state", state)}')
    print(f'Result:.......{util.getEnumToString("exit_codes", -ctypes.c_int32(result).value)}')
    print(f'Graph Handle:{hex(int(streamHandle))}')
    print(f'-----------------------------------------')

def writeStatbufItem(idx, item):
    print(f'{util.getEnumToString("graph_state", idx)} Failures:....{item[0]}')

def main():
    args = util.argparser()
    # Get the current working directory
    cwd = os.getcwd()
    outputDir = os.path.join(cwd, util.createLogDir())
    #get file path
    full_path = os.path.realpath(__file__)
    path, filename = os.path.split(full_path)
    os.chdir(path)
    parseGraphBin(args.fileName, args.display, outputDir)


if __name__ == "__main__":
   main()