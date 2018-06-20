#pragma once

#include <thread>
#include <string>
#include <deque>
#include <mutex>
#include "os_util.hpp"

enum UPDATE_STATUS
{
	UPDATE_READY = 0,	//׼��״̬
	UPDATE_RUNNING,		//��������
	UPDATE_COMPLETE		//�������
};

struct frimware_task {
	std::string ip_;
	uint16_t port_;
	std::string local_file_;
	std::string remote_file_;
	UPDATE_STATUS status_ = UPDATE_STATUS::UPDATE_READY;
};

class frimware_core_task {
public:
	frimware_core_task();
	~frimware_core_task();

public:
	void add_push_core_task(const std::string& ip, const uint16_t port, const std::string & local_file, const std::string& remote_file);
	void reduce_push_count();

private:
	void task_run();
	void initlization();
	void uinit();

private:
	std::thread* task_thread_ = nullptr;
	std::recursive_mutex r_mutex_;
	std::deque<frimware_task> core_deque_;
	nsp::os::waitable_handle timeout_wait_;
	nsp::os::waitable_handle push_waiter_;

	std::atomic<bool> is_exists_ = { false };

	//��ǰ�����·����ļ���
	std::atomic<int> current_push_number_ = { 0 };
};