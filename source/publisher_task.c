/******************************************************************************
 * File Name:   publisher_task.c
 *
 * Description: This file contains the task that and publishes MQTT messages on
 *              the topic 'MQTT_PUB_TOPIC', which shows the event data or meta
 *              data of connected sensor module.
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

#include "FreeRTOS.h"
#include "cybsp.h"
#include "cyhal.h"

/* Task header files */
#include "mqtt_task.h"
#include "publisher_task.h"
#include "subscriber_task.h"

/* Configuration file for MQTT client */
#include "mqtt_client_config.h"

/* Middleware libraries */
#include "cy_retarget_io.h"
#include "iot_mqtt.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
/* The maximum number of times each PUBLISH in this example will be retried. */
#define PUBLISH_RETRY_LIMIT (10)

/* A PUBLISH message is retried if no response is received within this
 * time (in milliseconds).
 */
#define PUBLISH_RETRY_MS (1000)

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/* FreeRTOS task handle for this task. */
TaskHandle_t publisher_task_handle;

QueueHandle_t mqtt_pub_q;

/* Structure to store publish message information. */
IotMqttPublishInfo_t publishInfo = {.qos = (IotMqttQos_t)MQTT_MESSAGES_QOS,
                                    .pTopicName = MQTT_PUB_TOPIC,
                                    .topicNameLength = (sizeof(MQTT_PUB_TOPIC) - 1),
                                    .retryMs = PUBLISH_RETRY_MS,
                                    .retryLimit = PUBLISH_RETRY_LIMIT};

/******************************************************************************
 * Function Name: publisher_task
 ******************************************************************************
 * Summary:
 *  Task that handles initialization of the user button GPIO, configuration of
 *  ISR, and publishing of MQTT messages to control the device that is actuated
 *  by the subscriber task.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void publisher_task(void *pvParameters)
{
    /* Status variable */
    int result;

    /* Status of MQTT publish operation that will be communicated to MQTT
     * client task using a message queue in case of failure during publish.
     */
    mqtt_result_t mqtt_publish_status = MQTT_PUBLISH_FAILURE;

    /* To avoid compiler warnings */
    (void)pvParameters;

    mqtt_pub_q = xQueueCreate(MQTT_PUB_QUEUE_LENGTH, MQTT_PUB_MSG_MAX_SIZE);
    if (mqtt_pub_q == NULL)
    {
        printf(" 'mqtt_pub_q' queue creation failed... Task suspend\n\n");
        vTaskSuspend(NULL);
    }

    /* Variable to receive new publish message from 'mqtt_pub_q' */
    char pub_msg[MQTT_PUB_MSG_MAX_SIZE];

    while (true)
    {
        memset(pub_msg, '\0', MQTT_PUB_MSG_MAX_SIZE);
        /* Wait for publish requirement from other tasks and callbacks. */
        if (pdTRUE == xQueueReceive(mqtt_pub_q, pub_msg, portMAX_DELAY))
        {
            /* Assign the publish message payload according to received device state. */
            publishInfo.pPayload = pub_msg;
            publishInfo.payloadLength = strlen(publishInfo.pPayload);

            printf("Publishing '%s' on the topic '%s'\n\n", (char *)publishInfo.pPayload, publishInfo.pTopicName);

            /* Publish the MQTT message with the configured settings. */
            result = IotMqtt_PublishSync(mqttConnection, &publishInfo, 0, MQTT_TIMEOUT_MS);

            if (result != IOT_MQTT_SUCCESS)
            {
                /* Inform the MQTT client task about the publish failure and suspend
                 * the task for it to be killed by the MQTT client task later.
                 */
                printf("MQTT Publish failed with error '%s'.\n\n", IotMqtt_strerror((IotMqttError_t)result));
                xQueueOverwrite(mqtt_status_q, &mqtt_publish_status);
                vTaskSuspend(NULL);
            }
        }
    }
}

/******************************************************************************
 * Function Name: publisher_cleanup
 ******************************************************************************
 * Summary:
 *  Cleanup function for the publisher task that disables the user button
 *  interrupt. This operation needs to be necessarily performed before deleting
 *  the publisher task.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void publisher_cleanup(void)
{
    vQueueDelete(mqtt_pub_q);
}

/* [] END OF FILE */
