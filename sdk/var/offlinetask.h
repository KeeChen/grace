﻿#if !defined DEF_OFFLINETASK_H
#define DEF_OFFLINETASK_H
#include "vartypes.h"

// 离线操作 
typedef struct var_offline_oper {
	uint64_t task_id_;
	int  code_;
	uint64_t params_[10];
} var_offline_oper_t;

// 离线节点(导航和操作) 
typedef struct var_offline_task_node {
	uint64_t task_id_;
	upl_t dest_upl_;
	position_t dest_pos_;
	// 参考轨迹个数 
	int cnt_trals_;
	// 离线操作个数 
	int cnt_opers_;
	char data[1];
	/*
		data指针相当于指向下面的两个不定长数组的数据：
		trail_t vec_trail_[cnt_trals_]; // 参考轨迹 
		var_offline_oper_t opts_[cnt_opers_]; // 离线操作 
	*/
} var_offline_task_node_t;

typedef struct var_offline_task {
	// 当前离线导航状态 
	var__status_describe_t track_status_;

	// 离线导航任务ID
	uint64_t user_task_id_;
	uint64_t ato_task_id_;

	// 离线任务节点个数
	int task_count_;
	// 当前执行到的离线任务(0:初始值，未执行)
	int task_current_exec_index_;
	// 离线任务节点数据，个数不定 
	var_offline_task_node_t tasks_[1];
} var_offline_task_t;

//free heap memory when next incoming offline task  arrived
//notify customer / rex navigation completed
//decline post_navigation_task(not customer) / post_add_navigation_task_traj / post_allocate_operation_task when offline task is existed
//decline offline task when either navigation task or operation task is existed

extern var_offline_task_t *var__get_offline_task();
extern int var__set_offline_task(const char *data, int cb);

#endif // DEF_OFFLINETASK_H
