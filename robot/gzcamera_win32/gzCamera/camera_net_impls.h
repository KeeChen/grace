#pragma once

#include <stdint.h>

#ifdef _USRDLL
#define METH dllexport
#else
#define METH dllimport
#endif

#ifndef EXP
#if _WIN32
#define EXP(type)   _declspec(METH) type 
#else
#define EXP(type)   type
#endif
#endif
#pragma pack(push, 1)

typedef struct{
	uint8_t  command_code_;
	uint8_t  empty1_[3];
	uint16_t length_;
	char	 module_type_[32];
	uint32_t check_code_;
	uint8_t  image_type_;
	uint8_t  image_quality_;
	uint16_t time_interval_;
	uint32_t time_exposure_;
	uint8_t  light_type_;
	uint8_t  gain_factor_;
	uint8_t  sampling_;
	uint8_t  timestamp_;
}config_parameter_t;

typedef struct
{
	uint8_t command_code_;
	uint8_t empty1_[3];
	uint16_t length_;
	char  module_type_[32];
	uint32_t check_code_;
	uint8_t data_;
}control_message_t;

typedef struct {
	double FocalLength_1;
	double FocalLength_2;
	double PrincipalPoint_1;
	double PrincipalPoint_2;
	double RadialDistortion_1;
	double RadialDistortion_2;
	double RadialDistortion_3;
	double TangentialDistortion_1;
	double TangentialDistortion_2;
}parameter_data_t;

typedef struct{
	uint8_t comm_code_;
	uint8_t empty_[3];
	uint32_t length_;
	char model_[32];
	uint8_t	parameter_data_[72];
}parameter_post_t;

struct gzimage_t {
	uint16_t image_id; // Ӳ�������ͼƬID
	uint16_t image_current_bytes; // �Ѿ��������ܳ���(�ֽ�), 
	uint16_t image_total_bytes; // �ܳ���(�ֽ�)
	uint16_t image_width;   // ԭʼͼ��
	uint16_t image_high;    // ԭʼͼ��
	uint64_t image_timestamp;   // ���ʱ���

	uint64_t image_top_recv_ts; // ͷ֡����ʱ��
	uint64_t image_buttom_recv_ts; // β֡����ʱ��
	uint64_t image_pkt_adjust_ts; // ��֡���еĵ���ʱ��
	uint64_t imgae_exec_ts; // ִ��ʱ��
};

typedef struct packet_config_parameter{
	uint8_t command;       //������	0x20
	uint8_t reserved[3];
	uint16_t length;           // ����	��̬У����֮������ݳ���
	char module[32];      // ģ���ͺ�	ASCII�룬��IPC36M��
	uint32_t checksum;      // ���ò���У����  ���ò���У������㷽�������˺�12�ֽڵ����ò�������uint32_t�����ۼ���ͣ����ۼӺ�Ϊ0��ȡ������������Ϊ���ò���У���롣
	uint8_t output_type;    // ���ͼƬ��ʽ	1 - JPEG��ʽ��2 - ԭͼ�Ҷȸ�ʽ
	uint8_t image_quality; // ͼƬ����	1-100��JPEGѹ���ʣ���JPEG��ʽ����Ч
	uint16_t cap_interval; // ������ʱ�� ��0 - ���������ʱ��������λms
	//ȫ����������С��30ms��30ms����
	//���и���������С��20ms��20ms����
	//0 - ����������
	uint32_t time_exposure; // �ع�ʱ��	0 - �Զ��ع⣻��0 - �ع�ʱ�䣬��λus
	uint8_t light;              // ���ⷽʽ	0 - �����⣬1 - ͬ�����⣬2 - ����
	uint8_t gain_factor;  // ���汶�� 
	// 0 - �Զ����� ��0 - �ֶ����棬����16~64 ʱ����16��Ϊ���汶����1~15��16��������64��64����    
	uint8_t sampling;      // ������
	uint8_t time_recode;  // ʱ������� 0 - ʹ�����ʱ��� 1 - ����1588Э�飬ʹ�þ���ʱ���
}packet_config_parameter;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif
	//��ʼ�������
	EXP(int) init_net_session(const char *module_address, int(*camera_rcv_callback_t)(void *image, const unsigned char *pic));

	//����������Ϣ
	EXP(int) post_config_info(packet_config_parameter config_info);

	//���Ϳ��Ʊ�
	EXP(int) post_control_info(uint8_t control_info);

	//�����ڲ�����
	EXP(int) post_parameter_info(char* parameter_path, parameter_data_t parameter_data);

	EXP(void) query_firmware_info(char *name, int *namelen, uint32_t *checksum);

	EXP(uint8_t) query_status();
	//���÷���ͼƬ��Ϣ�ص�
	//EXP(void) register_image_callback(int(*notify_image_callback)(const unsigned char* bytes, int id, int len, uint32_t width, uint32_t high, uint64_t time_stamp));
#ifdef __cplusplus
}
#endif
