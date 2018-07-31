#include "asio_manage.h"
#include <Windows.h>
#include "log.h"

//////////////////////////////////////////asio �첽����////////////////////////////////////////////////

asio_manage::asio_manage() :asio_timeout_waiter_(0){

}

asio_manage::~asio_manage(){
	if (timeout_thread_){
		if (timeout_thread_->joinable()){
			asio_timeout_waiter_.sig();
			timeout_thread_->join();
		}
		delete timeout_thread_;
		timeout_thread_ = nullptr;
	}

	asio_timeout_waiter_.reset();
}

//asio ����
int asio_manage::push(const int pkt_id, const std::shared_ptr<motion::asio_block>& asio){
	std::lock_guard<decltype(map_lock_)> locker(map_lock_);
	if (asio_map_.find(pkt_id) != asio_map_.end()){
#ifdef _DEBUG
		nspwarn << "can not push new asio_block into map,there is already exist pkt_id.";
#endif
		return -1;
	}
	asio_map_.insert(std::make_pair(pkt_id, asio));
	//�����߳�
	if (!timeout_thread_){
		timeout_thread_ = new std::thread(std::bind(&asio_manage::timeout_check, this));
	}
	return 0;
}

int asio_manage::push_data_forward(const int cmd, const std::shared_ptr<motion::asio_block>& asio){
	std::lock_guard<decltype(map_lock_)> locker(map_lock_);
	if (asio_map_data_forward_.find(cmd) != asio_map_data_forward_.end()){
#ifdef _DEBUG
		nspwarn << "can not push new asio_block into map,there is already exist pkt_id.";
#endif
		return -1;
	}
	asio_map_data_forward_.insert(std::make_pair(cmd, asio));
	//�����߳�
	if (!timeout_thread_){
		timeout_thread_ = new std::thread(std::bind(&asio_manage::timeout_check, this));
	}
	return 0;
}

//asioִ�У�����+ɾ����
int asio_manage::exec(const int pkt_id, const void*data){
	std::shared_ptr<motion::asio_block> asio;
	{
		std::lock_guard<decltype(map_lock_)> locker(map_lock_);
		auto iter = asio_map_.find(pkt_id);
		if (iter == asio_map_.end()){
#ifdef _DEBUG
			nsperror << "can not operate exec function,can not find id:" << pkt_id << " package.";
#endif
			return -1;
		}

		asio = iter->second;
		asio_map_.erase(iter);
	}
	if (asio->callback_function_){
		asio->callback_function_(data);
		return 0;
	}
#ifdef _DEBUG
	nsperror << "can not callback fuction,there isn't callback_function_ in asio_block.";
#endif
	return -1;
}

int asio_manage::exec_data_forward(const int cmd, const void*data){
	std::shared_ptr<motion::asio_block> asio;
	{
		std::lock_guard<decltype(map_lock_)> locker(map_lock_);
		auto iter = asio_map_data_forward_.find(cmd);
		if (iter == asio_map_data_forward_.end()){
#ifdef _DEBUG
			nsperror << "can not operate exec_data_forward function,can not find cmd id:" << cmd << " package.";
#endif
			return -1;
		}

		asio = iter->second;
		asio_map_data_forward_.erase(iter);
	}
	if (asio->callback_function_){
		asio->callback_function_(data);
		return 0;
	}
#ifdef _DEBUG
	nsperror << "can not callback fuction,there isn't callback_function_ in asio_block.";
#endif
	return -1;
}


int asio_manage::exec(const int pkt_id, const void* stream, const int error, const int cb){
	std::shared_ptr<motion::asio_block> asio;
	{
		std::lock_guard<decltype(map_lock_)> locker(map_lock_);
		auto iter = asio_map_.find(pkt_id);
		if (iter == asio_map_.end()){
#ifdef _DEBUG
			nsperror << "can not operate exec function,can not find id:" << pkt_id << " package.";
#endif
			return -1;
		}

		asio = iter->second;
		asio_map_.erase(iter);
	}
	if (asio->object_callback_function_){
		asio->ack_ = (unsigned char *)stream;
		asio->cb_ = cb;
		asio->error_ = error;
		asio->object_callback_function_(asio);
		return 0;
	}
#ifdef _DEBUG
	nsperror << "can not callback fuction,there isn't object_callback_function_ in asio_block.";
#endif
	return -1;
}

//asio�Ƴ�
int asio_manage::remove(const int pkt_id, std::shared_ptr<motion::asio_block>&asio){
	std::lock_guard<decltype(map_lock_)> locker(map_lock_);
	auto iter = asio_map_.find(pkt_id);
	if (iter == asio_map_.end()){
#ifdef _DEBUG
		nsperror << "can not remove in asio,can not find id:" << pkt_id << " package.";
#endif
		return -1;
	}
	asio = iter->second;
	asio_map_.erase(iter);
	return 0;
}

int asio_manage::remove_data_forward(const int cmd, std::shared_ptr<motion::asio_block>&asio){
	std::lock_guard<decltype(map_lock_)> locker(map_lock_);
	auto iter = asio_map_data_forward_.find(cmd);
	if (iter == asio_map_data_forward_.end()){
#ifdef _DEBUG
		nsperror << "can not remove in asio,can not find id:" << cmd << " package.";
#endif
		return -1;
	}
	asio = iter->second;
	asio_map_data_forward_.erase(iter);
	return 0;
}

//asio��������
int asio_manage::write(const int pkt_id, const std::shared_ptr<motion::asio_block>&asio, const std::function<int()>&MyWrite){
	if (!MyWrite){
#ifdef _DEBUG
		nsperror << "lamba function is not exist.";
#endif
		return -1;
	}
	if (push(pkt_id, asio) < 0){
		return -1;
	}
	if (MyWrite() < 0){
		std::shared_ptr<motion::asio_block> previews;
		remove(pkt_id, previews);
#ifdef _DEBUG
		nsperror << "failed to send packeage,the pkt_id is " << pkt_id;
#endif
		return -1;
	}
	return 0;
}

int asio_manage::write_data_forward(const int cmd, const std::shared_ptr<motion::asio_block>&asio, const std::function<int()>&MyWrite){
	if (!MyWrite){
#ifdef _DEBUG
		nsperror << "lamba function is not exist.";
#endif
		return -1;
	}
	if (push_data_forward(cmd, asio) < 0){
		return -1;
	}
	if (MyWrite() < 0){
		std::shared_ptr<motion::asio_block> previews;
		remove_data_forward(cmd, previews);
#ifdef _DEBUG
		nsperror << "failed to send packeage,the pkt_id is " << cmd;
#endif
		return -1;
	}
	return 0;
}

void asio_manage::timeout_check(){
	static const uint32_t ASIO_CHECK_INTERVAL = 1000;
	static const uint32_t ASIO_TIMEOUT = 2000;
	static const uint32_t ASIO_TIMEOUT_INIT_ROM = 5000;
	uint32_t tick;
	while (asio_timeout_waiter_.wait(ASIO_CHECK_INTERVAL) > 0){
		tick = ::GetTickCount();
		{
			std::lock_guard<decltype(map_lock_)> locker(map_lock_);

			auto iter = asio_map_data_forward_.begin();
			while (iter != asio_map_data_forward_.end()){
				if (tick != iter->second->tick_){
					uint32_t timeout = 0;
					if (iter->first != 0x13)
					{
						timeout = ASIO_TIMEOUT;
					}
					else
					{
						timeout = ASIO_TIMEOUT_INIT_ROM;
					}

					if (tick > iter->second->tick_){
						if ((tick - iter->second->tick_) > timeout){
#ifdef _DEBUG
							nspwarn << "asio manage get a timeout,the pkt_id is " << iter->first;
#endif
							//�ص��ϲ�
							iter->second->error_ = nsp::proto::errorno_t::kRequestTimeout;
							timeout_notify(iter->second);
							iter = asio_map_data_forward_.erase(iter);
							continue;
						}
					}
					else if ((tick + (MAXDWORD - iter->second->tick_)) > timeout){
#ifdef _DEBUG
						nspwarn << "asio manage get a overflow timeout,the pkt_id is " << iter->first;
#endif
						//�ص��ϲ�
						iter->second->error_ = nsp::proto::errorno_t::kRequestTimeout;
						timeout_notify(iter->second);
						iter = asio_map_data_forward_.erase(iter);
						continue;
					}
				}
				else{
					iter->second->refresh();
				}

				++iter;
			}

			iter = asio_map_.begin();
			while (iter != asio_map_.end()){
				if (tick != iter->second->tick_){
					if (tick > iter->second->tick_){
						if ((tick - iter->second->tick_) > ASIO_TIMEOUT){
#ifdef _DEBUG
							nspwarn << "asio manage get a timeout,the pkt_id is "<<iter->first;
#endif
							//�ص��ϲ�
							iter->second->error_ = nsp::proto::errorno_t::kRequestTimeout;
							timeout_notify(iter->second);
							iter = asio_map_.erase(iter);
							continue;
						}
					}
					else if ((tick + (MAXDWORD - iter->second->tick_)) > ASIO_TIMEOUT){
#ifdef _DEBUG
						nspwarn << "asio manage get a overflow timeout,the pkt_id is " << iter->first;
#endif
						//�ص��ϲ�
						iter->second->error_ = nsp::proto::errorno_t::kRequestTimeout;
						timeout_notify(iter->second);
						iter = asio_map_.erase(iter);
						continue;
					}
				}
				else{
					iter->second->refresh();
				}

				++iter;
			}
		}
	}

	// asio ����߳��˳��� ͨ���������Ѿ��Ͽ��� �κβд�� asio ������������
	std::lock_guard<decltype(map_lock_)> guard(map_lock_);
	asio_map_.clear();
}

void asio_manage::timeout_notify(const std::shared_ptr<motion::asio_block>& asio_block){
	motion::asio_data data;
	data.set_err(nsp::proto::errorno_t::kRequestTimeout);
	if (asio_block->callback_function_){
		asio_block->callback_function_((const char*)&data);
	}
	if (asio_block->object_callback_function_){
		asio_block->object_callback_function_(asio_block);
	}
}
