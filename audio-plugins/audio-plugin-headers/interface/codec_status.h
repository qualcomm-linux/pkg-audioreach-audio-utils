/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _CDC_STATUS_H_
#define _CDC_STATUS_H_


    /** @ingroup codec_status
      Indicates success. The operation completed without any errors. */
#define CDC_EOK                               (0x00)

    /** @ingroup codec_status
      Indicates a general failure. */
#define CDC_EFAILED                           (0x01)

    /** @ingroup codec_status
      Invalid operation parameters. */
#define CDC_EBADPARAM                         (0x02)

    /** @ingroup codec_status
      Unsupported routine or operation. */
#define CDC_EUNSUPPORTED                      (0x03)

    /** @ingroup codec_status
      Unsupported version. */
#define CDC_ENOTFOUND                          (0x04)

    /** @ingroup codec_status
      Unexpected problem was encountered. */
#define CDC_EUNEXPECTED                       (0x05)

    /** @ingroup codec_status
      Unhandled problem occurred. */
#define CDC_EPANIC                            (0x06)

    /** @ingroup codec_status
      Unable to allocate resource(s). */
#define CDC_ENORESOURCE                       (0x07)

    /** @ingroup codec_status
      Invalid handle. */
#define CDC_EHANDLE                           (0x08)

    /** @ingroup codec_status
      Operation is already processed. */
#define CDC_EALREADY                          (0x09)

    /** @ingroup codec_status
      Operation is not ready to be processed. */
#define CDC_ENOTREADY                         (0x0A)

    /** @ingroup codec_status
      Operation is pending completion. */
#define CDC_EPENDING                          (0x0B)

    /** @ingroup codec_status
      Operation cannot be accepted or processed. */
#define CDC_EBUSY                             (0x0C)

    /** @ingroup codec_status
      Operation aborted due to an error. */
#define CDC_EABORTED                          (0x0D)

    /** @ingroup codec_status
      Operation was preempted by a higher priority. */
#define CDC_EPREEMPTED                        (0x0E)

    /** @ingroup codec_status
      Operation requires intervention to complete. */
#define CDC_ECONTINUE                         (0x0F)

    /** @ingroup codec_status
      Operation requires immediate intervention to complete. */
#define CDC_EIMMEDIATE                        (0x10)

    /** @ingroup codec_status
      Operation is not implemented. */
#define CDC_ENOTIMPL                          (0x11)

    /** @ingroup codec_status
      Operation requires more data or resources. */
#define CDC_ENEEDMORE                         (0x12)

    /** @ingroup codec_status
      Operation is a local procedure call. */
#define CDC_ELPC                              (0x13)

    /** @ingroup codec_status
      Unable to allocate enough memory(s). */
#define CDC_ENOMEMORY                         (0x14)

    /** @ingroup codec_status
      Item does not exist. */
#define CDC_ENOTEXIST                         (0x15)

    /** @ingroup codec_status
      Generic BUS failure from the low-level interface. */
#define CDC_EBUS                              (0x16)


#endif //_CDC_INTF_STATUS_H_
