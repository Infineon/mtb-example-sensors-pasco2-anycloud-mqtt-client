/*****************************************************************************
 * File name: pasco2_task.c
 *
 * Description: This file uses PASCO2 library APIs to demonstrate the use of
 * CO2 sensor.
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
#include <stdio.h>

/* Header file includes */
#include "cybsp.h"
#include "cyhal.h"

/* Header file for local task */
#include "pasco2_config_task.h"
#include "pasco2_task.h"
#include "publisher_task.h"

/* Output pin for sensor PSEL line */
#define MTB_PASCO2_PSEL (P5_3)
/* Pin state to enable I2C channel of sensor */
#define MTB_PASCO2_PSEL_I2C_ENABLE (0U)
/* Output pin for PAS CO2 Wing Board power switch */
#define MTB_PASCO2_POWER_SWITCH (P10_5)
/* Pin state to enable power to sensor on PAS CO2 Wing Board*/
#define MTB_PASCO2_POWER_ON (1U)

/* Output pin for PAS CO2 Wing Board LED OK */
#define MTB_PASCO2_LED_OK (P9_0)
/* Output pin for PAS CO2 Wing Board LED WARNING  */
#define MTB_PASCO2_LED_WARNING (P9_1)

/* Pin state for PAS CO2 Wing Board LED off. */
#define MTB_PASCO_LED_STATE_OFF (0U)
/* Pin state for PAS CO2 Wing Board LED on. */
#define MTB_PASCO_LED_STATE_ON (1U)

/* I2C bus frequency */
#define I2C_MASTER_FREQUENCY (100000U)

/* Delay time after hardware initialization */
#define PASCO2_INITIALIZATION_DELAY (2000)
/* Delay time for retry when sensor data is not ready */
#define PASCO2_RETRY_DELAY (1000)

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
TaskHandle_t pasco2_task_handle = NULL;

/* Semaphore to protect PASCO2 driver context */
SemaphoreHandle_t sem_pasco2_context = NULL;
/* CO2 driver context */
mtb_pasco2_context_t mtb_pasco2_context = {0};

/*******************************************************************************
 * Local Variables
 ******************************************************************************/
static char local_pub_msg[MQTT_PUB_MSG_MAX_SIZE] = {0};

/*******************************************************************************
 * Function Name: pasco2_task
 *******************************************************************************
 * Summary:
 *   Initializes context object of PASCO2 library, sets default parameters values
 *   for sensor and continuously acquire data from sensor.
 *
 * Parameters:
 *   arg: thread
 *
 * Return:
 *   none
 ******************************************************************************/
void pasco2_task(void *pvParameters)
{
    cy_rslt_t result;

    /* To avoid compiler warnings */
    (void)pvParameters;

    /* I2C variables */
    cyhal_i2c_t cyhal_i2c;

    /* initialize i2c library*/
    cyhal_i2c_cfg_t i2c_master_config = {CYHAL_I2C_MODE_MASTER,
                                         0 /* address is not used for master mode */,
                                         I2C_MASTER_FREQUENCY};

    result = cyhal_i2c_init(&cyhal_i2c, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    result = cyhal_i2c_configure(&cyhal_i2c, &i2c_master_config);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize and enable PAS CO2 Wing Board power switch */
    cyhal_gpio_init(MTB_PASCO2_POWER_SWITCH, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, MTB_PASCO2_POWER_ON);
    /* Initialize and enable PAS CO2 Wing Board I2C channel communication*/
    cyhal_gpio_init(MTB_PASCO2_PSEL, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, MTB_PASCO2_PSEL_I2C_ENABLE);
    /* Initialize the LEDs on PAS CO2 Wing Board */
    cyhal_gpio_init(MTB_PASCO2_LED_OK, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, MTB_PASCO_LED_STATE_OFF);
    cyhal_gpio_init(MTB_PASCO2_LED_WARNING, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, MTB_PASCO_LED_STATE_OFF);

    /* Delay 2s to wait for pasco2 sensor get ready */
    vTaskDelay(pdMS_TO_TICKS(PASCO2_INITIALIZATION_DELAY));

    /* Initialize PAS CO2 sensor with default parameter values */
    result = mtb_pasco2_init(&mtb_pasco2_context, &cyhal_i2c);
    if (result != CY_RSLT_SUCCESS)
    {
        /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
        // printf("\x1b[2J\x1b[;H");
        if (CY_RSLT_GET_CODE(result) == MTB_PASCO2_SENSOR_NOT_FOUND)
        {
            printf("PAS CO2 Wing Board not found... Task suspend\n\n");
        }
        else
        {
            printf("An unexpected occurred during initialization of CO2 sensor, task suspend.\n\n");
        }
        vTaskSuspend(NULL);
    }

    /* Initiate semaphore mutex to protect 'pasco2_context' */
    sem_pasco2_context = xSemaphoreCreateMutex();
    if (sem_pasco2_context == NULL)
    {
        printf(" 'sem_pasco2_context' semaphore creation failed... Task suspend\n\n");
        vTaskSuspend(NULL);
    }

    /**
     * Create task for pasco2 sensor module configuration. Configuration parameters come from
     * Subscriber task. Subscribed topics are configured inside 'mqtt_client_config.c'.
     */
    if (pdPASS != xTaskCreate(pasco2_config_task,
                              PASCO2_CONFIG_TASK_NAME,
                              PASCO2_CONFIG_TASK_STACK_SIZE,
                              NULL,
                              PASCO2_CONFIG_TASK_PRIORITY,
                              &pasco2_config_task_handle))
    {
        printf("Failed to create PASCO2 config task!\n");
        CY_ASSERT(0);
    }

    /* Turn off User LED on CYSBSYSKIT-DEV-01 to indicate successful initialization of CO2 Wing Board */
    cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
    /* Turn on status LED on PAS CO2 Wing Board to indicate normal operation */
    cyhal_gpio_write(MTB_PASCO2_LED_OK, MTB_PASCO_LED_STATE_ON);

    for (;;)
    {
        uint16_t ppm = 0;

        if (xSemaphoreTake(sem_pasco2_context, portMAX_DELAY) == pdTRUE)
        {
            /* Read CO2 value from sensor */
            result = mtb_pasco2_get_ppm(&mtb_pasco2_context, &ppm);
            xSemaphoreGive(sem_pasco2_context);
        }

        if (result == CY_RSLT_SUCCESS)
        {
            /* New CO2 value is successfully read from sensor and print it to serial console */
            printf("CO2 PPM Level: %d\r\n", ppm);

            /* Turn-off warning LED*/
            cyhal_gpio_write(MTB_PASCO2_LED_WARNING, MTB_PASCO_LED_STATE_OFF);

            snprintf(local_pub_msg, sizeof(local_pub_msg), "{\"CO2 PPM Level\": \"%d\"}", ppm);
            /**
             * Send message back to publish queue. If queue is full, 'local_pub_msg' will be dropped.
             * So no result checking. */
            xQueueSendToBack(mqtt_pub_q, local_pub_msg, 0);

            vTaskDelay(PASCO2_PROCESS_DELAY);
            continue;
        }
        else if (CY_RSLT_GET_TYPE(result) == CY_RSLT_TYPE_INFO)
        {
            /* Sensor gave other information than CO2 value */
            if (CY_RSLT_GET_CODE(result) == MTB_PASCO2_PPM_PENDING)
            {
                /* New value is not available yet */
                printf("[INFO]: CO2 PPM value is not ready\r\n");
            }
            else if (CY_RSLT_GET_CODE(result) == MTB_PASCO2_SENSOR_BUSY)
            {
                /* Sensor is busy in internal processing */
                printf("[INFO]: CO2 sensor is busy\r\n");
            }
            else
            {
                printf("[INFO]: An unexpected occurred when accessing the CO2 sensor\r\n");
            }

            /* Turn-Off warning LED */
            cyhal_gpio_write(MTB_PASCO2_LED_WARNING, MTB_PASCO_LED_STATE_OFF);

            /* Sensor is polled in 'PASCO2_RETRY_DELAY' again */
            vTaskDelay(PASCO2_RETRY_DELAY);
        }
        else if (CY_RSLT_GET_TYPE(result) == CY_RSLT_TYPE_WARNING)
        {
            /* Sensor gave a warning regarding over-voltage, temperature, or communication problem */
            switch (CY_RSLT_GET_CODE(result))
            {
                case MTB_PASCO2_VOLTAGE_ERROR:
                    /* Sensor detected over-voltage problem */
                    printf("[WARNING]: CO2 Sensor Over-Voltage Error\r\n");
                    break;
                case MTB_PASCO2_TEMPERATURE_ERROR:
                    /* Sensor detected temperature problem */
                    printf("[WARNING]: CO2 Sensor Temperature Error\r\n");
                    break;
                case MTB_PASCO2_COMMUNICATION_ERROR:
                    /* Sensor detected communication problem with MCU */
                    printf("[WARNING]: CO2 Sensor Communication Error\r\n");
                    break;
                default:
                    printf("[WARNING]: Unexpected error\r\n");
                    break;
            }

            /* Turn-On warning LED to indicate warning to user from sensor */
            cyhal_gpio_write(MTB_PASCO2_LED_WARNING, MTB_PASCO_LED_STATE_ON);
        }
    }
}

/*******************************************************************************
 * Function Name: pasco2_task_cleanup
 ********************************************************************************
 * Summary:
 *   Cleanup all resources pasco2_task has used/created.
 *
 * Parameters:
 *   void
 *
 * Return:
 *   void
 *******************************************************************************/
void pasco2_task_cleanup(void)
{
    if (pasco2_config_task_handle != NULL)
    {
        vTaskDelete(pasco2_config_task_handle);
    }
}

/* [] END OF FILE */
