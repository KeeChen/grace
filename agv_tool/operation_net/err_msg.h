#pragma once

#include "asio.hpp"
#include <vector>

//�������д���״̬ת��
#define		NORMAL					"����"
#define		FAILED_READ			"��ȡʧ��"
#define		FAILED_WRITE			"д��ʧ��"
#define		FAILED_GET				"��ȡʧ��"

struct process_info
{
	uint32_t process_id;
	std::string process_name;
};

struct process_list_info : motion::asio_data
{
	std::vector<process_info> vct_process_;
};

struct common_msg : motion::asio_data
{
	std::string msg_;
};
