#pragma once

static const char* status_table[] = {
	"kStatusDescribe_Unknown",
	"kStatusDescribe_Idle",
	"kStatusDescribe_PrepareFinish",
	"kStatusDescribe_PendingFunction",
	// ͨ��״̬
	"kStatusDescribe_Running",
	"kStatusDescribe_Hang",
	"kStatusDescribe_Alertable",
	// ����
	"kStatusDescribe_Startup",
	"kStatusDescribe_Cancel",
	"kStatusDescribe_Pause",
	"kStatusDescribe_Resume",
	// ����״̬�� �� response_ ������
	"kStatusDescribe_FinalFunction",
	"kStatusDescribe_Completed",
	"kStatusDescribe_Terminated",
	"kStatusDescribe_Error"
};

static const char* elmo_control_mode[] = {
	"kDriverControlMode_Unknown",
	"kDriverControlMode_SpeedMode",
	"kDriverControlMode_PositionMode",
	"kDriverControlMode_CurrentMode"
};

static const char* vehicle_control_mode[] = {
	"kVehicleControlMode_Navigation",
	"kVehicleControlMode_Manual",
	"kVehicleControlMode_Calibrated"		// ΢��ģʽ
};

static const char* angle_encoder_type[] {
	"kAngleEncoderType_ABS",		// ���ԽǶ�
	"kAngleEncoderType_REL"			// ��ԽǶ�
};

static const char* dwheel_ctrlmod[] {
	"kDWheelCtrlMod_Speed",								// �ٶ�ģʽ
	"kDWheelCtrlMod_Dist",									// ����ģʽ
	"kDWheelCtrlMod_Position"								// λ��ģʽ
};

static const char* swheel_ctrlmod[] {
	"kSWheelCtrlMod_Angle",								// Ŀ��Ƕ�
	"kSWheelCtrlMod_DiscreteRate",							// ��ɢ�仯��
	"kSWheelCtrlMod_ContinueRate"							// �����仯��
};