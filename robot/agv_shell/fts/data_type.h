#pragma once

#include "asio.hpp"
#include "const.h"
#include <deque>
#include <map>
#include <string>


#ifdef _USRDLL
#define METH dllexport
#else
#define METH dllimport
#endif

#include <stdint.h>

#ifndef FTS_EXP

//����״̬
enum transmit_status{
	transmission_timeout = -100,	//���䳬ʱ
	transmission_interruption = -1,	//�����ж�
	transmission_complete = 0,		//�������
	transmiting,					//�������
	transmission_wait				//�ȴ�����
};

//file type 
enum file_type {
	type_normal = 0,
	type_upgrade = 1,
};
//oper type 
enum oper_type {
	oper_type_push = 1,
	oper_type_pull = 2,
};



#if _WIN32
#define FTS_EXP(type)  extern "C" _declspec(METH) type __stdcall
struct fts_parameter {
	uint32_t			block_size_pre_transfer = FILE_BLOCK_SIZE;
	uint32_t			timeout_cancel_transfer = TIMEOUT_TRANSFER;
	uint32_t			maximum_iops_pre_link;
	uint32_t			maximum_iops_global;
	uint32_t			memory_cache_size_pre_link;
	uint32_t			memory_cache_size_global;
	char				file_search_root[300];
	uint32_t 			window_size = TRANSFER_WINDOW_SIZE;
	void *				user_data;
	int					user_data_size;

	int(__stdcall *fts_callback)(const char *ipaddr, uint16_t port,
		const char *path_local, const char *path_remote,
		int status, int error, int step,
		void *user_data, int user_data_size);
};

#else
#define FTS_EXP(type)  extern "C" type
struct fts_parameter {
	uint32_t			block_size_pre_transfer;
	uint32_t			timeout_cancel_transfer;
	uint32_t			maximum_iops_pre_link;
	uint32_t			maximum_iops_global;
	uint32_t			memory_cache_size_pre_link;
	uint32_t			memory_cache_size_global;
	char				file_search_root[300];
	uint32_t 			window_size;
	void *				user_data;
	int					user_data_size;
	int(*fts_callback)(const char *ipaddr, uint16_t port,
		const char *path_local, const char *path_remote,
		int status, int error, int step,
		void *user_data, int user_data_size);
};
#endif //_WIN32
#endif //FTS_EXP


namespace file {
	struct st_file_head {
		std::string file_path;		//Ŀ�������ļ�ָ��·��
		std::string file_name;		//�ļ�����
		uint32_t file_type = 0;		//�ļ�����
		uint32_t file_block_num = 0;//������Ŀ
		uint64_t file_id = 0;		//����ʱ��Ψһ���
		uint64_t total_size;		//�ļ��ܴ�С
		uint64_t file_create_time;	//�ļ�����ʱ��
		uint64_t file_modify_time;	//����޸�ʱ��
		uint64_t file_access_time;	//�������ʱ��
		uint64_t oper_time;			//���һ�β���ʱ��, ���ڳ�ʱ��� 
		uint64_t oper_type;			//��/���ļ����� 
		void* hdl_fd = nullptr;		//fd
		FILE* hdl_file = nullptr;	//FILE
	};

	struct st_file_info : motion::asio_data, st_file_head{
		uint32_t file_attri;			//�ļ�����
		uint64_t file_crc32 = 0;		//�ļ�crc32��
	};

	struct write_task_info{
		st_file_head* p_head;
		uint32_t link;	//���ڶ���ɾ������ 
		int pkt_id;
		uint32_t cur_block_num = 0;		//��ǰ����Ŀ���
		uint64_t offset;
		std::string stream;
	};

	struct read_task_info{
		st_file_head* p_head;
		uint32_t link;	//���ڶ���ɾ������ 
		int pkt_id;
		uint32_t cur_block_num = 0;		//��ǰ����Ŀ���
		uint32_t read_size;
		uint64_t offset;
	};
	
	struct timeout_file {
		uint32_t link = 0;
		uint64_t file_id = 0;
	};
}
