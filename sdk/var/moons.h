﻿#if !defined VAR_STEPMOTOR_H
#define VAR_STEPMOTOR_H

#include "vartypes.h"
#include "object.h"

#pragma pack(push, 1)

typedef struct {
    // 全局只读
    /*+000*/ var__can_device_t candev_head_;

    /*+036*/ double profile_speed_; // 最大速度
    /*+044*/ double profile_acc_; // 最大加速度
    /*+052*/ double profile_dec_; // 最大减速度

    // 状态迁移
    /*+060*/ var__status_describe_t status_;

    // 可写(数据下行,可作为绑定目标)
    /*+072*/ enum control_mode_t control_mode_; // 控制模式: 速度模式
    /*+076*/ int64_t command_velocity_; // 目标速度
    /*+084*/ int64_t command_position_;
    /*+092*/ double command_current_;
    /*+100*/ int enable_; // 下发给驱动器，可使驱动器Enable和Disable

    // 提交(数据上行， 可作为绑定源)

    union {
#if __cplusplus
        struct {
#else
        struct _i_moos_internal_t {
#endif 
            /*+104*/ int state_; // CAN OPEN STATES
            /*+108*/ int error_code_;
            /*+112*/ uint64_t time_stamp_;
            /*+120*/ int64_t actual_velocity_;
            /*+128*/ int64_t actual_position_;
            /*+136*/ double actual_current_;
            /*+144*/ int enabled_; // 驱动器反馈，当前是否处于使能状态
            /*+148*/ int di_; // INT能表示32个DI(驱动器反馈)
            /*+152*/ int do_; // INT能表示32个DO(设给驱动器)
        } i;

        char u_;
    }; /*+156*/
} var__moos_t;

#pragma pack(pop)

#if !defined __cplusplus

extern
int var__load_moos();

/* 根据设备句柄或者设备ID获取 moos 设备所指向的 moos 对象原始数据区指针，必须手动  var__release_object_reference 释放
 */
extern
var__moos_t *var__get_moos(objhld_t hld);
extern
var__moos_t *var__get_moos_byid(int id);

/* 创建一份指定句柄或者ID的 moos 设备对象的拷贝, 使用完成后必须手动 var__release_moos_dup 释放
 */
extern
var__moos_t *var__create_moos_dup(objhld_t hld);
extern
var__moos_t *var__create_moos_dup_byid(int id);

/* 将籍由 var__create_moos_dup 或  var__create_moos_dup_byid 获得的 moos 对象拷贝提交到原始数据区
 */
extern
int var__commit_moos_dup(const var__moos_t *moos);

/* 释放 var__create_moos_dup 或 var__create_moos_dup_byid 所得的 moos 对象拷贝
 */
extern
void var__release_moos_dup(var__moos_t *moos);

#endif
#endif