#pragma once

#include <string>
#include "proto_udp_typedef.h"
#include <functional>

class net_task_download{
public:
	net_task_download(const std::string& ip, const FIRMWARE_SOFTWARE_TYPE type, const uint32_t reset_timeout, const uint32_t block_delay, const uint32_t is_control);
	net_task_download(const std::string& ip, const uint32_t reset_timeout, const uint32_t block_delay, uint8_t serial_id, uint8_t node_id);

	~net_task_download();

public:
	void on_task();
	//ע��ص�����������һ���յ����������ĸ�ip��ַ�������������������ĸ�Э�����ͣ������������ݲ���,��ǰ��ɰٷֱ�,�����ģ���ǰһƬ�����Ƿ�ɹ�
	void regiset_callback(const std::function<void(const std::string&, const int type, const int, const nsp::proto::errorno_t)>& func);
	//ע����������ص��汾����Ϣ����
	void regiset_get_vcu_callback(const std::function<void(const std::string&, const int operate_type, const std::string&, const nsp::proto::errorno_t)>& func);

private:
	int get_bin_file(const int block_number);
	void get_vcu_type_version_request();
	int write_bin_file_to_vcu(const int block_num);
	int post_reset_request();

	int initial_rom_update(int block_num, uint16_t check_sum);
private:
	std::string ip_addr_;
	FIRMWARE_SOFTWARE_TYPE f_type_;
	uint32_t is_control_;
	uint32_t reset_timeout_;
	uint32_t block_write_delay_;
	nsp::os::waitable_handle reset_wait_;
	nsp::os::waitable_handle block_write_wait_;
	uint8_t	node_id_;
	uint8_t  serial_code_;
	int	 is_can_type_;

	std::function<void(const std::string&, const int type, const int, const nsp::proto::errorno_t)> function_callback_;
	std::function<void(const std::string&, const int operate_type, const std::string&, const nsp::proto::errorno_t)> function_get_vcu_callback_;
};