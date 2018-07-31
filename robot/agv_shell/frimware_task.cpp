#include "frimware_task.h"
#include "udp_client_manager.h"
#include "agv_shell_server.h"
#include "file_read_handler.h"
#include "file_write_handler.h"
#include "log.h"
#include "agv_shell_proto.hpp"
#include "agv_shell_define.h"
#include "file_can_read.h"
/*---------------------------------------------------------------------------------------------------------------------------------*/

void frimware_update_task::on_task(){
	if (nsp::tcpip::endpoint::build(target_ep_, vcu_endpoint_) < 0)
	{
		loerror("agv_shell") << "failed to build target endpoint " << target_ep_;
		return;
	}

	reset_timeout_ = nsp::toolkit::singleton<udp_client_manager>::instance()->get_reset_wait_time();
	file_size_ = nsp::toolkit::singleton<file_read_handler>::instance()->get_file_size();
	//�����ȡ���ļ�������Ƭ�ɶ��ٿ�
	uint64_t file_block_num = 0;

	if (frimwre_type_ == CAN_CUSTOM_SOFTWARE)
	{
		file_block_num = nsp::toolkit::singleton<file_can_read>::instance()->get_file_block_num();
		
		//canЭ������ӳ�ʼ��rom����
		int ini_flag = 0;
		for (int i = 0; i < 3; i++)
		{
			if (post_can_init_rom_update(file_block_num) == 0)
			{
				ini_flag = 1;
				break;	
			}
		}
		if (ini_flag == 0)
		{
			loerror("agv_shell") << "failed to init can custom ROM update.";
			//֪ͨ��ʼ��ʧ��
			nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_update(link_,
				frimwre_type_, FRIMWARE_STATUS::kFailCanInitRom, 0);
			return;
		}
	}
	else{
		file_block_num = file_size_ / FRIMWARE_FILE_BLOCK_SIZE;
		file_block_num = (file_size_ % FRIMWARE_FILE_BLOCK_SIZE == 0) ? file_block_num : (file_block_num + 1);
	}
	
	for (int i = 0; i < 3; i++){
		//��ǰΪcanЭ���������������������������Ҫ��ȡ�ļ����бȶԣ�ֱ�ӽ�����������
		if (frimwre_type_ == CAN_CUSTOM_SOFTWARE)
		{
			if (write_can_custom_bin(file_block_num) < 0){
				current_status_ = FRIMWARE_STATUS::kFailUpdate;
				continue;
			}
			if (post_reset_request() == 0){
				//��ǰ״̬��λ
				current_status_ = FRIMWARE_STATUS::kRestarting;
				post_frimware_update(link_, frimwre_type_, current_status_, current_step_);
				//�����ȴ����ȴ�VCU���������ȡ�汾��
				reset_wait_.wait(reset_timeout_);
				//�ٴλ�ȡ��Ϣ
				nsp::toolkit::singleton<agv_shell_server>::instance()->on_get_frimware_info(link_, frimwre_type_, target_ep_);
				//��ȡ��Ϣ���
				current_status_ = FRIMWARE_STATUS::kNormal;
			}
			else{
				current_status_ = FRIMWARE_STATUS::kFailRestart;
			}
			break;
		}
		else
		{
			if (write_bin_file_to_vcu(file_block_num) < 0){
				current_status_ = FRIMWARE_STATUS::kFailUpdate;
				continue;
			}
			//���سɹ������VCU�����ٻ�ȡ�ո��·���bin�ļ����ȶ��������ļ��Ƿ���ͬ����ͬ���ʾ�ļ�һ��
			if (get_bin_file(file_block_num) == 0){
				if (post_reset_request() == 0){
					//��ǰ״̬��λ
					current_status_ = FRIMWARE_STATUS::kRestarting;
					post_frimware_update(link_, frimwre_type_, current_status_, current_step_);
					//�����ȴ����ȴ�VCU���������ȡ�汾��
					reset_wait_.wait(reset_timeout_);
					//�ٴλ�ȡ��Ϣ
					nsp::toolkit::singleton<agv_shell_server>::instance()->on_get_frimware_info(link_, frimwre_type_, target_ep_);
					//��ȡ��Ϣ���
					current_status_ = FRIMWARE_STATUS::kNormal;
				}
				else{
					current_status_ = FRIMWARE_STATUS::kFailRestart;
				}
				break;
			}
			else{
				current_status_ = FRIMWARE_STATUS::kFailCompare;
			}
		}
	}
	post_frimware_update(link_, frimwre_type_, current_status_, current_step_);
}

//��VCUд��Ƭ����
int frimware_update_task::write_bin_file_to_vcu(const int block_num){
	for (int block_index = 0; block_index < block_num; block_index++){
		current_step_ = ((double)(block_index + 1) / block_num) * 100;
		//ÿһƬ���д��ʧ�ܣ�������д������
		for (int post_count = 0; post_count < 3; post_count++){
			//����д��VCU�̼����󣬷���һƬ����Ƭ
			recv_vcu_data recv_data;
			motion::asio_data asio_data_;
			nsp::os::waitable_handle water(0);
			std::string buffer;
			uint32_t read_size;
			if ((block_index + 1) == block_num)
			{
				read_size = file_size_ - block_index * FRIMWARE_FILE_BLOCK_SIZE;
			}
			else
			{
				read_size = FRIMWARE_FILE_BLOCK_SIZE;
			}
			nsp::toolkit::singleton<file_read_handler>::instance()->get_file_buffer(block_index * FRIMWARE_FILE_BLOCK_SIZE, read_size, buffer);
			int res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_vcu_write_bin_file(frimwre_type_, vcu_endpoint_, 
				block_index*FRIMWARE_FILE_BLOCK_SIZE,buffer, std::make_shared<motion::asio_block>([&](const void*data){
					if (!data){
						water.sig();
						return;
					}
					asio_data_ = *(motion::asio_data*)data;
					if (asio_data_.get_err() < 0){
						water.sig();
						return;
					}
					water.sig();
			}));
			
			if (res < 0)
			{
				loerror("agv_shell") << "failed to send write block file to vcu.";
				//���Ͳ��ɹ�
				if (post_count == 2) return -1;
				else continue;
			}
			//��ʱ�ȴ�
			water.wait();
			water.reset();

			if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful)
			{
				//�ȴ�д����һƬ��ʱ��
				block_write_wait_.wait(100);
				break;
			}
			else{
				loerror("agv_shell") << "send write block file to vcu timeout.";
				//�������û���յ�VCU�Ļذ�֡���ݣ���ֱ�ӷ��أ����ٷ�������İ�
				if (post_count == 2) return -1;
				else continue;
			}
		}
		//���д��ɹ�������д�����֪ͨ
		post_frimware_update(link_, frimwre_type_, FRIMWARE_STATUS::kBusy, current_step_);
	}
	return 0;
}

//canЭ�����bin�ļ�
int frimware_update_task::write_can_custom_bin(const int block_num)
{
	loinfo("agv_shell") << "write can custom bin file,the node id is:" << node_id_ << " the serial code is:" << serial_code_;
	int local_step = 0;
	for (int block_index = 0; block_index < block_num; block_index++){
		current_step_ = ((double)(block_index + 1) / block_num) * 100;
		//ÿһƬ���д��ʧ�ܣ�������д������
		for (int post_count = 0; post_count < 3; post_count++){
			//����д��VCU�̼����󣬷���һƬ����Ƭ
			recv_vcu_data recv_data;
			motion::asio_data asio_data_;
			nsp::os::waitable_handle water(0);
			size_t nIndex = block_index;
			int res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_can_write_bin_file(vcu_endpoint_, ++nIndex,
				nsp::toolkit::singleton<file_can_read>::instance()->get_file_block_data(block_index),
				std::make_shared<motion::asio_block>([&](const void*data){
				if (!data){
					water.sig();
					return;
				}
				asio_data_ = *(motion::asio_data*)data;
				if (asio_data_.get_err() < 0){
					water.sig();
					return;
				}
				recv_data = *(recv_vcu_data*)data;

				water.sig();
			}), serial_code_, node_id_);


			if (res < 0)
			{
				loerror("agv_shell") << "failed to send write block file to vcu.";
				//���Ͳ��ɹ�
				if (post_count == 2) return -1;
				else continue;
			}
			//��ʱ�ȴ�
			water.wait();
			water.reset();

			if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful)
			{
				int nLength = recv_data.data_length_;
				nsp::proto::udp::can_data_t can_data;
				can_data.build((unsigned char *)recv_data.data_context_.data(), nLength);

				uint16_t check_index = can_data.can_serial_index_1 + (can_data.can_serial_index_0 << 8);
				if (check_index != nIndex)
				{
					loerror("agv_shell") << "get can custom check index:" << check_index << " is not equal to local block index:" << nIndex;
					if (post_count == 2)return -1;
					continue;
				}
				//�ȴ�д����һƬ��ʱ��
				//block_write_wait_.wait(100);
				break;
			}
			else{
				loerror("agv_shell") << "send write block file to can custom timeout.";
				//�������û���յ�VCU�Ļذ�֡���ݣ���ֱ�ӷ��أ����ٷ�������İ�
				if (post_count == 2) return -1;
				else continue;
			}
		}
		//���д��ɹ�������д�����֪ͨ
		if (current_step_ != local_step){
			post_frimware_update(link_, frimwre_type_, FRIMWARE_STATUS::kBusy, current_step_);
			local_step = current_step_;
		}

	}
	return 0;
}

//��VCU��ȡƬ����
int frimware_update_task::get_bin_file(const int block_number){

	char* buffer_file;
	try{
		buffer_file = new char[file_size_];
	}
	catch (...){
		loerror("agv_shell") << "failed to new memory.";
		return -1;
	}
	for (int block_index = 0; block_index < block_number; block_index++){
		motion::asio_data asio_data_;
		nsp::os::waitable_handle water(0);
		recv_bin_data recv_data;
		//���һ���������ݿ��С��һ������һ��block�飬���Դ˴���Ҫ�ֿ�����
		uint32_t read_size = 0;
		if (block_index != (block_number - 1)){
			read_size = FRIMWARE_FILE_BLOCK_SIZE;
		}
		else{
			read_size = file_size_ - (uint64_t)block_index * FRIMWARE_FILE_BLOCK_SIZE;
		}
		
		int res = -1;
		if (frimwre_type_ == CAN_CUSTOM_SOFTWARE)
		{

		}
		else
		{
			res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_vcu_read_bin_file(FIRMWARE_SOFTWARE_TYPE::COMPARE_SOFTWARE,
				vcu_endpoint_, block_index*FRIMWARE_FILE_BLOCK_SIZE, read_size, std::make_shared<motion::asio_block>([&](const void*data){
				if (!data){
					water.sig();
					return;
				}
				asio_data_ = *(motion::asio_data*)data;
				if (asio_data_.get_err() < 0){
					water.sig();
					return;
				}
				recv_data = *(recv_bin_data*)data;
				water.sig();
			}));
		}
		
		if (res < 0)
		{
			//���Ͳ��ɹ�
			loerror("agv_shell") << "failed to send read bin block file request.";
			delete[] buffer_file;
			return -1;
		}
		water.wait();
		water.reset();
		if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful)
		{
			//��������
			memcpy(buffer_file + recv_data.offset_, recv_data.data_context_.c_str(), recv_data.data_context_.size());
		}
		else{
			loerror("agv_shell") << "get timeout while wait the asio package of send read bin block file request.";
			delete[] buffer_file;
			return -1;
		}
	}

	//md5�Ƚ������ļ�
	unsigned char md5_vcu_file[16], md5_local_file[16];
	nsp::toolkit::md5<unsigned char>((unsigned char*)buffer_file, file_size_, md5_vcu_file);
	std::string local_file;
	nsp::toolkit::singleton<file_read_handler>::instance()->get_file_buffer(0, file_size_, local_file);
	nsp::toolkit::md5<unsigned char>((unsigned char*)local_file.c_str(), local_file.size(), md5_local_file);

	if (memcmp(md5_local_file, md5_vcu_file, 16) != 0){
		loerror("agv_shell") << "the target file md5 isn't compare to local file.";
		delete[] buffer_file;
		return -1;
	}
	delete[] buffer_file;
	return 0;
}

//������������
int frimware_update_task::post_reset_request(){
	//���������ź�
	motion::asio_data asio_data_;
	nsp::os::waitable_handle water(0);
	int res = -1;
	if (frimwre_type_ == CAN_CUSTOM_SOFTWARE)
	{
		res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_can_reset_request(vcu_endpoint_, node_id_,serial_code_,
			std::make_shared<motion::asio_block>([&](const void*data){
			if (!data){
				water.sig();
				return;
			}
			asio_data_ = *(motion::asio_data*)data;
			if (asio_data_.get_err() < 0){
				water.sig();
				return;
			}
			water.sig();
		}));
	}
	else
	{
		uint32_t file_size = nsp::toolkit::singleton<file_read_handler>::instance()->get_file_size();
		res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_vcu_reset_request(vcu_endpoint_, file_size,
			std::make_shared<motion::asio_block>([&](const void*data){
			if (!data){
				water.sig();
				return;
			}
			asio_data_ = *(motion::asio_data*)data;
			if (asio_data_.get_err() < 0){
				water.sig();
				return;
			}
			water.sig();
		}));
	}

	if (res < 0)
	{
		//���Ͳ��ɹ�
		loerror("agv_shell") << "failed to send restart vcu package.";
		return -1;
	}
	water.wait();
	water.reset();
	if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful){
		return 0;
	}
	else{
		loerror("agv_shell") << "receive restart vcu package ack timeout.";
		return -1;
	}
}

//���͹̼�����״̬������
void frimware_update_task::post_frimware_update(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE f_type, const FRIMWARE_STATUS status, const int step)
{
	nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_update(link, f_type, status, step);
}

//canЭ���ʼ������
int frimware_update_task::post_can_init_rom_update(const int block_number)
{
	if (block_number <= 0)
	{
		return -1;
	}

	motion::asio_data asio_data_;
	nsp::os::waitable_handle water(0);

	loinfo("agv_shell") << "can custom init ROM,the node id is:" << node_id_ << " the serial code is:" << serial_code_;

	int res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_can_init_rom_request(vcu_endpoint_, node_id_,serial_code_,
		std::make_shared<motion::asio_block>([&](const void*data){
		if (!data){
			water.sig();
			return;
		}
		asio_data_ = *(motion::asio_data*)data;
		if (asio_data_.get_err() < 0){
			water.sig();
			return;
		}

		water.sig();
	}), block_number);

	if (res < 0)
	{
		loerror("agv_shell") << "failed to post can custom ROM initlization package.";
		return -1;
	}
	water.wait();
	water.reset();
	if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful)
	{
		loinfo("agv_shell") << "receive can custom init ROM update package.";
		return 0;
	}
	else{
		loerror("agv_shell") << "failed to receive can custom init ROM,the pkt error is:" << asio_data_.get_err();
		return -1;
	}
}

/*---------------------------------------------------------------------------------------------------------------------------------*/

void frimware_get_info_task::on_task(){
	nsp::tcpip::endpoint ep;
	if (nsp::tcpip::endpoint::build(target_ep_, ep) < 0)
	{
		loerror("agv_shell") << "failed to build target endpoint "<< target_ep_;
		return;
	}

	version_control_ = nsp::toolkit::singleton<udp_client_manager>::instance()->get_version_control();
	
	//��δ����ɹ�������£���3���������
	std::string msg;
	bool error = true;
	//��canЭ���ȡ����������Ϣʱֻ��ȡ�汾��Ϣ,�ͺ���Ϣ,������Ϣһ�Ų���Ҫ��ȡ
	if (frimwre_type_ == CAN_CUSTOM_SOFTWARE)
	{
		//����ת������
		for (int i = 0; i < 3; i++)
		{
			if (start_data_foward(ep) == 0)
			{
				error = false;
				break;
			}
			error = true;
		}
		/*nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_update(link_, 
			frimwre_type_, error ? FRIMWARE_STATUS::kFailStartForward : FRIMWARE_STATUS::kNormal, 0);*/

		//��ȡ�汾��Ϣ
		for (int i = 0; i < 3; i++){
			if (get_vcu_version(ep) == 0){
				msg = vcu_version_;
				error = false;
				break;
			}
			error = true;
		}
		nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_info(link_, frimwre_type_, error ?
			FRIMWARE_STATUS::kFailReadVCUInfo : FRIMWARE_STATUS::kNormal, VCU_MESSAGE::VCU_Version, msg);

		//��ȡ�ͺ���Ϣ
		for (int i = 0; i < 3; i++){
			if (get_vcu_type(ep) == 0){
				msg = vcu_type_;
				error = false;
				break;
			}
			error = true;
		}
		nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_info(link_, frimwre_type_, error ?
			FRIMWARE_STATUS::kFailReadVCUInfo : FRIMWARE_STATUS::kNormal, VCU_MESSAGE::VCU_Type, msg);
	}
	else
	{
		for (int i = 0; i < 3; i++){
			if (get_vcu_type(ep) == 0){
				msg = vcu_type_;
				error = false;
				break;
			}
			error = true;
		}
		nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_info(link_, frimwre_type_, error ?
			FRIMWARE_STATUS::kFailReadVCUInfo : FRIMWARE_STATUS::kNormal, VCU_MESSAGE::VCU_Type, msg);


		for (int i = 0; i < 3; i++){
			if (get_vcu_version(ep) == 0){
				msg = vcu_version_;
				error = false;
				break;
			}
			error = true;
		}
		nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_info(link_, frimwre_type_, error ?
			FRIMWARE_STATUS::kFailReadVCUInfo : FRIMWARE_STATUS::kNormal, VCU_MESSAGE::VCU_Version, msg);

		for (int i = 0; i < 3; i++){
			if (get_vcu_cpu(ep) == 0){
				msg = cpu_info_;
				error = false;
				break;
			}
			error = true;
		}
		nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_info(link_, frimwre_type_, error ?
			FRIMWARE_STATUS::kFailReadVCUInfo : FRIMWARE_STATUS::kNormal, VCU_MESSAGE::VCU_CPU, msg);
	}

}

//���ͻ�ȡVCU��������
int frimware_get_info_task::get_vcu_type(const nsp::tcpip::endpoint& ep){
	
	motion::asio_data asio_data_;
	nsp::os::waitable_handle water(0);
	recv_vcu_data recv_data;
	int res;
	if (frimwre_type_ == CAN_CUSTOM_SOFTWARE)
	{
		res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_can_type_request(ep, node_id_, serial_code_,
			std::make_shared<motion::asio_block>([&](const void*data){
			if (!data){
				water.sig();
				return;
			}
			asio_data_ = *(motion::asio_data*)data;
			if (asio_data_.get_err() < 0){
				water.sig();
				return;
			}
			recv_data = *(recv_vcu_data*)data;
			water.sig();
		}));
	}
	else
	{
		res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_vcu_type_request(ep, 
			std::make_shared<motion::asio_block>([&](const void*data){
			if (!data){
				water.sig();
				return;
			}
			asio_data_ = *(motion::asio_data*)data;
			if (asio_data_.get_err() < 0){
				water.sig();
				return;
			}
			recv_data = *(recv_vcu_data*)data;
			water.sig();
		}));
	}
	if (res < 0)
	{
		loerror("agv_shell") << "failed to post vct type request.";
		return -1;
	}
	water.wait();
	water.reset();
	if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful)
	{
		if (frimwre_type_ == CAN_CUSTOM_SOFTWARE)
		{
			int nLength = recv_data.data_length_;
			nsp::proto::udp::can_data_t can_data;
			can_data.build((unsigned char *)recv_data.data_context_.data(), nLength);
			vcu_type_ = can_data.data_;
			loinfo("agv_shell") << "receive can custom driver type reponse.";
		}
		else
		{
			loinfo("agv_shell") << "receive vcu type reponse.";
			vcu_type_ = recv_data.data_context_;
		}
		return 0;
	}
	else{
		loerror("agv_shell") << "get an error while post vcu type request,the package error:" << asio_data_.get_err();
		return -1;
	}
}

//canЭ��������ת������
int frimware_get_info_task::start_data_foward(const nsp::tcpip::endpoint& ep)
{
	//���ͻ�ȡVCU��������
	motion::asio_data asio_data_;
	nsp::os::waitable_handle water(0);
	int res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_can_data_forward(ep, serial_code_,
		std::make_shared<motion::asio_block>([&](const void*data){
		if (!data){
			water.sig();
			return;
		}
		asio_data_ = *(motion::asio_data*)data;
		if (asio_data_.get_err() < 0){
			water.sig();
			return;
		}
		water.sig();
	}));
	if (res < 0)
	{
		loerror("agv_shell") << "failed to post start can bus forward data.";
		return -1;
	}
	water.wait();
	water.reset();
	
	if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful)
	{
		loinfo("agv_shell") << "receive start can forward reponse.";
		return 0;
	}
	else{
		loerror("agv_shell") << "get an error while post start_data_foward request,the package error:" << asio_data_.get_err();
		return -1;
	}
}

//�����ͻ�ȡVCU�̼��汾��Ϣ
int frimware_get_info_task::get_vcu_version(const nsp::tcpip::endpoint& ep){
	
	motion::asio_data asio_data_;
	nsp::os::waitable_handle water(0);
	recv_vcu_data recv_data;
	int res = -1;
	if (frimwre_type_ == CAN_CUSTOM_SOFTWARE)
	{
		res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_can_version_request(ep,node_id_,serial_code_,
			std::make_shared<motion::asio_block>([&](const void*data){
			if (!data){
				water.sig();
				return;
			}
			asio_data_ = *(motion::asio_data*)data;
			if (asio_data_.get_err() < 0){
				water.sig();
				return;
			}
			recv_data = *(recv_vcu_data*)data;
			water.sig();
		}));
	}
	else
	{
		res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_vcu_version_request(frimwre_type_, ep,
			version_control_, std::make_shared<motion::asio_block>([&](const void*data){
			if (!data){
				water.sig();
				return;
			}
			asio_data_ = *(motion::asio_data*)data;
			if (asio_data_.get_err() < 0){
				water.sig();
				return;
			}
			recv_data = *(recv_vcu_data*)data;
			water.sig();
		}));
	}

	if (res < 0)
	{
		loerror("agv_shell") << "failed to post get vcu version request.";
		return -1;
	}
	water.wait();
	water.reset();

	if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful)
	{
		if (frimwre_type_ == CAN_CUSTOM_SOFTWARE)
		{
			int nLength = recv_data.data_length_;
			nsp::proto::udp::can_data_t can_data;
			can_data.build((unsigned char *)recv_data.data_context_.data(), nLength);
			vcu_version_ = can_data.data_;
			loinfo("agv_shell") << "receive can custom driver version reponse.";
		}
		else
		{
			loinfo("agv_shell") << "receive vcu version reponse.";
			vcu_version_ = recv_data.data_context_;
		}
		return 0;
	}
	else{
		loerror("agv_shell") << "get an error while post get vcu version request,the package error:" << asio_data_.get_err();
		return -1;
	}
}

//�����ͻ�ȡCPU�汾��Ϣ
int frimware_get_info_task::get_vcu_cpu(const nsp::tcpip::endpoint& ep){
	
	motion::asio_data asio_data_;
	nsp::os::waitable_handle water(0);
	recv_vcu_data recv_data;
	int res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_cpu_type_requst(ep, std::make_shared<motion::asio_block>([&](const void*data){
		if (!data){
			water.sig();
			return;
		}
		asio_data_ = *(motion::asio_data*)data;
		if (asio_data_.get_err() < 0){
			water.sig();
			return;
		}
		recv_data = *(recv_vcu_data*)data;
		water.sig();
	}));
	if (res < 0){
		loerror("agv_shell") << "failed to post get cpu info request.";
		return -1;
	}
	water.wait();
	water.reset();
	if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful) {
		loinfo("agv_shell") << "receive cpu information reponse.";
		cpu_info_ = recv_data.data_context_;
		return 0;
	}
	else {
		loerror("agv_shell") << "get an error while post get cpu info request,the package error:" << asio_data_.get_err();
		return -1;
	}
}

/*--------------------------------------------------------------------------------------------------------------------------------*/

void frimware_download_task::on_task(){
	uint64_t block_number = 1;
	if (frimware_length_ != 0){
		block_number = (frimware_length_ % FRIMWARE_FILE_BLOCK_SIZE == 0) ? (frimware_length_ / FRIMWARE_FILE_BLOCK_SIZE) : (frimware_length_ / FRIMWARE_FILE_BLOCK_SIZE + 1);
	}
	
	if (nsp::toolkit::singleton<udp_client_manager>::instance()->init_network() < 0){
		return;
	}
	nsp::tcpip::endpoint vcu_endpoint;
	if (nsp::tcpip::endpoint::build(target_ep_, vcu_endpoint) < 0)
	{
		loerror("agv_shell") << "failed to build target endpoint " << target_ep_;
		return;
	}
		
	loinfo("agv_shell") << "start to download bin file request.";
	current_status_ = download_bin_file_from_vcu(vcu_endpoint, block_number) < 0 ? FRIMWARE_STATUS::kFailDownload : FRIMWARE_STATUS::kNormal;

	nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_update(link_, frimware_type_, current_status_, current_step_);

	//���agv_shell���سɹ�����ͨ��ftsת�����ͻ���
	if (current_status_ == FRIMWARE_STATUS::kNormal){
		nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_download(link_, file_name_);
	}
}

int frimware_download_task::download_bin_file_from_vcu(const nsp::tcpip::endpoint& vcu_endpoint_, const int block_number)
{
	file_write_handler write_handler;
	if (write_handler.create_file(file_name_, frimware_length_) < 0){
		return -1;
	}

	for (int i = 0; i < block_number; i++){
		current_step_ = (double(i + 1) / block_number) * 100;
		motion::asio_data asio_data_;
		nsp::os::waitable_handle water(0);
		recv_bin_data recv_data;
		//���һ���������ݿ��С��һ������һ��block�飬���Դ˴���Ҫ�ֿ�����
		int res = -1;
		if (i != (block_number - 1)){
			res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_vcu_read_bin_file(frimware_type_, vcu_endpoint_, i*FRIMWARE_FILE_BLOCK_SIZE, FRIMWARE_FILE_BLOCK_SIZE,
				std::make_shared<motion::asio_block>([&](const void*data){
				if (!data){
					water.sig();
					return;
				}
				asio_data_ = *(motion::asio_data*)data;
				if (asio_data_.get_err() < 0){
					water.sig();
					return;
				}
				recv_data = *(recv_bin_data*)data;
				water.sig();
			}));
		}
		else{
			res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_vcu_read_bin_file(frimware_type_, vcu_endpoint_, i*FRIMWARE_FILE_BLOCK_SIZE,
				frimware_length_ - i * FRIMWARE_FILE_BLOCK_SIZE, std::make_shared<motion::asio_block>([&](const void*data){
				if (!data){
					water.sig();
					return;
				}
				asio_data_ = *(motion::asio_data*)data;
				if (asio_data_.get_err() < 0){
					water.sig();
					return;
				}
				recv_data = *(recv_bin_data*)data;
				water.sig();
			}));
		}
		if (res < 0)
		{
			loerror("agv_shell") << "failed to post download bin file request.";
			//���Ͳ��ɹ�
			write_handler.close_file();
			return -1;
		}
		water.wait();
		water.reset();
		if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful)
		{
			//�ж�д���Ƿ�ɹ�������ɹ�����ص�����ҳ�棬���ɹ���ֱ�ӷ���
			if (write_handler.write_file(file_name_, recv_data.offset_, recv_data.data_context_) < 0){
				loerror("agv_shell") << "failed to write download bin file block.";
				write_handler.close_file();
				return -1;
			}
			current_status_ = FRIMWARE_STATUS::kBusy;
			nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_update(link_, frimware_type_, current_status_, current_step_);
		}
		else{
			loerror("agv_shell") << "send download bin file block from vcu is timeout.";
			//�������û���յ�VCU�Ļذ�֡���ݣ���ֱ�ӷ��أ����ٷ�������İ�
			write_handler.close_file();
			return -1;
		}
	}
	write_handler.close_file();
	return 0;
}

/*--------------------------------------------------------------------------------------------------------------------------------*/
void frimware_restart_task::on_task(){
	if (nsp::tcpip::endpoint::build(target_ep_, vcu_endpoint_) < 0)
	{
		loerror("agv_shell") << "failed to build target endpoint " << target_ep_;
		return;
	}

	uint32_t reset_timeout_ = nsp::toolkit::singleton<udp_client_manager>::instance()->get_reset_wait_time();

	if (post_reset_request() == 0){
		//��ǰ״̬��λ
		current_status_ = FRIMWARE_STATUS::kRestarting;
		post_frimware_update(link_, frimware_type_, current_status_, 0);
		//�����ȴ����ȴ�VCU���������ȡ�汾��
		reset_wait_.wait(reset_timeout_);
		//�ٴλ�ȡ��Ϣ
		nsp::toolkit::singleton<agv_shell_server>::instance()->on_get_frimware_info(link_, frimware_type_, target_ep_);
		//��ȡ��Ϣ���
		current_status_ = FRIMWARE_STATUS::kNormal;
	}
	else{
		current_status_ = FRIMWARE_STATUS::kFailRestart;
	}
	post_frimware_update(link_, frimware_type_, current_status_, 100);
}

int frimware_restart_task::post_reset_request(){
	//���������ź�
	uint32_t file_size = 0;// nsp::toolkit::singleton<file_read_handler>::instance()->get_file_size();
	motion::asio_data asio_data_;
	nsp::os::waitable_handle water(0);
	int res = nsp::toolkit::singleton<udp_client_manager>::instance()->post_vcu_reset_request(vcu_endpoint_, file_size,
		std::make_shared<motion::asio_block>([&](const void*data){
		if (!data){
			water.sig();
			return;
		}
		asio_data_ = *(motion::asio_data*)data;
		if (asio_data_.get_err() < 0){
			water.sig();
			return;
		}
		water.sig();
	}));

	if (res < 0)
	{
		//���Ͳ��ɹ�
		loerror("agv_shell") << "failed to send restart vcu package.";
		return -1;
	}
	water.wait();
	water.reset();
	if (asio_data_.get_err() == nsp::proto::errorno_t::kSuccessful)
	{
		return 0;
	}
	else
	{
		loerror("agv_shell") << "receive restart vcu package ack timeout.";
		return -1;
	}
}

//���͹̼�����״̬������
void frimware_restart_task::post_frimware_update(const uint32_t link, const FIRMWARE_SOFTWARE_TYPE f_type, const FRIMWARE_STATUS status, const int step)
{
	nsp::toolkit::singleton<agv_shell_server>::instance()->post_frimware_update(link, f_type, status, step);
}
