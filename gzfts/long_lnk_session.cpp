#include "long_lnk_session.h"
#include "network_manager.h"
#include "long_lnk_receiver.h"
#include "long_lnk_sender.h"
#include "file_manager.h"

long_lnk_session::long_lnk_session() : nsp::tcpip::tcp_application_client<nsp::proto::nspdef::protocol>(){}

long_lnk_session::~long_lnk_session()
{
}

long_lnk_session::long_lnk_session(HTCPLINK lnk) : nsp::tcpip::tcp_application_client<nsp::proto::nspdef::protocol>(lnk)
{

}

void long_lnk_session::on_disconnected(const HTCPLINK previous)
{
	//�رն�д�ļ����
	nsp::toolkit::singleton<file_manager>::instance()->remove_fw_long_file_handler(previous);
	//ɾ��sender��session����
	nsp::toolkit::singleton<long_lnk_sender>::instance()->delete_map_link_session(previous);
}

void long_lnk_session::on_recvdata(const std::string &pkt)
{
	std::shared_ptr<nsp::proto::proto_head> head = nsp::proto::shared_for<nsp::proto::proto_head>(pkt.c_str(), nsp::proto::proto_head::type_length());

	if (!head) {
		loerror("fts") << "ERROR kBuildFatal,can not build the package type.";
		close();
		return;
	}

	auto ty = head->type_;
	if (dispatch_package(ty, pkt, head)<0) {
		close();
	}
}

int  long_lnk_session::dispatch_package(int package_type, const std::string &pkt_data, const std::shared_ptr<nsp::proto::proto_head> &head)
{
	//��¼��ǰʱ���
	refresh_tick_ = nsp::os::gettick();

	int retval = -1;
	switch (package_type) {
	case PKTYPE_READ_FILE_LONG_LINK_REQUEST:
		retval = recv_request_file_long(nsp::proto::shared_for<nsp::proto::_proto_request_file_long_lnk>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_READ_FILE_LONG_LINK_REQUEST_ACK:
		retval = recv_fts_file_head_long(nsp::proto::shared_for<nsp::proto::_proto_write_head_long_lnk>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_READ_FILE_BLOCK_LONG_REQUEST:
		retval = recv_fts_requst_file_data(nsp::proto::shared_for<nsp::proto::_proto_long_lnk_request_file_data>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_READ_FILE_BLOCK_LONG_REQUEST_ACK:
		retval = recv_fts_requst_file_data_ack(nsp::proto::shared_for<nsp::proto::_proto_file_long_lnk_data>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_RW_FILE_LONG_LNK_STATUS:
		retval = recv_fts_file_complete(nsp::proto::shared_for<nsp::proto::_proto_long_lnk_identify>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_LONG_LINK_ALIVE:
		retval = recv_fts_long_alive();
		break;
	case PKTYPE_LONG_LINK_ALIVE_ACK:
		alive_count = 0;
		retval = 0;
		break;
	default:
		retval = -1;
		break;
	}
	return retval;
}

//////////////////////////////////////////����Ϊ�����Ӳ���//////////////////////////////////////////////////////////////
int long_lnk_session::post_pull_file_long(const std::string& path_remote, const std::string& path_local, const int overwrite, const uint64_t pull_id)
{
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_request_file_long_lnk req_file(PKTYPE_READ_FILE_LONG_LINK_REQUEST, pkt_id);
	req_file.file_path_ = path_remote;
	req_file.long_lnk_ = 1;
	req_file.pull_id_ = pull_id;
	return psend(&req_file);
}

//pull�����Ӳ�����receive session�յ���ȡ�ļ�ͷ�������
int long_lnk_session::recv_request_file_long(const std::shared_ptr<nsp::proto::_proto_request_file_long_lnk>& pkt_data)
{
	if (!pkt_data) {
		return -1;
	}

	nsp::file::file_info f_head;
	f_head.file_id_ = pkt_data->pull_id_;
	int error_code = 0;
	int res = nsp::toolkit::singleton<long_lnk_receiver>::instance()->fts_pull_get_file_info(this->lnk_, pkt_data->file_path_, f_head, error_code, true);
	if (res < 0){
		loinfo("fts") << "[pull file head request]failed to read file head info.";
		//֪ͨsender�ˣ���ȡ�ļ�ʧ�ܣ�
		post_pull_file_error(pkt_data->pull_id_, nsp::proto::file_status::read_error, error_code);
		return 0;
	}

	nsp::proto::_proto_write_head_long_lnk pkt_head(PKTYPE_READ_FILE_LONG_LINK_REQUEST_ACK, pkt_data->head_.id_);
	pkt_head.file_name_ = pkt_data->file_path_;
	pkt_head.total_size_ = f_head.file_size_;
	pkt_head.head_.err_ = res;
	pkt_head.file_create_time_ = f_head.file_create_time_;
	pkt_head.file_access_time_ = f_head.file_access_time_;
	pkt_head.file_modify_time_ = f_head.file_modify_time_;
	pkt_head.file_id_ = f_head.file_id_;

	return psend(&pkt_head);
}

//pull�����Ӳ�����sender session�յ��ļ�ͷ��Ϣ
int long_lnk_session::recv_fts_file_head_long(const std::shared_ptr<nsp::proto::_proto_write_head_long_lnk> &data)
{
	if (!data){
		return -1;
	}

	if (data->head_.err_<0) {
		loerror("fts") << "[pull file head reponse]failed pull file from server,server maybe read file error.";
		return -1;
	}

	nsp::file::file_head filehead;
	filehead.path_remote = nsp::toolkit::singleton<long_lnk_sender>::instance()->get_local_file_path(data->file_id_);
	filehead.is_overwrite = 0;
	filehead.total_size = data->total_size_;
	filehead.file_access_time_ = data->file_access_time_;
	filehead.file_create_time_ = data->file_create_time_;
	filehead.file_modify_time_ = data->file_modify_time_;
	nsp::toolkit::singleton<long_lnk_sender>::instance()->set_pull_file_size(data->file_id_, data->total_size_);

	int error_code = 0;
	if (nsp::toolkit::singleton<long_lnk_sender>::instance()->creat_file(this->lnk_, filehead, error_code, data->file_id_) < 0){
		loerror("fts") << "[pull file head reponse]failed to create file " << filehead.path_remote << " in local.";
		//�ص�֪ͨ�ϲ�
		nsp::toolkit::singleton<long_lnk_sender>::instance()->fts_long_lnk_callback(data->file_id_, nsp::proto::file_status::write_error, 0, error_code);
		//֪ͨ�Զ�дʧ��
		post_pull_file_error(data->file_id_, nsp::proto::file_status::write_error, error_code);
		return 0;
	}
	if (data->total_size_ == 0) {
		loinfo("fts") << "pull file : " << filehead.path_remote << "  successfully in local.";
		//�ص�֪ͨ�ϲ�
		nsp::toolkit::singleton<long_lnk_sender>::instance()->fts_long_lnk_callback(data->file_id_, nsp::proto::file_status::complete, 100, 0);
		//֪ͨ�Զ����
		post_pull_complete(data->file_id_);
		return 0;
	}
	//��������
	if (!pull_task_){
		try{
			pull_task_ = std::make_shared<sender_pull_long_task>();
		}
		catch (...){
			loinfo("fts") << "failed to create send pull task share ptr.";
			return -1;
		}
	}
	uint64_t offset; uint32_t length;

	pull_task_->set_file_info(data->file_id_, data->total_size_, block_size_pre_transfer_);
	pull_task_->pull_block_request_info(data->file_id_, offset, length);

	//�����ȡ��һƬ������
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_long_lnk_request_file_data req_data(PKTYPE_READ_FILE_BLOCK_LONG_REQUEST, pkt_id);
	req_data.file_id_ = data->file_id_;
	req_data.file_offset_ = offset;
	req_data.file_length_ = length;
	return psend(&req_data);
}

//pull������receive session�յ���ȡָ��offset�ļ�����������
int long_lnk_session::recv_fts_requst_file_data(const std::shared_ptr<nsp::proto::_proto_long_lnk_request_file_data> &data)
{
	if (!data) {
		return -1;
	}

	if (data->head_.err_<0) {
		loerror("fts") << "[pull file block request]the request package maybe error in the proto head error fields";
		return -1;
	}

	//ȡÿƬ������
	if (nsp::toolkit::singleton<long_lnk_receiver>::instance()->fts_pull_get_file_block_stream(data->head_.id_, this->lnk_,
		data->file_id_, data->file_offset_, data->file_length_) < 0){
		loerror("fts") << "[pull file block request]failed pull file from server,can not get file_stream on receiver local.";
		//֪ͨ�Զ˲���ʧ��
		post_pull_file_error(data->file_id_, nsp::proto::read_error, -1);
	}
	return 0;
}

int long_lnk_session::post_pull_file_error(const uint64_t file_id ,const int status, const int err_code){
	lowarn("fts") << "get an error while pull file from remote.";
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_long_lnk_identify req_data(PKTYPE_RW_FILE_LONG_LNK_STATUS, pkt_id);
	req_data.head_.err_ = status;
	req_data.file_id = file_id;
	req_data.identify_ = nsp::file::current_identify::pull_identify;
	req_data.pkt_identify_ = nsp::file::pkt_identify::sender_pkt;
	req_data.error_code_ = err_code;

	return psend(&req_data);
}

//pull����,������һ֡��������
int long_lnk_session::post_pull_file_block_response(const uint32_t pkt_id, const uint64_t file_id, const uint64_t offset, const std::string& data){
	nsp::proto::_proto_file_long_lnk_data pkt_ack(PKTYPE_READ_FILE_BLOCK_LONG_REQUEST_ACK, pkt_id);
	pkt_ack.file_id_ = file_id;
	pkt_ack.file_data_.assign(data.c_str(), data.size());
	pkt_ack.file_offset_ = offset;
	return psend(&pkt_ack);
}

//pull������sender session���յ��ļ�������
int long_lnk_session::recv_fts_requst_file_data_ack(const std::shared_ptr<nsp::proto::_proto_file_long_lnk_data> &data)
{
	//д��������
	nsp::file::file_block f_blcok;
	f_blcok.offset = data->file_offset_;
	f_blcok.stream = data->file_data_;
	f_blcok.file_id = data->file_id_;

	if (nsp::toolkit::singleton<long_lnk_sender>::instance()->write_file(this->lnk_, f_blcok) < 0){
		loinfo("fts") << "[pull file block reponse]failed to write file stream into memory.";
		//֪ͨ�Զ��ͷž��
		post_pull_file_error(data->file_id_, nsp::proto::write_error, -1);
	}
	return 0;
}

void long_lnk_session::get_file_status(const uint64_t fd, file_callabck_data& data)
{
	if (pull_task_){
		pull_task_->is_file_complete(fd) ? data.status = nsp::proto::file_status::complete : data.status = nsp::proto::file_status::normal;
	}
	else{
		data.status = nsp::proto::file_status::normal;
	}
}

int long_lnk_session::post_pull_next_block_request(const uint64_t file_id, const uint64_t offset)
{
	uint64_t offset_tmp; uint32_t read_size;
	if (pull_task_)pull_task_->pull_block_request_info(file_id, offset_tmp, read_size);
	uint32_t id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_long_lnk_request_file_data req_data(PKTYPE_READ_FILE_BLOCK_LONG_REQUEST, id);
	req_data.file_offset_ = offset;
	req_data.file_length_ = read_size;
	req_data.file_id_ = file_id;
	return psend(&req_data);
}

int long_lnk_session::post_pull_complete(const uint64_t file_id){
	//�رձ��ؾ��
	close_file_handler_and_remove_fd(this->lnk_, file_id);
	//���һ֡��ɣ��ذ�֪ͨreceive��
	uint32_t id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_long_lnk_identify req_data(PKTYPE_RW_FILE_LONG_LNK_STATUS, id);
	req_data.head_.err_ = nsp::proto::file_status::complete;
	req_data.identify_ = nsp::file::current_identify::pull_identify;
	req_data.pkt_identify_ = nsp::file::pkt_identify::receiver_pkt;
	req_data.file_id = file_id;
	return psend(&req_data);
}

//�����ļ�������
int long_lnk_session::recv_fts_file_complete(const std::shared_ptr<nsp::proto::_proto_long_lnk_identify> &data)
{
	nsp::file::current_identify identify = (nsp::file::current_identify)(int)data->identify_;
	switch (data->head_.err_.value_)
	{
	case nsp::proto::file_status::complete:
		loinfo("fts") << "receive pull file is finished";
		file_rw_handler(identify, data->pkt_identify_, 100, data->file_id, (nsp::proto::file_status)(int)data->head_.err_, 0);
		break;
	case nsp::proto::file_status::write_error:
		lowarn("fts") << "receive write file error";
		file_rw_handler(identify, data->pkt_identify_, 0, data->file_id, (nsp::proto::file_status)(int)data->head_.err_, data->error_code_);
		break;
	case nsp::proto::file_status::read_error:
		if (identify == nsp::file::current_identify::pull_identify && data->pkt_identify_ == nsp::file::pkt_identify::sender_pkt){
			//�ص�֪ͨ�������
			nsp::toolkit::singleton<long_lnk_sender>::instance()->fts_long_lnk_callback(data->file_id ,-1, 0, data->error_code_);
			loinfo("fts") << "receive read file handler error package,the error code is " << data->error_code_;
			//�رձ��ؾ��
			close_file_handler_and_remove_fd(this->lnk_, data->file_id);
		}
		break;
	default:
		loerror("fts") << "get undefine RW_FILE_STATUS package";
		return -1;
	}
	return 0;
}

void long_lnk_session::file_rw_handler(const nsp::file::current_identify identify, const int pkt_type, 
	const int step, const uint64_t file_id,
	const nsp::proto::file_status status, const int error_code){
	switch (identify)
	{
	case nsp::file::current_identify::pull_identify:
		//recevie ���յ����������رն����
		if (nsp::toolkit::singleton<long_lnk_receiver>::instance()->close_fts_file_handler(this->lnk_, file_id))
		{
			loinfo("fts") << "pull close the file handler successfully.";
		}
		else
		{
			loerror("fts") << "failed to close file hanlder.";
		}
		break;
	case nsp::file::current_identify::push_identify:

		break;
	default:
		break;
	}

}

void long_lnk_session::close_file_handler_and_remove_fd(const uint32_t lnk, const uint64_t fd)
{
	//�رձ��ؾ��
	nsp::toolkit::singleton<long_lnk_sender>::instance()->close_fts_file_handler(this->lnk_, fd);
	//�ͷ���Դ
	if (pull_task_)
	{
		pull_task_->remove_file_id(fd);
	}
	//�ͷ���Դ
	nsp::toolkit::singleton<long_lnk_sender>::instance()->remove_file_id(fd);
}


int long_lnk_session::post_alive_pkt()
{
	nsp::proto::proto_head pkt(PKTYPE_LONG_LINK_ALIVE);
	return psend(&pkt);
}

int long_lnk_session::recv_fts_long_alive()
{
	alive_count = 0;
	nsp::proto::proto_head pkt(PKTYPE_LONG_LINK_ALIVE_ACK);
	return psend(&pkt);
}