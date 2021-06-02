/******************************************************************************
 * File Name:   subscriber_task.c
 *
 * Description: This file contains the task that subscribes to the topic
 *              'MQTT_SUB_TOPIC', and configure the sensor module based on the
 *              notifications received from the MQTT subscriber callback.
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
#include "string.h"

/* Task header files */
#include "mqtt_task.h"
#include "pasco2_config_task.h"
#include "subscriber_task.h"

/* Configuration file for MQTT client */
#include "mqtt_client_config.h"

/* Middleware libraries */
#include "cy_retarget_io.h"
#include "iot_mqtt.h"

/******************************************************************************
 * Macros
 ******************************************************************************/
/* The number of MQTT topics to be subscribed to. */
#define SUBSCRIPTION_COUNT (1)

/******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static void mqtt_subscription_callback(void *pCallbackContext, IotMqttCallbackParam_t *pPublishInfo);

/******************************************************************************
 * Global Variables
 *******************************************************************************/
/* Task handle for this task. */
TaskHandle_t subscriber_task_handle;

/* Configure the subscription information structure. */
IotMqttSubscription_t subscribeInfo = {.qos = (IotMqttQos_t)MQTT_MESSAGES_QOS,
                                       .pTopicFilter = MQTT_SUB_TOPIC,
                                       .topicFilterLength = (sizeof(MQTT_SUB_TOPIC) - 1),
                                       /* Configure the callback function to handle incoming MQTT messages */
                                       .callback.function = mqtt_subscription_callback};

/* Semaphore used to protect message payload */
SemaphoreHandle_t sem_sub_payload = NULL;
/* Subscribed message paylaod. Maximum with size of 512 byte. */
char sub_msg_payload[512] = {0};

/******************************************************************************
 * Function Name: subscriber_task
 ******************************************************************************
 * Summary:
 *  Task that sets up the user LED GPIO, subscribes to topic - 'MQTT_SUB_TOPIC',
 *  and controls the user LED based on the received task notification.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void subscriber_task(void *pvParameters)
{
    /* Status variable */
    int result = EXIT_SUCCESS;

    /* Status of MQTT subscribe operation that will be communicated to MQTT
     * client task using a message queue in case of failure in subscription.
     */
    mqtt_result_t mqtt_subscribe_status = MQTT_SUBSCRIBE_FAILURE;

    /* To avoid compiler warnings */
    (void)pvParameters;

    /* Initialize semaphore to protect payload */
    sem_sub_payload = xSemaphoreCreateMutex();
    if (sem_sub_payload == NULL)
    {
        printf(" 'sem_sub_payload' semaphore creation failed... Task suspend\n\n");
        vTaskSuspend(NULL);
    }

    /* Subscribe with the configured parameters. */
    result = IotMqtt_SubscribeSync(mqttConnection, &subscribeInfo, SUBSCRIPTION_COUNT, 0, MQTT_TIMEOUT_MS);
    if (result != EXIT_SUCCESS)
    {
        /* Notify the MQTT client task about the subscription failure and
         * suspend the task for it to be killed by the MQTT client task later.
         */
        printf("MQTT Subscribe failed with error '%s'.\n\n", IotMqtt_strerror((IotMqttError_t)result));
        xQueueOverwrite(mqtt_status_q, &mqtt_subscribe_status);
        vTaskSuspend(NULL);
    }

    printf("MQTT client subscribed to the topic '%.*s' successfully.\n\n",
           subscribeInfo.topicFilterLength,
           subscribeInfo.pTopicFilter);

    vTaskSuspend(NULL);
}

/******************************************************************************
 * Function Name: mqtt_subscription_callback
 ******************************************************************************
 * Summary:
 *  Callback to handle incoming MQTT messages. This callback prints the
 *  contents of an incoming message and notifies the subscriber task with the
 *  LED state as per the received message.
 *
 * Parameters:
 *  void *pCallbackContext : Parameter defined during MQTT Subscribe operation
 *                           using the IotMqttCallbackInfo_t.pCallbackContext
 *                           member (unused)
 *  IotMqttCallbackParam_t * pPublishInfo : Information about the incoming
 *                                          MQTT PUBLISH message passed by
 *                                          the MQTT library.
 *
 * Return:
 *  void
 *
 ******************************************************************************/
static void mqtt_subscription_callback(void *pCallbackContext, IotMqttCallbackParam_t *pPublishInfo)
{
    /* Received MQTT message */
    const char *pPayload = pPublishInfo->u.message.info.pPayload;
    const size_t payloadLength = pPublishInfo->u.message.info.payloadLength;

    /* To avoid compiler warnings */
    (void)pCallbackContext;

    /* Print information about the incoming PUBLISH message. */
    printf("Incoming MQTT message received:\n"
           "Subscription topic filter: %.*s\n"
           "Published topic name: %.*s\n"
           "Published QoS: %d\n"
           "Published payload: %.*s\n\n",
           pPublishInfo->u.message.topicFilterLength,
           pPublishInfo->u.message.pTopicFilter,
           pPublishInfo->u.message.info.topicNameLength,
           pPublishInfo->u.message.info.pTopicName,
           pPublishInfo->u.message.info.qos,
           pPublishInfo->u.message.info.payloadLength,
           pPayload);

    if (payloadLength >= sizeof(sub_msg_payload))
    {
        printf("Subscribed topic: '%.*s', received message too long. Buffer overflow.\n",
               pPublishInfo->u.message.info.topicNameLength,
               pPublishInfo->u.message.info.pTopicName);
        return;
    }

    if (xSemaphoreTake(sem_sub_payload, portMAX_DELAY) == pdTRUE)
    {
        memset(sub_msg_payload, '\0', sizeof(sub_msg_payload));
        memcpy(sub_msg_payload, pPayload, payloadLength);
        xSemaphoreGive(sem_sub_payload);

        /* Notify the pasco2 configuration task. */
        xTaskNotifyGive(pasco2_config_task_handle);
    }
}

/******************************************************************************
 * Function Name: mqtt_unsubscribe
 ******************************************************************************
 * Summary:
 *  Function that unsubscribes from the topic specified by the macro
 *  'MQTT_SUB_TOPIC'. This operation is called during cleanup by the MQTT client
 *  task.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void mqtt_unsubscribe(void)
{
    IotMqttError_t result =
        IotMqtt_UnsubscribeSync(mqttConnection, &subscribeInfo, SUBSCRIPTION_COUNT, 0, MQTT_TIMEOUT_MS);
    if (result != IOT_MQTT_SUCCESS)
    {
        printf("MQTT Unsubscribe operation failed with error '%s'!\n", IotMqtt_strerror(result));
    }
}

/* [] END OF FILE */
