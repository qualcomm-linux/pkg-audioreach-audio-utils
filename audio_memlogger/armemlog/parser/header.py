#    Header parser file for mem logger
#    Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
#    SPDX-License-Identifier: BSD-3-Clause-Clear
import sys
from xml.dom import minidom
import re
import CppHeaderParser

DATATYPES_DICT = {"char":0,}

def analyzer(cppHeader, classname, root, queue, arr, arr_prefix):
    x = 0
    while(x < len(cppHeader.classes[classname]["properties"]["public"])):
        properties = cppHeader.classes[classname]["properties"]["public"][x]
        datatype = properties["type"]
        temp = ""

        if("array_size" in properties):
            sizeS = properties["array_size"]
            size = 0
            if((sizeS).isnumeric()):
                size = int(sizeS)
            else:
                for define in cppHeader.defines:
                    type2 = define
                    index = type2.find(sizeS)
                    
                    if(index >= 0):
                        size = int(type2[index + len(sizeS) + 1:])
                        break

            result = DATATYPES_DICT.get(datatype)
            if result is not None:
                print("     " + datatype + " " + properties["name"] + "[" + str(sizeS) + "]")
                field = root.createElement('field')
                field.setAttribute('name', str(properties["name"]))
                field.setAttribute('size', datatype + "[" + str(sizeS) + "]")
                queue.appendChild(field)
            else:
                i = 1
                prefix = ""
                while(i <= size):
                    struct_index = datatype.find("struct")
                    if(struct_index >= 0):
                        analyzer(cppHeader, datatype[struct_index + 7 :], root, queue, i, properties["name"])
                    else:
                        print("     " + datatype + " " + properties["name"] + '_' + str(i))
                        field = root.createElement('field')
                        field.setAttribute('name', properties["name"] + '_' + str(i))
                        field.setAttribute('size', datatype)
                        queue.appendChild(field)

                    i = i + 1

        else:
            struct_index = datatype.find("struct")
            if(struct_index >= 0):
                analyzer(cppHeader, datatype[struct_index + 7 :], root, queue, arr_prefix)
                
            else:
                field = root.createElement('field')

                if(arr != 0):
                    print("     " + datatype + " " + arr_prefix + '_' + str(arr) + '_' + properties["name"])
                    field.setAttribute('name', arr_prefix + '_' + str(arr) + '_' + properties["name"])
                    
                else:
                    print("     " + datatype + " " +  properties["name"])
                    field.setAttribute('name', properties["name"])

                field.setAttribute('size', datatype)
                queue.appendChild(field)
        x = x + 1


def main(argv):
    header = argv[0]
    cppHeader = CppHeaderParser.CppHeader(header)
    root = minidom.Document()

    xml = root.createElement("mem_logger") 
    root.appendChild(xml)

    queues = root.createElement('queues')
    xml.appendChild(queues)

    for classname in cppHeader.classes.keys():
        if(re.search("_queue$", classname)):
            print(classname)
            queue = root.createElement('queue')
            queue.setAttribute('name', classname.upper())
            queues.appendChild(queue)
            
            analyzer(cppHeader, classname, root,queue, 0, "")


    xml_str = root.toprettyxml(indent = "\t") 
    save_path_file = "config.xml"

    with open(save_path_file, "w") as f:
        f.write(xml_str) 

if __name__ == "__main__":
   main(sys.argv[1:])