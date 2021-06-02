/******************************************************************************
 * File Name:   mqtt_task.h
 *
 * Description: This file is the public interface of mqtt_task.c
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
#include "iot_mqtt.h"
#include "queue.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
/* Task parameters for MQTT Client Task. */
#define MQTT_CLIENT_TASK_PRIORITY   (2)
#define MQTT_CLIENT_TASK_STACK_SIZE (1024 * 2)

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/* Data-type of various MQTT operation results. */
typedef enum
{
    MQTT_SUBSCRIBE_SUCCESS,
    MQTT_SUBSCRIBE_FAILURE,
    MQTT_PUBLISH_SUCCESS,
    MQTT_PUBLISH_FAILURE,
    MQTT_DISCONNECT
} mqtt_result_t;

/*******************************************************************************
 * Extern variables
 ******************************************************************************/
extern IotMqttConnection_t mqttConnection;
extern QueueHandle_t mqtt_status_q;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
void mqtt_client_task(void *pvParameters);

/* [] END OF FILE */
