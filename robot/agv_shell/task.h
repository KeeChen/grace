#ifndef __GRACE_ROBOT_AGVSHELL_TASK_H__
#define __GRACE_ROBOT_AGVSHELL_TASK_H__

#include "endpoint.h"
#include "icom/nisdef.h"
#include <vector>
#include <stdint.h>
#include <string>

/***************************************base_task********************************************/
class base_task {
public:
	HTCPLINK link_;
public:
	base_task(){};
	virtual ~base_task(){};
	virtual void on_task() = 0;
};
/***************************************��ѯM������״̬********************************************/
class query_keepalive_status_task : public base_task {
	uint32_t pkt_id_;
	nsp::tcpip::endpoint vcu_endpoint_;
	std::string vcu_keepalive_status_;//vcu ����״̬
	
public:
	query_keepalive_status_task(HTCPLINK link, uint32_t id);
	~query_keepalive_status_task();
	void on_task() override final;
	
};

/***************************************����M������״̬********************************************/
class set_keepalive_status_task : public base_task {
	uint32_t pkt_id_;
	int status_;//���� vcu ״̬
	nsp::tcpip::endpoint vcu_endpoint_;
	
public:
	set_keepalive_status_task(HTCPLINK link, uint32_t id, int status);
	~set_keepalive_status_task();
	void on_task() override final;
	
};

/***************************************������̿�������********************************************/
class deal_process_cmd_task {
	int command_;
	int process_id_all_;
	std::vector<std::string > vec_cmd_param_;
	
public:
	deal_process_cmd_task(int cmd, int process_id);
	~deal_process_cmd_task();
	int process_task();
	void add_cmd_param(std::string& param);
	
};


#endif /* __GRACE_ROBOT_AGVSHELL_TASK_H__ */
