# AnyCloud: MQTT client with XENSIV&trade; sensors - PASCO2

## Overview

This code example demonstrates implementing an MQTT client using the [MQTT client library](https://github.com/cypresssemiconductorco/mqtt) for  XENSIV&trade; sensor with Cypress connectivity devices. The library uses the following:

 - AWS IoT device SDK MQTT client library that includes an MQTT 3.1.1 client

 - sensor-xensiv-pasco2 library that is configured for to detect the CO2 status

In this example, the MQTT client RTOS task establishes a connection with the configured MQTT broker, and creates three tasks - Publisher, Subscriber, and PASCO2. The Publisher task publishes messages on the  `MQTT_PUB_TOPIC` topic when a new CO2 value is read back. The Subscriber task subscribes to `MQTT_SUB_TOPIC`. The PASCO2 task initializes a context object of the pasco2 library and continuously reads the CO2 data from the sensor module. <br><br>

From 'pasco2_task', there is another PASCO2 configuration task created to enable dynamically configuring the sensor-xensiv-pasco2 library. From the MQTT broker, users can send JSON message to the MQTT client within `MQTT_SUB_TOPIC` to perform the configuration. <br>


## Sequence of operation

1. A new CO2 value is read from the module.

2. The pasco2_task notifies the Publisher task with the new CO2 value.

3. The Publisher task publishes this new CO2 data to the remote server on a topic defined. You can verify this data from the MQTT broker.

4. The MQTT broker sends the configuration as a JSON string to this client to configure the 'sensor-xensiv-pasco2' library. This allows the example to perform remote configuration.

**Notes:**

1. The pasco2_task is suspended if pasco2 wing board is not connected to the feather kit.

2. Check the sensor-xensiv-pasco2 library documentation for information on configuration JSON objects.

<br>

## Requirements

- [ModusToolbox&trade; software](https://www.cypress.com/products/modustoolbox-software-environment) v2.2 or later

    **Note:** This code example version requires ModusToolbox software version 2.2 or later and is not backward compatible with v2.1 or older versions.

- Board support package (BSP) minimum required version: 2.0.0
- Programming language: C
- Associated parts: All [PSoC&trade; 6 MCU](http://www.cypress.com/PSoC6) parts with SDIO, AIROC™ CYW43012Wi-Fi & Bluetooth® combo chip,  AIROC™ CYW4343W Wi-Fi & Bluetooth® combo chip


## Supported toolchains (make variable 'TOOLCHAIN')

- GNU Arm® embedded compiler v9.3.1 (`GCC_ARM`) - Default value of `TOOLCHAIN`


## Supported kit (make variable 'TARGET')

- PSoC&trade; 6 rapid IoT connect platform RP01 feather kit (`CYSBSYSKIT-DEV-01`) - Default value of `TARGET`

**Note:** This example requires a PSoC&trade; 6 MCU device with at least 2 MB flash and 1 MB SRAM, and therefore does not support other PSoC&trade; 6 MCU kits.


## Hardware setup

This example uses the board's default configuration. See the kit user guide to ensure that the board is configured correctly.


## Software setup

Install a terminal emulator if you don't have one. Instructions in this document use [Tera Term](https://ttssh2.osdn.jp/index.html.en).

This example requires no additional software or tools.


## Using the code example

Create the project and open it using one of the following:

<details><summary><b>In Eclipse IDE for ModusToolbox&trade;</b></summary>

1. Click the **New Application** link in the **Quick Panel** (or, use **File** > **New** > **ModusToolbox Application**). This launches the [Project Creator](http://www.cypress.com/ModusToolboxProjectCreator) tool.

2. Pick a kit supported by the code example from the list shown in the **Project Creator - Choose Board Support Package (BSP)** dialog.

   When you select a supported kit, the example is reconfigured automatically to work with the kit. To work with a different supported kit later, use the [Library manager](https://www.cypress.com/ModusToolboxLibraryManager) to choose the BSP for the supported kit. You can use the Library Manager to select or update the BSP and firmware libraries used in this application. To access the Library manager, click the link from the **Quick Panel**.

   You can also just start the application creation process again and select a different kit.

   If you want to use the application for a kit not listed here, you may need to update the source files. If the kit does not have the required resources, the application may not work.

3. In the **Project Creator - Select Application** dialog, choose the example by enabling the checkbox.

4. Optionally, change the suggested **New Application Name**.

5. Enter the local path in the **Application(s) Root Path** field to indicate where the application needs to be created.

   Applications that can share libraries can be placed in the same root path.

6. Click **Create** to complete the application creation process.

For more details, see the [Eclipse IDE for ModusToolbox&trade; user guide](https://www.cypress.com/MTBEclipseIDEUserGuide) (locally available at *{ModusToolbox&trade; install directory}/ide_{version}/docs/mt_ide_user_guide.pdf*).

</details>

<details><summary><b>In command-line interface (CLI)</b></summary>

ModusToolbox&trade; provides the Project Creator as both a GUI tool and a command line tool to easily create one or more ModusToolbox&trade; applications. See the "Project Creator Tools" section of the [ModusToolbox&trade; user guide](https://www.cypress.com/ModusToolboxUserGuide) for more details.

Alternatively, you can manually create the application using the following steps:

1. Download and unzip this repository onto your local machine, or clone the repository.

2. Open a CLI terminal and navigate to the application folder.

   On Windows, use the command line "modus-shell" program provided in the ModusToolbox&trade; installation instead of a standard Windows command line application. This shell provides access to all ModusToolbox&trade; tools. You can access it by typing `modus-shell` in the search box in the Windows menu.

   In Linux and macOS, you can use any terminal application.

   **Note:** The cloned application contains a default BSP file (*TARGET_xxx.mtb*) in the *deps* folder. Use the [Library manager](https://www.cypress.com/ModusToolboxLibraryManager) (`make modlibs` command) to select and download a different BSP file, if required. If the selected kit does not have the required resources or is not [supported](#supported-kits-make-variable-target), the application may not work.

3. Import the required libraries by executing the `make getlibs` command.

Various CLI tools include a `-h` option that prints help information to the terminal screen about that tool. For more details, see the [ModusToolbox&trade; user guide](https://www.cypress.com/ModusToolboxUserGuide) (locally available at *{ModusToolbox&trade; install directory}/docs_{version}/mtb_user_guide.pdf*).

</details>

<details><summary><b>In third-party IDEs</b></summary>

1. Follow the instructions from the **In command-line interface (CLI)** section to create the application, and import the libraries using the `make getlibs` command.

2. Export the application to a supported IDE using the `make <ide>` command.

    For a list of supported IDEs and more details, see the "Exporting to IDEs" section of the [ModusToolbox&trade; user guide](https://www.cypress.com/ModusToolboxUserGuide) (locally available at *{ModusToolbox&trade; install directory}/docs_{version}/mtb_user_guide.pdf*.

3. Follow the instructions displayed in the terminal to create or import the application as an IDE project.

</details>

## Operation

1. Connect the board to your PC using the provided USB cable through the KitProg3 USB connector.

2. Modify the user configuration files in the *configs* directory as follows:

      1. **Wi-Fi Configuration:** Set the Wi-Fi credentials in *configs/wifi_config.h*: Modify the macros `WIFI_SSID`, `WIFI_PASSWORD`, and `WIFI_SECURITY` to match with that of the Wi-Fi network that you want to connect.

      2. **MQTT Configuration:** Set up the MQTT client and configure the credentials in *configs/mqtt_client_config.h* as follows:

         - **For AWS IoT MQTT:**

             1. Set up the MQTT device (also known as a *Thing*) in the AWS IoT core as described in the [Getting started with AWS IoT tutorial](https://docs.aws.amazon.com/iot/latest/developerguide/iot-gs.html).

                **Note:** While setting up your device, ensure that the policy associated with this device permits all MQTT operations (*iot:Connect*, *iot:Publish*, *iot:Receive*, and *iot:Subscribe*) for the resource used by this device. For testing purposes, it is recommended to have the following policy document which allows all *MQTT Policy Actions* on all *Amazon Resource Names (ARNs)*.
                ```
                {
                    "Version": "2012-10-17",
                    "Statement": [
                        {
                            "Effect": "Allow",
                            "Action": "iot:*",
                            "Resource": "*"
                        }
                    ]
                }
                ```

             2. Set `MQTT_BROKER_ADDRESS` to your custom endpoint on the **Settings** page of the AWS IoT Console. This has the format `ABCDEFG1234567.iot.<region>.amazonaws.com`.

             3. Download the following certificates and keys that are created and activated in the previous step:
                   - A certificate for the AWS IoT Thing - *xxxxxxxxxx.certificate.pem.crt*
                   - A public key - *xxxxxxxxxx.public.pem.key*
                   - A private key - *xxxxxxxxxx.private.pem.key*
                   - Root CA "RSA 2048 bit key: Amazon Root CA 1" for AWS IoT from [CA Certificates for Server Authentication](https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html#server-authentication-certs) - *xxxxxxxxxx.AmazonRootCA1.pem*

             4. Using these certificates and keys, enter the following parameters in *mqtt_client_config.h* in Privacy-Enhanced Mail (PEM) format:
                   - `CLIENT_CERTIFICATE` - *xxxxxxxxxx.certificate.pem.crt*
                   - `CLIENT_PRIVATE_KEY` - *xxxxxxxxxx.private.pem.key*
                   - `ROOT_CA_CERTIFICATE` - *xxxxxxxxxx.AmazonRootCA1.pem*

                You can either convert the values to strings manually following the format shown in *mqtt_client_config.h* or you can use the HTML utility available [here](https://github.com/cypresssemiconductorco/amazon-freertos/blob/master/tools/certificate_configuration/PEMfileToCString.html) to convert the certificates and keys from PEM format to C string format. You need to clone the repository from GitHub to use the utility.

         - **For public Mosquitto broker:**

           **Note:** The public test MQTT broker at [test.mosquitto.org](https://test.mosquitto.org/) uses the SHA-1 hashing algorithm for certificate signing. As cautioned by Mbed TLS, SHA-1 is considered a weak message digest. The use of SHA-1 for certificate signing constitutes a security risk. It is recommended to avoid dependencies on it, and consider stronger message digests instead.

           Note that this code example enables SHA-1 support in Mbed TLS and MQTT client libraries in order to support secure TLS-based connections to the public test MQTT broker as described [here](https://github.com/cypresssemiconductorco/mqtt/tree/latest-v2.X#quick-start).

             1. Set `MQTT_BROKER_ADDRESS` as `"test.mosquitto.org"`.

             2. Set `AWS_IOT_MQTT_MODE` to `0` as you are connecting to an MQTT broker other than AWS IoT.

             3. For a *secure connection* that requires client authentication using TLS (port 8884), configure as follows:

                   - `MQTT_PORT` as `8884`.
                   - `MQTT_SECURE_CONNECTION` as `1`.
                   - Generate the client certificate and the private key according to the instructions from [test.mosquitto.org](https://test.mosquitto.org/). The root CA certificate is available [here](https://test.mosquitto.org/ssl/mosquitto.org.crt).

                     Using the client certificate, private key, and root CA certificate, configure the `CLIENT_CERTIFICATE`, `CLIENT_PRIVATE_KEY`, and `ROOT_CA_CERTIFICATE` macros respectively.

                     You can either convert the PEM format values to strings manually following the format shown in *mqtt_client_config.h* or you can use the HTML utility available [here](https://github.com/cypresssemiconductorco/amazon-freertos/blob/master/tools/certificate_configuration/PEMfileToCString.html) to convert the certificates and keys from PEM format to C string format. You need to clone the repository from GitHub to use the utility.
                   - In the application *Makefile*, set the variable `ENABLE_SECURE_MOSQUITTO_BROKER_SUPPORT` as `1` to enable SHA-1 support.

                For *non-TLS connections* (port 1883), configure the macros as follows:

                   - `MQTT_PORT` as `1883`.
                   - `MQTT_SECURE_CONNECTION` as `0`.

         - **Other MQTT client configuration macros:**

             1. `MQTT_PUB_TOPIC`: The MQTT topic to the Publisher in this example.
             2. `MQTT_SUB_TOPIC`: The MQTT topic to the Subscriber in this example.
             3. `ENABLE_LWT_MESSAGE`: Set to `1` to use MQTT's Last Will and Testament (LWT) feature. This is an MQTT message that will be published by the MQTT broker if the MQTT connection is unexpectedly closed.
             4. `GENERATE_UNIQUE_CLIENT_ID`: Every active MQTT connection must have a unique client identifier. If this macro is set to `1`, the device will generate a unique client identifier by appending a timestamp to the string specified by the `MQTT_CLIENT_IDENTIFIER` macro. This feature is useful if you are using the same code on multiple kits simultaneously.

         Although this code example provides instructions only for AWS IoT and public test Mosquitto broker, the MQTT client implemented in this example is generic. It is expected to work with other MQTT brokers (see the [list of publicly-accessible MQTT brokers](https://github.com/mqtt/mqtt.github.io/wiki/public_brokers)) with appropriate configurations.

      3. Other configuration files: You can optionally modify the configuration macros in the following files according to your application:
         - *configs/iot_config.h* used by the [MQTT library](https://github.com/cypresssemiconductorco/mqtt)

         - *configs/FreeRTOSConfig.h* used by the [FreeRTOS library](https://github.com/cypresssemiconductorco/freertos)

3. Open a terminal program and select the KitProg3 COM port. Set the serial port parameters to 8N1 and 115200 baud.

4. Program the board using one of the following:

   <details><summary><b>Using Eclipse IDE for ModusToolbox&trade;</b></summary>

      1. Select the application project in the Project Explorer.

      2. In the **Quick Panel**, scroll down, and click **\<Application Name> Program (KitProg3_MiniProg4)**.
   </details>

   <details><summary><b>Using CLI</b></summary>

     From the terminal, execute the `make program` command to build and program the application using the default toolchain to the default target. You can specify a target and toolchain manually:
      ```
      make program TARGET=<BSP> TOOLCHAIN=<toolchain>
      ```

      Example:
      ```
      make program TARGET=CYSBSYSKIT-DEV-01 TOOLCHAIN=GCC_ARM
      ```
   </details>

   After programming, the application starts automatically. Observe the messages on the UART terminal, and wait for the device to make all the required connections.

   **Figure 1. Application initialization status**

   ![](images/app_init.png)

5. Once the initialization is complete, confirm that subscription to the topic is successful.

6. If PASCO2 wing boards are connected and PSoC6 reads new CO2 data back successfully, the following messages are displayed in Tera Term:

   **Figure 2. Get new CO2 data**

   ![](images/module_output.png)

   And go to AWS IoT-Core, on the left side bar open `MQTT Test Client` from `Test` menu to subscribe `MQTT_PUB_TOPIC`, which is **pasco2_status**. Then user should see the following:

   **Figure 3. Get new CO2 data on the MQTT broker**

   ![](images/module_output_from_aws.png)

7. When you configure the device from the MQTT broker (first half of Figure 4), the configuration JSON message will be sent to the device within `MQTT_SUB_TOPIC`. The message will be printed out in Tera Term terminal. Besides, the device will check and apply the configuration, and give feedback to the MQTT broker (second half of Figure 4).

   **Figure 4. Receive configuration message on MQTT broker**

   ![](images/module_config_from_aws.png)

   **Note:** The warning message appears only because the received message is not in JSON format.


   **Figure 5. Receive configuration message**

   ![](images/module_config.png)

   **Table 1. Configuration JSON objects**

   (See XENSIV&trade; PASCO2 library documentation)

   | Key  |  Default value     | valid values |
   | :------- | :------------    | :--------------------|
   | `pasco2_measurement_period` | 10 | 10 - 4095 s|

8. Confirm that the following messages are printed when no wing boards are connected.

   **Figure 6. No wing board connected**

   ![](images/module_missing.png)

This example can be programmed on multiple kits (*Only when `GENERATE_UNIQUE_CLIENT_ID` is set to `1`*).

Alternatively, the publish and subscribe functionalities of the MQTT client can be individually verified if the MQTT broker supports a Test MQTT client like the AWS IoT.

- *To verify the subscribe functionality*: Using the Test MQTT client, publish messages on the topic specified by the `MQTT_SUB_TOPIC` macro in *mqtt_client_config.h*

- *To verify the publish functionality*: From the Test MQTT client, subscribe to the MQTT topic specified by the `MQTT_PUB_TOPIC` macro and confirm that the messages published by the kit are displayed on the Test MQTT client's console.

## Debugging

You can debug the example to step through the code. In the IDE, use the **\<Application Name> Debug (KitProg3_MiniProg4)** configuration in the **Quick Panel**. For more details, see the "Program and debug" section in the [Eclipse IDE for ModusToolbox&trade; user guide](https://www.cypress.com/MTBEclipseIDEUserGuide).

**Note:** **(Only while debugging)** On the CM4 CPU, some code in `main()` may execute before the debugger halts at the beginning of `main()`. This means that some code executes twice - once before the debugger stops execution, and again after the debugger resets the program counter to the beginning of `main()`. See [KBA231071](https://community.cypress.com/docs/DOC-21143) to learn about this and for the workaround.

## Design and implementation

This example implements three RTOS tasks: MQTT client, Publisher, and Subscriber. The main function initializes the BSP and the retarget-io library, and creates the MQTT client task.

The MQTT client task initializes the Wi-Fi connection manager (WCM) and connects to a Wi-Fi access point (AP) using the Wi-Fi network credentials that are configured in *wifi_config.h*. Upon a successful Wi-Fi connection, the task initializes the MQTT library and establishes a connection with the MQTT broker/server.

The MQTT connection is configured to be secure by default; the secure connection requires a client certificate, a private key, and the Root CA certificate of the MQTT broker that are configured in *mqtt_client_config.h*.

After a successful MQTT connection, the Subscriber and Publisher tasks are created. The MQTT client task then waits for messages from the other two tasks and callbacks, and handles the cleanup operations of various libraries if the messages indicate failure.

The Subscriber task subscribes to messages on the topic specified by the `MQTT_SUB_TOPIC` macro that can be configured in *mqtt_client_config.h*. When the subscribe operation fails, a message is sent to the MQTT client task over a message queue. When the Subscriber task receives a message from the broker, it prints the information.

The pasco2 task notifies the Publisher task upon getting a new CO2 value. The Publisher task then publishes messages (*CO2 PPM Level: xxx*) on the topic specified by the `MQTT_PUB_TOPIC` macro. When the publish operation fails, a message is sent over a queue to the MQTT client task.

When a failure has been encountered, the MQTT client task handles the cleanup operations of various libraries, thereby terminating any existing MQTT and Wi-Fi connections and deleting the MQTT, Publisher, and Subscriber tasks.


### Resources and settings

**Table 2. Application source files**

|File name            |Comments         |
| ------------------------|-------------------- |
| *main.c* |Contains the application entry point. It initializes the UART for debugging and then initializes the controller tasks|
| *mqtt_client_config.c* |Global variables for MQTT connection|
| *mqtt_task.c* |Contains the task function to: <br> 1. Establish an MQTT connection <br> 2. Start Publisher and Subscriber tasks <br> 3. Start the PASCO2 task|
| *publisher_task.c* |Contains the task function to publish message to the MQTT broker|
| *subscriber_task.c* |Contains the task function to subscribe messages from the MQTT broker|
| *pasco2_task.c* |Contains the task function to get the CO2 value from the sensor|
| *pasco2_config_task.c* |Contains the task function to configure the sensor-xensiv-pasco2 library |

<br>

## Related resources

| Application notes                                            |                                                              |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| [AN228571](https://www.cypress.com/AN228571) – Getting started with PSoC&trade; 6 MCU on ModusToolbox&trade; | Describes PSoC&trade; 6 MCU devices and how to build your first application with ModusToolbox&trade; |
| [AN215656](https://www.cypress.com/AN215656) – PSoC&trade; 6 MCU: dual-CPU system design | Describes the dual-CPU architecture in PSoC&trade; 6 MCU, and shows how to build a simple dual-CPU design |
| **Code examples**                                            |                                                              |
| [Using ModusToolbox&trade;](https://github.com/cypresssemiconductorco/Code-Examples-for-ModusToolbox-Software) | |
| **Device documentation**                                     |                                                              |
| [PSoC&trade; 6 MCU datasheets](https://www.cypress.com/search/all?f[0]=meta_type%3Atechnical_documents&f[1]=resource_meta_type%3A575&f[2]=field_related_products%3A114026) | [PSoC&trade; 6 technical reference manuals](https://www.cypress.com/search/all/PSoC%206%20Technical%20Reference%20Manual?f[0]=meta_type%3Atechnical_documents&f[1]=resource_meta_type%3A583) |
| **Development kits**                                         | Buy at www.cypress.com                                       |
| [CYSBSYSKIT-DEV-01](https://github.com/cypresssemiconductorco/TARGET_CYSBSYSKIT-DEV-01) Rapid IoT Connect Developer Kit |  |
| **Libraries**                                                |                                                              |
| PSoC&trade; 6 peripheral driver library (PDL) and docs  | [mtb-pdl-cat1](https://github.com/cypresssemiconductorco/mtb-pdl-cat1) on GitHub |
| Hardware abstraction layer (HAL) Library and docs    | [mtb-hal-cat1](https://github.com/cypresssemiconductorco/mtb-hal-cat1) on GitHub |
| Retarget-IO - A utility library to retarget the standard input/output (STDIO) messages to a UART port | [retarget-io](https://github.com/cypresssemiconductorco/retarget-io) on GitHub |
| Sensor XENSIV&trade; PASCO2 - A library to detect the CO2 value using XENSIV™ PASCO2 | [sensor-xensiv-pasco2](https://github.com/cypresssemiconductorco/sensor-xensiv-pasco2) on GitHub |
| **Middleware**                                               |                                                              |
| MQTT client library and docs                                      | [mqtt](https://github.com/cypresssemiconductorco/mqtt) on GitHub |
| Wi-Fi connection manager (WCM) library and docs                   | [wifi-connection-manager](https://github.com/cypresssemiconductorco/wifi-connection-manager) on GitHub |
| Wi-Fi middleware core library and docs                            | [wifi-mw-core](https://github.com/cypresssemiconductorco/wifi-mw-core) on GitHub|
| Links to all PSoC&trade; 6 MCU middleware                           | [psoc6-middleware](https://github.com/cypresssemiconductorco/psoc6-middleware) on GitHub |
| **Tools**                                                    |                                                              |
| [Eclipse IDE for ModusToolbox&trade;](https://www.cypress.com/modustoolbox) | The cross-platform, Eclipse-based IDE for IoT designers that supports application configuration and development targeting converged MCU and wireless systems. |

## Other resources

Cypress provides a wealth of data at www.cypress.com to help you select the right device, and quickly and effectively integrate it into your design.

For PSoC&trade; 6 MCU devices, see [How to design with PSoC&trade; 6 MCU - KBA223067](https://community.cypress.com/docs/DOC-14644) in the Cypress community.

## Document history

Document title: *CE233236* - *AnyCloud: MQTT client with XENSIV&trade; sensors - PASCO2*

| Version | Description of Change |
| ------- | --------------------- |
| 0.5.0   | New code example.      |

------

All other trademarks or registered trademarks referenced herein are the property of their respective owners.

![banner](images/ifx-cy-banner.png)

-------------------------------------------------------------------------------

© Cypress Semiconductor Corporation, 2020-2021. This document is the property of Cypress Semiconductor Corporation, an Infineon Technologies company, and its affiliates ("Cypress").  This document, including any software or firmware included or referenced in this document ("Software"), is owned by Cypress under the intellectual property laws and treaties of the United States and other countries worldwide.  Cypress reserves all rights under such laws and treaties and does not, except as specifically stated in this paragraph, grant any license under its patents, copyrights, trademarks, or other intellectual property rights.  If the Software is not accompanied by a license agreement and you do not otherwise have a written agreement with Cypress governing the use of the Software, then Cypress hereby grants you a personal, non-exclusive, nontransferable license (without the right to sublicense) (1) under its copyright rights in the Software (a) for Software provided in source code form, to modify and reproduce the Software solely for use with Cypress hardware products, only internally within your organization, and (b) to distribute the Software in binary code form externally to end users (either directly or indirectly through resellers and distributors), solely for use on Cypress hardware product units, and (2) under those claims of Cypress’s patents that are infringed by the Software (as provided by Cypress, unmodified) to make, use, distribute, and import the Software solely for use with Cypress hardware products.  Any other use, reproduction, modification, translation, or compilation of the Software is prohibited.
<br>
TO THE EXTENT PERMITTED BY APPLICABLE LAW, CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARD TO THIS DOCUMENT OR ANY SOFTWARE OR ACCOMPANYING HARDWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  No computing device can be absolutely secure.  Therefore, despite security measures implemented in Cypress hardware or software products, Cypress shall have no liability arising out of any security breach, such as unauthorized access to or use of a Cypress product.  CYPRESS DOES NOT REPRESENT, WARRANT, OR GUARANTEE THAT CYPRESS PRODUCTS, OR SYSTEMS CREATED USING CYPRESS PRODUCTS, WILL BE FREE FROM CORRUPTION, ATTACK, VIRUSES, INTERFERENCE, HACKING, DATA LOSS OR THEFT, OR OTHER SECURITY INTRUSION (collectively, "Security Breach").  Cypress disclaims any liability relating to any Security Breach, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any Security Breach.  In addition, the products described in these materials may contain design defects or errors known as errata which may cause the product to deviate from published specifications.  To the extent permitted by applicable law, Cypress reserves the right to make changes to this document without further notice. Cypress does not assume any liability arising out of the application or use of any product or circuit described in this document.  Any information provided in this document, including any sample design information or programming code, is provided only for reference purposes.  It is the responsibility of the user of this document to properly design, program, and test the functionality and safety of any application made of this information and any resulting product.  "High-Risk Device" means any device or system whose failure could cause personal injury, death, or property damage.  Examples of High-Risk Devices are weapons, nuclear installations, surgical implants, and other medical devices.  "Critical Component" means any component of a High-Risk Device whose failure to perform can be reasonably expected to cause, directly or indirectly, the failure of the High-Risk Device, or to affect its safety or effectiveness.  Cypress is not liable, in whole or in part, and you shall and hereby do release Cypress from any claim, damage, or other liability arising from any use of a Cypress product as a Critical Component in a High-Risk Device.  You shall indemnify and hold Cypress, including its affiliates, and its directors, officers, employees, agents, distributors, and assigns harmless from and against all claims, costs, damages, and expenses, arising out of any claim, including claims for product liability, personal injury or death, or property damage arising from any use of a Cypress product as a Critical Component in a High-Risk Device.  Cypress products are not intended or authorized for use as a Critical Component in any High-Risk Device except to the limited extent that (i) Cypress’s published data sheet for the product explicitly states Cypress has qualified the product for use in a specific High-Risk Device, or (ii) Cypress has given you advance written authorization to use the product as a Critical Component in the specific High-Risk Device and you have signed a separate indemnification agreement.
<br>
Cypress, the Cypress logo, and combinations thereof, WICED, ModusToolBox, PSoC, CapSense, EZ-USB, F-RAM, and Traveo are trademarks or registered trademarks of Cypress or a subsidiary of Cypress in the United States or in other countries.  For a more complete list of Cypress trademarks, visit cypress.com.  Other names and brands may be claimed as property of their respective owners.
