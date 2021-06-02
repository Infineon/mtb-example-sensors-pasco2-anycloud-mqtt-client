/******************************************************************************
 * File Name:   pasco2_config_task.c
 *
 * Description: This file contains the task that handles parsing the new
 *              configuration coming from remote server and setting it to the
 *              sensor-xensiv-pasco2 library.
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

/* Header file from system */
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

/* Header file from library */
#include "cy_json_parser.h"

/* Header file for local tasks */
#include "pasco2_config_task.h"
#include "pasco2_task.h"
#include "publisher_task.h"
#include "subscriber_task.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
TaskHandle_t pasco2_config_task_handle = NULL;

/*******************************************************************************
 * Local Variables
 ******************************************************************************/
static mtb_pasco2_config_t pasco2_config = {0};
static char local_pub_msg[MQTT_PUB_MSG_MAX_SIZE] = {0};
static char json_value[32] = {0};

/*******************************************************************************
 * Function Name: json_parser_cb
 *******************************************************************************
 * Summary:
 *   Callback function that parses incoming json string.
 *
 * Parameters:
 *   json_object: incoming json object
 *   arg: callback data. Here it should be the context of mtb_pasco2_context_t.
 *
 * Return:
 *   none
 ******************************************************************************/
static cy_rslt_t json_parser_cb(cy_JSON_object_t *json_object, void *arg)
{
    mtb_pasco2_context_t *context = (mtb_pasco2_context_t *)arg;

    bool bad_entry = false;
    /* Reset and get new value for each new json object entry */
    memset(json_value, '\0', 32);
    memcpy(json_value, json_object->value, json_object->value_length);

    /* Supported keys and values for pasco2 configuration */
    if (memcmp(json_object->object_string, "pasco2_measurement_period", json_object->object_string_length) == 0)
    {
        pasco2_config.measurement_period = atoi(json_value);
        if (mtb_pasco2_set_config(context, &pasco2_config) == MTB_PASCO2_SUCCESS)
        {
            snprintf(local_pub_msg,
                     sizeof(local_pub_msg),
                     "Config => %.*s: %.*s",
                     json_object->object_string_length,
                     json_object->object_string,
                     json_object->value_length,
                     json_object->value);
        }
        else
        {
            snprintf(local_pub_msg, sizeof(local_pub_msg), "pasco2_measurement_period set configuration failed.");
        }
    }
    else
    {
        /* Invalid input json key */
        bad_entry = true;
        snprintf(local_pub_msg,
                 sizeof(local_pub_msg),
                 "{\"%.*s\": \"invalid json key\"}",
                 json_object->object_string_length,
                 json_object->object_string);
    }

    /* Send message back to publish queue. If queue is full, 'local_pub_msg' will be dropped. So no result checking. */
    xQueueSendToBack(mqtt_pub_q, local_pub_msg, 0);

    return bad_entry ? CY_RSLT_JSON_GENERIC_ERROR : CY_RSLT_SUCCESS;
}

/*******************************************************************************
 * Function Name: pasco2_config_task
 *******************************************************************************
 * Summary:
 *      Parse incoming json string, and set new configuration to
 *      sensor-xensiv-pasco2 library.
 *
 * Parameters:
 *   pvParameters: thread
 *
 * Return:
 *   none
 ******************************************************************************/
void pasco2_config_task(void *pvParameters)
{
    cy_rslt_t result;

    /* To avoid compiler warnings */
    (void)pvParameters;

    /* Register JSON parser to parse input configuration JSON string */
    cy_JSON_parser_register_callback(json_parser_cb, (void *)&mtb_pasco2_context);

    while (true)
    {
        /* Block till a notification is received from the subscriber task. */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* Get mutex to block any other json parse jobs */
        if (xSemaphoreTake(sem_sub_payload, portMAX_DELAY) == pdTRUE)
        {
            /* Get mutex to block mtb_radar_sensing_process in radar task */
            if (xSemaphoreTake(sem_pasco2_context, portMAX_DELAY) == pdTRUE)
            {
                result = cy_JSON_parser(sub_msg_payload, strlen(sub_msg_payload));
                if (result != CY_RSLT_SUCCESS)
                {
                    printf("pasco2_config_task: json parser error!\n");
                }
                xSemaphoreGive(sem_pasco2_context);
            }
            xSemaphoreGive(sem_sub_payload);
        }
    }
}

/* [] END OF FILE */
