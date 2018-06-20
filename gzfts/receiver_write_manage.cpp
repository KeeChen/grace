#include "receiver_write_manage.h"
#include "log.h"

receiver_write_manage::receiver_write_manage(){

}

receiver_write_manage::~receiver_write_manage(){

}

//���뵱ǰ����д���ļ�
void receiver_write_manage::add_write_file(const uint32_t link, const std::string& file_path){
	std::lock_guard<decltype(re_mutex_)> lock(re_mutex_);
	auto iter = map_write_files_.find(link);
	if (iter == map_write_files_.end()){
		map_write_files_.insert(std::make_pair(link, file_path));
	}
	return;
}

//���ҵ�ǰ��Ҫд����ļ��Ƿ��Ѿ�����,����ֵ��false = δ�ҵ���true = ���ҵ�
bool receiver_write_manage::search_write_file(const std::string& file_path, uint32_t& link){
	std::lock_guard<decltype(re_mutex_)> lock(re_mutex_);
	for (auto iter : map_write_files_){
		if (iter.second == file_path){
			link = iter.first;
			return true;
		}
	}
	return false;
}

//�����ǰlink�����Ӧ��д����ļ�
void receiver_write_manage::remove_write_file(const uint32_t link){
	std::lock_guard<decltype(re_mutex_)> lock(re_mutex_);
	auto iter = map_write_files_.find(link);
	if (iter == map_write_files_.end()){
		return;
	}
	map_write_files_.erase(iter);
}