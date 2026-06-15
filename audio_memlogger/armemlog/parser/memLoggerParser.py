#    Main Parser file for mem logger
#    Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
#    SPDX-License-Identifier: BSD-3-Clause-Clear
import sys
import os
import memLoggerUtils

util = memLoggerUtils.MemLoggerUtil()
#import state_parser
#import kpi_parser

def parseBin(inFile, display, callflow):

    # Get the current working directory
    cwd = os.getcwd()
    outputDir = os.path.join(cwd, util.createLogDir())
    #get file path
    full_path = os.path.realpath(__file__)
    path, filename = os.path.split(full_path)
    os.chdir(path)
    import state_parser
    import kpi_parser
    import graph_parser
    import spf_reset_parser
    binFiles = []

    #parse all files if dir is passed
    if os.path.isdir(inFile):
        for filename in os.listdir(inFile):
            f = os.path.join(inFile, filename)
            if os.path.isfile(f):
                binFiles.append(f)
    elif os.path.isfile(inFile):
        binFiles.append(inFile)
    for file in binFiles:
        if "pal_state_queue" in file:
            state_parser.parsePalStateBin(file, display, callflow, outputDir)
        elif "kpi_queue" in file:
            kpi_parser.parseKPIBin(file, display, outputDir)
        elif "graph" in file:
            graph_parser.parseGraphBin(file, display, outputDir)
        elif "spf_reset" in file:
            spf_reset_parser.parseSPFResetBin(file, display, outputDir)
        else:
            print("do not know how to praser file "+ file)

def main():
    args = util.argparser()
    parseBin(args.fileName, args.display, args.callflow)


if __name__ == "__main__":
   main()
