#pragma once

#include "data_type.h"
#include "file_shell_handler.h"
#include <string>
#include <stdint.h>

#if _WIN32
#include <iostream>
#include <io.h>
#include <Windows.h>

#define HANDLE_INVALID		-1
#else
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>

#endif

class file_read_headler{
	uint64_t file_size_ = 0;
public:
	file_read_headler();
	~file_read_headler();

public:
	//Ŀ¼�������ļ� 
	int get_fts_listdir(const std::string& current_path, std::string & dir, const std::string& prefix = "");
	//���ļ���Ϣ, ���ļ� 
	void* read_file_head(const std::string& current_path, uint64_t& file_size, uint64_t& create_time, uint64_t& modify_time, uint64_t& access_time,int &error_code);
	//���ļ���Ϣ, �����ļ� 
	int read_folder_info(const std::string& current_path, uint64_t& create_time, uint64_t& modify_time, uint64_t& access_time, int& error_code); 
	int read_file_stream(const void* file_handler,const uint64_t offset,const uint32_t read_size, std::string& data);
	void close_handler(const void*file_handler);

	// Ŀ¼�µ��ļ���Ϣ��crc32 ֵ 
	int get_fts_current_catalog(const std::string& current_path, std::vector<file::st_file_info>& vct_f);
	//û�ã����� 
	int read_file_id(const void* file_handler, uint64_t & f_id, int &error_code);

private:
	uint64_t calculate_crc32(const void* file_handler, uint64_t file_size, uint64_t file_block_size);
	uint64_t calculate_folder_crc32(const std::string& folder_path);

private:
    uint8_t time_zone_;
	file_shell_handler shell_handler_;

};