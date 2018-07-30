#pragma once

#include <string>
#include <stdint.h>
#include "file_shell_handler.h"

#if _WIN32
#include <windows.h>
#include <Shlwapi.h>
#else
#if defined _LARGEFILE64_SOURCE
#undef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 
#else
#define _LARGEFILE64_SOURCE 
#endif
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>
#include <fcntl.h>
#endif


class file_write_handler{

public:
	file_write_handler();
	~file_write_handler();

public:
	//�����ļ�, ���� &fd
	void* create_file(const std::string& file_path, const uint64_t& create_time, const uint64_t& modify_time, const uint64_t& access_time, int & error_code);
	//�����ļ�, ���� FILE*
	FILE* create_file(const std::string& file_name, const unsigned long file_size);
	//�����ļ���
	int create_folder(const std::string& file_path, int & error_code);
	//д���ļ�, ������Ϊ  fd 
	int write_file_block(const void* handler, const uint64_t offset, const std::string& stream);
	//д���ļ�, ������Ϊ FILE* 
	int write_file(FILE* file_handle, const unsigned long offset,const std::string& data);
	int close_handler(const void* handler);
	inline int close_file(FILE* file_handle) {
		return (file_handle != NULL) ? fclose(file_handle) : -1;
	}
	//ɾ���ļ�
	int delete_file(const std::string&file_path);
	//�����ļ�ʱ�� 
	int reset_file_time(const std::string& file_name, const uint64_t& create_time, const uint64_t& modify_time, const uint64_t& access_time);

private:
	file_shell_handler shell_handler_;
	uint8_t time_zone_;
};
