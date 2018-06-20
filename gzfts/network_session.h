#pragma once

#include "application_network_framwork.hpp"
#include "old.hpp"
#include "proto_definion.h"
#include "proto_file_head_msg.h"
#include "proto_file_block.h"
#include "proto_file_block_ack.h"
#include "proto_msg.h"
#include "proto_typedef.h"
#include <memory>
#include "data_type.h"
#include "network_manager.h"
#include "sender_push_task.h"
#include "sender_pull_task.h"

#if WIN32
#include <Windows.h>
#endif

struct file_data{
	std::string ipaddr;
	uint16_t port;
	std::string path_local;
	std::string path_remote;
	int status = -1;
	int error = 0;
	int step = 0;
	void *user_data=nullptr; 
	int user_data_size = 0;
	uint64_t total_size = 0;
};

class network_session : public nsp::tcpip::tcp_application_client<nsp::proto::nspdef::protocol>
{
public:
	network_session();
	network_session(HTCPLINK lnk);
	~network_session();

public:
	uint64_t get_interval();
	uint32_t get_link();
	int push_file_to_remote(const std::string &path,const nsp::file::file_info& f_info,const int overwrite);
	
protected:
	virtual void on_disconnected(const HTCPLINK previous) override final;
	virtual void on_recvdata(const std::string &pkt) override final;

public:
	//pull ���������ͻ�ȡĳ���ļ���Ϣ����
	int post_pull_requst_file(const std::string& path_remote,const std::string& path_local,const int overwrite);
	int post_pull_file_block_response(const uint32_t pkt_id, const uint64_t offset, const std::string& data);
	int post_get_listdir(const std::string&input_catalog, const std::shared_ptr<asio_block>&asio_ack);
	int push_file_block_stream(const uint64_t offset, const std::string& data);
	int post_pull_next_block_request(const uint64_t offset);
	//pull������sender д�����һ֡������֮�󣬷���������ݸ�receive
	int post_pull_complete();
	int post_push_next_block_request(const uint64_t offset);
	void get_file_total_size_and_status(file_data& data);
	void set_remote_local_ep(const file_data f_data, const uint32_t block_pre_size);

	int post_delete_file_requst(const std::string&path_remote, const std::shared_ptr<asio_block>&asio_ack);

	int post_push_write_file_status(const int status, const int error_code);
	int post_pull_read_file_error(const int status, const int err_code);

	int post_transmit_timeout();
	int post_catalog_request(const std::string& input_folder, const std::shared_ptr<asio_block>&asio_ack);
	int post_catalog_reponse(const uint32_t pkt_id, const int error, const std::vector<nsp::file::file_info>& vct_info);

	void set_remote_endpoin(const nsp::tcpip::endpoint& ep){ remote_ep_ = ep; }
	nsp::tcpip::endpoint get_remote_endpoint(){ return remote_ep_; }

private:
	std::atomic<uint64_t> refresh_tick_{ 0 };//��¼��ǰʱ���
	int dispatch_package(int package_type, const std::string &pkt_data, const std::shared_ptr<nsp::proto::proto_head> &head);
	
private:
	//push������receive session�յ�ͷ�ļ���Ϣ������ͬ�ȴ�С�ļ�
	int push_recv_file_head_msg(const std::shared_ptr<nsp::proto::proto_file_head_msg_t> &pkt);
	//push���������յ�ͷ�ļ���Ϣ֮�󣬷��͵�һ֡����
	int push_file_head_msg_ack(const std::shared_ptr<nsp::proto::proto_head> &head);
	//push������receiver session���յ�������֮�󣬿����������ڴ���
	int push_recv_write_file_block(const std::shared_ptr<nsp::proto::proto_file_block_t> &head);
	//push������sender �յ���һ֡����Ӧ�����������һ֡����
	int push_recv_file_block_ack(const std::shared_ptr<nsp::proto::proto_file_block_ack_t> &head);

	int recv_get_listdir_ack(const std::shared_ptr<nsp::proto::proto_msg> &head);
	int recv_get_catalog_info(const std::shared_ptr<nsp::proto::_proto_request_file> &head);
	int recv_get_current_catalog_info(const int pkt_id, const std::shared_ptr<nsp::proto::_proto_request_file>& data);

	//pull������receive session�յ���ȡ�ļ�����
	int recv_requst_file(const std::shared_ptr<nsp::proto::_proto_request_file>&pkt_data);
	//pull������sender session�յ��ļ�ͷ��Ϣ
	int recv_fts_file_head(const std::shared_ptr<nsp::proto::proto_write_head> &data);
	//pull������receive session�յ���ȡָ��offset�ļ�����������
	int recv_fts_requst_file_data(const std::shared_ptr<nsp::proto::_proto_request_file_data> &data);
	//pull������sender session���յ��ļ�������
	int recv_fts_requst_file_data_ack( const std::shared_ptr<nsp::proto::_proto_file_data> &data);
	//�����ļ�������
	int recv_fts_file_complete(const std::shared_ptr<nsp::proto::_proto_identify> &data);
	//ɾ���ļ�����
	int recv_fts_delete_files(const std::shared_ptr<nsp::proto::_proto_file_delete>&data);
	//ɾ���ļ��ذ�
	int recv_fts_delete_files_ack(const std::shared_ptr<nsp::proto::proto_head>&head);
	//sender�ص�����һ��
	void fts_callback(const uint32_t status,const uint32_t step, const uint32_t error_code);

	void file_rw_handler(const nsp::file::current_identify,const int step,const nsp::proto::file_status status,const int error_code);

	int recv_transmit_timout(const std::shared_ptr<nsp::proto::proto_head> &head);

	int recv_current_catalog_reponse(const std::shared_ptr<nsp::proto::proto_head>& head_t, const std::string& data_str);

private:
	int is_overwrite_ = -1;
	//�ļ��ܴ�С
	uint64_t file_total_size_ = 0;
	//���ر����ļ�·��
	std::string write_file_path_;
	file_data callback_data_;
	//ÿһƬĬ�ϵĴ�������ݿ��С
	uint32_t block_size_pre_transfer_;

	std::shared_ptr<sender_push_task> push_task_ = nullptr;
	std::shared_ptr<sender_pull_task> pull_task_ = nullptr;

	nsp::tcpip::endpoint remote_ep_;

#ifndef _WIN32
	uint64_t create_time_ = 0;
	uint64_t modify_time_ = 0;
	uint64_t access_time_ = 0;
#endif
};
