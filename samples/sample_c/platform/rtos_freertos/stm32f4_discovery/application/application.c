/**
 ********************************************************************
 * @file    application.c
 * @date    2019/8/23
 * @brief
 *
 * @copyright (c) 2021 DJI. All rights reserved.
 *
 * All information contained herein is, and remains, the property of DJI.
 * The intellectual and technical concepts contained herein are proprietary
 * to DJI and may be covered by U.S. and foreign patents, patents in process,
 * and protected by trade secret or copyright law.  Dissemination of this
 * information, including but not limited to data and other proprietary
 * material(s) incorporated within the information, in any form, is strictly
 * prohibited without the express written consent of DJI.
 *
 * If you receive this source code without DJI’s authorization, you may not
 * further disseminate the information, and you must immediately remove the
 * source code and notify DJI of its removal. DJI reserves the right to pursue
 * legal actions against you for any loss(es) or damage(s) caused by your
 * failure to do so.
 *
 *********************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include <widget/test_widget_speaker.h>
#include <hms/test_hms.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "usb_host.h"
#include "led.h"
#include "pps.h"
#include "apply_high_power.h"
#include "uart.h"
#include "flash_if.h"
#include "upgrade_platform_opt_stm32.h"

#include "application.h"
#include "hal_uart.h"
#include "osal.h"
#include "dji_sdk_app_info.h"
#include "dji_sdk_config.h"
#include "dji_core.h"
#include "dji_aircraft_info.h"
#include "dji_logger.h"

#include "utils/util_misc.h"
#include "camera_emu/test_payload_cam_emu_base.h"
#include "fc_subscription/test_fc_subscription.h"
#include "gimbal_emu/test_payload_gimbal_emu.h"
#include "xport/test_payload_xport.h"
#include "payload_collaboration/test_payload_collaboration.h"
#include "widget/test_widget.h"
#include "widget_interaction_test/test_widget_interaction.h"
#include "data_transmission/test_data_transmission.h"
#include "time_sync/test_time_sync.h"
#include "positioning/test_positioning.h"
#include "upgrade/test_upgrade.h"
#include "power_management/test_power_management.h"

#include "speaker.h"
#include "PWM.h"
#include "pca9685.h"
//#include <time.h>

/* Private constants ---------------------------------------------------------*/
#define RUN_INDICATE_TASK_FREQ_1HZ        1
#define RUN_INDICATE_TASK_FREQ_0D1HZ      0.1f

#define DJI_USE_WIDGET_INTERACTION        0

/* Private types -------------------------------------------------------------*/

/* Private values -------------------------------------------------------------*/
static bool s_isApplicationStart = false;

/* Private functions declaration ---------------------------------------------*/
static T_DjiReturnCode DjiUser_PrintConsole(const uint8_t *data, uint16_t dataLen);
static T_DjiReturnCode DjiUser_FillInUserInfo(T_DjiUserInfo *userInfo);

//uint8_t psdk_start[] = {'p','s','d','k','初','始','化','成','功'};

/* Exported functions definition ---------------------------------------------*/
void DjiUser_StartTask(void const *argument)
{
    T_DjiReturnCode returnCode;
    T_DjiUserInfo userInfo;
    T_DjiAircraftInfoBaseInfo aircraftInfoBaseInfo;
    T_DjiOsalHandler osalHandler = {
        .TaskCreate = Osal_TaskCreate,
        .TaskDestroy = Osal_TaskDestroy,
        .TaskSleepMs = Osal_TaskSleepMs,
        .MutexCreate= Osal_MutexCreate,
        .MutexDestroy = Osal_MutexDestroy,
        .MutexLock = Osal_MutexLock,
        .MutexUnlock = Osal_MutexUnlock,
        .SemaphoreCreate = Osal_SemaphoreCreate,
        .SemaphoreDestroy = Osal_SemaphoreDestroy,
        .SemaphoreWait = Osal_SemaphoreWait,
        .SemaphoreTimedWait = Osal_SemaphoreTimedWait,
        .SemaphorePost = Osal_SemaphorePost,
        .Malloc = Osal_Malloc,
        .Free = Osal_Free,
        .GetTimeMs = Osal_GetTimeMs,
        .GetTimeUs = Osal_GetTimeUs,
        .GetRandomNum = Osal_GetRandomNum,
    };
    T_DjiLoggerConsole printConsole = {
        .func = DjiUser_PrintConsole,
        .consoleLevel = DJI_LOGGER_CONSOLE_LOG_LEVEL_INFO,
        .isSupportColor = true,
    };
    T_DjiHalUartHandler uartHandler = {
        .UartInit = HalUart_Init,
        .UartDeInit = HalUart_DeInit,
        .UartWriteData = HalUart_WriteData,
        .UartReadData = HalUart_ReadData,
        .UartGetStatus = HalUart_GetStatus,
    };
		
    T_DjiFirmwareVersion firmwareVersion = {
        .majorVersion = 1,
        .minorVersion = 0,
        .modifyVersion = 0,
        .debugVersion = 0,
    };
		

    UART_Init(DJI_CONSOLE_UART_NUM, DJI_CONSOLE_UART_BAUD);
	//UART_Init(TTS_CONSOLE_UART_NUM, TTS_CONSOLE_UART_BAUD);
    DEBUG_USART_Config();

    //Led_Init(LED3);

//Attention: if you want to run payload sdk on extension port, please define the macro USE_USB_HOST_UART.
#if USE_USB_HOST_UART
    MX_USB_HOST_Init();
    Osal_TaskSleepMs(2000);
#endif

    returnCode = DjiPlatform_RegOsalHandler(&osalHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("register osal handler error");
        goto out;
    }

    returnCode = DjiPlatform_RegHalUartHandler(&uartHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("register hal uart handler error");
        goto out;
    }

    returnCode = DjiLogger_AddConsole(&printConsole);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("add printf console error");
        goto out;
    }

    returnCode = DjiUser_FillInUserInfo(&userInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("fill user info error, please check user info config");
        goto out;
    }

    returnCode = DjiCore_Init(&userInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("core init error");
        goto out;
    }

    returnCode = DjiAircraftInfo_GetBaseInfo(&aircraftInfoBaseInfo);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("get aircraft base info error");
        goto out;
    }

    returnCode = DjiCore_SetAlias("SENEN");
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("set alias error");
        goto out;
    }

    returnCode = DjiCore_SetFirmwareVersion(firmwareVersion);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("set firmware version error");
        goto out;
    }

    returnCode = DjiCore_SetSerialNumber("PSDK12345678XX");
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("set serial number error");
        goto out;
    }

#ifdef CONFIG_MODULE_SAMPLE_POWER_MANAGEMENT_ON
    T_DjiTestApplyHighPowerHandler applyHighPowerHandler = {
        .pinInit = DjiTest_HighPowerApplyPinInit,
        .pinWrite = DjiTest_WriteHighPowerApplyPin,
    };

    returnCode = DjiTest_RegApplyHighPowerHandler(&applyHighPowerHandler);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("register apply high power handler error");
    }

    returnCode = DjiTest_PowerManagementStartService();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("power management init error");
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_WIDGET_ON       
    returnCode = DjiTest_WidgetStartService();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("widget sample init error");
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_WIDGET_SPEAKER_ON
    returnCode = DjiTest_WidgetSpeakerStartService();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("widget speaker sample init error");
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_DATA_TRANSMISSION_ON
    returnCode = DjiTest_DataTransmissionStartService();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("widget sample init error");
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_FC_SUBSCRIPTION_ON
    returnCode = DjiTest_FcSubscriptionStartService();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("data subscription sample init error\n");
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_HMS_CUSTOMIZATION_ON
    returnCode = DjiTest_HmsCustomizationStartService();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("hms test init error");
    }
#endif
#ifdef CONFIG_MODULE_SAMPLE_CAMERA_ON
    if (aircraftInfoBaseInfo.aircraftType == DJI_AIRCRAFT_TYPE_M300_RTK
        && aircraftInfoBaseInfo.djiAdapterType == DJI_SDK_ADAPTER_TYPE_NONE) {
        USER_LOG_WARN("Not support camera emu sample.");
    } else {
        returnCode = DjiTest_CameraEmuBaseStartService();
        if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
            USER_LOG_ERROR("camera emu common init error");
        }
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_GIMBAL_EMU_ON
    if ((aircraftInfoBaseInfo.aircraftType == DJI_AIRCRAFT_TYPE_M300_RTK ||
         aircraftInfoBaseInfo.aircraftType == DJI_AIRCRAFT_TYPE_M350_RTK)
        && aircraftInfoBaseInfo.djiAdapterType == DJI_SDK_ADAPTER_TYPE_NONE) {
        USER_LOG_WARN("Not support gimbal emu sample.");
    } else {
        if (aircraftInfoBaseInfo.djiAdapterType == DJI_SDK_ADAPTER_TYPE_SKYPORT_V2 ||
            aircraftInfoBaseInfo.djiAdapterType == DJI_SDK_ADAPTER_TYPE_NONE) {
            if (DjiTest_GimbalStartService() != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
                USER_LOG_ERROR("psdk gimbal init error");
            }
        }
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_XPORT_ON
    if (aircraftInfoBaseInfo.djiAdapterType == DJI_SDK_ADAPTER_TYPE_XPORT) {
        if (DjiTest_XPortStartService() != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
            USER_LOG_ERROR("psdk xport init error");
        }
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_PAYLOAD_COLLABORATION_ON
    if (aircraftInfoBaseInfo.mountPosition != DJI_MOUNT_POSITION_EXTENSION_PORT) {
        returnCode = DjiTest_PayloadCollaborationStartService();
        if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
            USER_LOG_ERROR("Payload collaboration sample init error\n");
        }
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_TIME_SYNC_ON
    T_DjiTestTimeSyncHandler testTimeSyncHandler = {
        .PpsSignalResponseInit = DjiTest_PpsSignalResponseInit,
        .GetNewestPpsTriggerLocalTimeUs = DjiTest_GetNewestPpsTriggerLocalTimeUs,
    };

    if (DjiTest_TimeSyncRegHandler(&testTimeSyncHandler) != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("regsiter time sync handler error");
        goto out;
    }

    if (DjiTest_TimeSyncStartService() != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("psdk time sync init error");
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_POSITIONING_ON
    if ((aircraftInfoBaseInfo.aircraftType == DJI_AIRCRAFT_TYPE_M300_RTK ||
         aircraftInfoBaseInfo.aircraftType == DJI_AIRCRAFT_TYPE_M350_RTK)
        && aircraftInfoBaseInfo.mountPosition != DJI_MOUNT_POSITION_TYPE_EXTENSION_PORT) {
        if (DjiTest_PositioningStartService() != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
            USER_LOG_ERROR("psdk positioning init error");
        }
    }
#endif

#ifdef CONFIG_MODULE_SAMPLE_UPGRADE_ON
    T_DjiTestUpgradePlatformOpt stm32UpgradePlatformOpt = {
        .rebootSystem = DjiUpgradePlatformStm32_RebootSystem,
        .cleanUpgradeProgramFileStoreArea = DjiUpgradePlatformStm32_CleanUpgradeProgramFileStoreArea,
        .createUpgradeProgramFile = DjiUpgradePlatformStm32_CreateUpgradeProgramFile,
        .writeUpgradeProgramFile = DjiUpgradePlatformStm32_WriteUpgradeProgramFile,
        .readUpgradeProgramFile = DjiUpgradePlatformStm32_ReadUpgradeProgramFile,
        .closeUpgradeProgramFile = DjiUpgradePlatformStm32_CloseUpgradeProgramFile,
        .replaceOldProgram = DjiUpgradePlatformStm32_ReplaceOldProgram,
        .setUpgradeRebootState = DjiUpgradePlatformStm32_SetUpgradeRebootState,
        .getUpgradeRebootState = DjiUpgradePlatformStm32_GetUpgradeRebootState,
        .cleanUpgradeRebootState = DjiUpgradePlatformStm32_CleanUpgradeRebootState,
    };
    T_DjiTestUpgradeConfig testUpgradeConfig = {
        .firmwareVersion = {
            1,
            0,
            0,
            0
        },
        .transferType = DJI_FIRMWARE_TRANSFER_TYPE_DCFTP,
        .needReplaceProgramBeforeReboot = false
    };
    if (DjiTest_UpgradeStartService(&stm32UpgradePlatformOpt, testUpgradeConfig) !=
        DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        printf("psdk upgrade init error");
    }
#endif

    returnCode = DjiCore_ApplicationStart();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS) {
        USER_LOG_ERROR("start sdk application error");
    }

    s_isApplicationStart = true;

    //SYN_FrameInfo(psdk_start);
    while (1) {
        Osal_TaskSleepMs(500);
        //Led_Trigger(LED3);
    }

out:
    vTaskDelete(xTaskGetCurrentTaskHandle());
}

#ifndef __CC_ARM
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif

void DjiUser_MonitorTask(void const *argument)
{
    static uint32_t runIndicateTaskStep = 0;
    T_UartBufferState readBufferState = {0};
    T_UartBufferState writeBufferState = {0};
#if (configUSE_TRACE_FACILITY == 1)
    int32_t i = 0;
    int32_t j = 0;
    TaskStatus_t *lastTaskStatusArray = NULL;
    TaskStatus_t *currentTaskStatusArray = NULL;
    uint8_t lastTaskStatusArraySize = 0;
    uint8_t currentTaskStatusArraySize;
    uint8_t cpuOccupyPercentage;
#endif

    while (1) {
        Osal_TaskSleepMs(1000 / RUN_INDICATE_TASK_FREQ_0D1HZ);

        if (s_isApplicationStart == false) {
            continue;
        }
        // report UART buffer state
#ifdef USING_UART_PORT_1
        UART_GetBufferState(UART_NUM_1, &readBufferState, &writeBufferState);
        USER_LOG_DEBUG("Uart1 read buffer state: countOfLostData %d, maxUsedCapacityOfBuffer %d.",
                       readBufferState.countOfLostData, readBufferState.maxUsedCapacityOfBuffer);
        USER_LOG_DEBUG("Uart1 write buffer state: countOfLostData %d, maxUsedCapacityOfBuffer %d.",
                       writeBufferState.countOfLostData, writeBufferState.maxUsedCapacityOfBuffer);
#endif

#ifdef USING_UART_PORT_2
        UART_GetBufferState(UART_NUM_2, &readBufferState, &writeBufferState);
        USER_LOG_DEBUG("Uart2 read buffer state: countOfLostData %d, maxUsedCapacityOfBuffer %d.",
                       readBufferState.countOfLostData, readBufferState.maxUsedCapacityOfBuffer);
        USER_LOG_DEBUG("Uart2 write buffer state: countOfLostData %d, maxUsedCapacityOfBuffer %d.",
                       writeBufferState.countOfLostData, writeBufferState.maxUsedCapacityOfBuffer);
#endif

#ifdef USING_UART_PORT_3
        UART_GetBufferState(UART_NUM_3, &readBufferState, &writeBufferState);
        USER_LOG_DEBUG("Uart3 read buffer state: countOfLostData %d, maxUsedCapacityOfBuffer %d.",
                       readBufferState.countOfLostData, readBufferState.maxUsedCapacityOfBuffer);
        USER_LOG_DEBUG("Uart3 write buffer state: countOfLostData %d, maxUsedCapacityOfBuffer %d.",
                       writeBufferState.countOfLostData, writeBufferState.maxUsedCapacityOfBuffer);
#endif

#ifdef USING_UART_PORT_4
        UART_GetBufferState(UART_NUM_4, &readBufferState, &writeBufferState);
        USER_LOG_DEBUG("Uart4 read buffer state: countOfLostData %d, maxUsedCapacityOfBuffer %d.",
                       readBufferState.countOfLostData, readBufferState.maxUsedCapacityOfBuffer);
        USER_LOG_DEBUG("Uart4 write buffer state: countOfLostData %d, maxUsedCapacityOfBuffer %d.",
                       writeBufferState.countOfLostData, writeBufferState.maxUsedCapacityOfBuffer);
#endif

        // report system performance information.
        // Attention: report system performance part is not intended for normal application runtime use but as a debug aid.
        if (USER_UTIL_IS_WORK_TURN(runIndicateTaskStep++, RUN_INDICATE_TASK_FREQ_0D1HZ,
                                   RUN_INDICATE_TASK_FREQ_1HZ)) {
#if (configUSE_TRACE_FACILITY == 1)
            currentTaskStatusArraySize = uxTaskGetNumberOfTasks();
            currentTaskStatusArray = osalHandler->Malloc(currentTaskStatusArraySize * sizeof(TaskStatus_t));
            if (currentTaskStatusArray == NULL) {
                continue;
            }

            currentTaskStatusArraySize = uxTaskGetSystemState(currentTaskStatusArray, currentTaskStatusArraySize, NULL);
            USER_LOG_DEBUG("task information:");
            USER_LOG_DEBUG("task name\trun time (%%)\tstack left (byte)\tnumber");
            for (i = 0; i < currentTaskStatusArraySize; i++) {
                cpuOccupyPercentage = 0;
                for (j = 0; j < lastTaskStatusArraySize; ++j) {
                    if (currentTaskStatusArray[i].xTaskNumber == lastTaskStatusArray[j].xTaskNumber) {
                        cpuOccupyPercentage =
                            (currentTaskStatusArray[i].ulRunTimeCounter - lastTaskStatusArray[j].ulRunTimeCounter) /
                            configTICK_RATE_HZ / RUN_INDICATE_TASK_FREQ_0D1HZ;
                        break;
                    }
                }
                USER_LOG_DEBUG("%-16s\t%u\t%u\t%u", currentTaskStatusArray[i].pcTaskName, cpuOccupyPercentage,
                               (unsigned int) currentTaskStatusArray[i].usStackHighWaterMark * sizeof(StackType_t),
                               (unsigned int) currentTaskStatusArray[i].xTaskNumber);
            }
            osalHandler->Free(lastTaskStatusArray);
            lastTaskStatusArray = currentTaskStatusArray;
            lastTaskStatusArraySize = currentTaskStatusArraySize;
#endif
        }
        USER_LOG_INFO("Used heap size: %d/%d.\r\n", configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize(),
                      configTOTAL_HEAP_SIZE);
    }
}

#ifndef __CC_ARM
#pragma GCC diagnostic pop
#endif

/*mypwm founction*/
//extern TIM_HandleTypeDef g_timx_pwm_chy_handle;
int time1 = 0;
int time2 = 0;
int time3 = 0;
int time4 = 0;
int time5 = 0;
int time6 = 0;
int time7 = 0;
int time8 = 0;
int count = 0;
// void DjiUser_MyPwm1Task(void const *argument){
//     PCA9685_Init(50,180);
//     //舵机1初始化
//     setAngle(0,145);
//     //舵机2初始化
//     setAngle(1,145);
//     //舵机3初始化
//     setAngle(2,145);
//     //舵机4初始化
//     setAngle(3,145);+26
    
//     //舵机5初始化
//     setAngle(4,102);
//     //舵机6初始化
//     //setAngle(4,102);
//     //舵机7初始化
//     setAngle(5,77);
//      while(1){

//         setAngle(0,145);

//         vTaskDelay(3000);

//         setAngle(0,60);

//         vTaskDelay(3000);
//         // if(DjiUser_GetValue(2,6) == 1 ){
//         //     if(DjiUser_GetValue(2,0) == 1 ){
//         //         setAngle(0,60);
//         //     }
//         //     else{
//         //         setAngle(0,145);
//         //     }
//         //      //舵机2
//         //     if(DjiUser_GetValue(2,1) == 1){
//         //         setAngle(1,60);
//         //     }
//         //     else{
//         //         setAngle(1,145);
//         //     }
//         //     //舵机3
//         //     if(DjiUser_GetValue(2,2) == 1){
//         //         setAngle(2,60);
//         //     }
//         //     else{
//         //         setAngle(2,145);
//         //     }

//         //     //舵机4
//         //     if(DjiUser_GetValue(2,3) == 1){
//         //         setAngle(3,60);
//         //     }
//         //     else{
//         //         setAngle(3,145);
//         //     }

//         //     //舵机5
//         //     if(DjiUser_GetValue(2,4) == 1){
//         //         setAngle(4,0);
//         //         vTaskDelay(1200);
//         //         DjiUser_SetValue(2,4,0);
                
//         //     }
//         //     else{
//         //         //DjiUser_SetValue(2,4,0);
//         //         setAngle(4,102);
//         //     }
//         //     //舵机6
//         //     if(DjiUser_GetValue(2,5) == 1){
//         //         setAngle(5,179);
//         //         vTaskDelay(1200);
//         //         DjiUser_SetValue(2,5,0);
                
//         //     }
//         //     else{
//         //         setAngle(5,77);
//         //     }

//         // }

//         // else{
//         //     DjiUser_SetValue(2,0,0);
//         //     DjiUser_SetValue(2,1,0);
//         //     DjiUser_SetValue(2,2,0);
//         //     DjiUser_SetValue(2,3,0);
//         //     DjiUser_SetValue(2,4,0);
//         //     DjiUser_SetValue(2,5,0);

//         //     //舵机1初始化
//         //     setAngle(0,145);
//         //     //舵机2初始化
//         //     setAngle(1,145);
//         //     //舵机3初始化
//         //     setAngle(2,145);
//         //     //舵机4初始化
//         //     setAngle(3,145);
//         //     //舵机5初始化
//         //     setAngle(4,102);
//         //     //舵机6初始化
//         //     //setAngle(4,102);
//         //     //舵机7初始化
//         //     setAngle(5,77);
//         // }

//     }
// }
int count1 = 0;
int wegstate1 = 0;
int wegstate2 = 0;
int wegstate3 = 0;
int wegstate4 = 0;

// 自动关闭
void DjiUser_MyPwm1Task(void const *argument){
    PCA9685_Init(50,180);
    //舵机1初始化
    setAngle(0,145);
    //舵机2初始化
    setAngle(1,145);
    //舵机3初始化
    setAngle(2,145);
    //舵机4初始化
    setAngle(3,145);
    //舵机5初始化
    setAngle(4,130);
    //舵机6初始化
    //setAngle(4,102);
    //舵机7初始化
    setAngle(5,60);

    setAngle(6,170);
    //PCA9685_setPWM(0,0,103);
    // PCA9685_setPWM(1,0,103);
    // PCA9685_setPWM(2,0,103);
    // PCA9685_setPWM(3,0,103);
    // PCA9685_setPWM(4,0,236);
    // PCA9685_setPWM(5,0,236);
    // PCA9685_setPWM(6,0,236);
    // PCA9685_setPWM(7,0,236);
    
    //__HAL_TIM_SET_COMPARE(&g_timx_pwm1_chy_handle, GTIM_TIMX_PWM1_CHY, 2000);
    while(1){
        if(DjiUser_GetValue(2,6) == 1 && time1 < 4500){
            count1 = 0;
            //USER_LOG_INFO("TIME1:%d",time1);
            time1++;
             //舵机1
            //oldvalue1 = DjiUser_GetValue(2,0);
            if(DjiUser_GetValue(2,0) == 1 && time2 < 4500){
                time1=0;
                time2++;
                setAngle(0,60);
            }
            else{
                if(time2 > 4495){
                    //DjiUser_SetValue(2,0,0);
                    DjiUser_SetValue(2,6,0);
                }
                setAngle(0,145);
            }
             //舵机2
            if(DjiUser_GetValue(2,1) == 1 && time3 < 4500){
                time1=0;
                time3++;
                setAngle(1,60);
            }
            else{
                if(time3 > 4495){
                    //DjiUser_SetValue(2,1,0);
                    DjiUser_SetValue(2,6,0);
                }
                setAngle(1,145);
            }
            //舵机3
            if(DjiUser_GetValue(2,2) == 1 && time4 < 4500){
                time1=0;
                time4++;
                setAngle(2,60);
            }
            else{
                if(time4 > 4495){
                    //DjiUser_SetValue(2,2,0);
                    DjiUser_SetValue(2,6,0);
                }
                setAngle(2,145);
            }

            //舵机4
            if(DjiUser_GetValue(2,3) == 1 && time5 < 4500){
                time1=0;
                time5++;
                setAngle(3,60);
            }
            else{
                if(time5 > 4495){
                    //DjiUser_SetValue(2,3,0);
                    DjiUser_SetValue(2,6,0);
                }
                setAngle(3,145);
            }

            //舵机5
            if(DjiUser_GetValue(2,4) == 1 && time6 < 500){
                time1=0;
                time6++;
                setAngle(4,60);
                setAngle(5,130);
                vTaskDelay(1200);
                DjiUser_SetValue(2,4,0);
                
            }
            else{
                //DjiUser_SetValue(2,4,0);
                setAngle(4,130);
                setAngle(5,60);
            }
            // setAngle(3,temp3);
            // temp3 += 1;
            // if(temp3 >= 180){
            //     temp3 = 0;
            // }
            //舵机6
            if(DjiUser_GetValue(2,5) == 1 && time7 < 500){
                time1=0;
                time7++;
                setAngle(6,70);
                vTaskDelay(1200);
                DjiUser_SetValue(2,5,0);
                
            }
            else{
                setAngle(6,170);
            }

            // //舵机7
            // if(DjiUser_GetValue(2,5) == 1 && time7 < 500){
            //     time1=0;
            //     time8++;
            //     setAngle(5,170);
            //     vTaskDelay(1200);
            //     DjiUser_SetValue(2,5,0);
                
            // }
            // else{
            //     setAngle(5,70);
            // }

            if(DjiUser_GetValue(2,0) == 0){
                time2 = 0;
            }
            if(DjiUser_GetValue(2,1) == 0){
                time3 = 0;
            }
            if(DjiUser_GetValue(2,2) == 0){
                time4 = 0;
            }
            if(DjiUser_GetValue(2,3) == 0){
                time5 = 0;
            }
            if(DjiUser_GetValue(2,4) == 0){
                time6 = 0;
            }
            if(DjiUser_GetValue(2,5) == 0){
                time7 = 0;
            }
            

            //舵机7
            // if(DjiUser_GetValue(2,5) == 1){
            //     setAngle(5,0);
            //     vTaskDelay(1200);
            //     DjiUser_SetValue(2,5,0);
                
            // }
            // else{
            //     setAngle(5,102);
            // }



        }

        else{
            count1 += 1;

            if(count1 == 1){
                if(DjiUser_GetValue(2,0) == 0){
                    setAngle(0,145);
                    DjiUser_SetValue(2,0,0);
                }
                else{
                    setAngle(0,60);
                    DjiUser_SetValue(2,0,1);
                }

                if(DjiUser_GetValue(2,1) == 1){
                    setAngle(0,60);
                    DjiUser_SetValue(2,1,1);
                }
                else{
                    setAngle(1,145);
                    DjiUser_SetValue(2,1,0);
                }

                if(DjiUser_GetValue(2,2) == 1){
                    setAngle(0,60);
                    DjiUser_SetValue(2,2,1);
                }
                else{
                    setAngle(2,145);
                    DjiUser_SetValue(2,2,0);
                }

                if(DjiUser_GetValue(2,3) == 1){
                    setAngle(0,60);
                    DjiUser_SetValue(2,3,1);
                }
                else{
                    setAngle(3,145);
                    DjiUser_SetValue(2,3,0);
                }

                
                //DjiUser_SetValue(2,0,0);
                //DjiUser_SetValue(2,1,0);
                //DjiUser_SetValue(2,2,0);
                //DjiUser_SetValue(2,3,0);
                wegstate1 = DjiUser_GetValue(2,0);
                wegstate2 = DjiUser_GetValue(2,1);
                wegstate3 = DjiUser_GetValue(2,2);
                wegstate4 = DjiUser_GetValue(2,3);
                


            }

            //舵机1初始化
            //setAngle(0,145);
            //舵机2初始化
            //setAngle(1,145);
            //舵机3初始化
            //setAngle(2,145);
            //舵机4初始化
            //setAngle(3,145);
            //舵机5初始化
            DjiUser_SetValue(2,0,wegstate1);
            DjiUser_SetValue(2,1,wegstate2);
            DjiUser_SetValue(2,2,wegstate3);
            DjiUser_SetValue(2,3,wegstate4);

            DjiUser_SetValue(2,4,0);
            DjiUser_SetValue(2,5,0);
            if(time1 > 4495){
                    DjiUser_SetValue(2,6,0);
            }
            setAngle(4,130);
            //舵机6初始化
            //setAngle(4,102);
            //舵机7初始化
            setAngle(5,60);
            setAngle(6,170);

            time1 = 0;
            time2 = 0;
            time3 = 0;
            time4 = 0;
            time5 = 0;
            time6 = 0;
            time7 = 0;
            time8 = 0;
        }

        //舵机1
        if(DjiUser_GetValue(2,0) == 1){
            setAngle(0,60);
        }
        else{
            setAngle(0,145);
        }
        //舵机2
        if(DjiUser_GetValue(2,1) == 1){
            setAngle(1,60);
        }
        else{
            setAngle(1,145);
        }
        //舵机3
        if(DjiUser_GetValue(2,2) == 1){
            setAngle(2,60);
        }
        else{
            setAngle(2,145);
        }
        //舵机4
        if(DjiUser_GetValue(2,3) == 1){
            setAngle(3,60);
            // vTaskDelay(1200);
            // DjiUser_SetValue(2,4,0);
            
        }
        else{
            setAngle(3,145);
        }
        // setAngle(3,temp3);
        // temp3 += 1;
        // if(temp3 >= 180){
        //     temp3 = 0;
        // }
        // 舵机5
        // if(DjiUser_GetValue(2,5) == 1){
        //     setAngle(4,0);
        //     vTaskDelay(1200);
        //     DjiUser_SetValue(2,5,0);
            
        // }
        // else{
        //     setAngle(4,102);
        // }
        // vTaskDelay(100);
        // setAngle(0,60);
        // vTaskDelay(3000);
        // setAngle(0,145);
        // vTaskDelay(3000);
        // temp3 -= 1;
        // if(temp3 <= 0){
        //    temp3 = 143;
        // }
        // vTaskDelay(500);
        // temp3 = DjiUser_GetValue(2,4);
        // USER_LOG_INFO("temp3 = %d", temp3);
        // if(DjiUser_GetValue(2,4) == 1){
        //      setAngle(0,60);
        // }
        // else{
        //     setAngle(0, 120);
        // }

   }
}


// extern TIM_HandleTypeDef g_timx_pwm_chy_handle;
// void DjiUser_MyPwmTask(void const *argument){

//     int temp3=DjiUser_GetValue(3,3);

//     gtim_timx_pwm_chy_init(20000-1,84-1);

//     //参数1:arr 参数2:psc
//     //初始化pwm1
//     //gtim_timx_motor1_chy_init(200 , 8400);
//     //初始化pwm2
//     //gtim_timx_motor2_chy_init(200 , 8400);
//     //初始化pwm3
//     //gtim_timx_motor3_chy_init(200 , 8400);
//     //int32_t temp3=DjiUser_GetValue(3,3);                                                                                                                                                                                        )
//     //uint8_t a = 0;
//     while(1){

//         temp3=2000 - 10*DjiUser_GetValue(3,2);
//         __HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, temp3);

//         // for(int i=1000;i<=2200;i++){
// 		// 	i+=30;
// 		// 	//pwm_val+=i;
// 		// 	__HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, i);  /* ?????????? */
// 		// 	vTaskDelay(30);
// 		// }
// 		// for(int j=2200;j>=1000;j--){
// 		// 	j-=30;
// 		// 	//pwm_val+=i;
// 		// 	__HAL_TIM_SET_COMPARE(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY, j);  /* ?????????? */
// 		// 	vTaskDelay(30);
// 		// }
//         // //DjiUser_GetValue()
//     }

// }


// void DjiUser_MySpeakerTask(void const *argument){
// 	uint8_t a[] = {0x70,0x73,0x64,0x6b,0xe5,0x88,0x9d,0xe5,0xa7,0x8b,0xe5,0x8c,0x96,0xe6,0x88,0x90,0xe5,0x8a,0x9f};
//     uint8_t version[] = {0xe7,0x89,0x88,0xe6,0x9c,0xac,0xe5,0x8f,0xb7};
//     uint8_t versionchange[] = {0xe5,0xb1,0xb1,0xe8,0x83,0xbd,0xe5,0x96,0x8a,0xe8,0xaf,0x9d,0xe5,0x99,0xa8,0xe4,0xb8,0x80,0xe7,0x82,0xb9,0xe9,0x9b,0xb6,0xe7,0x82,0xb9,0xe4,0xb8,0x80};
//     vTaskDelay(5000);
//     SYN_FrameInfo(a);
//     vTaskDelay(5000);
//     SYN_FrameInfo(version);
//     vTaskDelay(1000); 
//     SYN_FrameInfo(versionchange);
//     int32_t temp1=DjiUser_GetValue(3,5)/10;
//     int32_t temp2=0;
//     int32_t temp3=DjiUser_GetValue(3,5)/10;
//     int32_t temp4=0;
// 	while(1){
//         //temp1 = DjiUser_GetValue(3,5)/10;
//         //uint8_t a[5] = {'h','e','l','l','o'};
//         if(TTS[0] != '\0'){
//             SYN_FrameInfo(TTS);
//             //*TTS = NULL;
//             TTS[0] = '\0';
//             //vTaskDelay(5000);
//         }
//         else{
//             TTS[0] = '\0';
//         }

//         //Volinme
//         if(temp1 != DjiUser_GetValue(3,5)/10){
//             temp1=DjiUser_GetValue(3,5)/10;
//             temp2=DjiUser_GetValue(3,5)/10;
//             Volinme(temp2);
//             //SYN_FrameInfo(a);
//             USER_LOG_INFO("%x\n",temp2+48);
//             vTaskDelay(1000);
//         }

//         //talker
//         // if(temp3 != DjiUser_GetValue(3,5)/20){
//         //     temp3=DjiUser_GetValue(3,5)/20;
//         //     temp4=DjiUser_GetValue(3,5)/20;
//         //     talker(temp4);
//         //     //SYN_FrameInfo(a);
//         //     USER_LOG_INFO("%x\n",temp4+48);
//         //     vTaskDelay(1000);
//         // }


//         // if(DjiUser_GetValue(3,5)/20 == 1){
//         //     Volinme(1);
//         //     vTaskDelay(1000);
//         //     SYN_FrameInfo(a);
//         //     USER_LOG_INFO("1\n");
//         // }
//         // else if (DjiUser_GetValue(3,5)/20 == 2)
//         // {
//         //     Volinme(2);
//         //     vTaskDelay(1000);
//         //     SYN_FrameInfo(a);
//         //     USER_LOG_INFO("2\n");
//         // }
//         // else if (DjiUser_GetValue(3,5)/20 == 3)
//         // {
//         //     Volinme(3);
//         //     vTaskDelay(1000);
//         //     SYN_FrameInfo(a);
//         //     USER_LOG_INFO("3\n");
//         // }
//         // else if (DjiUser_GetValue(3,5)/20 == 4)
//         // {
//         //     Volinme(4);
//         //     vTaskDelay(1000);
//         //     SYN_FrameInfo(a);
//         //     USER_LOG_INFO("4\n");
//         // }
//         // else if (DjiUser_GetValue(3,5)/20 == 5)
//         // {
//         //     Volinme(5);
//         //     vTaskDelay(1000);
//         //     SYN_FrameInfo(a);
//         //     USER_LOG_INFO("5\n");
//         // }
        
        
        
// 	}
// }

// void Led_MyTrigger(void){

//     Led_On(LED1);
//     vTaskDelay(10);
//     Led_Off(LED1);
//     vTaskDelay(150);

// }

// int count1=0;
// int count2=0;
// int countstate=0;

// int state=0;
// int iconstate1=0;
// int iconstate2=0;
// int iconstate3=0;
// void DjiUser_MyLedTask(void const *argument){
//     //state=0;
//     Led_Init(LED1);
//     count1=0;
    
//     while(1){

//         //count1++;
//         //USER_LOG_INFO("count:",count1);
//         //常亮1
//         if(DjiUser_GetValue(2,0)==0 && iconstate1 == 1){
//             Led_MyTrigger();
//             state=0;
//             iconstate1 = 0;
//         }

//         if(DjiUser_GetValue(2,0)==1 && iconstate1 == 0){
            
//             DjiUser_SetValue(2,1,0);
//             //DjiUser_SetValue(2,2,0);
//             iconstate2=0;
//             iconstate3=0;
//             switch (state)
//             {
//             case 0:
//                 Led_MyTrigger();
//                 //vTaskDelay(5000);
//                 /* code */
//                 break;
//             // case 1:
//             //     break;
//             // case 2:
//             //     Led_MyTrigger();
//             //     Led_MyTrigger();
//             // case 3:
//             //     Led_MyTrigger();
//             //     Led_MyTrigger();
//             default:
//                 Led_MyTrigger();
//                 Led_MyTrigger();
//                 break;
//             }
//             state = 1;
//             iconstate1 = 1;
//             count1 = 0;
//             vTaskDelay(5000);
//         }

//         //常量2
//         // if(DjiUser_GetValue(2,1)==0 && iconstate2 == 1){
//         //     Led_MyTrigger();
//         //     state=0;
//         //     iconstate2 = 0;
//         // }

//         // if(DjiUser_GetValue(2,1)==1 && iconstate2 == 0){
            
//         //     DjiUser_SetValue(2,0,0);
//         //     DjiUser_SetValue(2,2,0);
//         //     iconstate1=0;
//         //     iconstate3=0;
//         //     switch (state)
//         //     {
//         //     case 0:
//         //         Led_MyTrigger();
//         //         Led_MyTrigger();
//         //         //vTaskDelay(5000);
//         //         /* code */
//         //         break;
//         //     // case 1:
//         //     //     Led_MyTrigger();
//         //     //     Led_MyTrigger();
//         //     //     break;
//         //     // case 2:
//         //     //     break;
//         //     // case 3:
//         //     //     Led_MyTrigger();
//         //     //     Led_MyTrigger();
//         //     default:
//         //         Led_MyTrigger();
//         //         Led_MyTrigger();
//         //         Led_MyTrigger();
//         //         break;
//         //     }
//         //     state = 2;
//         //     iconstate2 = 1;
//         //     vTaskDelay(5000);
//         // }

//         //爆闪
//         if(DjiUser_GetValue(2,1)==0 && iconstate3 == 1){
//             Led_MyTrigger();
//             state=0;
//             iconstate3 = 0;
//         }

//         if(DjiUser_GetValue(2,1)==1 && iconstate3 == 0){
            
//             DjiUser_SetValue(2,0,0);
//             //DjiUser_SetValue(2,2,0);
//             iconstate1=0;
//             iconstate2=0;
//             switch (state)
//             {
//             case 0:
//                 Led_MyTrigger();
//                 Led_MyTrigger();
//                 Led_MyTrigger();
//                 //vTaskDelay(5000);
//                 /* code */
//                 break;
//             // case 1:
//             //     Led_MyTrigger();
//             //     Led_MyTrigger();
//             //     break;
//             // case 2:
//             //     break;
//             // case 3:
//             //     Led_MyTrigger();
//             //     Led_MyTrigger();
//             default:
//                 Led_MyTrigger();
//                 Led_MyTrigger();
//                 Led_MyTrigger();
//                 Led_MyTrigger();
//                 break;
//             }
//             state = 3;
//             iconstate3 = 1;
//             vTaskDelay(5000);
//         }

//         // count1++;
//         // USER_LOG_INFO("%d\n",count1);
//         // //常亮1
//         // if(DjiUser_GetValue(2,0)==0 && iconstate1 == 1){
//         //     if(countstate==1){
//         //         if(count1>=6000){
//         //             Led_MyTrigger();
//         //             count1=0;
//         //             state=0;
//         //             iconstate1 = 0;
//         //             countstate=0;
//         //         }
//         //         else{
//         //             count1=0;
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             state=0;
//         //             iconstate1 = 0;
//         //             countstate=0;

//         //         }
//         //     }
//         // }

//         // else if(DjiUser_GetValue(2,0)==1 && iconstate1 == 0){
//         //     if (countstate==0)
//         //     {
//         //         countstate=1;
//         //         count1=0;
//         //         count1=0;
//         //         DjiUser_SetValue(2,1,0);
//         //         DjiUser_SetValue(2,2,0);
//         //         iconstate2=0;
//         //         iconstate3=0;
//         //         switch (state)
//         //         {
//         //         case 0:
//         //             Led_MyTrigger();
//         //             //vTaskDelay(5000);
//         //             /* code */
//         //             break;
//         //         case 1:
//         //             break;
//         //         case 2:
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //         case 3:
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //         default:
//         //             break;
//         //         }
//         //         state = 1;
//         //         iconstate1 = 1;
//         //     }
//         //     else{
//         //         if(count1>=6000){
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             USER_LOG_INFO("count:",count1);
//         //             count1=0;
//         //             DjiUser_SetValue(2,1,0);
//         //             DjiUser_SetValue(2,2,0);
//         //             iconstate2=0;
//         //             iconstate3=0;
//         //             state = 1;
//         //             iconstate1 = 1;
//         //         }
//         //         else{
//         //             count1=0;
//         //             DjiUser_SetValue(2,1,0);
//         //             DjiUser_SetValue(2,2,0);
//         //             iconstate2=0;
//         //             iconstate3=0;
//         //             switch (state)
//         //             {
//         //             case 0:
//         //                 Led_MyTrigger();
//         //                 //vTaskDelay(5000);
//         //                 /* code */
//         //                 break;
//         //             case 1:
//         //                 break;
//         //             case 2:
//         //                 Led_MyTrigger();
//         //                 Led_MyTrigger();
//         //                 Led_MyTrigger();
//         //             case 3:
//         //                 Led_MyTrigger();
//         //                 Led_MyTrigger();
//         //             default:
//         //                 break;
//         //             }
//         //             state = 1;
//         //             iconstate1 = 1;
//         //         }
//         //     }
//         // }


//         // //常亮2
//         // if(DjiUser_GetValue(2,1)==0 && iconstate2 == 1){
//         //     if(countstate==1){
//         //         if(count1>=6000){
//         //             Led_MyTrigger();
//         //             count1=0;
//         //             state=0;
//         //             iconstate2 = 0;
//         //             iconstate3 = 0;
//         //             countstate=0;
//         //         }
//         //         else{
//         //             count1=0;
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             state=0;
//         //             iconstate2 = 0;
//         //             iconstate3 = 0;
//         //             countstate=0;
//         //         }
//         //     }
            
//         // }
//         // else if(DjiUser_GetValue(2,1)==1 && iconstate2 == 0){
//         //     if (countstate==0)
//         //     {
//         //         countstate=1;
//         //         count1=0;
//         //         count1=0;
//         //         DjiUser_SetValue(2,0,0);
//         //         DjiUser_SetValue(2,2,0);
//         //         iconstate1=0;
//         //         iconstate3=0;
//         //         switch (state)
//         //         {
//         //         case 0:
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             //vTaskDelay(5000);
//         //             /* code */
//         //             break;
//         //         case 1:
//         //             Led_MyTrigger();
//         //             break;
//         //         case 2:
//         //             break;
//         //         case 3:
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             break;
//         //         default:
//         //             break;
//         //         }
//         //         state = 2;
//         //         iconstate2 = 1;
//         //     }
//         //     else{
//         //         if(count1>=6000){
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             count1=0;
//         //             DjiUser_SetValue(2,0,0);
//         //             DjiUser_SetValue(2,2,0);
//         //             iconstate1=0;
//         //             iconstate3=0;
//         //             state = 2;
//         //             iconstate2 = 1;
//         //         }
//         //         else{
//         //             count1=0;
//         //             DjiUser_SetValue(2,0,0);
//         //             DjiUser_SetValue(2,2,0);
//         //             iconstate1=0;
//         //             iconstate3=0;
//         //             switch (state)
//         //             {
//         //             case 0:
//         //                 Led_MyTrigger();
//         //                 Led_MyTrigger();
//         //                 //vTaskDelay(5000);
//         //                 /* code */
//         //                 break;
//         //             case 1:
//         //                 Led_MyTrigger();
//         //                 break;
//         //             case 2:
//         //                 break;
//         //             case 3:
//         //                 Led_MyTrigger();
//         //                 Led_MyTrigger();
//         //                 Led_MyTrigger();
//         //                 break;
//         //             default:
//         //                 break;
//         //             }
//         //             state = 2;
//         //             iconstate2 = 1;
//         //         }
//         //     }
//         // }

//         // //爆闪
//         // if(DjiUser_GetValue(2,2)==0 && iconstate3 == 1){
//         //     if(countstate==1){
//         //         if(count1>=6000){
//         //             Led_MyTrigger();
//         //             count1=0;
//         //             state=0;
//         //             iconstate3 = 0;
//         //             countstate=0;
//         //         }
//         //         else{
//         //             count1=0;
//         //             Led_MyTrigger();
//         //             state=0;
//         //             iconstate3 = 0;
//         //             countstate=0;
//         //         }
//         //     }
//         // }
//         // else if(DjiUser_GetValue(2,2)==1 && iconstate3 == 0){
//         //     if (countstate==0)
//         //     {
//         //         countstate=1;
//         //         count1=0;
//         //         count1=0;
//         //         DjiUser_SetValue(2,0,0);
//         //         DjiUser_SetValue(2,1,0);
//         //         iconstate1=0;
//         //         iconstate2=0;
//         //         switch (state)
//         //         {
//         //         case 0:
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             //vTaskDelay(5000);
//         //             /* code */
//         //             break;
//         //         case 1:
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             break;
//         //         case 2:
//         //             Led_MyTrigger();
//         //             break;
//         //         case 3:
//         //             break;
//         //         default:
//         //             break;
//         //         }
//         //         state = 3;
//         //         iconstate3 = 1;
//         //     }
//         //     else{
//         //         if(count1>=6000){
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             Led_MyTrigger();
//         //             count1=0;
//         //             DjiUser_SetValue(2,0,0);
//         //             DjiUser_SetValue(2,1,0);
//         //             iconstate1=0;
//         //             iconstate2=0;
//         //             state = 3;
//         //             iconstate3 = 1;
//         //         }
//         //         else{
//         //             count1=0;
//         //             DjiUser_SetValue(2,0,0);
//         //             DjiUser_SetValue(2,1,0);
//         //             iconstate1=0;
//         //             iconstate2=0;
//         //             switch (state)
//         //             {
//         //             case 0:
//         //                 Led_MyTrigger();
//         //                 Led_MyTrigger();
//         //                 Led_MyTrigger();
//         //                 //vTaskDelay(5000);
//         //                 /* code */
//         //                 break;
//         //             case 1:
//         //                 Led_MyTrigger();
//         //                 Led_MyTrigger();
//         //                 break;
//         //             case 2:
//         //                 Led_MyTrigger();
//         //                 break;
//         //             case 3:
//         //                 break;
//         //             default:
//         //                 break;
//         //             }
//         //             state = 3;
//         //             iconstate3 = 1;
//         //         }
//         //     }
//         // }

//         // //Led_Off(LED1);
//         // //vTaskDelay(10);
//         // //Led_On(LED1);
//         // //vTaskDelay(5000);
//         // // Led_On(LED1);
//         // // vTaskDelay(50);
//         // // Led_Off(LED1);
//         // // vTaskDelay(5000);
//     }

// }

#ifndef __CC_ARM
#pragma GCC diagnostic pop
#endif

/* Private functions definition-----------------------------------------------*/
static T_DjiReturnCode DjiUser_FillInUserInfo(T_DjiUserInfo *userInfo)
{
    memset(userInfo->appName, 0, sizeof(userInfo->appName));
    memset(userInfo->appId, 0, sizeof(userInfo->appId));
    memset(userInfo->appKey, 0, sizeof(userInfo->appKey));
    memset(userInfo->appLicense, 0, sizeof(userInfo->appLicense));
    memset(userInfo->developerAccount, 0, sizeof(userInfo->developerAccount));
    memset(userInfo->baudRate, 0, sizeof(userInfo->baudRate));

    if (strlen(USER_APP_NAME) >= sizeof(userInfo->appName) ||
        strlen(USER_APP_ID) > sizeof(userInfo->appId) ||
        strlen(USER_APP_KEY) > sizeof(userInfo->appKey) ||
        strlen(USER_APP_LICENSE) > sizeof(userInfo->appLicense) ||
        strlen(USER_DEVELOPER_ACCOUNT) >= sizeof(userInfo->developerAccount) ||
        strlen(USER_BAUD_RATE) > sizeof(userInfo->baudRate)) {
        USER_LOG_ERROR("Length of user information string is beyond limit. Please check.");
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    if (!strcmp(USER_APP_NAME, "your_app_name") ||
        !strcmp(USER_APP_ID, "your_app_id") ||
        !strcmp(USER_APP_KEY, "your_app_key") ||
        !strcmp(USER_APP_LICENSE, "your_app_license") ||
        !strcmp(USER_DEVELOPER_ACCOUNT, "your_developer_account") ||
        !strcmp(USER_BAUD_RATE, "your_baud_rate")) {
        USER_LOG_ERROR(
            "Please fill in correct user information to 'samples/sample_c/platform/rtos_freertos/stm32f4_discovery/application/dji_sdk_app_info.h' file.");
        return DJI_ERROR_SYSTEM_MODULE_CODE_INVALID_PARAMETER;
    }

    strncpy(userInfo->appName, USER_APP_NAME, sizeof(userInfo->appName) - 1);
    memcpy(userInfo->appId, USER_APP_ID, USER_UTIL_MIN(sizeof(userInfo->appId), strlen(USER_APP_ID)));
    memcpy(userInfo->appKey, USER_APP_KEY, USER_UTIL_MIN(sizeof(userInfo->appKey), strlen(USER_APP_KEY)));
    memcpy(userInfo->appLicense, USER_APP_LICENSE,
           USER_UTIL_MIN(sizeof(userInfo->appLicense), strlen(USER_APP_LICENSE)));
    memcpy(userInfo->baudRate, USER_BAUD_RATE, USER_UTIL_MIN(sizeof(userInfo->baudRate), strlen(USER_BAUD_RATE)));
    strncpy(userInfo->developerAccount, USER_DEVELOPER_ACCOUNT, sizeof(userInfo->developerAccount) - 1);

    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

static T_DjiReturnCode DjiUser_PrintConsole(const uint8_t *data, uint16_t dataLen)
{
    UART_Write(DJI_CONSOLE_UART_NUM, (uint8_t *) data, dataLen);

    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
}

#ifdef __cplusplus
}
#endif
/****************** (C) COPYRIGHT DJI Innovations *****END OF FILE****/
