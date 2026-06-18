#include "barometer_task.h"
#include "bmp585.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* --------------------------------------------------------------------------
 * Configuration
 * -------------------------------------------------------------------------- */

/** How often the task wakes up and reads the sensor. */
#define BARO_TASK_PERIOD_MS     50U

/** FreeRTOS task stack size in words (not bytes). */
#define BARO_TASK_STACK_WORDS   256U

/** FreeRTOS task priority — set below flight-critical tasks, above logging. */
#define BARO_TASK_PRIORITY      (tskIDLE_PRIORITY + 2U)

/** Queue depth: hold up to 4 unread samples before overwriting.
 *  A consumer that can't keep up will always get the latest batch. */
#define BARO_QUEUE_DEPTH        4U

/* --------------------------------------------------------------------------
 * Module-private handles
 * -------------------------------------------------------------------------- */
static TaskHandle_t  s_task_handle  = NULL;
static QueueHandle_t s_data_queue   = NULL;

/* --------------------------------------------------------------------------
 * Task function
 * -------------------------------------------------------------------------- */
static void baro_task(void *arg)
{
    (void)arg;

    BMP585_Data_t sample;
    TickType_t    last_wake = xTaskGetTickCount();

    for (;;)
    {
        /* Block until next period — gives a stable, drift-free cadence */
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(BARO_TASK_PERIOD_MS));

        /* Read sensor */
        if (BMP585_ReadData(&sample) != HAL_OK)
        {
            /*
             * Read failed.  Options depending on your system policy:
             *   - Log the error via your telemetry/logging system
             *   - Attempt re-init if failures accumulate
             *   - Post a sentinel value so consumers know data is stale
             *
             * For now we simply skip this cycle and try again next period.
             */
            continue;
        }

        /*
         * Publish to queue.
         * xQueueOverwrite() replaces the oldest item if the queue is full —
         * consumers always get fresh data rather than blocking or dropping.
         * If you need a full history (e.g. for logging), switch to
         * xQueueSendToBack() with a deeper queue.
         */
        xQueueOverwrite(s_data_queue, &sample);
    }
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

HAL_StatusTypeDef BaroTask_Init(float sea_level_pa)
{
    /* 1. Initialise the sensor driver -------------------------------------- */
    BMP585_SetSeaLevelPa(sea_level_pa);

    if (BMP585_Init() != HAL_OK) return HAL_ERROR;

    /* 2. Create the data queue --------------------------------------------- */
    s_data_queue = xQueueCreate(BARO_QUEUE_DEPTH, sizeof(BMP585_Data_t));
    if (s_data_queue == NULL) return HAL_ERROR;

    /* 3. Create the task --------------------------------------------------- */
    BaseType_t result = xTaskCreate(
        baro_task,
        "BaroTask",
        BARO_TASK_STACK_WORDS,
        NULL,
        BARO_TASK_PRIORITY,
        &s_task_handle
    );

    return (result == pdPASS) ? HAL_OK : HAL_ERROR;
}

QueueHandle_t BaroTask_GetQueue(void)
{
    return s_data_queue;
}
