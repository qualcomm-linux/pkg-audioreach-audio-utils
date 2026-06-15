#ifndef MEM_LOGGER_H
#define MEM_LOGGER_H
/**
* \file mem_logger.h
* \brief
*      Defines platform agnostic APIs for logging items to memory
       and dumping them to the file system.

*  Copyright (c) Qualcomm Innovation Center, Inc. All rights reserved.
*  SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include <stddef.h>
#include <sys/time.h>
#include <expat.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_LOGGER_BUF_SIZE 1024
#define MEMLOG_CFG_FILE "/vendor/etc/mem_logger_config.xml"

typedef enum
{
    MEM_LOGGER_TIME_START,
    MEM_LOGGER_TIME_END,
}latency_print_loc;

typedef enum
{
    PAL_STATE_Q,
    KPI_Q,
    GRAPH_Q,
    SPF_RESET_Q,
    QUEUE_MAX,
}mem_logger_queue_datatype;

typedef enum
{
    GRAPH_STATBUF,
    SPF_RESET_STATBUF,
    STATBUF_MAX,
}mem_logger_statbuf_datatype;

struct mem_logger_queue
{
    uint32_t maxsize;    // maximum capacity of the queue
    int32_t front;      // front points to the front element in the queue (if any)
    int32_t rear;       // rear points to the last element in the queue
    uint32_t size;       // current capacity of the queue
    void **items;     // array to store queue elements
};

typedef struct mem_logger_queue *memLoggerQueuePointer;

/// @brief Initializes the desired queue with info from the config file
/// @param typeT The queue type that you want to initialize
/// @param cfgFilePath The path to the config file with queue info
/// @return 0 if successful, error code otherwise
int32_t memLoggerInitQ(mem_logger_queue_datatype typeT, const char* cfgFilePath);

/// @brief Deinitializes the desired queue
/// @param typeT The queue type that you want to deinitialize
/// @return 0 if successful, error code otherwise
int32_t memLoggerDeinitQ(mem_logger_queue_datatype typeT);

/// @brief Initializes the desired static buffer with info from the config file
/// @param typeT The static buffer type that you want to initialize
/// @param cfgFilePath The path to the config file with statbuf info
/// @return 0 if successful, error code otherwise
int32_t memLoggerInitStatbuf(mem_logger_statbuf_datatype typeT, const char* cfgFilePath);

/// @brief Deinitializes the desired static buffer
/// @param typeT The static buffer type that you want to deinitialize
/// @return 0 if successful, error code otherwise
int32_t memLoggerDeinitStatbuf(mem_logger_statbuf_datatype typeT);

/// @brief Utility function to return the size of the queue
/// @param typeT The queue type that you want to check size of
/// @return Size of desired queue
uint32_t memLoggerGetQueueSize(mem_logger_queue_datatype typeT);

/// @brief Utility function to check if the queue is initialized
/// @return true if initialized, false if not
bool memLoggerIsQueueInitialized();

/// @brief Utility function to check if the queue type is enabled or not
/// @param typeT The queue type that you want to check
/// @return true if enabled, false if not
bool memLoggerIsQueueEnabled(mem_logger_queue_datatype typeT);

/// @brief Utility function to check if the static buffer type is enabled or not
/// @param typeT The static buffer type that you want to check
/// @return true if enabled, false if not
bool memLoggerIsStatbufEnabled(mem_logger_statbuf_datatype typeT);

/// @brief Utility function to check if the queue is empty or not
/// @param typeT The queue type that you want to check
/// @return true if empty, false if not
bool memLoggerIsQueueEmpty(mem_logger_queue_datatype typeT);

/// @brief Utility function to add an element to the queue
/// @param typeT The queue type you want to enqueue to
/// @param payload The payload you want to enqueue
/// @return 0 if successful, error code otherwise
int32_t memLoggerEnqueue(mem_logger_queue_datatype typeT, void *payload);

/// @brief Increments element of desired static buffer
/// @param typeT The static buffer type you want to increment
/// @param element The element within the statbuf that you want to increment
/// @return 0 if successful, error code otherwise
int32_t memLoggerIncrementStatbuf(mem_logger_statbuf_datatype typeT, uint64_t element);

/// @brief Gets the current value of desired static buffer element
/// @param typeT The static buffer type you want to get
/// @param element The element within the statbuf that you want to get
/// @return The value of the element
uint64_t memLoggerGetStatbufElem(mem_logger_statbuf_datatype typeT, uint64_t element);

/// @brief Sets the value of desired static buffer element
/// @param typeT The static buffer type you want to set
/// @param element The element within the statbuf that you want to set
/// @param value The value you want to set the element to
/// @return 0 if successful, error code otherwise
int32_t memLoggerSetStatbufElement(mem_logger_statbuf_datatype typeT, uint64_t element, uint64_t value);

/// @brief dumps queue to binary file
/// @param typeT The queue type you want to dump
/// @return 0 if successful, error code otherwise
int32_t memLoggerDumpQueueToFile(mem_logger_queue_datatype typeT);

/// @brief dumps static buffer to binary file
/// @param typeT The static buffer type you want to dump
/// @return 0 if successful, error code otherwise
int32_t memLoggerDumpStatbufToFile(mem_logger_statbuf_datatype typeT);

/// @brief dumps all queues/static buffers to their respective binary files
/// @return 0 if successful, error code otherwise
int32_t memLoggerDumpAllToFile();

/// @brief Retrieves current timestamp in system
/// @return the timestamp
uint64_t memLoggerFetchTimestamp();

/// @brief Utility function to print latency of functions (disabled by default)
/// @param type The print location (start or end of function)
void memLoggerPrintTime(latency_print_loc type);

#ifdef __cplusplus
}
#endif

#endif /* MEM_LOGGER_H */