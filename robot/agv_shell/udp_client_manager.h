#pragma once

#include "udp_session.h"
#include "singleton.hpp"
#include <map>
#include <string>
#include <memory>
#include "proto_udp_typedef.h"

class udp_client_manager{
	udp_client_manager();
	~udp_client_manager();
	friend class nsp::toolkit::singleton<udp_client_manager>;

public:
	int init_network();
	int post_vcu_type_request(const nsp::tcpip::endpoint& ep, const std::shared_ptr<motion::asio_block> &asio_ack);
	int post_vcu_version_request(const FIRMWARE_SOFTWARE_TYPE type, const nsp::tcpip::endpoint& ep, const int is_control_, const std::shared_ptr<motion::asio_block> &asio_ack);
	int post_vcu_reset_request(const nsp::tcpip::endpoint& ep, const uint32_t file_size, const std::shared_ptr<motion::asio_block> &asio_ack);
	int post_vcu_write_bin_file(const FIRMWARE_SOFTWARE_TYPE type, const nsp::tcpip::endpoint& ep, const int block_offset,
		const std::string&file_block_data, const std::shared_ptr<motion::asio_block>& asio_ack);
	int post_vcu_read_bin_file(const FIRMWARE_SOFTWARE_TYPE type, const nsp::tcpip::endpoint& ep, const int block_offset,
		const int buffer_length, const std::shared_ptr<motion::asio_block>& asio_ack);
	int post_cpu_type_requst(const nsp::tcpip::endpoint& ep, const std::shared_ptr<motion::asio_block> &asio_ack);
	int post_vcu_keep_alive_request(const nsp::tcpip::endpoint& ep);
	int post_query_vcu_keep_alive_request(const nsp::tcpip::endpoint& ep, const std::shared_ptr<motion::asio_block> &asio_ack);
	int post_set_vcu_keep_alive_request(const nsp::tcpip::endpoint& ep, int status, const std::shared_ptr<motion::asio_block> &asio_ack);

	//can��������ҵ��
	int post_can_write_bin_file(const nsp::tcpip::endpoint& ep, const int block_offset, const std::string&file_block_data,
		const std::shared_ptr<motion::asio_block>& asio_ack, uint8_t code_serial, uint8_t nodeid);
	int post_can_reset_request(const nsp::tcpip::endpoint& ep, const uint8_t node_id, const uint8_t code_serial,
		const std::shared_ptr<motion::asio_block> &asio_ack);
	int post_can_version_request(const nsp::tcpip::endpoint& ep, const uint8_t nodeid, const uint8_t code_serial,
		const std::shared_ptr<motion::asio_block> &asio_ack);
	int post_can_type_request(const nsp::tcpip::endpoint& ep, const uint8_t node_id, const uint8_t code_serial,
		const std::shared_ptr<motion::asio_block> &asio_ack);
	int post_can_data_forward(const nsp::tcpip::endpoint& ep, const uint8_t code_serial, 
		const std::shared_ptr<motion::asio_block> &asio_ack);
	int post_can_init_rom_request(const nsp::tcpip::endpoint& ep, const uint8_t node_id, const uint8_t code_serial,
		const std::shared_ptr<motion::asio_block> &asio_ack,const int & block_number);


	void set_m_core_endpoint(const std::string& ipv4,const uint16_t port);
	nsp::tcpip::endpoint get_m_core_endpoint(){ return m_core_ep_obj_; }
	void set_camera_endpoint(const std::string& ipv4, const uint16_t port);
	nsp::tcpip::endpoint get_camera_endpoint(){ return camera_ep_obj_; }
	void set_version_control(const int c_v){ version_control_ = c_v; }
	int get_version_control(){ return version_control_; }
	void set_reset_wait_time(const uint32_t time){ reset_timeout_ = time; }
	uint32_t get_reset_wait_time(){ return reset_timeout_; }
	void set_local_endpoint(const std::string& ipv4,const uint16_t port);
	nsp::tcpip::endpoint get_local_ep_obj_(){ return local_ep_obj_; }

private:
	void clean_session();

private:
	nsp::tcpip::endpoint local_ep_obj_;
	nsp::tcpip::endpoint m_core_ep_obj_;
	nsp::tcpip::endpoint camera_ep_obj_;
	int version_control_;
	uint32_t reset_timeout_;

	std::shared_ptr<udp_session> net_session_;
};