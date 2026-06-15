/**
 *
 * \file mem_logger.cpp
 *
 * \brief
 *      Defines implementation for a circular queue.
 * \cond
 *  Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
 *  SPDX-License-Identifier: BSD-3-Clause-Clear
 * \endcond
 *
 */
#include <mem_logger.h>
#include "pal_state_queue.h"
#include "kpi_queue.h"
#include "graph_queue.h"
#include "spf_reset_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <ctime>
#include <vector>
#include <string>
#include <cstring>
#include <dirent.h>
#include <sys/types.h>
#include <map>
#include "ar_osal_log.h"
#include "ar_osal_file_io.h"
#include "ar_osal_mutex.h"

#define LOG_TAG "MEMLOG"
#define BUFFER_SIZE 100000
#define TS_BUFFER_SIZE 64
#define CFG_PRINT_ELEM "printtime"
#define CFG_QUEUE_ELEM "queue"
#define CFG_STATBUF_ELEM "statbuf"
#define CFG_NAME "name"
#define CFG_SIZE "size"
#define CFG_ENBL "enable"
#define CFG_OUTF "outputFile"
#define CFG_MAXBIN "maxBinFiles"
#define FILE_SPLIT "."
#define FOLDER_SPLIT "/\\"
#define FILE_TS_FORMAT "_%a_%b_%e_%H-%M-%S_%Y"

const std::map<uint32_t, size_t> memLoggerQueueToSize
{
    {PAL_STATE_Q, sizeof(struct pal_state_queue)},
    {KPI_Q, sizeof(struct kpi_queue)},
    {GRAPH_Q, sizeof(struct graph_queue)},
    {SPF_RESET_Q, sizeof(struct ssr_pdr_queue)},
};

const std::map<std::string, mem_logger_queue_datatype> memLoggerNameToQueueDataType
{
    {std::string{"PAL_STATE_Q"}, PAL_STATE_Q},
    {std::string{"KPI_Q"}, KPI_Q},
    {std::string{"GRAPH_Q"}, GRAPH_Q},
    {std::string{"SPF_RESET_Q"}, SPF_RESET_Q},
    {std::string{"QUEUE_MAX"}, QUEUE_MAX},
};

const std::map<std::string, mem_logger_statbuf_datatype> memLoggerNameToStatbufDataType
{
    {std::string{"GRAPH_STATBUF"}, GRAPH_STATBUF},
    {std::string{"SPF_RESET_STATBUF"}, SPF_RESET_STATBUF},
    {std::string{"STATBUF_MAX"}, STATBUF_MAX},
};

// Data structure to represent a circular queue

//queue initialization should take some size and some type of data
//the size of the queue divided by the type of data size should tell us how many entries
//make sure to round down on this division
//memLoggerEnqueue should take a void* payload which will fill the queue

memLoggerQueuePointer queues[QUEUE_MAX];
std::vector<std::vector<uint64_t>> statbufs(STATBUF_MAX);
ar_osal_mutex_t lock;
time_t start_time;
time_t end_time;
/* track the current level in the xml tree */
static uint32_t depth = 0;
static char* last_content;
uint32_t size = 0;
bool enable[QUEUE_MAX + STATBUF_MAX] = {0};
size_t qSize[QUEUE_MAX];
size_t maxBinFiles[QUEUE_MAX + STATBUF_MAX] = {0};
std::string filePath[QUEUE_MAX + STATBUF_MAX];
std::string fileName[QUEUE_MAX + STATBUF_MAX];
std::string fileType[QUEUE_MAX + STATBUF_MAX];
bool init = false;
bool enablePrintTime = false;

// Utility function to dequeue the front element
int32_t memLoggerDequeue(mem_logger_queue_datatype typeT, void *payload)
{
    memLoggerQueuePointer *q = nullptr;
    int32_t ret = 0;

    if (!memLoggerIsQueueEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Queue dump for disabled type detected");
        goto exit;
    }

    if (queues[typeT] == NULL)
    {
        AR_LOG_ERR(LOG_TAG,"Invalid Type\n");
        ret = -EINVAL;
        goto exit;
    }

    q = &queues[typeT];

    if (memLoggerIsQueueEmpty(typeT))    // front == rear
    {
        AR_LOG_ERR(LOG_TAG,"No items in queue cannot memLoggerDequeue");
        ret = -ENODATA;
        goto exit;
    }

    /*copy to payload and clear out mem spot*/
    memcpy(payload, (*q)->items[(*q)->front], memLoggerQueueToSize.at(typeT));
    memset((*q)->items[(*q)->front], 0, memLoggerQueueToSize.at(typeT));

    (*q)->front = ((*q)->front + 1) % (*q)->maxsize;  // circular queue
    (*q)->size--;

    AR_LOG_DEBUG(LOG_TAG,"front = %d, rear = %d\n", (*q)->front, (*q)->rear);

exit:
    return ret;
}

void xmlQueueHandler(const char **attribute)
{
    mem_logger_queue_datatype qName = QUEUE_MAX;

    for (uint32_t i = 0; attribute[i]; i += 2)
    {
        std::string val(attribute[i+1]);
        if (!strcmp(attribute[i], CFG_NAME))
        {
            qName = memLoggerNameToQueueDataType.at(val);
        }
        else if (qName != QUEUE_MAX)
        {
            if (!strcmp(attribute[i], CFG_SIZE))
            {
                qSize[qName] = atoi(attribute[i + 1]);
            }
            else if (!strcmp(attribute[i], CFG_ENBL))
            {
                enable[qName] = !strcmp(attribute[i + 1], "true");
            }
            else if (!strcmp(attribute[i], CFG_OUTF))
            {
                size_t folder_split_point = val.find_last_of(FOLDER_SPLIT) + 1;
                size_t file_split_point = val.find(FILE_SPLIT);
                filePath[qName] = val.substr(0, folder_split_point);
                fileName[qName] = val.substr(folder_split_point, file_split_point - folder_split_point);
                fileType[qName] = val.substr(file_split_point);
            }
            else if (!strcmp(attribute[i], CFG_MAXBIN))
            {
                maxBinFiles[qName] = atoi(attribute[i + 1]);
            }
        }
    }
}

void xmlStatbufHandler(const char **attribute)
{
    mem_logger_statbuf_datatype statbufName = STATBUF_MAX;

    for (uint32_t i = 0; attribute[i]; i += 2)
    {
        std::string val(attribute[i+1]);
        if (!strcmp(attribute[i], CFG_NAME))
        {
            statbufName = memLoggerNameToStatbufDataType.at(val);
        }
        else if (statbufName < STATBUF_MAX)
        {
            if (!strcmp(attribute[i], CFG_ENBL))
            {
                // stores enable flag after queue enables
                enable[QUEUE_MAX + statbufName] = !strcmp(attribute[i + 1], "true");
                if (enable[QUEUE_MAX + statbufName])
                {
                    if (statbufName == GRAPH_STATBUF)
                    {
                        statbufs[statbufName] = std::vector<uint64_t>(GRAPH_STATE_MAX);
                    }
                    else if (statbufName == SPF_RESET_STATBUF)
                    {
                        statbufs[statbufName] = std::vector<uint64_t>(SPF_RESET_STATE_MAX);
                    }
                }
            }
            else if (!strcmp(attribute[i], CFG_OUTF))
            {
                // stores filenames/types after queue types
                size_t folder_split_point = val.find_last_of(FOLDER_SPLIT) + 1;
                size_t file_split_point = val.find(FILE_SPLIT);
                filePath[QUEUE_MAX + statbufName] = val.substr(0, folder_split_point);
                fileName[QUEUE_MAX + statbufName] = val.substr(folder_split_point, file_split_point - folder_split_point);
                fileType[QUEUE_MAX + statbufName] = val.substr(file_split_point);
            }
            else if (!strcmp(attribute[i], CFG_MAXBIN))
            {
                maxBinFiles[QUEUE_MAX + statbufName] = atoi(attribute[i + 1]);
            }
        }
    }
}

/* first when start element is encountered */
void xmlStartElementHandler(void *data, const char *element, const char **attribute)
{
    // If element is printtime
    if (!strcmp(element, CFG_PRINT_ELEM))
    {
        enablePrintTime = !strcmp(attribute[1], "true");
    }
    else if (!strcmp(element, CFG_QUEUE_ELEM))
    {
        xmlQueueHandler(attribute);
    }
    else if (!strcmp(element, CFG_STATBUF_ELEM))
    {
        xmlStatbufHandler(attribute);
    }
    depth++;
}

/* decrement the current level of the tree */
void xmlEndElementHandler(void *data, const char *el)
{
    depth--;
}

int32_t parseConfigXML(char *buff, size_t buff_size, const char* cfg)
{
    AR_LOG_DEBUG(LOG_TAG, "Entering Parse XML \n");
    int32_t rc = 0;
    ar_fhandle handle = NULL;
    size_t bytes_read = 0;
    rc = ar_fopen(&handle, cfg, AR_FOPEN_READ_ONLY);

    if (rc != 0)
    {
        AR_LOG_ERR(LOG_TAG, "Failed to open file\n");
        return rc;
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, xmlStartElementHandler, xmlEndElementHandler);
    memset(buff, '\n', buff_size);
    rc = ar_fread(handle, buff, buff_size, &bytes_read);

    if (rc != 0)
    {
        AR_LOG_ERR(LOG_TAG, "Failed to read file\n");
        return rc;
    }

    AR_LOG_DEBUG(LOG_TAG, "Attempting Parse XML \n");
    /* parse the xml */
    if (XML_Parse(parser, buff, buff_size - 1, XML_TRUE) == XML_STATUS_ERROR)
    {
        AR_LOG_ERR(LOG_TAG, "Error: %s\n", XML_ErrorString(XML_GetErrorCode(parser)));
        rc = -EINVAL;
    }

    ar_fclose(handle);
    XML_ParserFree(parser);
    return rc;
}

void memLoggerPrintTime(latency_print_loc type)
{
    if (!enablePrintTime)
    {
        return;
    }

    struct tm* ptr;

    if (type == MEM_LOGGER_TIME_START)
    {
        start_time = time(NULL);
        ptr = localtime(&start_time);
        if (ptr)
        {
            AR_LOG_DEBUG(LOG_TAG, "Start time is %s", asctime(ptr));
        }
        else
        {
            AR_LOG_DEBUG(LOG_TAG, "Error retrieving start time");
        }
    }
    else if (type == MEM_LOGGER_TIME_END)
    {
        end_time = time(NULL);
        ptr = localtime(&end_time);
        if (ptr)
        {
            AR_LOG_DEBUG(LOG_TAG, "End time is %s", asctime(ptr));
            AR_LOG_DEBUG(LOG_TAG, "The function took %.2f seconds", difftime(start_time, end_time));
        }
        else
        {
            AR_LOG_DEBUG(LOG_TAG, "Error retrieving end time");
        }
    }
}

int32_t memLoggerInit(const char* cfgFilePath)
{
    int32_t ret = 0;

    if (init == false)
    {
        char buffer[BUFFER_SIZE];
        ret = parseConfigXML(buffer, BUFFER_SIZE, cfgFilePath);

        if (ret)
        {
            AR_LOG_ERR(LOG_TAG, "Error with XML Parse: %d \n", ret);
            goto exit;
        }

        AR_LOG_DEBUG(LOG_TAG, "XML parsed successfully \n");
        ret = ar_osal_mutex_create(&lock);

        if (ret)
        {
            AR_LOG_ERR(LOG_TAG, "Error with mutex creation: %d \n", ret);
            goto exit;
        }

        AR_LOG_DEBUG(LOG_TAG, "Mutex created successfully \n");
        init = true;
    }

exit:
    return ret;
}

void memLoggerDeinit()
{
    int i = 0;
    bool isAllNull = true;

    for (i = 0; i < QUEUE_MAX; i++)
    {
        isAllNull = isAllNull && (queues[i] == NULL);
    }

    for (i = 0; i < STATBUF_MAX; i++)
    {
        isAllNull = isAllNull && (statbufs[i].empty());
    }

    if (isAllNull)
    {
        AR_LOG_DEBUG(LOG_TAG,"All queues/statbufs deinitialized, destroying mutex lock...\n");
        ar_osal_mutex_destroy(lock);
    }
}

// Utility function to initialize a queue
int32_t memLoggerInitQ(mem_logger_queue_datatype typeT, const char* cfgFilePath)
{
    int32_t ret = 0;
    memLoggerQueuePointer *q = nullptr;
    uint32_t cells = 0;
    AR_LOG_DEBUG(LOG_TAG, "Initializing Queue Memory \n");
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    ret = memLoggerInit(cfgFilePath);

    if (ret)
    {
        AR_LOG_ERR(LOG_TAG, "Failed to initialize: %d \n", ret);
        goto exit;
    }

    if (!memLoggerIsQueueEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Queue initialization for disabled type detected");
        goto exit;
    }

    if (queues[typeT] != NULL)
    {
        AR_LOG_ERR(LOG_TAG, "Type is already initiated \n");
        ret = -EALREADY;
        goto exit;
    }

    AR_LOG_DEBUG(LOG_TAG, "Attempting memory allocation \n");
    queues[typeT] = (struct mem_logger_queue*) malloc(sizeof(struct mem_logger_queue));

    if (queues[typeT] == NULL)
    {
        AR_LOG_ERR(LOG_TAG, "Failed to allocate the memory! \n");
        ret = -ENOMEM;
        goto exit;
    }

    AR_LOG_DEBUG(LOG_TAG, "Attempting queue item memory allocation \n");
    cells = (qSize[typeT] - sizeof(struct mem_logger_queue)) / memLoggerQueueToSize.at(typeT);
    AR_LOG_DEBUG(LOG_TAG, "number of cell is %d", cells);
    queues[typeT]->items = (void**) malloc(cells * sizeof(void*));

    for (int i = 0; i < cells; i++)
    {
        queues[typeT]->items[i] = malloc(memLoggerQueueToSize.at(typeT));
        if (queues[typeT]->items[i] == NULL)
        {
            AR_LOG_ERR(LOG_TAG, "Failed to allocate the memory for queue item! \n");
            /*clean up mem*/
            if (i > 0)
            {
                for (int j=i-1; j >= 0; j--)
                {
                    free(queues[typeT]->items[j]);
                }
            }
            ret = -EINVAL;
            goto exit;
        }
    }

    if (queues[typeT]->items == NULL)
    {
        AR_LOG_ERR(LOG_TAG, "Failed to allocate the memory for queue! \n");
        ret = -EINVAL;
        goto exit;
    }

    queues[typeT]->maxsize = cells;
    queues[typeT]->front = 0;
    queues[typeT]->rear = -1;
    queues[typeT]->size = 0;
    q = &queues[typeT];
    AR_LOG_DEBUG(LOG_TAG, "maxsize = %d, type = %d \n", (*q)->maxsize, typeT);

exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

int32_t memLoggerDeinitQ(mem_logger_queue_datatype typeT)
{
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    int32_t ret = 0;
    memLoggerQueuePointer *q = nullptr;
    int i = 0;

    if (!memLoggerIsQueueEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Queue deinitialization for disabled type detected");
        goto exit;
    }

    if (queues[typeT] == NULL)
    {
        AR_LOG_ERR(LOG_TAG,"Invalid Type\n");
        ret = -EINVAL;
        goto exit;
    }

    q = &queues[typeT];

    for (i=0; i<(*q)->maxsize; i++)
    {
        free((*q)->items[i]);
    }

    free((*q)->items);
    free(*q);
    (*q) = NULL;
    memLoggerDeinit();

exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

int32_t memLoggerInitStatbuf(mem_logger_statbuf_datatype typeT, const char* cfgFilePath)
{
    int32_t ret = 0;
    int i = 0;
    AR_LOG_DEBUG(LOG_TAG, "Initializing Statbuf Memory \n");
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    ret = memLoggerInit(cfgFilePath);

    if (ret)
    {
        AR_LOG_ERR(LOG_TAG, "Failed to initialize: %d \n", ret);
        goto exit;
    }

    if (!memLoggerIsStatbufEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Statbuf initialization for disabled type detected");
        goto exit;
    }

    for (i; i < statbufs[typeT].size(); i++)
    {
        statbufs[typeT][i] = 0;
    }

exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

int32_t memLoggerDeinitStatbuf(mem_logger_statbuf_datatype typeT)
{
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    int32_t ret = 0;

    if (!memLoggerIsStatbufEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Statbuf deinitialization for disabled type detected");
        goto exit;
    }

    statbufs[typeT].clear();
    memLoggerDeinit();

exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

uint32_t memLoggerGetQueueSize(mem_logger_queue_datatype typeT)
{
    memLoggerQueuePointer *q = &queues[typeT];
    uint32_t value = 0;

    if ((*q) == NULL)
    {
        value = -EINVAL;
    }
    else
    {
        value = (*q)->size;
    }
    return value;
}

bool memLoggerIsQueueInitialized()
{
    return init;
}

bool memLoggerIsQueueEnabled(mem_logger_queue_datatype typeT)
{
    return enable[typeT];
}

bool memLoggerIsStatbufEnabled(mem_logger_statbuf_datatype typeT)
{
    if (QUEUE_MAX + typeT < QUEUE_MAX + STATBUF_MAX)
    {
        return enable[QUEUE_MAX + typeT];
    }
    return false;
}

bool memLoggerIsQueueEmpty(mem_logger_queue_datatype typeT)
{
    memLoggerQueuePointer *q = &queues[typeT];
    uint32_t length = memLoggerGetQueueSize(typeT);
    return length <= 0;
}

int32_t memLoggerDumpMaxFileCheck(int typeT, time_t now)
{
    int32_t ret = 0;
    DIR *dir;
    struct dirent *file;
    struct tm curTime;
    std::string fileNameToCheck;
    std::string curTS;
    std::string oldestFile;
    int32_t files_written = 0;

    dir = opendir(filePath[typeT].c_str());
    if (dir != NULL)
    {
        while ((file = readdir(dir)))
        {
            fileNameToCheck = std::string(file->d_name);
            if (fileNameToCheck.find(fileName[typeT]) != std::string::npos)
            {
                files_written += 1;
                // Isolates current timestamp from file name
                curTS = fileNameToCheck.substr(fileName[typeT].length(), fileNameToCheck.length() - fileName[typeT].length() - fileType[typeT].length());
                if (strptime(curTS.c_str(), FILE_TS_FORMAT, &curTime))
                {
                    if (difftime(mktime(&curTime), now) < 0)
                    {
                        now = mktime(&curTime);
                        oldestFile = fileNameToCheck;
                    }
                }
                else
                {
                    AR_LOG_DEBUG(LOG_TAG, "Unable to parse timestamp for file %s", file->d_name);
                }
            }
        }
        (void) closedir (dir);
    }
    else
    {
        AR_LOG_ERR(LOG_TAG, "Could not read directory during file dump for type %d", typeT);
        ret = -EINVAL;
        goto exit;
    }

    if (files_written >= maxBinFiles[typeT])
    {
        AR_LOG_DEBUG(LOG_TAG, "Max number of bin files reached for type %d", typeT);
        ret = ar_fdelete((filePath[typeT] + oldestFile).c_str());
        if (ret)
        {
            AR_LOG_ERR(LOG_TAG, "Error removing oldest file for type %d: %d", typeT, ret);
            goto exit;
        }
    }
exit:
    return ret;
}

int32_t memLoggerDumpQueueToFile(mem_logger_queue_datatype typeT)
{
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    int32_t ret = 0;
    memLoggerQueuePointer *q = nullptr;
    bool isEmpty = true;
    size_t bytes_written = 0;
    void* payload = nullptr;
    ar_fhandle handle = NULL;
    time_t now = time(NULL);
    struct tm* ptr;
    char timestamp[TS_BUFFER_SIZE];
    std::string curFilePath;

    if (!memLoggerIsQueueEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Queue dump for disabled type detected: %d", typeT);
        goto exit;
    }

    ar_osal_mutex_lock(lock);

    if (queues[typeT] == NULL)
    {
        AR_LOG_ERR(LOG_TAG,"Invalid Type\n");
        ret = -EINVAL;
        goto unlock;
    }

    ptr = localtime(&now);
    if (ptr)
    {
        if (strftime(timestamp, TS_BUFFER_SIZE, FILE_TS_FORMAT, ptr))
        {
            curFilePath = filePath[typeT] + fileName[typeT] + timestamp + fileType[typeT];
        }
        else
        {
            AR_LOG_ERR(LOG_TAG, "Unable to format timestamp during queue dump for type %d", typeT);
            ret = -EINVAL;
            goto unlock;
        }
    }
    else
    {
        AR_LOG_ERR(LOG_TAG, "Unable to retrieve timestamp during queue dump for type %d", typeT);
        ret = -EINVAL;
        goto unlock;
    }

    ret = memLoggerDumpMaxFileCheck(typeT, now);
    if (ret)
    {
        AR_LOG_ERR(LOG_TAG, "File dump failed during max file check");
        goto unlock;
    }

    q = &queues[typeT];
    AR_LOG_DEBUG(LOG_TAG, "Attempting to dump type %d to file", typeT);

    ret = ar_fopen(&handle, curFilePath.c_str(), AR_FOPEN_WRITE_ONLY_APPEND);

    if (ret)
    {
        AR_LOG_ERR(LOG_TAG,"failed to open the file %s:error:%d \n", curFilePath.c_str(), ret);
        goto unlock;
    }

    isEmpty = memLoggerIsQueueEmpty(typeT);

    while (!isEmpty)
    {
        payload = malloc(memLoggerQueueToSize.at(typeT));

        if (!payload)
        {
            AR_LOG_ERR(LOG_TAG,"Failed to allocate memory for queue type %d: %d\n", typeT, ret);
            ret = -ENOMEM;
            goto unlock;
        }

        ret = memLoggerDequeue(typeT, payload);

        if (ret)
        {
            AR_LOG_ERR(LOG_TAG,"Dequeue failed for queue type %d: %d\n", typeT, ret);
            goto unlock;
        }

        bytes_written = 0;
        ret = ar_fwrite(handle, payload, memLoggerQueueToSize.at(typeT), &bytes_written);

        if (ret)
        {
            AR_LOG_ERR(LOG_TAG,"Item write failed for queue type %d: %d\n", typeT, ret);
            goto unlock;
        }

        AR_LOG_DEBUG(LOG_TAG, "Wrote item of queue type %d", typeT);
        free(payload);
        payload = nullptr;
        isEmpty = memLoggerIsQueueEmpty(typeT);
    }

    ret = ar_fclose(handle);

    if (ret)
    {
        AR_LOG_ERR(LOG_TAG,"failed to seek file %d \n", ret);
    }

unlock:
    ar_osal_mutex_unlock(lock);
exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

int32_t memLoggerDumpStatbufToFile(mem_logger_statbuf_datatype typeT)
{
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    int32_t ret = 0;
    int32_t i = 0;
    size_t bytes_written = 0;
    uint64_t *valAddr;
    ar_fhandle handle = NULL;
    time_t now = time(NULL);
    struct tm* ptr;
    char timestamp[TS_BUFFER_SIZE];
    std::string curFilePath;

    if (!memLoggerIsStatbufEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Static buffer dump for disabled type detected");
        goto exit;
    }

    ar_osal_mutex_lock(lock);

    if (statbufs[typeT].empty())
    {
        AR_LOG_ERR(LOG_TAG,"Invalid Type\n");
        ret = -EINVAL;
        goto unlock;
    }

    ptr = localtime(&now);
    if (ptr)
    {
        if (strftime(timestamp, TS_BUFFER_SIZE, FILE_TS_FORMAT, ptr))
        {
            curFilePath = filePath[QUEUE_MAX + typeT] + fileName[QUEUE_MAX + typeT] + timestamp + fileType[QUEUE_MAX + typeT];
        }
        else
        {
            AR_LOG_ERR(LOG_TAG, "Unable to format timestamp for static buffer dump");
            ret = -EINVAL;
            goto unlock;
        }
    }
    else
    {
        AR_LOG_ERR(LOG_TAG, "Unable to retrieve timestamp for static buffer dump");
        ret = -EINVAL;
        goto unlock;
    }

    ret = memLoggerDumpMaxFileCheck(QUEUE_MAX + typeT, now);
    if (ret)
    {
        AR_LOG_ERR(LOG_TAG, "File dump failed during max file check");
        goto unlock;
    }

    AR_LOG_DEBUG(LOG_TAG, "Attempting to dump type %d to file", typeT);

    ret = ar_fopen(&handle, curFilePath.c_str(), AR_FOPEN_WRITE_ONLY_APPEND);
    if (ret)
    {
        AR_LOG_ERR(LOG_TAG,"failed to open the file %s:error:%d \n", curFilePath.c_str(), ret);
        goto unlock;
    }

    valAddr = &statbufs[typeT][0];
    ret = ar_fwrite(handle, valAddr, sizeof(uint64_t) * statbufs[typeT].size(), &bytes_written);

    if (ret)
    {
        AR_LOG_ERR(LOG_TAG,"Item write failed for static buffer type %d: %d\n", typeT, ret);
        goto unlock;
    }

    ret = ar_fclose(handle);

    if (ret)
    {
        AR_LOG_ERR(LOG_TAG,"failed to seek file %d \n", ret);
    }

    for (i; i < statbufs[typeT].size(); i++)
    {
        statbufs[typeT][i] = 0;
    }

unlock:
    ar_osal_mutex_unlock(lock);
exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

int32_t memLoggerDumpAllToFile()
{
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    int32_t ret = 0;
    for (int queueType = 0; queueType != QUEUE_MAX; queueType++)
    {
        mem_logger_queue_datatype typeQ = static_cast<mem_logger_queue_datatype>(queueType);
        if (memLoggerIsQueueEnabled(typeQ))
        {
            AR_LOG_DEBUG(LOG_TAG, "Dumping queue type %d", typeQ);
            ret = memLoggerDumpQueueToFile(typeQ);
        }

        if (ret)
        {
            AR_LOG_ERR(LOG_TAG,"memLoggerDumpAllToFile: failed to dump type %d: %d \n", queueType, ret);
        }
    }
    for (int statbufType = 0; statbufType != STATBUF_MAX; statbufType++)
    {
        mem_logger_statbuf_datatype typeT = static_cast<mem_logger_statbuf_datatype>(statbufType);
        if (memLoggerIsStatbufEnabled(typeT))
        {
            AR_LOG_DEBUG(LOG_TAG, "Dumping static buffer type %d", typeT);
            ret = memLoggerDumpStatbufToFile(typeT);
        }

        if (ret)
        {
            AR_LOG_ERR(LOG_TAG,"memLoggerDumpAllToFile: failed to dump type %d: %d \n", statbufType, ret);
        }
    }
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

uint64_t memLoggerFetchTimestamp()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

// Utility function to add an element `x` to the queue
int32_t memLoggerEnqueue(mem_logger_queue_datatype typeT, void *payload)
{
    int32_t ret = 0;
    uint32_t length;
    memLoggerQueuePointer *q = nullptr;
    memLoggerPrintTime(MEM_LOGGER_TIME_START);

    if (!memLoggerIsQueueEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Queue dump for disabled type detected");
        goto exit;
    }

    ar_osal_mutex_lock(lock);

    if (queues[typeT] == nullptr)
    {
        AR_LOG_ERR(LOG_TAG, "Failed to memLoggerEnqueue specific type: %d\n", typeT);
        ret = -EINVAL;
        goto unlock;
    }

    q = &queues[typeT];
    length = memLoggerGetQueueSize(typeT);

    if(length < 0)
    {
        AR_LOG_ERR(LOG_TAG, "Failed to check queue size %d \n", ret);
        goto unlock;
    }

    if (length == (*q)->maxsize)
    {
        AR_LOG_DEBUG(LOG_TAG, "Circling back to the front\n");
        (*q)->front = ((*q)->front + 1) % (*q)->maxsize;  // circular queue
        (*q)->size--;
    }

    (*q)->rear = ((*q)->rear + 1) % (*q)->maxsize;    // circular queue
    /*copy payload to rear element*/
    memcpy((*q)->items[(*q)->rear], payload, memLoggerQueueToSize.at(typeT));
    (*q)->size++;
    length = memLoggerGetQueueSize(typeT);

    if (length < 0)
    {
        AR_LOG_ERR(LOG_TAG, "Failed to check queue size %d\n", ret);
        goto unlock;
    }

    AR_LOG_DEBUG(LOG_TAG, "front = %d, rear = %d, maxsize = %d\n", (*q)->front, (*q)->rear, (*q)->maxsize);

unlock:
    ar_osal_mutex_unlock(lock);
exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

int32_t memLoggerIncrementStatbuf(mem_logger_statbuf_datatype typeT, uint64_t element)
{
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    uint32_t ret = 0;

    if (!memLoggerIsStatbufEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Statbuf increment for disabled type detected");
        goto exit;
    }

    ar_osal_mutex_lock(lock);

    if (statbufs[typeT].size() <= element)
    {
        AR_LOG_ERR(LOG_TAG,"Invalid Type\n");
        ret = -EINVAL;
        goto unlock;
    }

    statbufs[typeT][element] += 1;

unlock:
    ar_osal_mutex_unlock(lock);
exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

uint64_t memLoggerGetStatbufElem(mem_logger_statbuf_datatype typeT, uint64_t element)
{
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    uint64_t ret = -1;

    if (!memLoggerIsStatbufEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Statbuf get for disabled type detected");
        goto exit;
    }

    ar_osal_mutex_lock(lock);

    if (statbufs[typeT].size() <= element)
    {
        AR_LOG_ERR(LOG_TAG,"Invalid Type\n");
        ret = -EINVAL;
        goto unlock;
    }

    ret = statbufs[typeT][element];

unlock:
    ar_osal_mutex_unlock(lock);
exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}

int32_t memLoggerSetStatbufElement(mem_logger_statbuf_datatype typeT, uint64_t element, uint64_t value)
{
    memLoggerPrintTime(MEM_LOGGER_TIME_START);
    int32_t ret = 0;

    if (!memLoggerIsStatbufEnabled(typeT))
    {
        AR_LOG_DEBUG(LOG_TAG, "Statbuf set for disabled type detected");
        goto exit;
    }

    ar_osal_mutex_lock(lock);

    if (statbufs[typeT].size() <= element)
    {
        AR_LOG_ERR(LOG_TAG,"Invalid Type\n");
        ret = -EINVAL;
        goto unlock;
    }

    statbufs[typeT][element] = value;

unlock:
    ar_osal_mutex_unlock(lock);
exit:
    memLoggerPrintTime(MEM_LOGGER_TIME_END);
    return ret;
}
