#    Utility functions for Parser for mem logger
#    Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
#    SPDX-License-Identifier: BSD-3-Clause-Clear
from xml.dom import minidom
from datetime import datetime
import io
from urllib import request, parse
import json
import argparse
import os

WEBSEQ = 'http://webseq.qualcomm.com/'
ENUM_CONFIG = "./enum_replacements.xml"
CONFIG = "./config.xml"

class MemLoggerUtil:

    enums = minidom.parse(ENUM_CONFIG)
    config_file = minidom.parse(CONFIG)

    def argparser(self):
        parser=argparse.ArgumentParser(
            description='''Qualcomm Memory Logger Bin File Parser. ''',
            epilog="""Contact Josh Kirsch or Patrick Paxson for assistance or bug reports."""
        )
        parser.add_argument('-f', '--file', dest='fileName', type=str, help='location of bin file to parse')
        parser.add_argument('-d', '--display', default=False, action=argparse.BooleanOptionalAction, help='print output to stdout instead of a file')
        parser.add_argument( '--callflow', default=False, action=argparse.BooleanOptionalAction, help='generates call flow')
        args=parser.parse_args()
        return args

    def getTimeStamp(self, time):
        timestamp = int(time)
        milliSeconds = timestamp % 1000
        timestamp = timestamp / 1000
        time = timestamp
        date = datetime.fromtimestamp(timestamp).strftime('%Y-%m-%d %H:%M:%S') + f".{milliSeconds}"
        return date

    def getFormat(self, name):
        format = ""
        config = self.config_file.getElementsByTagName(name)[0]
        format = config.getAttribute("format")
        return format

    def getEnumToString(self, type, value):
        val = str(value)
        enum = self.enums.getElementsByTagName(type)[0]
        items = enum.getElementsByTagName("item")
        for item in items:
            if int(item.getAttribute("value"),0) == value:
                val = item.getAttribute("name")
                break
        return val

    def generate_callflow_image(self, callflow, fileName):
        req_data = {
            'message': callflow,
            'apiVersion': '1',
            'format': 'png',
            'style': 'roundgreen'
        }
        encoded_req_data = parse.urlencode(req_data).encode()
        req =  request.Request(WEBSEQ, data = encoded_req_data)
        resp = request.urlopen(req)
        resp_obj = json.loads(resp.read().decode('utf-8'))

        png_url = request.urlopen(WEBSEQ + resp_obj['img'])
        with open(fileName, "wb") as file:
            file.write(png_url.read())
            file.close()

    def createLogDir(self):
        dir = "parsedLogs"
        if not os.path.exists(dir):
            os.makedirs(dir)
        return dir