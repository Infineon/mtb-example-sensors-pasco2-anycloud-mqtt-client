/******************************************************************************
 * File Name:   pasco2_task.h
 *
 * Description: This file contains the function prototypes and constants used
 *   in pasco2_task.c.
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

/* Header file includes */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/* Header file for library */
#include "mtb_pasco2.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
#define PASCO2_TASK_NAME       "PASCO2 SENSOR TASK"
#define PASCO2_TASK_PRIORITY   (2)
#define PASCO2_TASK_STACK_SIZE (1024 * 4)

/* Delay time after each call to Ifx_RadarSensing_Process */
#define PASCO2_PROCESS_DELAY (10000)

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
extern TaskHandle_t pasco2_task_handle;

extern SemaphoreHandle_t sem_pasco2_context;
extern mtb_pasco2_context_t mtb_pasco2_context;

/*******************************************************************************
 * Functions
 *******************************************************************************/
void pasco2_task(void *pvParameters);
void pasco2_task_cleanup(void);

/* [] END OF FILE */
