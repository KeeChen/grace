#pragma once

#include <stdint.h>
#include <string>
#include "proto_udp_typedef.h"
#include "endpoint.h"

/***************************���¹̼�bin�ļ�********************************/
class frimware_update_task{
public:
	//node_id,serial_typeΪ��������ʹ��
	frimware_update_task(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE ty, const std::string& target_ep,
		const uint32_t node_id = 0, const uint32_t serial_type = 0) :
		link_(link),
		frimwre_type_(ty),
		target_ep_(target_ep),
		node_id_(node_id),
		serial_type_(serial_type){
		serial_code_ = 0x00;
		serial_code_ += 0x0f && serial_type;
	}
	~frimware_update_task(){};

public:
	void on_task();

private:
	int write_bin_file_to_vcu(const int block_num);
	int get_bin_file(const int block_number);
	int post_reset_request();
	void post_frimware_update(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE f_type, const FRIMWARE_STATUS status, const int step);
	int post_can_init_rom_update(const int block_number);
	int write_can_custom_bin(const int block_number);

private:
	uint32_t link_;
	FIRMWARE_SOFTWARE_TYPE frimwre_type_;
	uint32_t reset_timeout_;

	std::string target_ep_;
	nsp::tcpip::endpoint vcu_endpoint_;
	uint64_t file_size_;

	int current_step_;//��ǰ����
	FRIMWARE_STATUS current_status_;//��ǰ״̬

	nsp::os::waitable_handle reset_wait_;//�����ȴ��¼�
	nsp::os::waitable_handle block_write_wait_;//ÿһƬд��ȴ��¼�

	uint8_t node_id_;
	uint8_t serial_code_;
	uint32_t serial_type_;
};

/****************************��ȡ�̼��汾���ͺţ�cpu��Ϣ����******************************/
class frimware_get_info_task{
public:
	//node_id,serial_typeΪ��������ʹ��
	frimware_get_info_task(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE ty,const std::string& target_ep,
		const uint32_t node_id = 0, const uint32_t serial_type = 0) :
		link_(link),
		frimwre_type_(ty),
		target_ep_(target_ep),
		node_id_(node_id),
		serial_type_(serial_type)
	{
		serial_code_ = 0x00;
		serial_code_ += 0x0f && serial_type;
	};
	~frimware_get_info_task(){};

public:
	void on_task();
	
private:
	int get_vcu_type(const nsp::tcpip::endpoint& );
	int get_vcu_cpu(const nsp::tcpip::endpoint&);
	int get_vcu_version(const nsp::tcpip::endpoint&);
	int start_data_foward(const nsp::tcpip::endpoint& ep);

private:
	uint32_t link_;
	FIRMWARE_SOFTWARE_TYPE frimwre_type_;
	int version_control_;

	std::string target_ep_;
	std::string vcu_type_;//vcu�ͺ�
	std::string vcu_version_;//vcu�汾��
	std::string cpu_info_;//cpu��Ϣ

	uint8_t node_id_;
	uint8_t serial_code_;
	uint32_t serial_type_;
};

/*********************************����bin�ļ�ת�����ͻ���**************************************/
class frimware_download_task{
public:
	frimware_download_task(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE ty, const uint32_t frimware_length, const std::string file, const std::string& target_ep) :
		link_(link),
		file_name_(file),
		frimware_type_(ty),
		frimware_length_(frimware_length),
		target_ep_(target_ep){};
	~frimware_download_task(){};

public:
	void on_task();

private:
	int download_bin_file_from_vcu(const nsp::tcpip::endpoint& ep, const int block_num);

private:
	uint32_t link_;
	std::string file_name_;
	FIRMWARE_SOFTWARE_TYPE frimware_type_;
	uint32_t frimware_length_;
	std::string target_ep_;

	int current_step_;//��ǰ����
	FRIMWARE_STATUS current_status_;//��ǰ״̬
};

/***************************************����M�˻������********************************************/
class frimware_restart_task{
public:
	frimware_restart_task(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE ty, const std::string& target_ep) :
		link_(link),
		frimware_type_(ty), 
		target_ep_(target_ep){};
	~frimware_restart_task(){};

public:
	void on_task();

private:
	int post_reset_request();
	void post_frimware_update(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE f_type, const FRIMWARE_STATUS status, const int step);

private:
	uint32_t link_;
	FIRMWARE_SOFTWARE_TYPE frimware_type_;
	nsp::tcpip::endpoint vcu_endpoint_;
	std::string target_ep_;

	FRIMWARE_STATUS current_status_;//��ǰ״̬
	nsp::os::waitable_handle reset_wait_;//�����ȴ��¼�
};
