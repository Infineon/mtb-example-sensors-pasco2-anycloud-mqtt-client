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
#include <inttypes.h>
#include <stdio.h>

/* Header file includes */
#include "cybsp.h"
#include "cyhal.h"

/* Header file for local task */
#include "pasco2_config_task.h"
#include "pasco2_task.h"
#include "publisher_task.h"
#include "xensiv_dps3xx_mtb.h"

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

#define DEFAULT_PRESSURE_VALUE (1015.0F)

/* Delay time after hardware initialization */
#define PASCO2_INITIALIZATION_DELAY (2000)

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
TaskHandle_t pasco2_task_handle = NULL;
/* Semaphore to protect PASCO2 driver context */
SemaphoreHandle_t sem_pasco2_context = NULL;
xensiv_pasco2_t xensiv_pasco2 = {0};
/* Delay time after each call to PAS CO2 Process.Default is 10 seconds */
uint32_t pasco2_process_delay_s = 10;

/*******************************************************************************
 * Local Variables
 ******************************************************************************/

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

    xensiv_dps3xx_t xensiv_dps3xx;
    bool use_dps = true;

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

    /* Initialize and enable PAS CO2 Wing Board I2C channel communication*/
    result = cyhal_gpio_init(MTB_PASCO2_PSEL, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, MTB_PASCO2_PSEL_I2C_ENABLE);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize and enable PAS CO2 Wing Board power switch */
    result = cyhal_gpio_init(MTB_PASCO2_POWER_SWITCH, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, MTB_PASCO2_POWER_ON);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the LEDs on PAS CO2 Wing Board */
    result = cyhal_gpio_init(MTB_PASCO2_LED_OK, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, MTB_PASCO_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    result = cyhal_gpio_init(MTB_PASCO2_LED_WARNING, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, MTB_PASCO_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Delay 2s to wait for pasco2 sensor get ready */
    vTaskDelay(pdMS_TO_TICKS(PASCO2_INITIALIZATION_DELAY));

    result = xensiv_dps3xx_mtb_init_i2c(&xensiv_dps3xx, &cyhal_i2c, XENSIV_DPS3XX_I2C_ADDR_ALT);
    if (result != CY_RSLT_SUCCESS)
    {
        use_dps = false;
    }

    /* Initialize PAS CO2 sensor with default parameter values */
    result = xensiv_pasco2_mtb_init_i2c(&xensiv_pasco2, &cyhal_i2c);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("PAS CO2 device initialization error\n");
        printf("Exiting pasco2_task task\n");
        // exit current thread (suspend)
        vTaskSuspend(NULL);
    }
    /* Configure PAS CO2 Wing board interrupt to enable voltage converter */
    xensiv_pasco2_interrupt_config_t int_config = {
    .b.int_func = XENSIV_PASCO2_INTERRUPT_FUNCTION_EARLY,
    .b.int_typ = (uint32_t)XENSIV_PASCO2_INTERRUPT_TYPE_HIGH_ACTIVE
    };
    result = xensiv_pasco2_set_interrupt_config(&xensiv_pasco2, int_config);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("PAS CO2 interrupt configuration error");
        CY_ASSERT(0);
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

    /* Stop LED blinking timer, turn on LED to indicate user that turn-on phase is over and entering ready state */
    result = cyhal_timer_stop(&led_blink_timer);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
    cyhal_gpio_write(CYBSP_USER_LED, false); /* USER_LED is active low */
    /* Turn on status LED on PAS CO2 Wing Board to indicate normal operation */
    cyhal_gpio_write(MTB_PASCO2_LED_OK, MTB_PASCO_LED_STATE_ON);

    publisher_data_t publisher_q_data = {0};
    publisher_q_data.cmd = PUBLISH_MQTT_MSG;

    for (;;)
    {
        uint16_t ppm = 0;

        if (xSemaphoreTake(sem_pasco2_context, portMAX_DELAY) == pdTRUE)
        {
            /* Read pressure value from sensor */
        float32_t pressure;
        if (use_dps == true)
        {
            float32_t temperature;
            /* Read pressure value from sensor */
            result = xensiv_dps3xx_read(&xensiv_dps3xx, &pressure, &temperature);
            if (result != CY_RSLT_SUCCESS)
            {
                printf("Error while reading from pressure sensor\r\n");
                CY_ASSERT(0);
            }
        }
        else
        {
            pressure = DEFAULT_PRESSURE_VALUE;
        }
            /* Read CO2 value from sensor */
            result = xensiv_pasco2_mtb_read(&xensiv_pasco2, (uint16_t)pressure, &ppm);

            xSemaphoreGive(sem_pasco2_context);
        }

        if (result == CY_RSLT_SUCCESS)
        {
            /* Turn-off warning LED*/
            cyhal_gpio_write(MTB_PASCO2_LED_WARNING, MTB_PASCO_LED_STATE_OFF);

            memset(publisher_q_data.data, 0, sizeof(publisher_q_data.data));
            snprintf(publisher_q_data.data, sizeof(publisher_q_data.data), "{\"CO2 PPM Level\": \"%d\"}", ppm);
            /**
             * Send message back to publish queue. If queue is full, 'local_pub_msg' will be dropped.
             * So no result checking. */
            xQueueSendToBack(publisher_task_q, &publisher_q_data, 0);
        }
        else
        {
            if (CY_RSLT_GET_CODE(result) == XENSIV_PASCO2_READ_NRDY)
            {
                /* New value is not available yet */
                printf("CO2 PPM value is not ready\n");
            }
            else if (CY_RSLT_GET_CODE(result) == XENSIV_PASCO2_ERR_COMM)
            {
                /* I2C communication error */
                printf("I2C communication error\n");
            }
            else
            {
                printf("Unexpected error\n");
            }
        }

        xensiv_pasco2_status_t sensor_status;
        if (xensiv_pasco2_get_status(&xensiv_pasco2, &sensor_status) == CY_RSLT_SUCCESS)
        {
            bool error_status = false;
            if (sensor_status.u & XENSIV_PASCO2_REG_SENS_STS_ICCER_MSK)
            {
                /* Sensor detected communication problem with MCU */
                printf("CO2 Sensor Communication Error\n");
                error_status = true;
            }

            if (sensor_status.u & XENSIV_PASCO2_REG_SENS_STS_ORVS_MSK)
            {
                /* Sensor detected over-voltage problem */
                printf("CO2 Sensor Over-Voltage Error\n");
                error_status = true;
            }

            if (sensor_status.u & XENSIV_PASCO2_REG_SENS_STS_ORTMP_MSK)
            {
                /* Sensor detected temperature problem */
                printf("CO2 Sensor Temperature Error\n");
                error_status = true;
            }

            /* Turn-On warning LED to indicate warning to user from sensor */
            cyhal_gpio_write(MTB_PASCO2_LED_WARNING, error_status ? MTB_PASCO_LED_STATE_ON : MTB_PASCO_LED_STATE_OFF);
        }

        vTaskDelay(pdMS_TO_TICKS(pasco2_process_delay_s*1000));
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
