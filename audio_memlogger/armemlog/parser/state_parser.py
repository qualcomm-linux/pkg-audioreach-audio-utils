#    PAL State Parser file for mem logger
#    Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
#    SPDX-License-Identifier: BSD-3-Clause-Clear
from struct import unpack, unpack_from, calcsize
from datetime import datetime
import sys
import os
import memLoggerUtils

util = memLoggerUtils.MemLoggerUtil()

TIMESTAMP = 0
STREAM_HANDLE = 1
DEV_1_SAMPLE_RATE = 2
DEV_1_DEVICE_NAME = 3
DEV_1_BIT_WIDTH = 4
DEV_1_CHANNELS = 5
DEV_2_SAMPLE_RATE = 6
DEV_2_DEVICE_NAME = 7
DEV_2_BIT_WIDTH = 8
DEV_2_CHANNELS = 9
DEV_3_SAMPLE_RATE = 10
DEV_3_DEVICE_NAME = 11
DEV_3_BIT_WIDTH = 12
DEV_3_CHANNELS = 13
QUEUE_STATE = 14
ERROR_CODE = 15
STREAM_TYPE = 16
DIRECTION = 17
SESSION_HANDLE = 18

def parsePalStateBin(inFile, display, callflowEnabled, outputDir):

    FORMAT = util.getFormat("PAL_STATE_QUEUE")
    ACD_FORMAT = util.getFormat("acd_info")
    baseSize = calcsize(FORMAT)
    unionSize = calcsize(ACD_FORMAT)
    totalSize = baseSize + unionSize
    output_file_name = os.path.splitext(os.path.basename(inFile))
    #print("base : "+ str(baseSize) + " union : " + str(unionSize) + " Total Size: " + str(totalSize))
    if callflowEnabled:
        callFlow = "title Pal State Sequence\n\n"
    original_stdout = sys.stdout # Save a reference to the original standard output
    openStreams = []
    errorStreams = []
    loc = 0
    file_stat = os.stat(inFile)
    file = open(inFile, 'rb').read()

    if not display:
        path = os.path.join(outputDir, output_file_name[0]+".txt")
        output = open(path, 'w')
        sys.stdout = output

    print(f'-----------------------------------------')
    print(f'-----------PAL STATE QUEUE---------------')
    while loc < file_stat.st_size:
        qItem = unpack_from(FORMAT, file, loc)
        if not qItem:
            break
        loc += baseSize
        #write base object
        writeItem(qItem)
        #write stream info specific union object
        if util.getEnumToString("pal_stream_type_t", qItem[STREAM_TYPE]) == "ACD":
            acd_item = unpack_from(ACD_FORMAT, file, loc)
            writeACD(acd_item)
        loc += unionSize


        #update open streams
        for item in openStreams:
            if item[STREAM_HANDLE] == qItem[STREAM_HANDLE]:
                #remove an insert new state if not going to idle
                if callflowEnabled:
                    state = util.getEnumToString("stream_state_t", item[QUEUE_STATE])
                    newState = util.getEnumToString("stream_state_t", qItem[QUEUE_STATE])
                    if newState == "STREAM_IDLE":
                        newState = "STREAM_CLOSED"
                    type = util.getEnumToString("pal_stream_type_t", qItem[STREAM_TYPE])
                    handle = hex(int(qItem[STREAM_HANDLE]))
                    if qItem[ERROR_CODE] != 0:
                        callFlow += "note over "+state+","+newState+"\n"+type+"("+handle+")\nTransition to "+ newState+ " FAILED\nend note\n"
                    else:
                        callFlow += state + " -> " +newState +": "+type+"("+handle+")\n"
                #only remove and item if transition was a success
                if qItem[ERROR_CODE] == 0:
                    openStreams.remove(item)
        if qItem[QUEUE_STATE] != 0:
            if qItem[ERROR_CODE] == 0:
                openStreams.append(qItem)
            if callflowEnabled:
                state = util.getEnumToString("stream_state_t", qItem[QUEUE_STATE])
                if state == "STREAM_OPENED":
                    type = util.getEnumToString("pal_stream_type_t", qItem[STREAM_TYPE])
                    handle = hex(int(qItem[STREAM_HANDLE]))
                    callFlow += "STREAM_CLOSED -> " + state +": "+type+"("+handle+")\n"
        if qItem[ERROR_CODE] != 0:
            errorStreams.append(qItem)

    #print out open streams
    print(f'-----------------------------------------')
    if(openStreams):
        print(f'\n\n--------PAL STREAMS STILL ACTIVE---------')
        for field in openStreams:
            writeItem(field)
    else:
        print("\n\n------NO ACTIVE STREAMS IN SYSTEM--------")

    if(errorStreams):
        print(f'\n\n----PAL STREAMS TRANSITION FAILURES------')
        for field in errorStreams:
            writeItem(field)
    else:
        print("\n\n------------NO ERROR STREAMS-------------")

    if not display:
            output.close()
            sys.stdout = original_stdout
            print('generating file '+ outputDir + "\\" + output_file_name[0] +'.txt')

    if(callflowEnabled):
        callFlowName = output_file_name[0] + "_CallFlow.png"
        print("generating callflow: "+ outputDir + "\\" +callFlowName)
        path = os.path.join(outputDir, callFlowName)
        util.generate_callflow_image(callFlow, path)

def writeItem(item):
    #timeStamp, handle, sr1, d1, bw1, c1, sr2, d2, bw2, c2, sr3, d3, bw3, c3, state, streamType, direction, error, sHandle, = item
    date = util.getTimeStamp(item[TIMESTAMP])
    streamType = util.getEnumToString("pal_stream_type_t", item[STREAM_TYPE])
    print(f'-----------------------------------------')
    print(f'Timestamp:........{date}')
    print(f'stream_handle:....{hex(int(item[STREAM_HANDLE]))}')
    print(f'state:............{util.getEnumToString("stream_state_t", item[QUEUE_STATE])}')
    print(f'stream_type:......{streamType}')
    print(f'direction:........{util.getEnumToString("pal_stream_direction_t",item[DIRECTION])}')
    if item[ERROR_CODE] != 0:
        print(f'error.............THIS TRANSITION FAILED! {item[ERROR_CODE]}')
    print(f'device info:')
    if( item[DEV_1_DEVICE_NAME] != 0):
        print(f'  device: {util.getEnumToString("pal_device_id_t",item[DEV_1_DEVICE_NAME])}\n\tsample_rate:{item[DEV_1_SAMPLE_RATE]}\n\tbit_width:{item[DEV_1_BIT_WIDTH]}\n\tchannels:{item[DEV_1_CHANNELS]}')
    if( item[DEV_2_DEVICE_NAME] != 0):
        print(f'  device: {util.getEnumToString("pal_device_id_t",item[DEV_2_DEVICE_NAME])}\n\tsample_rate:{item[DEV_2_SAMPLE_RATE]}\n\tbit_width:{item[DEV_2_BIT_WIDTH]}\n\tchannels:{item[DEV_2_CHANNELS]}')
    if( item[DEV_3_DEVICE_NAME] != 0):
        print(f'  device: {util.getEnumToString("pal_device_id_t",item[DEV_3_DEVICE_NAME])}\n\tsample_rate:{item[DEV_3_SAMPLE_RATE]}\n\tbit_width:{item[DEV_3_BIT_WIDTH]}\n\tchannels:{item[DEV_3_CHANNELS]}')

    print(f'-----------------------------------------')

def writeACD(item):
    print(f'ACD INFO:')
    print(f'  state_id:     {item[0]}')
    print(f'  eng_state_id: {item[1]}')
    print(f'  event_id:     {item[2]}')
    print(f'  context_id:   {item[3]}')
    print(f'  CP name:      {item[4].decode("utf-8")}')
    print(f'  model id:     {item[5]}')

def main():
    args = util.argparser()
    # Get the current working directory
    cwd = os.getcwd()
    outputDir = os.path.join(cwd, util.createLogDir())
    #get file path
    full_path = os.path.realpath(__file__)
    path, filename = os.path.split(full_path)
    os.chdir(path)
    parsePalStateBin(args.fileName, args.display, args.callflow, outputDir)


if __name__ == "__main__":
   main()