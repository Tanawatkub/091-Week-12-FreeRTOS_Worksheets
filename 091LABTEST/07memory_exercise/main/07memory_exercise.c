#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

static const char *TAG = "MEMORY_EXERCISE";

// ============================ UTILITIES ============================
static void print_heap_status(const char *label)
{
    ESP_LOGI(TAG, "--- %s ---", label);
    ESP_LOGI(TAG, "Free heap: %d bytes", xPortGetFreeHeapSize());
    ESP_LOGI(TAG, "Min ever free heap: %d bytes", xPortGetMinimumEverFreeHeapSize());
}

// ============================ EXERCISE 1 ============================
// Compare Static vs Dynamic Allocation

#define STATIC_STACK_SIZE 2048
static StackType_t static_stack[STATIC_STACK_SIZE];
static StaticTask_t static_tcb;

void static_task(void *p)
{
    while (1) {
        ESP_LOGI(TAG, "[Static] Running task...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void dynamic_task(void *p)
{
    while (1) {
        ESP_LOGI(TAG, "[Dynamic] Running task...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void exercise1(void)
{
    ESP_LOGI(TAG, "===== Exercise 1: Static vs Dynamic Allocation =====");
    print_heap_status("Before Static");

    TaskHandle_t static_handle = xTaskCreateStatic(
        static_task, "StaticTask", STATIC_STACK_SIZE, NULL, 5, static_stack, &static_tcb);

    print_heap_status("After Static");

    TaskHandle_t dynamic_handle = NULL;
    xTaskCreate(dynamic_task, "DynamicTask", 2048, NULL, 5, &dynamic_handle);

    print_heap_status("After Dynamic");

    vTaskDelay(pdMS_TO_TICKS(5000));
    vTaskDelete(dynamic_handle);
    ESP_LOGI(TAG, "Dynamic task deleted");
    print_heap_status("After delete Dynamic");
}

// ============================ EXERCISE 2 ============================
// Memory Pool System

typedef struct {
    void *pool_start;
    size_t block_size;
    size_t num_blocks;
    uint8_t *alloc_map;
    SemaphoreHandle_t mutex;
} mem_pool_t;

mem_pool_t *create_pool(size_t block_size, size_t num_blocks)
{
    mem_pool_t *p = pvPortMalloc(sizeof(mem_pool_t));
    if (!p) return NULL;

    p->block_size = block_size;
    p->num_blocks = num_blocks;
    p->pool_start = pvPortMalloc(block_size * num_blocks);
    p->alloc_map = pvPortMalloc(num_blocks);
    memset(p->alloc_map, 0, num_blocks);
    p->mutex = xSemaphoreCreateMutex();
    ESP_LOGI(TAG, "Pool created: %d blocks x %d bytes", num_blocks, block_size);
    return p;
}

void *pool_alloc(mem_pool_t *p)
{
    if (!p) return NULL;
    xSemaphoreTake(p->mutex, portMAX_DELAY);
    for (int i = 0; i < p->num_blocks; i++) {
        if (p->alloc_map[i] == 0) {
            p->alloc_map[i] = 1;
            void *block = (uint8_t *)p->pool_start + (i * p->block_size);
            ESP_LOGI(TAG, "Allocated block %d -> %p", i, block);
            xSemaphoreGive(p->mutex);
            return block;
        }
    }
    xSemaphoreGive(p->mutex);
    ESP_LOGW(TAG, "No free blocks");
    return NULL;
}

void pool_free(mem_pool_t *p, void *ptr)
{
    if (!p || !ptr) return;
    xSemaphoreTake(p->mutex, portMAX_DELAY);
    size_t offset = (uint8_t *)ptr - (uint8_t *)p->pool_start;
    int index = offset / p->block_size;
    if (index >= 0 && index < p->num_blocks) {
        p->alloc_map[index] = 0;
        ESP_LOGI(TAG, "Freed block %d (%p)", index, ptr);
    }
    xSemaphoreGive(p->mutex);
}

void exercise2(void)
{
    ESP_LOGI(TAG, "===== Exercise 2: Memory Pool =====");
    mem_pool_t *pool = create_pool(64, 10);

    void *blocks[4];
    for (int i = 0; i < 4; i++)
        blocks[i] = pool_alloc(pool);

    pool_free(pool, blocks[1]);
    pool_free(pool, blocks[3]);
    pool_alloc(pool); // reuse freed

    print_heap_status("After Pool Demo");
}

// ============================ EXERCISE 3 ============================
// Memory Leak Detection Demo

typedef struct alloc_node {
    void *ptr;
    size_t size;
    struct alloc_node *next;
} alloc_node_t;

static alloc_node_t *head = NULL;
static SemaphoreHandle_t leak_mutex;

void *track_malloc(size_t size)
{
    void *ptr = pvPortMalloc(size);
    if (!ptr) return NULL;
    alloc_node_t *node = pvPortMalloc(sizeof(alloc_node_t));
    node->ptr = ptr;
    node->size = size;
    node->next = head;
    head = node;
    return ptr;
}

void track_free(void *ptr)
{
    alloc_node_t **curr = &head;
    while (*curr) {
        if ((*curr)->ptr == ptr) {
            alloc_node_t *to_free = *curr;
            *curr = (*curr)->next;
            vPortFree(to_free);
            break;
        }
        curr = &(*curr)->next;
    }
    vPortFree(ptr);
}

void check_leaks(void)
{
    alloc_node_t *curr = head;
    size_t leaks = 0;
    while (curr) {
        ESP_LOGW(TAG, "Leak detected: %p (%d bytes)", curr->ptr, curr->size);
        leaks += curr->size;
        curr = curr->next;
    }
    if (leaks == 0)
        ESP_LOGI(TAG, "✅ No memory leaks");
    else
        ESP_LOGW(TAG, "⚠️  Total leaked: %d bytes", leaks);
}

void exercise3(void)
{
    ESP_LOGI(TAG, "===== Exercise 3: Leak Detection =====");
    leak_mutex = xSemaphoreCreateMutex();

    void *a = track_malloc(256);
    void *b = track_malloc(128);
    track_free(b); // free one

    ESP_LOGI(TAG, "Checking leaks...");
    check_leaks();

    // clean up
    track_free(a);
    check_leaks();
}

// ============================ EXERCISE 4 ============================
// Memory Monitoring

void memory_monitor(void *p)
{
    while (1) {
        print_heap_status("Monitor");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void exercise4(void)
{
    ESP_LOGI(TAG, "===== Exercise 4: Memory Monitoring =====");
    xTaskCreate(memory_monitor, "MemMon", 2048, NULL, 3, NULL);
}

// ============================ MAIN ============================

void app_main(void)
{
    ESP_LOGI(TAG, "===== FreeRTOS Memory Management Exercises =====");
    print_heap_status("System Boot");

    int mode = 4;  // 🔧 Change between 1–4 for each exercise

    switch (mode) {
        case 1: exercise1(); break;
        case 2: exercise2(); break;
        case 3: exercise3(); break;
        case 4: exercise4(); break;
        default: ESP_LOGW(TAG, "Invalid mode");
    }
}
