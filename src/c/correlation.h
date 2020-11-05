/*
 * Copyright (c) 2019
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _DEVICE_DEVICE_CORRELATION_H
#define _DEVICE_DEVICE_CORRELATION_H 1

#define DEVICE_CRLID_HDR "correlation-id"

const char *device_get_crlid (void);
void device_alloc_crlid (const char *id);
void device_free_crlid (void);

#endif
