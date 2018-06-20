#pragma once

#include "tcp_session.h"
#include "singleton.hpp"
#include "common_data.h"
#include "../common/agvinfo_proto.hpp"
#include "os_util.hpp"

#include <memory>
#include <thread>

// map�е�keyΪMacAddress 
typedef std::multimap<std::string, proto_agvinfo> map_agvinfo;
#define INVALID_VHID (~0)
#define DEFAULT_MTPORT (4409)
#define DEFAULT_SHPORT (4410)
#define DEFAULT_FTSPORT (4411)

class tcp_manager{
	tcp_manager();
	~tcp_manager();
	friend class nsp::toolkit::singleton<tcp_manager>;

public:
	int tcp_listen(const std::string& ipv4, const uint16_t port);
	// ��xml�ļ��ж�ȡagvinfo��vec_xml_agvinfo
	int load_agvinfo_xml();
	// ��map_xml_agvinfoд�뵽xml�ļ� 
	int save_agvinfo_xml();
	// �������map_xml_agvinfo
	map_agvinfo& get_xml_agvinfo();
	// �������map_xml_agvinfo��Ҫ���� 
	std::recursive_mutex & get_mutex_agvinfo();

	// �������
	void check_heart_beat();
	// ����֪ͨ�ͻ���agvinfo�����仯��־(�������Һ�֪ͨ���ͻ���) 
	void set_notify_agvinfo_update();
	// ����Ƿ���Ҫ֪ͨ�ͻ���agvinfo�����仯�� 
	int check_notify_agvinfo_update();
	// ֪ͨ�ͻ���agvinfo�����仯�� 
	void post_notify_agvinfo_update();
	// ���յ�udp��Ϣ����Ҫ����һ̨����Ϣ 
	void insert_macinfo(const robot_mac_info_t& mac_info);
	// ���յ�udp��Ϣ����Ҫ����һ̨����Ϣ 
	void update_macinfo(const robot_mac_info_t& mac_info);
	// ���յ�udp��Ϣ����Ҫɾ��һ̨����Ϣ 
	void delete_macinfo(const robot_mac_info_t& mac_info);


private:
	// ��ȡxml�ļ���crc32У���� 
	uint32_t get_xml_crc32();
	// ��ʱ���xml�ļ��ı��߳�
	void check_thread();
	// ���xml�ļ���
	void check_xml_update();
	// ��ʼ������: ��ʼ��crc32У���룬���ζ�ȡxml�ļ�����������߳�
	void initlizatin();
	// ����ʼ���������ȴ�����߳��˳� 
	void uinit();

private:
	std::shared_ptr<nsp::tcpip::tcp_application_service<tcp_session>> server_ptr_;

	nsp::os::waitable_handle wait_check_;
	std::thread* check_th_ = nullptr;
	std::atomic<bool> exit_th_{ false };

	// ��Ҫ֪ͨ�ͻ������ݸ��±�־ 0:����Ҫ֪ͨ 1:��Ҫ֪ͨ
	int need_notify;
	// xml�ļ���CRC32У���� 
	uint32_t crc32_xml_old;
	uint32_t crc32_xml_new;
	// ����map_xml_agvinfoʱʹ��(ͬʱֻ��һ���ͻ��˶�д����)
	std::recursive_mutex r_mutex_agvinfo;
	// agvȫ����Ϣ(KeyΪMacAddress) 
	map_agvinfo map_xml_agvinfo;
};
