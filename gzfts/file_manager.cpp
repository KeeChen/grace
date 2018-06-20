#include "file_manager.h"
#include "log.h"
#include "receiver_manager.h"
#include "sender_manager.h"
#ifdef _WIN32
#include <io.h>
#endif

file_manager::file_manager(){
	initlization();
}

file_manager::~file_manager(){

}

void file_manager::initlization(){
	//����5�����߳�,5��д�߳�
	for (int i = 0; i < 5; i++){
		std::shared_ptr<file_rw_task_thread> task = std::make_shared<file_rw_task_thread>();
		map_rw_task_.insert(std::make_pair(i, task));
	}
	return;
}

//�����ļ�
int file_manager::create_files(const uint64_t link, const nsp::file::file_head& f_head, int & error_code, bool is_long_lnk, const uint64_t file_id){
	if (f_head.path_remote.size() <= 0) return 0;

	std::string curr_folder_path;
	file_write_handler write_handler;
	//�ж���������Ƿ�Ϊ�ļ��в�����������򴴽��ļ���
	if (f_head.path_remote.at(f_head.path_remote.size() - 1) == '/'){
		curr_folder_path = f_head.path_remote;
		if (f_head.path_remote.find(".lnk") != std::string::npos){
			lnk_handler_.find_lnk_file(f_head.path_remote, curr_folder_path);
		}
		return create_directory(curr_folder_path, error_code);
	}

	//�ж��ļ���·���Ƿ���ڣ���������ڣ�����Ҫ�����ϲ��ļ���
	std::string folder_path = f_head.path_remote.substr(0, f_head.path_remote.find_last_of('/'));
	//�ж�����·�����Ƿ���lnk��ݷ�ʽ����
	if (folder_path.find(".lnk") != std::string::npos){
		//Ѱ��lnkָ�������·��
		if (!lnk_handler_.find_lnk_file(folder_path, curr_folder_path)){
			//δ�ܴ���lnk�ļ��У���ʹ�����������ΪĿ��·��
			curr_folder_path = folder_path;
		}
	}
	else{
		curr_folder_path = folder_path;
	}

	int type = nsp::os::is_dir(curr_folder_path);
	
	if (type < 0){
		std::lock_guard<decltype(folder_mutex_)> lock(folder_mutex_);
		curr_folder_path += '/';
		if (recreate_file_path(curr_folder_path) < 0){//nsp::os::mkdir(curr_folder_path) < 0
#ifdef _WIN32
			error_code = GetLastError();
#else
            error_code = errno;
#endif
			loerror("fts") << "failed to create folder: " << curr_folder_path << " the error code is " << error_code;
			return -1;
		}
	}

	void* file_handler = nullptr;
	if ((file_handler = write_handler.create_file(f_head.path_remote, f_head.file_create_time_, f_head.file_modify_time_, f_head.file_access_time_, error_code)) == nullptr){
		loerror("fts") << "failed to create file " << f_head.path_remote << " the error code is " << error_code;
		return -1;
	}
	if (!is_long_lnk)
	{
		push_file_handler(link, file_handler);
	}
	else
	{
		push_fd_handler(link, file_id, file_handler);
	}
	return 0;
}

std::string file_manager::convert_positive(const std::string& str, const char preview_cr, const  char new_cr){
	std::string result = str;
	for (size_t i = 0; i < result.size(); i++){
		if (result.at(i) == preview_cr){
			result.at(i) = new_cr;
		}
	}
	return result;
}

int file_manager::recreate_file_path(const std::string& file_name)
{
	std::string tmp_file = convert_positive(file_name, '\\', '/');

	size_t pos = tmp_file.find("/");
	if (pos == std::string::npos)return -1;
	std::string temp_path = tmp_file.substr(pos + 1, tmp_file.length() - pos - 1);//ȥ���̷��磨"E:\\")
	std::string root_path = tmp_file.substr(0, pos + 1);
	pos = temp_path.find("/");
	if (pos == std::string::npos)return -1;
	std::string path;
	do{
		path = root_path + temp_path.substr(0, pos);
#ifdef _WIN32
		if (nsp::os::mkdir(path) < 0){
			loerror("fts") << "recreate_file_path mkdir path error:" << path;
			return -1;
		}
#else
        struct stat st;
        if (lstat(path.c_str(), &st) < 0) {
        	loerror("fts") << "failed to get file stat for create, folder name:" << path;
        } else {
            char real_folder_path[128] = {0};
            if ( S_ISLNK(st.st_mode) ) {
            	int folder_len = readlink(path.c_str(), real_folder_path, 127);
            	if ( folder_len >= 0 ) {
            		real_folder_path[folder_len] = '\0';
            		path = real_folder_path;
            	} else {
            		loinfo("fts") << "readlink file_len:" << folder_len << ", errno:" << errno;
            		//return -1;
            	}
            }
        }
#endif
		root_path = path + "/";
		temp_path = temp_path.substr(pos + 1, temp_path.length() - pos - 1);
		pos = temp_path.find("/");
	} while (pos != std::string::npos);
#ifndef _WIN32
	if (nsp::os::is_dir(path) < 0) {
    	if (nsp::os::mkdir_s(path) < 0) {
    		loerror("fts") << "recreate_file_path mkdir error: " << errno;
    		return -1;
    	}
    }
#endif
	return 0;
}

//��ȡ�ļ���Ϣ
int file_manager::read_files(const uint32_t link, const std::string& file_path, nsp::file::file_info& f_info, int&error_code){
	void* file_handler = nullptr;
	file_read_headler read_handler;
	//�������Ҫ���ȡ�����ļ��У�����ö�ȡ�ļ�����Ϣ����
	if (nsp::os::is_dir(file_path) > 0){
		return read_handler.read_folder_info(file_path, f_info.file_create_time_, f_info.file_modify_time_, f_info.file_access_time_, error_code);
	}
	
	if ((file_handler = read_handler.read_file_head(file_path, f_info.file_size_,
		f_info.file_create_time_, f_info.file_modify_time_, f_info.file_access_time_, error_code)) == nullptr){
		loerror("fts") << "failed read file:" << file_path << " ,the error code is " << error_code;
		return -1;
	}
	push_file_handler(link, file_handler);
	return 0;
}

//����������¶�ȡ�ļ���Ϣ����ͬһ��lnk���ӵ�����»��ж���ļ������������ڣ��ʴ�ʱ��Ҫʹ���ļ�ID������
int file_manager::read_files_long_lnk(const uint32_t link, const std::string& file_path, nsp::file::file_info& f_info, int&error_code)
{
	void* file_handler = nullptr;
	file_read_headler read_handler;
	//�������Ҫ���ȡ�����ļ��У�����ö�ȡ�ļ�����Ϣ����
	if (nsp::os::is_dir(file_path) > 0){
		return read_handler.read_folder_info(file_path, f_info.file_create_time_, f_info.file_modify_time_, f_info.file_access_time_, error_code);
	}

	if ((file_handler = read_handler.read_file_head(file_path, f_info.file_size_,
		f_info.file_create_time_, f_info.file_modify_time_, f_info.file_access_time_, error_code)) == nullptr){
		loerror("fts") << "failed read file:" << file_path << " ,the error code is " << error_code;
		return -1;
	}
	/*if (read_handler.read_file_id(file_handler, f_info.file_id_, error_code) < 0)
	{
		loerror("fts") << "failed to get file ID ";
		return -1;
	}*/
	push_fd_handler(link,f_info.file_id_, file_handler);
	return 0;
}

//��ȡָ������������Ϣ
int file_manager::read_file_block_stream(const uint32_t pkt_id, const uint32_t link, 
	const nsp::file::current_identify identify, const uint64_t offset, const uint32_t read_size){
	void* file_handler;
	{
		std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
		auto iter = map_file_handler_.find(link);
		if (iter == map_file_handler_.end()){
			loerror("fts") << "read_file_block_stream can not find link in the map_file_handler.";
			return -1;
		}
		file_handler = iter->second;
	}

	int index = ((long)file_handler & 0xffffffff) % 5;
	auto iter_thread = map_rw_task_.find(index);
	if (iter_thread == map_rw_task_.end()){
		loerror("fts") << "read_file_block_stream can not find read task in the map_rw_task_.";
		return -1;
	}

	iter_thread->second->add_read_task(file_handler, pkt_id, link, offset, read_size, identify);
	return 0;
}

//�����Ӷ�ȡ��������Ϣ
int file_manager::read_file_block_long_lnk(const uint32_t pkt_id, const uint32_t link, const uint64_t file_id,
	const nsp::file::current_identify identify, const uint64_t offset, const uint32_t read_size)
{
	void* file_handler;
	{
		std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
		auto iter = map_fd_.find(link);
		if (iter == map_fd_.end()){
			loerror("fts") << "read_file_block_long_lnk can not find link in the map_file_identify.";
			return -1;
		}
		auto identify_iter = iter->second.find(file_id);
		if (identify_iter == iter->second.end())
		{
			loerror("fts") << "read_file_block_long_lnk can not find file id in the map_file_identify.";
			return -1;
		}
		file_handler = identify_iter->second;
	}

	int index = ((long)file_handler & 0xffffffff) % 5;
	auto iter_thread = map_rw_task_.find(index);
	if (iter_thread == map_rw_task_.end()){
		loerror("fts") << "read_file_block_stream can not find read task in the map_rw_task_.";
		return -1;
	}

	iter_thread->second->add_read_task(file_handler, pkt_id, link, offset, 
		read_size, identify, true, file_id);
	return 0;
}

//д�ļ�����Ϣ
int file_manager::write_file_block_stream(const uint32_t link, const nsp::file::current_identify identify, 
	const nsp::file::file_block& f_blcok, const bool is_long_lnk){
	void* file_handler;

	if (!is_long_lnk)
	{
		std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
		auto iter = map_file_handler_.find(link);
		if (iter == map_file_handler_.end()){
			loerror("fts") << "write_file_block_stream can not find link in the map_file_handler.";
			return -1;
		}
		file_handler = iter->second;
	}
	else
	{
		std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
		auto iter = map_fd_.find(link);
		if (iter == map_fd_.end())
		{
			loerror("fts") << "write_file_block_stream can not find link in the long link.";
			return -1;
		}
		auto fd_iter = iter->second.find(f_blcok.file_id);
		if (fd_iter == iter->second.end())
		{
			loerror("fts") << "write_file_block_stream can not find file handler in the long link.";
			return -1;
		}
		file_handler = fd_iter->second;
	}

	int index = ((long)file_handler & 0xffffffff) % 5;
	auto iter_thread = map_rw_task_.find(index);
	if (iter_thread == map_rw_task_.end()){
		loerror("fts") << "write_file_block_stream can not find write task in the map_rw_task_.";
		return -1;
	}
	if (!is_long_lnk)
	{
		iter_thread->second->add_write_task(file_handler, link, f_blcok, identify);
	}
	else
	{
		iter_thread->second->add_write_task(file_handler, link, f_blcok, identify, true, f_blcok.file_id);
	}
	return 0;
}

//ɾ��ָ���ļ��ӿ�
int file_manager::delete_file(const std::string& file_path){
	std::string handler_path;
	//�жϸ�·���Ƿ�Ϊlnk��ݷ�ʽ
	if (file_path.find(".lnk") != std::string::npos){
		//Ѱ��lnkָ�������·��
		if (!lnk_handler_.find_lnk_file(file_path, handler_path)){
			//δ�ܴ���lnk�ļ��У���ʹ�����������ΪĿ��·��
			handler_path = file_path;
		}
	}
	else{
		handler_path = file_path;
#ifndef _WIN32
    struct stat st;
    if (lstat(file_path.c_str(), &st) < 0) {
    	loerror("fts") << "failed to get file stat for delete, file name:" << file_path;
    	return -1;
    }
    
    char real_file[128] = {0};
    if ( S_ISLNK(st.st_mode) ) {
    	int file_len = readlink(file_path.c_str(), real_file, 127);
    	if ( file_len >= 0 ) {
    		real_file[file_len] = '\0';
    		handler_path = real_file;
    	} else {
    		loinfo("fts") << "readlink file_len:" << file_len << ", errno:" << errno;
    	}
    }
#endif
	}
	int res = nsp::os::is_dir(handler_path);
	if (res > 0){
		//Ŀ¼
		return (nsp::os::rmdir_s(handler_path) >= 0 ? 0 : -1);//rmdir_s == 0 ɾ���ɹ���other ɾ��ʧ��
	}
	else if (res == 0){
		//�ļ�
		return (nsp::os::rmfile(handler_path) >= 0 ? 0 : -1);//rmfile == 0ɾ���ɹ���other��ɾ��ʧ��
	}
	else{
		//����
		loerror("fts") << "can not find or delete file:" << handler_path;
		return -1;
	}
}

//���ļ������link��������
void file_manager::push_file_handler(const uint32_t link, void* handler){
	std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
	map_file_handler_.insert(std::make_pair(link, handler));
}

//���������link��file id,�ļ����
void file_manager::push_fd_handler(const uint32_t link, const uint64_t file_id, void* handler)
{
	std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
	auto iter = map_fd_.find(link);
	if (iter == map_fd_.end())
	{
		std::map<uint64_t, void*> tmp;
		tmp.insert(std::make_pair(file_id, handler));
		map_fd_.insert(std::make_pair(link,tmp));
	}
	else
	{
		auto id_iter = iter->second.find(file_id);
		if (id_iter == iter->second.end())
		{
			iter->second.insert(std::make_pair(file_id, handler));
		}
		else
		{
			id_iter->second = handler;
		}
	}
}

//�رճ�������ĳ���ļ����
bool file_manager::close_fts_file_handler(const uint32_t link, const uint64_t file_id)
{
	std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
	auto iter = map_fd_.find(link);
	if (iter == map_fd_.end())
	{
		return false;
	}
	auto iter_fd = iter->second.find(file_id);
	if (iter_fd == iter->second.end())
	{
		return false;
	}
	file_write_handler read_handler;
	bool result = read_handler.close_handler(iter_fd->second);
	iter->second.erase(iter_fd);
	return result;
}

//�Ƚ�Դ�ļ���С��Ŀ���ļ���С�Ƿ�һ�£������һ�£���ɾ��Ŀ���ļ�
void file_manager::is_file_complete(const std::string& file_path, const uint64_t src_file_size){
	//д�ļ��Ƿ������ûд��
#ifdef _WIN32
	if (_access(file_path.c_str(), 0) != -1){
#else
	if (access(file_path.c_str(), F_OK) != -1){
#endif
		uint64_t  file_size, c_time, a_time, m_time;
		int error_code;
		file_read_headler read_handler;
		//ȡ�Ѿ�д�˵��ļ��Ĵ�С��Դ�ļ���С�Ƚ�
		read_handler.close_handler(read_handler.read_file_head(file_path, file_size, c_time, m_time, a_time, error_code));
		if (src_file_size > file_size){
			loinfo("fts") << "the size of writiing file does not match the source file size.";
			file_write_handler write;
			write.delete_file(file_path);
			loinfo("fts") << "delete the file:" << file_path;
		}
	}
}

//ɾ����д�ļ����
void file_manager::remove_rw_file_handler(const uint32_t link){
	std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
	auto iter = map_file_handler_.find(link);
	if (iter == map_file_handler_.end()){
		return;
	}
	file_write_handler read_handler;
	read_handler.close_handler(iter->second);
	map_file_handler_.erase(iter);
}

//ɾ�������Ӷ�д�ļ����
void file_manager::remove_fw_long_file_handler(const uint32_t link)
{
	std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
	auto iter = map_fd_.find(link);
	if (iter == map_fd_.end()){
		return;
	}
	file_write_handler read_handler;
	for (const auto& iter_fd : iter->second)
	{
		read_handler.close_handler(iter_fd.second);
	}
	iter->second.clear();
	map_fd_.erase(iter);
}

//ɾ����д�߳��ж�д����������
void file_manager::clear_rw_task_deque(const uint32_t link){
	void* file_handler;
	{
		std::lock_guard<decltype(rw_mutex_)> lock(rw_mutex_);
		auto iter = map_file_handler_.find(link);
		if (iter == map_file_handler_.end()){
			return;
		}
		file_handler = iter->second;
	}

	int index = ((long)file_handler & 0xffffffff) % 5;
	auto iter_thread = map_rw_task_.find(index);
	if (iter_thread == map_rw_task_.end()){
		return;
	}
	iter_thread->second->clear_rw_deque(link);
}

int file_manager::create_directory(const std::string& dir, int & error_code){
#ifdef _WIN32
	if (!PathFileExistsA(dir.c_str())){
		if (recreate_file_path(dir) < 0){
			error_code = GetLastError();
			loerror("fts") << "failed to create directory: " << dir << " the error code is " << error_code;
			return -1;
		}
	}
	return 0; 
#else
    if (recreate_file_path(dir) < 0){
        error_code = errno;
    	loerror("fts") << "failed to create directory: " << dir << " the error code is " << error_code;
    	return -1;
    }
    return 0;
#endif
}

int file_manager::reset_file_time(const std::string& file_name, const uint64_t& create_time, const uint64_t& modify_time, const uint64_t& access_time)
{
	file_write_handler w_handler;
	return w_handler.reset_file_time(file_name, create_time, modify_time, access_time);
}