/******************************************************************************
 * File Name:   subscriber_task.h
 *
 * Description: This file is the public interface of subscriber_task.c
 *
 * Related Document: See README.md
 *
 * ===========================================================================
 * Copyright (C) 2021 Infineon Technologies AG. All rights reserved.
 * ===========================================================================
 *
 * ===========================================================================
 * Infineon Technologies AG (INFINEON) is supplying this file for use
 * exclusively with Infineon's sensor products. This file can be freely
 * distributed within development tools and software supporting such
 * products.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON
 * WHATSOEVER.
 * ===========================================================================
 */

#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Task parameters for Subscriber Task. */
#define SUBSCRIBER_TASK_PRIORITY           (2)
#define SUBSCRIBER_TASK_STACK_SIZE         (1024 * 2)

/* 32-bit task notification value denoting the device (LED) state. */
#define DEVICE_ON_STATE                    (0x00lu)
#define DEVICE_OFF_STATE                   (0x01lu)

/*******************************************************************************
* Extern Variables
*******************************************************************************/
extern TaskHandle_t subscriber_task_handle;
extern SemaphoreHandle_t sem_sub_payload;
extern char sub_msg_payload[512];

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void subscriber_task(void *pvParameters);
void mqtt_unsubscribe(void);

/* [] END OF FILE */
