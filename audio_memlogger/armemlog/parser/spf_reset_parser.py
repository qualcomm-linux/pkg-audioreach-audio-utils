#    SPF Reset Parser file for mem logger
#    Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
#    SPDX-License-Identifier: BSD-3-Clause-Clear
from struct import iter_unpack
import sys
import os
import memLoggerUtils

util = memLoggerUtils.MemLoggerUtil()

def parseSPFResetBin(inFile, display, outputDir):
    if "queue" in inFile:
        parseSPFResetQueueBin(inFile, display, outputDir)
    elif "statbuf" in inFile:
        parseSPFResetStatbufBin(inFile, display, outputDir)

def parseSPFResetStatbufBin(inFile, display, outputDir):
    STATBUF_FORMAT = util.getFormat("SPF_RESET_STATBUF")
    output_file_name = os.path.splitext(os.path.basename(inFile))
    file = open(inFile, 'rb').read()
    original_stdout = sys.stdout # Save a reference to the original standard output

    if not display:
        path = os.path.join(outputDir, output_file_name[0]+".txt")
        output = open(path, 'w')
        sys.stdout = output

    print(f'-----------------------------------------')
    print(f'---------SPF-RESET-STATBUF-STATS---------')
    for idx, qItem in enumerate(iter_unpack(STATBUF_FORMAT, file)):
        writeStatbufItem(idx, qItem)

def parseSPFResetQueueBin(inFile, display, outputDir):
    QUEUE_FORMAT = util.getFormat("SPF_RESET_QUEUE")
    output_file_name = os.path.splitext(os.path.basename(inFile))
    file = open(inFile, 'rb').read()
    original_stdout = sys.stdout # Save a reference to the original standard output

    if not display:
        path = os.path.join(outputDir, output_file_name[0]+".txt")
        output = open(path, 'w')
        sys.stdout = output

    print(f'-----------------------------------------')
    print(f'----------SPF-RESET-QUEUE-STATS----------')
    for qItem in iter_unpack(QUEUE_FORMAT, file):
        writeQueueItem(qItem)

def writeQueueItem(item):
    timeStamp, result = item
    date = util.getTimeStamp(timeStamp)
    print(f'-----------------------------------------')
    print(f'Timestamp:....{date}')
    print(f'Result:.......{util.getEnumToString("spf_reset_state", result)}')
    print(f'-----------------------------------------')

def writeStatbufItem(idx, item):
    print(f'SPF Reset {util.getEnumToString("spf_reset_state", idx)} Instances:....{item[0]}')

def main():
    args = util.argparser()
    # Get the current working directory
    cwd = os.getcwd()
    outputDir = os.path.join(cwd, util.createLogDir())
    #get file path
    full_path = os.path.realpath(__file__)
    path, filename = os.path.split(full_path)
    os.chdir(path)
    parseSPFResetBin(args.fileName, args.display, outputDir)


if __name__ == "__main__":
   main()