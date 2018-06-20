#include "tcp_manager.h"
#include "argvs.h"
#include "common_data.h"
#include "libagvinfo.h"
#include "errno.h"
#include "stdlib.h"

#include "rapidxml_iterators.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_utils.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

extern parament_run p_run;

tcp_manager::tcp_manager()
{
	initlizatin();
}

tcp_manager::~tcp_manager()
{
	uinit();
}

int tcp_manager::tcp_listen(const std::string& ipv4, const uint16_t port)
{
	if (server_ptr_)
	{
		return -1;
	}
	try
	{
		server_ptr_ = std::make_shared<nsp::tcpip::tcp_application_service<tcp_session>>();
	}
	catch (...)
	{
		return -1;
	}
	if (server_ptr_)
	{
		nsp::tcpip::endpoint ep;
		ep.ipv4(ipv4);
		ep.port(port);
		if (server_ptr_->begin(ep) < 0)
		{
			loerror(MODULE_NAME) << "failed to begin tcp listen:" << ep.to_string();
			return -1;
		}
		loinfo(MODULE_NAME) << "success to begin tcp listen:" << ep.to_string();
		return 0;
	}
	return -1;
}

void tcp_manager::initlizatin()
{
	need_notify = 0;
	// ��ʼ��crc32У���� 
	crc32_xml_new = crc32_xml_old = get_xml_crc32();
	// ��һ�ζ�ȡxml���ڴ��� 
	load_agvinfo_xml();
	try
	{
		check_th_ = new std::thread(std::bind(&tcp_manager::check_thread, this));
	}
	catch (...)
	{
		loerror(MODULE_NAME) << "failed to start check timeout thread.";
	}
}

void tcp_manager::uinit()
{
	exit_th_ = true;
	wait_check_.sig();
	if (check_th_)
	{
		if (check_th_->joinable())
		{
			check_th_->join();
		}
		delete check_th_;
		check_th_ = nullptr;
	}
	wait_check_.reset();
}

uint32_t tcp_manager::get_xml_crc32()
{
	uint32_t ret = 0;
	std::string file_path = nsp::os::get_module_directory<char>();
	std::string dock_path = file_path + POSIX__DIR_SYMBOL + AGVINFO_FILE_NAME;
	try {
		// ��дС����Ϣ֮ǰ����
		std::lock_guard<std::recursive_mutex> guard(r_mutex_agvinfo);
		std::basic_ifstream<char> file;
		std::vector<char> data;
		
		file.open(dock_path, std::ios::binary);
		if (!file.fail()) {
			file.seekg(0, std::ios::end);
			uint32_t size = (uint32_t)file.tellg();
			file.seekg(0);
			data.resize(size + 1);
			file.read(&data.front(), size);
			ret = nsp::toolkit::crc32(0, (const unsigned char *)&data[0], size);
		}
	}
	catch (...) {
		loerror(MODULE_NAME) << "failed to get xml's crc32";
	}

	return ret;
}

void tcp_manager::check_thread()
{
	uint32_t check_count = 0;

	while (wait_check_.wait(1000)) //ÿ�εȴ�1��(1000����)
	{
		if (exit_th_)break;
		check_count++;
		// ���xml�Ƿ����(ÿp_run.periods����һ��) 
		if (0 == (check_count % p_run.periods)) {
			nsp::toolkit::singleton<tcp_manager>::instance()->check_xml_update();
		}
		// �����������Ƿ񽡿�(ÿ30����һ��) 
		if (0 == (check_count % 30)) {
			nsp::toolkit::singleton<tcp_manager>::instance()->check_heart_beat();
		}
		// ����Ƿ���Ҫ֪ͨ�ͻ������ݸ���(ÿ2����һ��) 
		// ���С���ܶ�ͬʱ���߿��Ա���һ����֪ͨ�ܶ�� 
		if (0 == (check_count % 2)) {
			if (nsp::toolkit::singleton<tcp_manager>::instance()->check_notify_agvinfo_update()) {
				// ֪ͨ�ͻ��� 
				nsp::toolkit::singleton<tcp_manager>::instance()->post_notify_agvinfo_update();
			}
		}
	}
}

void tcp_manager::check_xml_update()
{
	int ret = 0;
	// ��дС����Ϣ֮ǰ����
	std::lock_guard<std::recursive_mutex> guard(r_mutex_agvinfo);
	crc32_xml_new = get_xml_crc32();
	if (crc32_xml_new != crc32_xml_old) {
		// ��xml��ȡ���µ��ڴ��� 
		ret = load_agvinfo_xml();
		if (ret == 0) {
			// ����xml�ɹ���֪ͨ���пͻ����и��� 
			need_notify = 1;
			crc32_xml_old = crc32_xml_new;
		}
	}
}

void tcp_manager::check_heart_beat()
{
	if (server_ptr_) {
		server_ptr_->notify_all([&](const std::shared_ptr<tcp_session>&client){
			if (client->check_heart_beat() < 0)
			{
				loerror(MODULE_NAME) << "failed to check heart beat,the client is endpoint is :" << client->remote().to_string();
				client->close();
			}
		});
	}
}

void tcp_manager::set_notify_agvinfo_update()
{
	// ��дС����Ϣ֮ǰ����
	std::lock_guard<std::recursive_mutex> guard(r_mutex_agvinfo);
	need_notify = 1;
}

int tcp_manager::check_notify_agvinfo_update()
{
	return need_notify;
}

void tcp_manager::post_notify_agvinfo_update()
{
	if (server_ptr_) {
		server_ptr_->notify_all([&](const std::shared_ptr<tcp_session>&client){
			if (client->post_notify_agvinfo_update() < 0)
			{
				loerror(MODULE_NAME) << "failed to noitfy agvinfo update,the client is endpoint is :" << client->remote().to_string();
				client->close();
			}
		});
		// ��дС����Ϣ֮ǰ����
		std::lock_guard<std::recursive_mutex> guard(r_mutex_agvinfo);
		if (need_notify) {
			need_notify = 0;
		}
	}
}

int tcp_manager::load_agvinfo_xml()
{
	std::string file_path = nsp::os::get_module_directory<char>();
	std::string dock_path = file_path + POSIX__DIR_SYMBOL + AGVINFO_FILE_NAME;
	rapidxml::xml_document<> doc;
	rapidxml::file<char> *file = nullptr;
	rapidxml::xml_node<>* node_agvs = nullptr;
	loinfo(MODULE_NAME) << "loadXml " << dock_path;
	unsigned long ul = 0;
	char *endptr = NULL;

	try {
		// ��дС����Ϣ֮ǰ����
		std::lock_guard<std::recursive_mutex> guard(r_mutex_agvinfo);

		file = new rapidxml::file<char>(dock_path.c_str());
		doc.parse<0>(file->data());
		// ���ڵ�agv_info 
		rapidxml::xml_node<>* root = doc.first_node("agv_info");
		if (root) {
			for (node_agvs = root->first_node(); node_agvs; node_agvs = node_agvs->next_sibling()) {
				if (strcmp(node_agvs->name(), "agvs") == 0) {
					// �ҵ�agvs�ڵ��� 
					break;
				}
			}
			// ��ȡǰ����� 
			map_agvinfo tmp_info;
			map_xml_agvinfo.swap(tmp_info);
			// ��ȡagvs�ڵ����� 
			if (node_agvs) {
				for (auto anode = node_agvs->first_node(); anode != NULL; anode = anode->next_sibling()){
					if (strcmp(anode->name(), "agv") != 0){
						// ����agv�ڵ�����ݺ���
						continue;
					}
					proto_agvinfo tmp_agvinfo;
					tmp_agvinfo.vhid = INVALID_VHID;
					tmp_agvinfo.mtport = DEFAULT_MTPORT;
					tmp_agvinfo.shport = DEFAULT_SHPORT;
					tmp_agvinfo.ftsport = DEFAULT_FTSPORT;
					for (auto anattr = anode->first_attribute(); anattr != NULL; anattr = anattr->next_attribute()) {
						errno = 0;
						if (strcmp(anattr->name(), "id") == 0) {
							ul = strtoul(anattr->value(), &endptr, 10);
							if (errno || (endptr && *endptr != '\0')) {
								// �д������(Ϊȱʡֵ)
							} else {
								tmp_agvinfo.vhid = ul;
							}
						} else if (strcmp(anattr->name(), "type") == 0) {
							tmp_agvinfo.vhtype = (uint32_t)atoi(anattr->value());
						} else if (strcmp(anattr->name(), "ip") == 0) {
							tmp_agvinfo.inet = anattr->value();
						} else if (strcmp(anattr->name(), "port") == 0) {
							ul = strtoul(anattr->value(), &endptr, 10);
							if (errno || ul < 1 || ul > 65535 || (endptr && *endptr != '\0')) {
								// �д������(Ϊȱʡֵ)
							} else {
								tmp_agvinfo.mtport = (uint16_t)ul;
							}
						} else if (strcmp(anattr->name(), "shell_port") == 0) {
							ul = strtoul(anattr->value(), &endptr, 10);
							if (errno || ul < 1 || ul > 65535 || (endptr && *endptr != '\0')) {
								// �д������(Ϊȱʡֵ)
							}
							else {
								tmp_agvinfo.shport = (uint16_t)ul;
							}
						} else if (strcmp(anattr->name(), "fts_port") == 0) {
							ul = strtoul(anattr->value(), &endptr, 10);
							if (errno || ul < 1 || ul > 65535 || (endptr && *endptr != '\0')) {
								// �д������(Ϊȱʡֵ)
							} else {
								tmp_agvinfo.ftsport = (uint16_t)ul;
							}
						} else if (strcmp(anattr->name(), "mac_addr") == 0) {
							tmp_agvinfo.hwaddr = anattr->value();
							std::transform(tmp_agvinfo.hwaddr.begin(), tmp_agvinfo.hwaddr.end(), 
								tmp_agvinfo.hwaddr.begin(), ::toupper);
						}
						else {
							// ������ϸ�������� 
							proto_agvattribute tmp_agvattr;
							tmp_agvattr.name = anattr->name();
							tmp_agvattr.describe = anattr->value();
							tmp_agvinfo.attrs.push_back(std::move(tmp_agvattr));
						}
					}
					tmp_agvinfo.status = AS_OFFLINE;
					map_xml_agvinfo.insert(std::make_pair(tmp_agvinfo.hwaddr, std::move(tmp_agvinfo)));
				}
			}
		}
	}
	catch (...)
	{
		lowarn(MODULE_NAME) << "loadXml err";
	}
	loinfo(MODULE_NAME) << "loadXml end";
	if (file) {
		delete file;
	}
	return 0;
}

int tcp_manager::save_agvinfo_xml()
{
	std::string file_path = nsp::os::get_module_directory<char>();
	std::string dock_path = file_path + POSIX__DIR_SYMBOL + AGVINFO_FILE_NAME;
	rapidxml::xml_document<> doc;
	rapidxml::file<char> *file = nullptr;
	rapidxml::xml_node<>* root = nullptr;
	rapidxml::xml_node<>* node_agvs = nullptr;
	loinfo(MODULE_NAME) << "saveXml " << dock_path;

	try {
		// ��дС����Ϣ֮ǰ����
		std::lock_guard<std::recursive_mutex> guard(r_mutex_agvinfo);

		try {
			file = new rapidxml::file<char>(dock_path.c_str());
			doc.parse<0>(file->data());
		} catch (...) {
			if (file) {
				delete file;
			}
			loinfo(MODULE_NAME) << "saveXml new file." << dock_path;
			// �ļ���ȡʧ�ܣ����Դ���һ���µ�xml�ļ� 
			std::ofstream new_doc(dock_path);
			new_doc << "";
			new_doc.close();
			file = new rapidxml::file<char>(dock_path.c_str());
		}
		// ���ڵ�agv_info 
		root = doc.first_node("agv_info");
		if (!root) {
			// û��agv_info�ڵ�Ļ�������һ���µ���Ϊ���ڵ� 
			root = doc.allocate_node(rapidxml::node_element, "agv_info", NULL);
			doc.append_node(root);
		}
		for (node_agvs = root->first_node(); node_agvs; node_agvs = node_agvs->next_sibling()) {
			if (strcmp(node_agvs->name(), "agvs") == 0) {
				// �ҵ�agvs�ڵ��� 
				break;
			}
		}
		if (node_agvs) {
			// ɾ��agvs�ڵ�������С�������� 
			node_agvs->remove_all_nodes();
			//root->append_node();
		} else {
			// û��agvs�ڵ�Ļ�������һ���µ�agvs�ڵ� 
			node_agvs = doc.allocate_node(rapidxml::node_element, "agvs", NULL);
			root->append_node(node_agvs);
		}
		// ����д��agvs�ڵ���С��������
		for (auto iter = map_xml_agvinfo.begin(); iter != map_xml_agvinfo.end(); iter++) {
			if (iter->second.vhid == INVALID_VHID) {
				// ��ЧID��С�����ݲ�д��xml�ļ� 
				continue;
			}
			rapidxml::xml_node<>* new_agv = doc.allocate_node(rapidxml::node_element, "agv", NULL);
			std::stringstream  attr_value;
			char* pkey = NULL;
			char* pvalue = NULL;
			attr_value.str("");
			attr_value << iter->second.vhid;
			pvalue = doc.allocate_string(attr_value.str().c_str());
			new_agv->append_attribute(doc.allocate_attribute("id", pvalue));
			attr_value.str("");
			attr_value << iter->second.vhtype;
			pvalue = doc.allocate_string(attr_value.str().c_str());
			new_agv->append_attribute(doc.allocate_attribute("type", pvalue));
			pvalue = doc.allocate_string(iter->second.inet.c_str());
			new_agv->append_attribute(doc.allocate_attribute("ip", pvalue));
			attr_value.str("");
			attr_value << iter->second.mtport;
			pvalue = doc.allocate_string(attr_value.str().c_str());
			new_agv->append_attribute(doc.allocate_attribute("port", pvalue));
			attr_value.str("");
			attr_value << iter->second.shport;
			pvalue = doc.allocate_string(attr_value.str().c_str());
			new_agv->append_attribute(doc.allocate_attribute("shell_port", pvalue));
			attr_value.str("");
			attr_value << iter->second.ftsport;
			pvalue = doc.allocate_string(attr_value.str().c_str());
			new_agv->append_attribute(doc.allocate_attribute("fts_port", pvalue));
			pvalue = doc.allocate_string(iter->second.hwaddr.c_str());
			new_agv->append_attribute(doc.allocate_attribute("mac_addr", pvalue));
			node_agvs->append_node(new_agv);
			for (auto attr = iter->second.attrs.begin(); attr != iter->second.attrs.end(); attr++) {
				pkey = doc.allocate_string(attr->name.c_str());
				pvalue = doc.allocate_string(attr->describe.c_str());
				new_agv->append_attribute(doc.allocate_attribute(pkey, pvalue));
			}
		}
		// �����µ�xml�ļ�
		std::ofstream new_file(dock_path);
		new_file << doc;
		new_file.close();
		// ������xml�ļ���crc32У��ֵ
		crc32_xml_new = crc32_xml_old= get_xml_crc32();
	}
	catch (...)
	{
		loinfo(MODULE_NAME) << "saveXml err";
	}
	loinfo(MODULE_NAME) << "saveXml end";
	if (file) {
		delete file;
	}
	return 0;
}

void tcp_manager::insert_macinfo(const robot_mac_info_t& mac_info)
{
	update_macinfo(mac_info);
}

void tcp_manager::update_macinfo(const robot_mac_info_t& mac_info)
{
	try {
		// ��дС����Ϣ֮ǰ����
		std::lock_guard<std::recursive_mutex> guard(r_mutex_agvinfo);

		if (map_xml_agvinfo.count(mac_info.mac_addr) == 0) {
			loinfo(MODULE_NAME) << "insert_macinfo unknow " << mac_info.mac_addr;
			// �µĳ���������,û��������Ϣ��״̬Ϊunknow 
			proto_agvinfo agvinfo;
			agvinfo.vhid = INVALID_VHID;
			agvinfo.mtport = DEFAULT_MTPORT;
			agvinfo.hwaddr = mac_info.mac_addr;
			agvinfo.shport = mac_info.shell_port;
			agvinfo.inet = mac_info.ipv4;
			agvinfo.ftsport = mac_info.fts_port;
			agvinfo.status = AS_UNKNOWN;
			// ����һ���µĳ��� 
			map_xml_agvinfo.insert(std::make_pair(agvinfo.hwaddr, std::move(agvinfo)));
			need_notify = 1;
			return;
		}

		auto range = map_xml_agvinfo.equal_range(mac_info.mac_addr);
		int update_flag = 0;
		for (auto iter = range.first; iter != range.second; iter++) {
			if (iter->second.hwaddr == mac_info.mac_addr) {
				// ��Mac��ַƥ�����Ϣ���£�״̬Ϊonline 
				if (iter->second.shport != mac_info.shell_port) {
					iter->second.shport = mac_info.shell_port;
					update_flag = need_notify = 1;
				}
				if (iter->second.inet != mac_info.ipv4) {
					iter->second.inet = mac_info.ipv4;
					update_flag = need_notify = 1;
				}
				if (iter->second.ftsport != mac_info.fts_port) {
					iter->second.ftsport = mac_info.fts_port;
					update_flag = need_notify = 1;
				}
				if (update_flag) {
					// shell_port��ipv4��fts_port�����仯�󣬱��浽xml�ļ���(xml�ļ�������״̬status) 
					save_agvinfo_xml();
				}
				if (INVALID_VHID != iter->second.vhid && AS_ONLINE != iter->second.status) {
					iter->second.status = AS_ONLINE;
					update_flag = need_notify = 1;
				}
				if (update_flag) {
					// shell_port��ipv4��fts_port��status�����仯 
					loinfo(MODULE_NAME) << "update_macinfo online " << mac_info.mac_addr;
				}
				break;
			}
		}
	}
	catch (...)
	{
		loinfo(MODULE_NAME) << "update_macinfo err";
	}

}

void tcp_manager::delete_macinfo(const robot_mac_info_t& mac_info)
{
	try {
		// ��дС����Ϣ֮ǰ����
		std::lock_guard<std::recursive_mutex> guard(r_mutex_agvinfo);

		auto range = map_xml_agvinfo.equal_range(mac_info.mac_addr);
		int update_flag = 0;
		for (auto iter = range.first; iter != range.second; iter++) {
			if (iter->second.hwaddr == mac_info.mac_addr) {
				// ��Mac��ַƥ�����Ϣ���£�offine
				if (iter->second.status == AS_ONLINE) {
					iter->second.status = AS_OFFLINE;
					need_notify = 1;
					loinfo(MODULE_NAME) << "delete_macinfo offline " << mac_info.mac_addr;
				}
				else if (iter->second.status == AS_UNKNOWN) {
					map_xml_agvinfo.erase(iter);
					loinfo(MODULE_NAME) << "delete_macinfo unknow(erase) " << mac_info.mac_addr;
					need_notify = 1;
				}
				// else(AS_OFFLINE do nothing)
				break;
			}
		}
	}
	catch (...)
	{
		loinfo(MODULE_NAME) << "delete_macinfo err";
	}
}

map_agvinfo& tcp_manager::get_xml_agvinfo()
{
	return map_xml_agvinfo;
}

std::recursive_mutex & tcp_manager::get_mutex_agvinfo()
{
	return r_mutex_agvinfo;
}
