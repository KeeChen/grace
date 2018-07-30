#include "file_write_handler.h"
#include "log.h"
#include "public.h"
#include "os_util.hpp"

file_write_handler::file_write_handler(){
#ifndef _WIN32
    time_zone_ = get_time_zone();
#endif
}

file_write_handler::~file_write_handler(){

}

void* file_write_handler::create_file(const std::string& file_path, const uint64_t& create_time, const uint64_t& modify_time, const uint64_t& access_time, int & error_code){

#ifdef _WIN32
	size_t pos = file_path.find_last_of("/");
	if (pos == 0 || pos == std::string::npos)return nullptr;

	std::string directory = file_path.substr(0, pos) + "/";
	std::string current_file_path;
	//�˴����жϴ�����ļ�����·���Ƿ���ڣ���������ڣ���ع���һ��鿴�Ƿ���lnk·��������������ݷ�ʽ·�������ȥ��ݷ�ʽ�ļ�����ȡ����
	if (!PathFileExistsA(directory.c_str())){
		shell_handler_.find_lnk_file(file_path, current_file_path);
	}
	else{
		current_file_path = file_path;
	}
	HANDLE handler = INVALID_HANDLE_VALUE;
	handler = CreateFileA(file_path.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == handler) {
		error_code = GetLastError();
		return nullptr;
	}

	FILETIME c_time, m_time, a_time;
	c_time.dwHighDateTime = create_time >> 32;
	c_time.dwLowDateTime = create_time & 0x00000000ffffffff;
	m_time.dwHighDateTime = modify_time >> 32;
	m_time.dwLowDateTime = modify_time & 0x00000000ffffffff;
	a_time.dwHighDateTime = access_time >> 32;
	a_time.dwLowDateTime = access_time & 0x00000000ffffffff;
	if(!SetFileTime(handler,&c_time,&a_time,&m_time)){
		error_code = GetLastError();
		CloseHandle(handler);
		return nullptr;
	}
	error_code = 0;
	return (void*)handler;
#else
	int fd;
	fd = open(file_path.c_str(), O_RDWR | O_TRUNC | O_CREAT, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	if (fd < 0){
		error_code = errno;
		return nullptr;
	}
	
	error_code = 0;
	struct stat st;
    if (lstat(file_path.c_str(), &st) < 0) {
    	loerror("fts") << "failed to get file stat for create, file name:" << file_path;
    	error_code = errno;
    }
    
    char real_file[128] = {0};
    std::string real_file_path = file_path;
    if ( S_ISLNK(st.st_mode) ) {
    	int file_len = readlink(file_path.c_str(), real_file, 127);
    	if ( file_len >= 0 ) {
    		real_file[file_len] = '\0';
    		real_file_path = real_file;
    	} else {
    		loinfo("fts") << "readlink file_len:" << file_len << ", errno:" << errno;
    		error_code = errno;
    	}
    }
	
    utimbuf file_time;
    file_time.actime = access_time/10000000 - 11644444800 - time_zone_ * 60 * 60;
    file_time.modtime = modify_time/10000000 - 11644444800 - time_zone_ * 60 * 60;
    if ( utime(real_file_path.c_str(), &file_time) < 0 ) {
        error_code = errno;
        loerror("fts") << "failed to change file utime:" << file_path;
    }
	return (void *)(long)fd;
#endif
}

FILE* file_write_handler::create_file(const std::string& file_name, const unsigned long file_size){
	FILE* file_handle;
	if ((file_handle = fopen(file_name.c_str(), "wb+")) == NULL){
		loerror("agv_shell") << "failed to create bin file:" << file_name;
		return nullptr;
	}
	//д�ļ�
	try{
		char* buffer = new char[file_size];
		memset(buffer, 0, file_size);
		if (fwrite(buffer, sizeof(char), file_size, file_handle) != file_size){
			loerror("agv_shell") << "failed to init file.";
			fclose(file_handle);
			delete[] buffer;
			return nullptr;
		}
		delete[] buffer;
	}
	catch (...){
		loerror("agv_shell") << "failed to allocate memory.";
		fclose(file_handle);
		return nullptr;
	}
	//��ָ��Ų���ļ���ͷ
	if (fseek(file_handle, 0, SEEK_SET) != 0){
		loerror("agv_shell") << "failed to move file ptr to begin.";
		fclose(file_handle);
		return nullptr;
	}
	return file_handle;
}

int file_write_handler::write_file_block(const void* handler, const uint64_t offset, const std::string& stream){
#ifdef _WIN32
	LARGE_INTEGER liDistanceToMove, pointer;
	liDistanceToMove.QuadPart = offset;
	if (!SetFilePointerEx((HANDLE)handler, liDistanceToMove, &pointer, FILE_BEGIN)){
		//δ�ܳɹ��ƶ�ָ��
		return -ESPIPE;
	}
	DWORD real_size = 0;
	if (!WriteFile((HANDLE)handler, stream.c_str(), stream.length(), &real_size, NULL)){
		//δ�ܳɹ�д���ļ�
		return -1;
	}
	//ǿ�ƽ�����������д�������,�˴������Ĵ�����CPUʱ�䣬��ʵ�ʲ��Թ����У��˴����������紫������
	//FlushFileBuffers((HANDLE)handler);
#else
	int fd = (long)handler & 0xffffffff;
	if (fd <= 0 ) {
		return -EINVAL;
	}
	
	off64_t retval = lseek64(fd, offset, SEEK_SET);
	if (retval == (off64_t)-1){
		return -1;
	}
	
	const char *pdata = stream.data();
	size_t wroff = 0;
	size_t cptotal = stream.size();
	//int retval;
	while (wroff < cptotal){
		retval = write(fd, pdata + wroff, cptotal - wroff);
		if (retval <= 0) {
			if (errno == EINTR) {
				continue;
			}
			return -1;
		}
		wroff += retval;
	}
#endif
	return 0;
}

int file_write_handler::write_file(FILE* file_handle, const unsigned long offset,const std::string& data){
	if(!file_handle) return -1;
	
	//��ָ��Ų���ļ���ͷ
	if (fseek(file_handle, offset, SEEK_SET) != 0){
		loerror("agv_shell") << "failed to move file ptr to offset:" << (uint32_t)offset;
		return -1;
	}
	//д������
	if (fwrite(data.c_str(), sizeof(char), data.size(), file_handle) != data.size()){
		loerror("agv_shell") << "failed to write block data to file.";
		return -1;
	}
	return 0;
}

int file_write_handler::close_handler(const void* handler){
#ifdef _WIN32
	if (INVALID_HANDLE_VALUE != handler){
		return CloseHandle((HANDLE)handler)==TRUE? 0 : -1;
	}
#else
	int fd = (long)handler & 0xffffffff;
	if (fd > 0) {
		if(close(fd) < 0)
		{
			return -1;
		}
		return 0;
	}
#endif
	return -1;
}

int file_write_handler::delete_file(const std::string&file_path)
{
#ifdef _WIN32
	return DeleteFileA(file_path.c_str()) ? 0 : -1;
#else
    int res = nsp::os::is_dir(file_path);
	if (res > 0){
		//Ŀ¼
        return (nsp::os::rmdir_s(file_path) == 0 ? 0 : -1);//rmdir_s == 0 ɾ���ɹ���other ɾ��ʧ��
	}
	else if (res == 0){
		//�ļ�
        return (nsp::os::rmfile(file_path) == 0 ? 0 : -1);//rmfile == 0ɾ���ɹ���other��ɾ��ʧ��
	}
	else{
		//����
		loerror("fts") << "can not find or delete file:" << file_path;
		return -1;
	}
    return 0;
#endif
}

int file_write_handler::create_folder(const std::string& file_path, int & error_code){

#ifdef _WIN32
	if(!PathFileExistsA(file_path.c_str())){
		if (!CreateDirectoryA(file_path.c_str(), NULL)){
			error_code = GetLastError();
			loerror("fts") << "can not create folder:" << file_path <<" ,the error code is " << error_code;
			return -1;
		}
		error_code = 0;
	}
	else loinfo("fts") << "the folder :" << file_path << " already exists.";
	return 0;
#else
    if( nsp::os::mkdir_s(file_path) < 0) {
        loerror("fts") << "can not create file:" << file_path;
        error_code = errno;
        return -1;
    }
    
    return 0;
#endif
}

int file_write_handler::reset_file_time(const std::string& file_name, const uint64_t& create_time, const uint64_t& modify_time, const uint64_t& access_time)
{
#ifndef _WIN32
	loinfo("fts") << "reset_file_time file_name:" << file_name << ", create_time=" << create_time << ", modify_time:" << modify_time << ", access_time:" << access_time;
	utimbuf file_time;
    file_time.actime = access_time/10000000 - 11644444800 - time_zone_ * 60 * 60;
    file_time.modtime = modify_time/10000000 - 11644444800 - time_zone_ * 60 * 60;
    if ( utime(file_name.c_str(), &file_time) < 0 ) {
        loerror("fts") << "failed to change file utime:" << file_name << ", errno=" << errno;
    }
#endif

	return 0;
}
