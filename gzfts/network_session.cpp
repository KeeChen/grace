#include "os_util.hpp"
#include "network_session.h"
#include "proto_definion.h"
#include "proto_file_block.h"
#include "receiver_manager.h"
#include "proto_msg.h"
#include "proto_file_block_ack.h"
#include "receiver_manager.h"
#include "log.h"
#include "file_manager.h"
#include "sender_manager.h"
#include "receiver_write_manage.h"

network_session::network_session() : nsp::tcpip::tcp_application_client<nsp::proto::nspdef::protocol>(){}

network_session::~network_session()
{
}

network_session::network_session(HTCPLINK lnk) : nsp::tcpip::tcp_application_client<nsp::proto::nspdef::protocol>(lnk)
{

}

uint64_t network_session::get_interval()
{
	if (refresh_tick_ == 0){
		return 0;
	}
	return nsp::os::gettick() - refresh_tick_;
}

uint32_t network_session::get_link(){
	return this->lnk_;
}

void network_session::on_disconnected(const HTCPLINK previous)
{
	//�رն�д�ļ����
	nsp::toolkit::singleton<file_manager>::instance()->remove_rw_file_handler(previous);
	//�鿴��ǰд����ļ��Ƿ�д��ɣ����δд��ɣ���ɾ�����ļ�
	nsp::toolkit::singleton<receiver_manager>::instance()->is_file_complete(write_file_path_, file_total_size_);
	//ɾ��sender��session����
	nsp::toolkit::singleton<sender_manager>::instance()->delete_map_link_session(previous);
	//ɾ��receiver��д�ļ�����
	nsp::toolkit::singleton<receiver_write_manage>::instance()->remove_write_file(previous);
#ifndef _WIN32
	//linux ���������д�ļ�֮������ʱ��
	if(!write_file_path_.empty())
	{
		nsp::toolkit::singleton<receiver_manager>::instance()->reset_file_time(write_file_path_, create_time_, modify_time_, access_time_);
	}
#endif
}

void network_session::on_recvdata(const std::string &pkt)
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


int  network_session::dispatch_package(int package_type, const std::string &pkt_data, const std::shared_ptr<nsp::proto::proto_head> &head)
{
	//��¼��ǰʱ���
	refresh_tick_ = nsp::os::gettick();

	int retval = -1;
	switch (package_type) {
	case PKTYPE_GET_CATALOG_INFO_REQUEST:
		retval = recv_get_catalog_info(nsp::proto::shared_for<nsp::proto::_proto_request_file>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_GET_CATALOG_INFO_REQUEST_ACK:
		retval = recv_get_listdir_ack(nsp::proto::shared_for<nsp::proto::proto_msg>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_PUSH_FILE_HEAD_REQUEST:
		retval = push_recv_file_head_msg(nsp::proto::shared_for<nsp::proto::proto_file_head_msg_t>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_PUSH_FILE_HEAD_REQUEST_ACK:
		retval = push_file_head_msg_ack(nsp::proto::shared_for<nsp::proto::proto_head>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_PUSH_FILE_BLOCK_REQUEST:
		retval = push_recv_write_file_block(nsp::proto::shared_for<nsp::proto::proto_file_block_t>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_PUSH_FILE_BLOCK_REQUEST_ACK:
		retval = push_recv_file_block_ack(nsp::proto::shared_for<nsp::proto::proto_file_block_ack_t>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_READ_FILE_HEAD_REQUEST:
		retval = recv_requst_file(nsp::proto::shared_for<nsp::proto::_proto_request_file>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_READ_FILE_HEAD_REQUEST_ACK:
		retval = recv_fts_file_head(nsp::proto::shared_for<nsp::proto::proto_write_head>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_READ_FILE_BLOCK_REQUEST:
		retval = recv_fts_requst_file_data(nsp::proto::shared_for<nsp::proto::proto_request_file_data>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_READ_FILE_BLOCK_REQUEST_ACK:
		retval = recv_fts_requst_file_data_ack(nsp::proto::shared_for<nsp::proto::proto_file_data>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_DELETE_FILE:
		retval = recv_fts_delete_files(nsp::proto::shared_for<nsp::proto::proto_file_delete>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_DELETE_FILE_ACK:
		retval = recv_fts_delete_files_ack(head);
		break;
	case PKTYPE_RW_FILE_STATUS:
		retval = recv_fts_file_complete(nsp::proto::shared_for<nsp::proto::_proto_identify>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_TRANSMIT_STATUS:
		retval = recv_transmit_timout(head);
		break;
	case PKTYPE_GET_CURRENT_CATALOG_REQUEST:
		retval = recv_get_current_catalog_info(head->id_, nsp::proto::shared_for<nsp::proto::_proto_request_file>(pkt_data.c_str(), pkt_data.length()));
		break;
	case PKTYPE_GET_CURRENT_CATALOG_REQUEST_ACK:
		{
			std::shared_ptr<nsp::proto::proto_file_info_list> share = nsp::proto::shared_for<nsp::proto::proto_file_info_list>(pkt_data.c_str(), pkt_data.length());
			retval = recv_current_catalog_reponse(head, pkt_data.substr(nsp::proto::proto_head::type_length()));
		}
		break;
	default:
		retval = -1;
		break;
	}
	return retval;
}

int network_session::push_file_to_remote(const std::string &path, const nsp::file::file_info& f_info, const int overwrite)
{
	//��������
	if (!push_task_){
		try{
			push_task_ = std::make_shared<sender_push_task>(this->lnk_);
		}
		catch (...){
			loerror("fts") << "failed to create push file task.";
			return -1;
		}
	}
	//���ò�����push������������
	push_task_->set_file_info(f_info.file_size_, f_info.file_block_num_, block_size_pre_transfer_);

	//�������
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::proto_file_head_msg_t pkt(PKTYPE_PUSH_FILE_HEAD_REQUEST, pkt_id);
	pkt.path_ = path;
	pkt.total_size_ = f_info.file_size_;
	pkt.total_blocks_ = f_info.file_block_num_;
	pkt.is_overwrite_ = overwrite;
	pkt.file_access_time_ = f_info.file_access_time_;
	pkt.file_create_time_ = f_info.file_create_time_;
	pkt.file_modify_time_ = f_info.file_modify_time_;

	return psend(&pkt);
}

//push������sender �����յ�ͷ�ļ���Ϣ֮�󣬷��͵�һ֡����
int network_session::push_file_head_msg_ack(const std::shared_ptr<nsp::proto::proto_head> &pkt)
{
	if (!pkt) {
		return -1;
	}
	//�жϰ��д����ʶλ
	if (pkt->err_<0) {
		loerror("fts") << "[push file head reponse]failed push file to server,server maybe can not create files.";
		return -1;
	}
	if (!push_task_){
		return -1;
	}
	//���͵�һ֡���ݣ���offsetΪ0
	return push_task_->push_file_block(0);
}

//push������ sender���ڶ�ȡһƬ������֮���͸��Զ�
int network_session::push_file_block_stream(const uint64_t offset, const std::string& data){
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();

	nsp::proto::proto_file_block_t pkt_ack(PKTYPE_PUSH_FILE_BLOCK_REQUEST, pkt_id);
	pkt_ack.block_data_.assign(data.c_str(), data.size());
	pkt_ack.offset_ = offset;

	return psend(&pkt_ack);
}

//push������receive session�յ�ͷ�ļ���Ϣ������ͬ�ȴ�С�ļ�
int network_session::push_recv_file_head_msg(const std::shared_ptr<nsp::proto::proto_file_head_msg_t> &pkt)
{
	if (!pkt) {
		loerror("fts") << "[push file head request]the package is invaild.";
		return -1;
	}

	nsp::file::file_head f_head;
	f_head.path_remote = pkt->path_;
	f_head.total_size = pkt->total_size_;
	f_head.is_overwrite = pkt->is_overwrite_;

	f_head.file_access_time_ = pkt->file_access_time_;
	f_head.file_create_time_ = pkt->file_create_time_;
	f_head.file_modify_time_ = pkt->file_modify_time_;

#ifndef _WIN32
	create_time_ = pkt->file_create_time_;
	modify_time_ = pkt->file_modify_time_;
	access_time_ = pkt->file_access_time_;
#endif

	write_file_path_ = f_head.path_remote;
	file_total_size_ = f_head.total_size;

	loinfo("fts") << "[push file head request]get a file head info:" << write_file_path_;

	//�鿴��ǰ������ǰ����ļ��Ƿ����ڱ�д�����������д��������˴�д���ļ�����
	uint32_t previous_link;//����overwriteʱǿ�ƶϿ�ǰ�������
	if (!nsp::toolkit::singleton<receiver_write_manage>::instance()->search_write_file(f_head.path_remote, previous_link)){
		nsp::toolkit::singleton<receiver_write_manage>::instance()->add_write_file(this->lnk_, f_head.path_remote);
	}
	else{
		lowarn("fts") << "[push file head request]current file " << f_head.path_remote << " is writting.";
		//���overwrite�ֶ�Ϊǿ�Ƹ��ǣ���ǰ����ļ����ʹ���Ͽ�
		if (f_head.is_overwrite == 0)return -1;
		//����ǿ�Ƹ���
		else nsp::toolkit::singleton<receiver_manager>::instance()->fts_overwrite_file(previous_link, f_head.path_remote);
	}

	int error_code = 0;
	int res = nsp::toolkit::singleton<receiver_manager>::instance()->fts_create_file_head(this->lnk_, f_head, error_code);
	if (res < 0){
		loerror("fts") << "[push file head request]failed to create file on receive local.";
		return post_push_write_file_status(nsp::proto::file_status::write_error, error_code);
	}
	
	if (pkt->total_size_ == 0){
		lowarn("fts") << "[push file head request]file total size is 0,the remote file is " << write_file_path_;
		//�ļ���д�꣬�����������֡
		return post_push_write_file_status(nsp::proto::file_status::complete, error_code);
	}

	nsp::proto::proto_head pkt_ack(PKTYPE_PUSH_FILE_HEAD_REQUEST_ACK, pkt->head_.id_);
	pkt_ack.err_ = res;
	return psend(&pkt_ack);
}

//push������receiver session���յ�������֮�󣬿����������ڴ���
int network_session::push_recv_write_file_block(const std::shared_ptr<nsp::proto::proto_file_block_t> &pkt)
{
	if (!pkt) {
		loerror("fts") << "[push file block reponse]the package of data stream is invaild.";
		return -1;
	}

	if (pkt->head_.err_<0) {
		return -1;
	}

	if (nsp::toolkit::singleton<receiver_manager>::instance()->fts_push_save_data(this->lnk_, pkt->offset_, pkt->block_data_) < 0){
		loerror("fts") << "[push file block reponse]failed to write file stream into memory.";
		return -1;
	}
	return 0;
}

//push������receive session�˷��ͻ�ȡ��һ֡��������
int network_session::post_push_next_block_request(const uint64_t offset)
{
	//���������һ֡offset�����ļ��ܴ�С����˵����ʱ�ļ���д��ɣ��������״̬�ذ�
	if (offset == file_total_size_){
		return post_push_write_file_status(nsp::proto::file_status::complete, 0);
	}
	else{
		uint32_t id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
		nsp::proto::proto_file_block_ack_t pkt_ack(PKTYPE_PUSH_FILE_BLOCK_REQUEST_ACK, id);
		pkt_ack.offset_ = offset;
		return psend(&pkt_ack);
	}
}

//push������sender �յ���һ֡����Ӧ�����������һ֡����
int network_session::push_recv_file_block_ack(const std::shared_ptr<nsp::proto::proto_file_block_ack_t> &pkt){
	if (!pkt) {
		return -1;
	}

	if (pkt->head_.err_<0) {
		loerror("fts") << "failed push file to server,server maybe can not input files block stream.";
		return -1;
	}

	if (!push_task_){
		return -1;
	}
	//������һ֡���ݣ���������ʼoffset�Ӱ��л�ȡ
	return push_task_->push_file_block(pkt->offset_);
}

int network_session::post_get_listdir(const std::string&input_catalog, const std::shared_ptr<asio_block>&asio_ack){
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_request_file pkt(PKTYPE_GET_CATALOG_INFO_REQUEST, pkt_id);
	pkt.file_path_ = input_catalog;
	return nsp::toolkit::singleton<net_manager>::instance()->write(pkt_id, [&]()->int{
		return psend(&pkt); 
	}, asio_ack);
}

int network_session::recv_get_catalog_info(const std::shared_ptr<nsp::proto::_proto_request_file> &pkt)
{
	std::string str;
	int error = nsp::toolkit::singleton<receiver_manager>::instance()->get_fts_listdir(pkt->file_path_, str);
	nsp::proto::proto_msg msg(PKTYPE_GET_CATALOG_INFO_REQUEST_ACK, pkt->head_.id_);
	msg.head_.err_ = error;
	msg.ls_dir_ = str;
	return psend(&msg);
}

int network_session::recv_get_current_catalog_info(const int pkt_id, const std::shared_ptr<nsp::proto::_proto_request_file>& data)
{
	nsp::toolkit::singleton<receiver_manager>::instance()->fts_get_current_catalog(data->file_path_, this->lnk_, pkt_id);
	return 0;
}

int network_session::recv_get_listdir_ack(const std::shared_ptr<nsp::proto::proto_msg> &head){
	nsp::file::list_dir_info list_dir;
	list_dir.set_err((nsp::proto::errorno_t)(int)head->head_.err_);
	list_dir.list_data_ = head->ls_dir_;
	return nsp::toolkit::singleton<net_manager>::instance()->exec(head->head_.id_, (char *)&list_dir);
}

int network_session::recv_current_catalog_reponse(const std::shared_ptr<nsp::proto::proto_head>& head_t, const std::string& data_str)
{
	nsp::file::list_dir_info list_dir;
	if (head_t->err_ != 0)
	{
		list_dir.set_err(nsp::proto::errorno_t::kUnsuccessful);
	}
	else
	{
		list_dir.set_err(nsp::proto::errorno_t::kSuccessful);
	}
	list_dir.list_data_.assign(data_str.c_str(), data_str.size());
	return nsp::toolkit::singleton<net_manager>::instance()->exec(head_t->id_, (char *)&list_dir);
}

//pull ���������ͻ�ȡĳ���ļ���Ϣ����
int network_session::post_pull_requst_file(const std::string& path_remote, const std::string& path_local, const int overwrite)
{
	is_overwrite_ = overwrite;
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::proto_request_file req_file(PKTYPE_READ_FILE_HEAD_REQUEST, pkt_id);
	req_file.file_path_ = path_remote;
	write_file_path_ = path_local;
	return psend(&req_file);
}

//pull������receive session�յ���ȡ�ļ�����
int network_session::recv_requst_file(const std::shared_ptr<nsp::proto::_proto_request_file>&pkt)
{
	if (!pkt) {
		return -1;
	}

	nsp::file::file_info f_head;
	int error_code = 0;
	int res = nsp::toolkit::singleton<receiver_manager>::instance()->fts_pull_get_file_info(this->lnk_, pkt->file_path_, f_head, error_code, false);
	if (res < 0){
		loinfo("fts") << "[pull file head request]failed to read file head info.";
		//֪ͨsender�ˣ���ȡ�ļ�ʧ�ܣ�
		post_pull_read_file_error(nsp::proto::file_status::read_error, error_code);
		return 0;
	}

	nsp::proto::proto_write_head pkt_head(PKTYPE_READ_FILE_HEAD_REQUEST_ACK, pkt->head_.id_);
	pkt_head.file_name_ = pkt->file_path_;
	pkt_head.total_size_ = f_head.file_size_;
	pkt_head.head_.err_ = res;
	pkt_head.file_create_time_ = f_head.file_create_time_;
	pkt_head.file_access_time_ = f_head.file_access_time_;
	pkt_head.file_modify_time_ = f_head.file_modify_time_;

	return psend(&pkt_head);

}

//pull������sender session�յ��ļ�ͷ��Ϣ
int network_session::recv_fts_file_head(const std::shared_ptr<nsp::proto::proto_write_head> &data)
{
	if (!data){
		return -1;
	}

	if (data->head_.err_<0) {
		loerror("fts") << "[pull file head reponse]failed pull file from server,server maybe read file error.";
		return -1;
	}

	nsp::file::file_head filehead;
	filehead.path_remote = write_file_path_;
	filehead.is_overwrite = is_overwrite_;
	filehead.total_size = data->total_size_;
	filehead.file_access_time_ = data->file_access_time_;
	filehead.file_create_time_ = data->file_create_time_;
	filehead.file_modify_time_ = data->file_modify_time_;
	file_total_size_ = data->total_size_;

	int error_code = 0;
	if (nsp::toolkit::singleton<sender_manager>::instance()->creat_file(this->lnk_, filehead, error_code) < 0){
		loerror("fts") << "[pull file head reponse]failed to create file " << filehead.path_remote << " in local.";
		fts_callback(nsp::proto::file_status::write_error, 0, error_code);
		return -1;
	}
	if (data->total_size_ == 0) {
		loinfo("fts") << "pull file : " << write_file_path_ << "  successfully in local.";
		//�ص�֪ͨ�ϲ�
		fts_callback(nsp::proto::file_status::complete, 100, 0);
		//�ر�����
		close();
		return 0;
	}
	//��������
	if (!pull_task_){
		try{
			pull_task_ = std::make_shared<sender_pull_task>();
		}
		catch (...){
			loinfo("fts") << "failed to create send pull task share ptr.";
			return -1;
		}
	}
	uint64_t offset; uint32_t length;
	pull_task_->set_file_info(data->total_size_, block_size_pre_transfer_);
	pull_task_->pull_block_request_info(offset, length);

	//�����ȡ��һƬ������
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::proto_request_file_data req_data(PKTYPE_READ_FILE_BLOCK_REQUEST, pkt_id);
	req_data.file_offset_ = offset;
	req_data.file_length_ = length;
	return psend(&req_data);
}

//pull������receive session�յ���ȡָ��offset�ļ�����������
int network_session::recv_fts_requst_file_data(const std::shared_ptr<nsp::proto::_proto_request_file_data> &data)
{
	if (!data) {
		return -1;
	}

	if (data->head_.err_<0) {
		loerror("fts") << "[pull file block request]the request package maybe error in the proto head error fields";
		return -1;
	}

	//ȡÿƬ������
	if (nsp::toolkit::singleton<receiver_manager>::instance()->fts_pull_get_file_block_stream(data->head_.id_,this->lnk_, data->file_offset_, data->file_length_) < 0){
		loerror("fts") << "[pull file block request]failed pull file from server,can not get file_stream on receiver local.";
		return -1;
	}
	return 0;
}

//pull����,������һ֡��������
int network_session::post_pull_file_block_response(const uint32_t pkt_id, const uint64_t offset, const std::string& data){
	nsp::proto::proto_file_data pkt_ack(PKTYPE_READ_FILE_BLOCK_REQUEST_ACK, pkt_id);
	pkt_ack.file_data_.assign(data.c_str(), data.size());
	pkt_ack.file_offset_ = offset;
	return psend(&pkt_ack);
}

//pull������sender session���յ��ļ�������
int network_session::recv_fts_requst_file_data_ack(const std::shared_ptr<nsp::proto::_proto_file_data> &data)
{
	//д��������
	nsp::file::file_block f_blcok;
	f_blcok.offset = data->file_offset_;
	f_blcok.stream = data->file_data_;

	if (nsp::toolkit::singleton<sender_manager>::instance()->write_file(this->lnk_, f_blcok) < 0){
		loinfo("fts") << "[pull file block reponse]failed to write file stream into memory.";
		return -1;
	}

	return 0;
}

int network_session::post_pull_complete(){
	loinfo("fts") << "pull file : " << write_file_path_ << "  successfully in local.";
	//���һ֡��ɣ��ذ�֪ͨreceive��
	uint32_t id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_identify req_data(PKTYPE_RW_FILE_STATUS, id);
	req_data.head_.err_ = nsp::proto::file_status::complete;
	req_data.identify_ = nsp::file::current_identify::pull_identify;
	return psend(&req_data);
}

int network_session::post_pull_next_block_request(const uint64_t offset)
{
	uint64_t offset_tmp; uint32_t read_size;
	if (pull_task_)pull_task_->pull_block_request_info(offset_tmp, read_size);
	uint32_t id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::proto_request_file_data req_data(PKTYPE_READ_FILE_BLOCK_REQUEST, id);
	req_data.file_offset_ = offset ;
	req_data.file_length_ = read_size;
	return psend(&req_data);
}

//�����ļ�������
int network_session::recv_fts_file_complete(const std::shared_ptr<nsp::proto::_proto_identify> &data)
{
	nsp::file::current_identify identify = (nsp::file::current_identify)(int)data->identify_;
	switch (data->head_.err_.value_)
	{
	case nsp::proto::file_status::complete:
		file_rw_handler(identify, 100, (nsp::proto::file_status)(int)data->head_.err_, 0);
		break;
	case nsp::proto::file_status::write_error:
		file_rw_handler(identify, 0, (nsp::proto::file_status)(int)data->head_.err_, data->error_code_);
		lowarn("fts") << "write file errror,the remote file is:" << callback_data_.path_remote << " data error is " << data->head_.err_ << " identify is :" << identify
			<< " error code is :" << data->error_code_;
		break;
	case nsp::proto::file_status::read_error:
		if (identify == nsp::file::current_identify::pull_identify){
			//�ص�֪ͨ�������
			fts_callback(-1, 0, data->error_code_);
			loinfo("fts") << "receive read file handler error package.";
			//�ر�����
			close();
		}
		break;
	default:
		loerror("fts") << "get undefine RW_FILE_STATUS package";
		return -1;
	}
	return 0;
}

void network_session::file_rw_handler(const nsp::file::current_identify identify, const int step, 
	const nsp::proto::file_status status, const int error_code){
	switch (identify)
	{
	case nsp::file::current_identify::pull_identify:
		//recevie ���յ����������رն����
		loinfo("fts") << "receive pull file is finished,so close the file handler.";
		break;
	case nsp::file::current_identify::push_identify:
		{
			//�ص�֪ͨ�������
			fts_callback(status, step, error_code);
		}
		loinfo("fts") << "push file :" << callback_data_.path_local << " successfully,so close the file handler.";
		break;
	default:
		break;
	}
	//�ر�����
	close();
}

int network_session::recv_fts_delete_files(const std::shared_ptr<nsp::proto::_proto_file_delete>&data){
	int res = nsp::toolkit::singleton<receiver_manager>::instance()->fts_delete_files(data->file_path_);
	if (res < 0){
		loerror("fts") << "failed to delete file:" << data->file_path_;
	}
	else{
		loinfo("fts") << "delete file : " << data->file_path_ << " successfully.";
	}
	nsp::proto::proto_head package(PKTYPE_DELETE_FILE_ACK, data->head_.id_);
	package.err_ = (res != 0 ? nsp::proto::errorno_t::kUnsuccessful : nsp::proto::errorno_t::kSuccessful);
	return psend(&package);
}

int network_session::recv_fts_delete_files_ack(const std::shared_ptr<nsp::proto::proto_head>&head){
	asio_data as_data;
	as_data.set_err((nsp::proto::errorno_t)(int)head->err_);
	nsp::toolkit::singleton<net_manager>::instance()->exec(head->id_, (char *)&as_data);
	return 0;
}

int network_session::post_push_write_file_status(const int status, const int error_code)
{
	//��¼��־
	if (error_code != 0)loerror("fts") << "write file :" << write_file_path_ << " error while push operation";

	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_identify req_data(PKTYPE_RW_FILE_STATUS, pkt_id);
	req_data.head_.err_ = status;
	req_data.identify_ = nsp::file::current_identify::push_identify;
	req_data.error_code_ = error_code;

	return psend(&req_data);
}

int network_session::post_pull_read_file_error(const int status, const int err_code){
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_identify req_data(PKTYPE_RW_FILE_STATUS, pkt_id);
	req_data.head_.err_ = status;
	req_data.identify_ = nsp::file::current_identify::pull_identify;
	req_data.error_code_ = err_code;

	return psend(&req_data);
}

void network_session::get_file_total_size_and_status(file_data& data)
{
	data = callback_data_;
	if (pull_task_){
		pull_task_->is_file_complete() ? data.status = nsp::proto::file_status::complete : data.status = nsp::proto::file_status::normal;
		data.total_size = pull_task_->get_file_size();
	}
	else{
		data.status = nsp::proto::file_status::normal;
	}
}

void network_session::set_remote_local_ep(const file_data f_data, const uint32_t block_pre_size)
{
	callback_data_ = f_data;
	block_size_pre_transfer_ = block_pre_size;
}

//����ɾ��Զ���ļ�
int network_session::post_delete_file_requst(const std::string&path_remote, const std::shared_ptr<asio_block>&asio_ack)
{
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_file_delete req_file(PKTYPE_DELETE_FILE, pkt_id);
	req_file.file_path_ = path_remote;

	return nsp::toolkit::singleton<net_manager>::instance()->write(pkt_id, [&]()->int{
		return psend(&req_file);
	}, asio_ack);
}

//���ʹ��䳬ʱ��
int network_session::post_transmit_timeout(){
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::proto_head package(PKTYPE_TRANSMIT_STATUS, pkt_id);
	package.err_ = transmission_timeout;
	return psend(&package);
}

int network_session::post_catalog_request(const std::string& input_folder, const std::shared_ptr<asio_block>&asio_ack)
{
	uint32_t pkt_id = nsp::toolkit::singleton<net_manager>::instance()->pkt_id();
	nsp::proto::_proto_request_file pkt(PKTYPE_GET_CURRENT_CATALOG_REQUEST, pkt_id);
	pkt.file_path_ = input_folder;
	return nsp::toolkit::singleton<net_manager>::instance()->write(pkt_id, [&]()->int{
		return psend(&pkt);
	}, asio_ack);
}

int network_session::post_catalog_reponse(const uint32_t pkt_id, const int error, const std::vector<nsp::file::file_info>& vct_info)
{
	nsp::proto::proto_file_info_list pkt(PKTYPE_GET_CURRENT_CATALOG_REQUEST_ACK, pkt_id);
	pkt.head_.err_ = error;
	for (const auto& iter : vct_info)
	{
		nsp::proto::proto_file_info f_info;
		f_info.file_name_ = iter.file_name_;
		f_info.file_size_ = iter.file_size_;
		f_info.file_attributes_ = iter.file_attri_;
		f_info.file_access_time_ = iter.file_access_time_;
		f_info.file_create_time_ = iter.file_create_time_;
		f_info.file_modify_time_ = iter.file_modify_time_;
		f_info.file_crc32_ = iter.file_crc32_;
#ifdef _WIN32
		//�ļ���Ϊ0���ļ�Ϊ1
		if (f_info.file_attributes_& FILE_ATTRIBUTE_DIRECTORY)f_info.file_type = 0;
		else f_info.file_type = 1;
#else
		f_info.file_type = iter.file_attri_;
#endif
		pkt.file_list_.push_back(f_info);
	}
	return psend(&pkt);
}

int network_session::recv_transmit_timout(const std::shared_ptr<nsp::proto::proto_head> &head){
	if (head->err_ == transmission_timeout){
		//�ص�֪ͨ�����ļ�����
		fts_callback(nsp::proto::file_status::write_error, 0, 1460);//��ʱ��ʶ
		loinfo("fts") << "receive timeout package,the local file is " << callback_data_.path_local << " and the remote file is " << callback_data_.path_remote;
	}
	//�ر�����
	close();
	return 0;
}

void network_session::fts_callback(const uint32_t status, const uint32_t step, const uint32_t error_code){
	callback_data_.status = status;
	callback_data_.step = step;
	callback_data_.error = error_code;
	nsp::toolkit::singleton<sender_manager>::instance()->fts_file_complete(callback_data_);
}