#if !defined VDECOM_H_
#define VDECOM_H_

#include "wheel.h"
#include "drive_unit.h"
#include "vehicle.h"
#include "var.h"
#include "navigation.h"
#include "clist.h"

enum TRIM_TYPE {
    NON = 0,
    DEC_DSTOP,
    ONLY_ACC,
    VEHICLE_LIMIT,
};

typedef struct _v_wheel {
    enum drive_unit_type_t _w_type;
    int index_drive;
    char left_or_right; //true��ʾleft
    double _x; //������������������ϵ�е�λ��
    double _y;
    double _v_x; //�����ֽ�����ٶ�
    double _v_y;
    double _ae; //�ֽ��õ��ĸ��������Ƕ�
    double _af; //��õĸ����ӵ�ʵ�ʽǶ�
    double _vp; //�����ֽ�����ٶ���ʵ�ʽǶȷ����ϵ�ͶӰֵ
    double _vo; //�Ż�����ֵ��ٶ�
    double _roll_wt; //����ϵ��
    double _slide_wt; //����ϵ����ע������SWheel��˵��������ϵ��Ϊ�㣬��������ٶ�Ҳ���·�����Fscore�в�������S�ֵ��ٶ�
    double _gauge;
    double _min_angle;
    double _max_angle;
    uint64_t time_stamp;
    uint64_t s_time_stamp;
    double max_speed;
	double _zero_angle;
    var__functional_object_t *_swheel_obj;
    var__functional_object_t *_dwheel_1_obj;
    var__functional_object_t *_dwheel_2_obj;
    var__functional_object_t *_sddwheel_obj;
} v_wheel;

//�ٶȺϳ�

typedef struct _v_chassis {
    double V_x;
    double V_y;
    double W;
    uint64_t time_stamp;
} v_chassis;

typedef struct v_wheel_with_time_ {

    union {
        double v_w;
        double angle;
    };
    uint64_t time_stamp;
} v_wheel_with_time;

void data_w_log(v_wheel * v_wheels, int num_wheel);
int v_composition(v_wheel * v_wheels, int num_wheel, int ref_enc, v_chassis *out); //�ٶȺϳ�
void sdd_v_composition(v_wheel * v_wheels, int num_wheel, int num_dri_unit, int ref_enc, v_chassis *out);
void first_v_decomposite(v_wheel *v_w, const list_entry_t *drive_unit, var__vehicle_t *veh);
void DoOptimize(double *Xresult, int N_wh, v_wheel* vwheels, double* Xinit, char *pval);
int GetOpti_va(int N_wh, v_wheel *vw, double* va_init);
void WriteToGlobal(int N_wh, v_wheel * vwheels);
void v_wheel_limited(int N_wh, v_wheel * vwheels);
double trim_solo_only_acc(double v_x, double v_x_last, double v_acc);
extern
int VwResolution(v_chassis *V_com, var__vehicle_t *veh, const list_entry_t *drive_unit);
extern
void trim(double v_x, double v_y, double w, var__navigation_t *nav, var__vehicle_t *veh, double deta_time, int trim_type, velocity_t *result);
extern
void trim2(double v, var__navigation_t *nav, var__vehicle_t *veh, double deta_time, int trim_type, double *result);
extern
void EnableDrive_unit(const list_entry_t *drive_unit, int aa);

#endif
