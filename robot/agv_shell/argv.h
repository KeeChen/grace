#pragma once

#include <stdio.h>
#include "getopt.h"
#include <string>
#include <stdint.h>

struct parament_run{
	std::string ipv4_;
	uint16_t shell_port_;
	uint16_t fts_port_;
	uint16_t fts_long_port_;
};

/* ���ɼ��̲����������� */
enum opt_invisible_indx {
	kInvisibleOptIndex_GetHelp = 'h',
	kInvisibleOptIndex_GetVersion = 'v',
	kInvisibleOptIndex_StartProcess = 's',
	kInvisibleOptIndex_ShellPort = 0x301,
	kInvisibleOptIndex_FtsPort,
	kInvisibleOptIndex_FtsLongPort,
	kInvisibleOptIndex_Default
};

static const struct option long_options[] = {
	{ "help", no_argument, NULL, kInvisibleOptIndex_GetHelp },
	{ "version", no_argument, NULL, kInvisibleOptIndex_GetVersion },
	{ "start-process", no_argument, NULL, kInvisibleOptIndex_StartProcess },
	{ "shell-port", required_argument, NULL, kInvisibleOptIndex_ShellPort },
	{ "fts-port", required_argument, NULL, kInvisibleOptIndex_FtsPort },
	{ "fts-long-port", required_argument, NULL, kInvisibleOptIndex_FtsLongPort },
	{ "default", no_argument, NULL, kInvisibleOptIndex_Default },
	{ NULL, 0, NULL, 0 }
};

int check_argv(int argc, char** argv);
