#pragma once

#include <string>
#include <stdint.h>

typedef struct robot_mac_info{
	std::string ipv4;
	uint16_t shell_port;
	uint16_t fts_port;
	std::string mac_addr;
	uint16_t pkt_timeout_count;//���յ����ĳ�ʱ����
}robot_mac_info_t;