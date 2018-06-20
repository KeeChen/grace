#ifndef __DIRVER_BASE_H__
#define __DIRVER_BASE_H__
#include "CML.h"
#include "var.h"
#include "error.h"
#include "errtypes.h"
#include <deque>
CML_NAMESPACE_USE();

#define DRIVER_ERROR_CONS_NUM   (15)
#define DRIVER_ERROR_WINDOW_THRES_PER (0.01)
#define DRIVER_ERROR_WINDOW_SIZE    1000

class TrackingErrorProtectorNew
{
public:
    TrackingErrorProtectorNew(double err_thres = 0.03, double acc_min = 0.02, int window_size = 5);
    void SetParams(double err_thres, double acc_min, int window_size);

    //ÿ���ڵ���
    bool InsertVelocity(double target, double actual, double &err);
    //����Ҫ���¿�ʼ����ʱ����
    void Clear();
private:
    double acc_min_; //ÿ�����ڵ���С���ٶ�
    double window_size_; //ʱ�䴰�ڴ�С
    double err_thres_; //�ٶȸ�����ֵ
    double limit_low_; //�����ٶ���Сֵ
    double limit_high_; //�����ٶ����ֵ
    double pre_actual_; //�������ٶ�
    //int resume_ignore_count_; //�������жϻָ��󣨲�����Clear��,���Զ��ٴ�InsertVelocity�������¿�ʼ��������ֹ��ͣ����ײһ�ָ��ͱ�����
    std::deque<double> track_error_speed_list_;
};

class driver_base
{
public:
    driver_base(var__types type);
	virtual ~driver_base();

	virtual int add_node(CML::CanOpen *canOpen, const var__functional_object_t *d) = 0;
	virtual int read_and_write(var__functional_object_t* var) = 0;

public:
    int rw(var__functional_object_t* var);
public:
	int set_driver_callback(void(*driver_call_back)(void *functional_object));
    int canio__set_error_ptr(var__functional_object_t *functional_object);

	int get_error_obj(int obj_id, var__error_item_t& err);
    void set_error_code(int obj_id, const var__error_item_t& err);
    void set_comm_err(int error_code);
	bool need_clearfault(int obj_id);
protected:
	void callback_data(void* v);
	uint64_t get_timestamp();
    
private:
    var__types __drv_type;
	void(*driver_call_back_)(void *functional_object);
    var__error_handler_t *error_var_;

    int __cur_window_count = 0;
    int __err_cout_in_window = 0;
    int __err_cons = 0;
    std::deque<int> __q_err_window;

    //�ٶȸ��ٱ���
public:
    int check_velocity_follow_err(var__functional_object_t* var,double cmd_velocity, double act_velocity,double);
private:
    double acc_min_ = 0.02; //ÿ�����ڵ���С���ٶ�
    int window_size_ = 100; //ʱ�䴰�ڴ�С
    double limit_low_ = 0; //�����ٶ���Сֵ
    double limit_high_ = 0; //�����ٶ����ֵ
    double pre_actual_ = 0; //�������ٶ�
    std::deque<double> track_error_speed_list_;

	var__error_item_t  __last_err_it;
    
};

#endif


