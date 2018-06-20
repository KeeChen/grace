#pragma once

#include "application_network_framwork.hpp"

// ������ⳬʱʱ�䶨�壺30��(30000����) 
#define TIME_SEND_HEART_BEAT 30000
#define TIME_OUT_HEART_BEAT  90000

class tcp_session : public nsp::tcpip::tcp_application_client<nsp::proto::nspdef::protocol>
{
public:
	tcp_session();
	~tcp_session();
	tcp_session(HTCPLINK lnk);

public:
	// ���/����������ⱨ��  
	int check_heart_beat();
	// ֪ͨ�ͻ���С�������и��� 
	int post_notify_agvinfo_update();

	// �ͻ������Ӿ��� 
	virtual int on_established() override final;

public:
	// �������ʱ��
	uint64_t heart_beat_tickt;

protected:
	virtual void on_recvdata(const std::string &pkt) override final;
	virtual void on_disconnected(const HTCPLINK previous) override final;

	int on_heart_beat(const std::string &pkt);
	int on_get_agvinfo(const std::string &pkt);
	int on_set_agvinfo(const std::string &pkt);
	int on_get_agvdetail(const std::string &pkt);
	int on_set_agvdetail(const std::string &pkt);
private:
};