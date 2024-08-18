/**
  ******************************************************************************
  * @file    task_list.h
  * @author  INTINS Vietnam
  * @brief   Header file of Spetrometer.
  ******************************************************************************
  * @attention
  *
  * Implement Task queue for handle spetrometer request
  *
  ******************************************************************************
*/
#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__
#include "stm32l4xx_hal.h"
#include <stdatomic.h>

#pragma pack(1)
struct task_struct {
    uint8_t buf[2062];
};

struct task_queue {
	struct task_struct *task;
	uint32_t read_index;
	uint32_t write_index;
	atomic_int n_tasks;
	uint32_t queue_size;
};
#pragma pack()

void free_task_list(struct task_queue *queue);
int put_task_to_queue(struct task_queue *queue, struct task_struct task);
int queue_is_empty(struct task_queue *queue);
int init_queue(struct task_queue *queue, uint32_t size);
struct task_struct get_new_task(struct task_queue *queue);

#endif /* TASK LIST */
