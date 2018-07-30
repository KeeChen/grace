#ifndef __AGV_SHELL_FILE_MANAGER_H__
#define __AGV_SHELL_FILE_MANAGER_H__

#include "data_type.h"
#include "file_read_headler.h"
#include "file_rw_task_thread.h"
#include "file_write_handler.h"
#include "singleton.hpp"
#include <map>
#include <mutex>

class file_manager{
	file_manager();
	~file_manager();
	friend class nsp::toolkit::singleton<file_manager>;

public:
	//�����ļ� 
	int create_files(const uint64_t link, file::st_file_head& f_head, int& error_code);
	//��ȡ�ļ���Ϣ 
	int read_file_info(const uint32_t link, file::st_file_head& f_head, int&error_code);
	//��ȡָ����������
	int read_file_block_stream(const uint32_t link, const int pkt_id, const uint64_t file_id, 
		const uint32_t block_num, const uint64_t offset, const uint32_t read_size);
	//д�ļ��� 
	int write_file_block_stream(const uint32_t link, const int pkt_id, const uint64_t file_id, 
		const uint32_t block_num, const uint64_t offset, const std::string& stream);
	//�Ƚ�Դ�ļ���С��Ŀ���ļ���С�Ƿ�һ�£������һ�£���ɾ��Ŀ���ļ�
	int file_complete(const uint32_t link, const uint64_t file_id);
	//ɾ���ļ� 
	int delete_file(const std::string& file_path);
	//�ر�������ĳ���ļ���� 
	int close_rw_file_handler(const uint32_t link, const uint64_t file_id);
	//ɾ�����������ж�д�ļ���� 
	void close_rw_file_handler(const uint32_t link);
	//ɾ����д�߳��ж������� 
	void clear_rw_task_deque(const uint32_t link);
	
	//����Ŀ¼ 
	int create_directory(const std::string& dir, int & error_code);
	//�����ļ�ʱ������ 
	int reset_file_time(const std::string& file_name, const uint64_t& create_time,
		const uint64_t& modify_time, const uint64_t& access_time);
		
	void check_file_timeout();
	
	inline void set_file_dir(std::string& dir) {
		file_dir = dir;
	}

	//��������״̬(״ֵ̬��1)��״ֵ̬�����ڼ䲻������ִ���������� 
	void set_upgrade();
	//�ͷ�����״̬(״ֵ̬��1)��״̬Ϊ��ʱ������������ 
	void release_upgrade();
private:
	void initlization();
	
	//���������link, file id, �ļ���Ϣ
	void inert_head_t_map(const uint32_t link, const uint64_t file_id, file::st_file_head&);
	//���� link+file_id ��ȡ�ļ�head �ṹָ�� 
	int get_head_f_map(const uint32_t link, const uint64_t file_id, file::st_file_head**);
	//��ʽ��֮�󴴽��ļ��� 
	int recreate_file_path(const std::string& file_name);
private:
	//����״̬ 0��������״̬ !0������״̬(�����ļ����ڴ��䣬�����ļ����ڽ�ѹ���ȶ�������ִ��)
	std::atomic<int> is_upgrade_{ 0 };
    std::map<uint32_t, std::shared_ptr<file_rw_task_thread>> map_rw_task_;//key:id, value:file_rw_task_thread
	
	std::recursive_mutex rw_mutex_;
	std::map<uint32_t,std::map<uint64_t, file::st_file_head>> map_file_head_;//key:link, <key��file id �ļ���,  value:�ļ���Ϣ >
	
	//�����ļ�����
	std::recursive_mutex folder_mutex_;
	//default store file directory
	std::string file_dir;
	
	file_read_headler read_handler_;
	file_write_handler write_handle_;
};

#endif  //__AGV_SHELL_FILE_MANAGER_H__
