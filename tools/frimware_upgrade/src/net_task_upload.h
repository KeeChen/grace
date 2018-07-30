#pragma once

#include <string>
#include "proto_udp_typedef.h"
#include <functional>
#include "file_write_handler.h"

class net_task_upload{
public:
	net_task_upload(const std::string& local_ip, const std::string& ip, const unsigned int port,
		const FIRMWARE_SOFTWARE_TYPE type, const unsigned int file_length, const std::string& f_name);
	~net_task_upload();

public:
	void on_task();
	//ע��ص�����������һ���յ����������ĸ�ip��ַ�������������������ĸ�Э�����ͣ������������ݲ���,��ǰ��ɰٷֱ�,�����ģ���ǰһƬ�����Ƿ�ɹ�
	void regiset_callback(const std::function<void(const std::string&, const int type, const int, const nsp::proto::errorno_t)>& func);

private:
	std::string ip_local_;
	std::string ip_addr_;
	uint32_t port_;
	FIRMWARE_SOFTWARE_TYPE f_type_;
	unsigned int firmware_length_;
	std::string file_name_;

	std::function<void(const std::string&, const int type, const int, const nsp::proto::errorno_t)> function_callback_;
};