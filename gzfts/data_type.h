#pragma once

#include <string>
#include "asio_manage.h"
#include <map>
#include <deque>

struct file_callabck_data{
	std::string ipaddr;
	uint16_t port;
	std::string path_local;
	std::string path_remote;
	int status = -1;
	int error = 0;
	int step = 0;
	uint64_t total_size = 0;
};

namespace nsp{
	namespace file{
		struct file_head{
			std::string path_remote;	//Ŀ�������ļ�ָ��·��
			uint64_t  total_size;		//�ļ��ܴ�С
			int  is_overwrite;			//�Ƿ�ǿ�Ƹ���
			uint64_t file_create_time_;	//�ļ�����ʱ��
			uint64_t file_modify_time_;	//����޸�ʱ��
			uint64_t file_access_time_;	//�������ʱ��
		};

		struct file_block{
			uint64_t offset;
			std::string stream;
			uint64_t file_id = 0;

			file_block& operator=(const file_block& lref){
				if (this == &lref)return *this;
				offset = lref.offset;
				stream = lref.stream;
				file_id = lref.file_id;
				return *this;
			}
		};

		struct file_info : asio_data{
			uint64_t file_size_ = 0;		//�ܴ�С
			uint32_t file_block_num_ = 0;	//������Ŀ
			uint64_t file_create_time_ = 0;	//�ļ�����ʱ��
			uint64_t file_modify_time_ = 0;	//����޸�ʱ��
			uint64_t file_access_time_ = 0;	//�������ʱ��
			uint32_t file_attri_;			//�ļ�����
			uint64_t file_crc32_ = 0;		//�ļ�crc32��
			std::string file_name_;			//�ļ�����
			uint64_t file_id_ = 0;				//�ļ�ID
		};

		struct list_dir_info : asio_data{
			std::string list_data_;
		};

		enum  current_identify
		{
			pull_identify, //pull���
			push_identify  //push���
		};

		enum pkt_identify
		{
			receiver_pkt,	//����receiver����İ�
			sender_pkt		//����sender����İ�
		};
	}
}
