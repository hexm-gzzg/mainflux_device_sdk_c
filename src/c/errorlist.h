/*
 * Copyright (c) 2018
 * IoTech Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef _DEVICE_ERRORLIST_H_
#define _DEVICE_ERRORLIST_H_ 1

#define DEVICE_OK (devsdk_error){ .code = 0, .reason = "Success" }
#define DEVICE_NO_CONF_FILE (devsdk_error){ .code = 1, .reason = "Unable to open configuration file" }
#define DEVICE_CONF_PARSE_ERROR (devsdk_error){ .code = 2, .reason = "Error while parsing configuration file" }
#define DEVICE_NO_DEVICE_IMPL (devsdk_error){ .code = 3, .reason = "Device implementation data was null" }
#define DEVICE_BAD_CONFIG (devsdk_error){ .code = 4, .reason = "Configuration is invalid" }
#define DEVICE_HTTP_SERVER_FAIL (devsdk_error){ .code = 5, .reason = "Failed to start HTTP server" }
#define DEVICE_NO_DEVICE_NAME (devsdk_error){ .code = 6, .reason = "No Device name was specified" }
#define DEVICE_NO_DEVICE_VERSION (devsdk_error){ .code = 7, .reason = "No Device version was specified" }
#define DEVICE_NO_CTX (devsdk_error){ .code = 8, .reason = "No connection context supplied" }
#define DEVICE_INVALID_ARG (devsdk_error){ .code = 9, .reason = "Invalid argument" }
// errors 10..13 superceded by DEVICE_HTTP_ERROR
#define DEVICE_DRIVER_UNSTART (devsdk_error){ .code = 14, .reason = "Protocol driver initialization failed" }
#define DEVICE_REMOTE_SERVER_DOWN (devsdk_error){ .code = 15, .reason = "Remote server unresponsive" }
#define DEVICE_PROFILE_PARSE_ERROR (devsdk_error){ .code = 16, .reason = "Error while parsing device profile" }
#define DEVICE_HTTP_CONFLICT (devsdk_error){ .code = 17, .reason = "HTTP 409 Conflict" }
#define DEVICE_CONSUL_RESPONSE (devsdk_error){ .code = 18, .reason = "Unable to process response from consul" }
#define DEVICE_PROFILES_DIRECTORY (devsdk_error){ .code = 19, .reason = "Problem scanning profiles directory" }
#define DEVICE_ASSERT_FAIL (devsdk_error){ .code = 20, .reason = "A reading did not match a specified assertion string" }
#define DEVICE_HTTP_ERROR (devsdk_error){ .code = 21, .reason = "HTTP request failed" }
#endif
