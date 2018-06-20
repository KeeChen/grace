#pragma once

#include <string>
#include <stdint.h>
#include <map>
#include <mutex>
#include <memory>
#include "long_lnk_session.h"
#include "gzfts_api.h"
#include "data_type.h"

typedef int(STDCALL *fts_callback)(const char *ipaddr, uint16_t port,
	const char *path_local, const char *path_remote,
	int status, int error, int step,
	void *user_data, int user_data_size);

class long_lnk_sender{
public:
	long_lnk_sender();
	~long_lnk_sender();

public:
	void add_file_info(const uint64_t file_id, const file_callabck_data& f_data);
	int fts_pull_long(const char *ipaddr_from, uint16_t port_from, const char *path_local, const char *path_remote, int overwrite);
	std::shared_ptr<long_lnk_session> create_session(const nsp::tcpip::endpoint& ep);
	void insert_session(const uint32_t link, const std::shared_ptr<long_lnk_session>& session);
	void fts_cancel(const char* ipaddr, const uint16_t port);

	std::string get_local_file_path(const uint64_t pull_id);
	void set_pull_file_size(const uint64_t pull_id, const uint64_t size);
	void fts_long_lnk_callback(const uint64_t pull_id, const int status, const int step, const int error_code);
	int creat_file(const uint32_t link, nsp::file::file_head& filehead, int& error_code, const uint64_t file_id);
	int write_file(const uint32_t link, const nsp::file::file_block& f_blcok);
	int fts_pull_write_file_error(const uint32_t link, const uint64_t fd, 
		const nsp::proto::file_status status, const int error_code);
	int fts_pull_next_block(const uint32_t link, 
		const uint32_t fd, const uint64_t offset);
	int fts_change_configure(const struct fts_parameter *param);

	void close_fts_file_handler(const uint32_t lnk, const uint64_t file_id);
	int delete_map_link_session(const uint32_t link);

	void remove_file_id(const uint32_t file_id);
	
private:
	uint64_t allocate_pull_identify(){ return pull_file_identify_++; }
	std::shared_ptr<long_lnk_session> search_session(const uint32_t link);

	void init();
	void uinit();
	void alive_check_thread();

private:
	std::recursive_mutex mutex_;
	//������в������ļ���Ϣ
	std::map<uint64_t, file_callabck_data> map_fd_;//key:�ļ�Ψһ��ʶ��value:�ļ�������Ϣ

	std::recursive_mutex session_mutex_;
	//������е�clinet session����ʹ��link��Ϊ����
	std::map<uint32_t, std::shared_ptr<long_lnk_session>>map_link_session_;

	//ÿ����ȡ���ļ��������һ��id���ʶ��
	uint64_t pull_file_identify_;

	fts_callback callback_fun_;//�û��ص�������֪ͨ�û���ǰ���ȣ��Զ�ip��port����Ϣ
	//ÿһƬĬ�ϵĴ�������ݿ��С
	uint32_t block_size_pre_transfer_ = FILE_BLOCK_SIZE;

	//��������߳�
	nsp::os::waitable_handle time_wait_;
	std::thread* alive_th_ = nullptr;
	std::atomic<int> is_exits_{ false };

};