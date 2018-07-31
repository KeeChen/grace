#ifndef __AGV_SHELL_SVR_H__
#define __AGV_SHELL_SVR_H__

#include "agv_shell_session.h"
#include "frimware_task.h"
#include "task.h"
#include "task_scheduler.hpp"
#include "singleton.hpp"

#if !defined AGVSH_PORT     
#define AGVSH_PORT (4410)           // ����������Ŀ��·��ӳ�䣬 ֻ��ʹ����� TCP:4410 �˿�
#endif

class agv_shell_server 
{
    agv_shell_server();
    ~agv_shell_server();
	friend class nsp::toolkit::singleton<agv_shell_server>;


public:
	int begin_shell_server(const int port, std::string ip = "0.0.0.0");

	int on_get_frimware_info(const uint32_t link, const int frimware_ty, const std::string& target_ep, const uint32_t node_id = 0, 
		const uint32_t serial_type = 0);
	int post_frimware_info(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE f_type, const FRIMWARE_STATUS status, 
		const VCU_MESSAGE msg_ty, const std::string& msg);

	int on_frimware_update(const uint32_t link, const int f_ty, const std::string& file_path, const std::string& target_ep, 
		const uint32_t node_id = 0, const uint32_t serial_type = 0);
	int post_frimware_update(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE f_type, const FRIMWARE_STATUS status, const int step);

	int on_frimware_download(const uint32_t link, const int frimware_ty, const int frimware_length,const std::string& target_ep);
	int post_frimware_download(const uint32_t link, const std::string& file_path);

	int on_frimware_restart(const uint32_t link, const int frimware_ty, const std::string& target_ep);
	void on_query_vcu_keep_alive_status(const HTCPLINK link);
	void on_set_vcu_keep_alive_status(const HTCPLINK link, int status);
	int post_vcu_keep_alive_status(const HTCPLINK link, const int proto_type, int status);
	//void on_deal_process_cmd(const HTCPLINK link, int cmd, int process_id); 
	void on_deal_process_cmd(const HTCPLINK link, const std::shared_ptr<nsp::proto::proto_command_process> p_info);

	uint16_t get_client_number(){ return vct_lnk_.size(); }
	void add_client_lnk(const uint32_t link);
	void reduce_client_lnk(const uint32_t link);

	void set_workpath(const std::string& path){ work_path_ = path; }
	std::string get_workpath(){ return work_path_; }

	void close_all();
	void post_notify_all(const nsp::proto::proto_interface& package);
	void post_file_mutex(const int status);
	void post_shell_version(const std::string &shell_version);

	void post_tar_backups(const uint32_t lnk, const std::string& des_file);

private:
	void initlization();
	void uinit();
	void post_frimware_download_complete(const uint32_t link, const std::string& file_path);

	void alive_check_thread();

private:
    std::shared_ptr<nsp::tcpip::tcp_application_service<agv_shell_session>> service_;

    nsp::tcpip::endpoint ep_;

	//��ȡM�˹̼���Ϣ
	std::shared_ptr<nsp::toolkit::task_thread_pool<frimware_get_info_task>> frimware_info_spool_ = nullptr;
	//ת�������̼���M�˻����
	std::shared_ptr<nsp::toolkit::task_thread_pool<frimware_update_task>> frimware_push_spool_ = nullptr;
	//����M�˹̼��ļ�
	std::shared_ptr<nsp::toolkit::task_thread_pool<frimware_download_task>> frimware_download_spool = nullptr;
	//�����̼��̳߳�
	std::shared_ptr<nsp::toolkit::task_thread_pool<frimware_restart_task>> frimware_restart_spool = nullptr;
	std::shared_ptr<nsp::toolkit::task_thread_pool<base_task>> base_task_spool = nullptr;

	//�洢��ǰ���ù���ƽ̨�ͻ��˵�link��Ҫ��ֻ����ǰ����ֻ��һ���ͻ��������Ϸ����
	std::vector<uint32_t> vct_lnk_;
	std::string work_path_;

	std::thread* alive_check_ = nullptr;
	std::atomic<int> is_exist_{ 0 };
	nsp::os::waitable_handle check_wait_;
};


#endif

