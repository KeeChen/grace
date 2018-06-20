#pragma once 

#include <iostream>
#include <string>
#include <vector>

struct opt
{
	std::string  __opt_long_name;  // ѡ������
	char		 __opt_short_name;	// ѡ�����
	std::vector<std::string>  __parameter_of_command;	// ��ѡ��Я���Ĳ���
};

struct parsed_command
{
	std::string __command;		// ��������
	std::vector<std::string>  __parameter_of_command;	// ���������ֱ�Ӳ���
	std::vector<opt>		__all_opt;  //����ѡ��
};

int prepareCmdLineArgs(const char *command, parsed_command &output); //�����ṩ�ķָ��
int start_process(const std::string& file_name, const std::string& cmd_line);

namespace cmd_parse
{
	//��������������ȥ���ַ�����ո�
	inline std::string trim_left(const std::string& source, const std::string& t = " ")
	{
		std::string src = source;
		return src.erase(0, source.find_first_not_of(t));
	}

	//��������������ȥ���ַ����ҿո�
	inline std::string trim_right(const std::string& source, const std::string& t = " ")
	{
		std::string src = source;
		return src.erase(source.find_last_not_of(t) + 1);
	}

	bool split(std::string& src, const char* separator, std::vector<std::string>& dest);
	int is_cmmand_parament(const std::vector<std::string>& vec, parsed_command &output);
	bool option_handler(const std::string& option_str, parsed_command &output);
	bool is_short_opt_exist(const char* opt_short, const parsed_command &cmd_par);
	bool is_long_opt_exist(const std::string& opt_str, const parsed_command &cmd_par);
	bool is_command_valid(const std::string& command_str);
	bool short_opt_handler(const std::string& option_str, parsed_command &output);
	bool long_opt_handler(const std::string& option_str, parsed_command &output);

}