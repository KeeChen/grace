#pragma once

#include "asio.hpp"

#define FIRMWARE_MAIN_OPERATE_CODE_MODULE_CMD				0x02
#define FIRMWARE_SUB_OPERATE_CODE_RESET_VCU					0x07
#define FIRMWARE_MAIN_OPERATE_CODE_GET_INFO					0x03
#define FIRMWARE_MAIN_OPERATE_CODE_RW_FIRMWARE				0x04
#define FIRMWARE_SUB_OPERATE_CODE_GET_VCU_TYPE				0x01

#define FIRMWARE_COMPLETE_GET_VCU_VERSION					0x50

#define FIRMWARE_SUB_OPERATE_CODE_GET_VCU_VERSION			0x80
#define FIRMWARE_SUB_OPERATE_CODE_GET_BOOT_VERSION			0x81
#define FIRMWARE_SUB_OPERATE_CODE_GET_MAIN_VERSION			0x82
#define FIRMWARE_SUB_OPERATE_CODE_GET_USER_VERSION			0x83
#define FIRMWARE_SUB_OPERATE_CODE_GET_CAMERA_VERSION		0x81
#define FIRMWARE_SUB_OPERATE_CODE_GET_CPU_VERSION			0x02  //add

#define FIRMWARE_SUB_OPERATE_CODE_READ_BOOT_FILE			0x01
#define FIRMWARE_SUB_OPERATE_CODE_READ_MAIN_FILE			0x02
#define FIRMWARE_SUB_OPERATE_CODE_READ_USER_FILE			0x03
#define FIRMWARE_SUB_OPERATE_CODE_READ_CAMERA_FILE			0x01
#define FIRMWARE_SUB_OPERATE_CODE_READ_NORAML_FILE			0x00 //��ȡ�ո��·�bin�ļ�������

#define FIRMWARE_UNCONTROL_GET_BOOT_VERSION					0x41
#define FIRMWARE_UNCONTROL_GET_MAIN_VERSION					0x42
#define FIRMWARE_UNCONTROL_GET_USER_VERSION					0x43
#define FIRMWARE_UNCONTROL_GET_CAMERA_VERSION				0x41

#define FIRMWARE_RESET_OPERATE_CODE_CALLBACK				0x90						


#define FIRMWARE_RW_FILE_BLOCK_LENGTH						8

enum FIRMWARE_SOFTWARE_TYPE
{
	BOOT_SOFTWARE = 0,
	MAIN_SOFTWARE,
	USER_SOFTWARE,
	CAMERA_SOFTWARE,
	COMPARE_SOFTWARE//�����ʾ��ȡ�ո��·����ļ�
};

enum EXECUTE_ACK
{
	ACK_Normal = 0x00,				//ִ������
	ACK_Version_Failed = 0x01,		//ͨѶЭ��治ƥ��
	ACK_Limits = 0x02,				//Ȩ�޲���
	ACK_Command_No = 0x03,			//�������
	ACK_Addr_Not_Write = 0x04,		//��ַ����д
	ACK_OverFlow_Memory = 0x05,		//���ݵ�ַ���
	ACK_Content_Invaild = 0x06,		//�������ݷǷ���У�鲻ͨ��
	ACK_VCU_Busy = 0x07,			//VCUæ���޷�ִ��
	ACK_Target_Port_Faild = 0x08,	//����ת����ͨѶ�˿ڴ��ڹ���
	ACK_Target_Not_exist = 0x09,	//����ת����Ŀ���豸������
	ACK_Target_Not_Reponse = 0x0A,	//����ת����Ŀ���豸δӦ��
	ACK_Data_Lose = 0x0B,			//���ݷ�֮֡���ж���
	ACK_Other_Problom =0xff			//��������
};

enum TABLEVIEW_BTN
{
	Btn_ReGet = 0x01,			//���»�ȡ
	Btn_Retry,						//����
	Btn_Degrade						//����
};

struct recv_vcu_data : motion::asio_data{
	std::string remote_ip_;
	unsigned int data_length_;
	std::string data_context_;
};

struct recv_bin_data : motion::asio_data{
	unsigned int offset_;
	unsigned int data_length_;
	std::string data_context_;
};