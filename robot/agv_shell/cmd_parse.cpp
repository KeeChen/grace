#include "cmd_parse.h"

#ifdef _WIN32
#include <Windows.h>
#endif

int start_process(const std::string& file_name, const std::string& cmd_line)
{
#ifdef _WIN32
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	si.dwFlags = STARTF_USESHOWWINDOW;//ָ��wShowWindow��Ա��Ч
	si.wShowWindow = TRUE;            //�˳�Ա��ΪTRUE�Ļ�����ʾ�½����̵�������
	BOOL bRet = CreateProcessA(
		file_name.c_str(),			//���ڴ�ָ����ִ���ļ����ļ���
		(LPTSTR)cmd_line.c_str(),	//�����в���
		NULL,						//Ĭ�Ͻ��̰�ȫ��
		NULL,						//Ĭ�Ͻ��̰�ȫ��
		FALSE,						//ָ����ǰ�����ھ�������Ա��ӽ��̼̳�
		CREATE_NEW_CONSOLE,			//Ϊ�½��̴���һ���µĿ���̨����
		NULL,						//ʹ�ñ����̵Ļ�������
		NULL,						//ʹ�ñ����̵���������Ŀ¼
		&si,
		&pi);
	if (bRet)
	{
		//�رվ��
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		printf("the new process id is :%d\n", pi.dwProcessId);
		printf("the main thread id of new process is��%d\n", pi.dwThreadId);
		return 0;
	}
	return -1;
#else
	return 0;
#endif
}

/***********************************************************************************
* ����command����������ѭ���¹���
* [��������] [�����������] [-option��ѡ��] [��ѡ�����] [--option��ѡ��] [��ѡ�����]
* eg:copy	 d:\1.txt e:\2.txt	-c						--normal		
* �������壺����Ϊ���ʻ��ߵ�����ĸ����������������ַ������֣�
* ��ѡ���ѡ�������'-'��ͷ��ֻ��Ϊ��ĸ����������������ַ������֣�
* ��ѡ���ѡ������ȥ�ش��������������ͬ��ѡ����ҵ���ͬ��ѡ�������ǰѡ�
* �������������������optionѡ���������Ҫ����ȥ�أ�������ȥ�������ַ������ִ���
* ����������ǰһ��option�����ǰoption�Ѿ����ڣ���ǰ����������
************************************************************************************/
int prepareCmdLineArgs(const char *command, parsed_command &output)
{
	if (NULL == command)
	{
		return -1;
	}
	std::vector<std::string> rec;
	std::string com_str = command;
	cmd_parse::split(com_str, " ", rec);
	int opt_start_index = 0;
	if (rec.size() == 0)// || ! cmd_parse::is_command_valid(rec[0]))//rec[0].at(0) == '-')
	{
		std::cout << "Missing command main." << std::endl;
		return -1;
	}
	output.__command = rec[0];//�������壻
	opt_start_index = cmd_parse::is_cmmand_parament(rec, output);//�ж���������֮���Ƿ�����������

	//��ȡopt����
	for (auto opt_index = opt_start_index; opt_index < rec.size();){
		std::string opt_string = rec[opt_index++];
		//��һ����
		if ('-' == opt_string.at(0) ){
			bool bSuccess = false;
			if (cmd_parse::option_handler(opt_string,output)){
				bSuccess = true;
			}
			//δ����output�����option������������ eg: -abbddcc 123 234 -ef --norma2l wer
			//ccѡ���ظ���������123��234����������norma2lѡ��Ϸ���������������
			while (opt_index < rec.size() && rec[opt_index].at(0) != '-'){
				if (bSuccess){
					if (output.__all_opt.size() > 0){
						output.__all_opt[output.__all_opt.size()- 1].__parameter_of_command.push_back(rec[opt_index]);
					}
				}
				opt_index++;
			}
		}
	}
	return 0;
}

//�ж�command���������Ƿ���Ч,Ҳ������long option��ѡ����Ч�Լ��
bool cmd_parse::is_command_valid(const std::string& command_str)
{
	for (size_t i = 0; i < command_str.size(); i++)
	{
		char temp = command_str[i];
		if ((temp >= 'A' && temp <= 'Z') ||
			(temp >= 'a' && temp <= 'z'))
		{
		}
		else
		{
			return false;
		}
	}
	return true;
}

//�ж�long option�Ƿ��Ѿ����ڣ�true=�Ѿ����ڣ�false=������
bool cmd_parse::is_long_opt_exist(const std::string& opt_str, const parsed_command &cmd_par)
{
	bool result = false;
	for (size_t i = 0; i < cmd_par.__all_opt.size(); i++)
	{
		if (opt_str == cmd_par.__all_opt[i].__opt_long_name)
		{
			result = true;
			break;
		}
	}
	return result;
}

//�ж�short option�Ƿ��Ѿ����ڣ�true=�Ѿ����ڣ�false=������
bool cmd_parse::is_short_opt_exist(const char* opt_short, const parsed_command &cmd_par)
{
	if (NULL == opt_short)
		return false;
	bool result = false;
	for (size_t i = 0; i < cmd_par.__all_opt.size(); i++)
	{
		if (*opt_short == cmd_par.__all_opt[i].__opt_short_name)
		{
			result = true;
			break;
		}
	}
	return result;
}

//����optionѡ��
bool cmd_parse::option_handler(const std::string& option_str, parsed_command &output)
{
	if (option_str.size() < 2)//option�ַ������ȱ�����ڵ���2
		return false;
	//�ڶ�����
	if (option_str.at(1) == '-')
	{
		return long_opt_handler(option_str, output);
	}
	else
	{
		return short_opt_handler(option_str, output);
	}
}

bool cmd_parse::long_opt_handler(const std::string& option_str, parsed_command &output)
{
	//��ѡ�� long option
	if (option_str.size() < 3) //��ѡ���ַ������ȱ�����ڵ���3�źϷ�
	{
		return false;
	}
	std::string temp = option_str.substr(2, option_str.size() - 2);
	//�Ƿ�Ϊ���ʼ��
	if (!is_command_valid(temp))
	{
		return false;
	}
	//ȥ���Լ��
	if (!is_long_opt_exist(temp, output))
	{
		opt opt_struct;
		opt_struct.__opt_long_name = temp;
		output.__all_opt.push_back(opt_struct);
		return true;
	}
	else
	{
		return false;
	}
}

bool cmd_parse::short_opt_handler(const std::string& option_str, parsed_command &output)
{
	bool result = false;
	//��ѡ�� short option
	for (size_t opt_index = 1; opt_index < option_str.size(); opt_index++)
	{
		char opt_short = option_str.at(opt_index);
		if ((opt_short >= 'A' && opt_short <= 'Z') ||
			(opt_short >= 'a' && opt_short <= 'z'))
		{
			//ȥ���Լ��
			if (!is_short_opt_exist(&opt_short, output))
			{
				opt opt_struct;
				opt_struct.__opt_short_name = opt_short;
				output.__all_opt.push_back(opt_struct);
				result = true;
			}
			else
			{
				result = false;
			}
		}
	}
	return result;
}

int cmd_parse::is_cmmand_parament(const std::vector<std::string>& vec, parsed_command &output)
{
	int index = 1;
	for (size_t i = 1; i < vec.size(); i++)
	{
		if (vec[i].at(0) != '-')
		{
			output.__parameter_of_command.push_back(vec[i]); //���ṹ������������������
			index = i + 1;
		}
		else
		{
			break;
		}
	}
	return index;
}

bool cmd_parse::split(std::string& src, const char* separator, std::vector<std::string>& dest)
{
	if (NULL == separator)
		return false;
	//��ȥ�����ҿո�
	std::string str_temp = trim_left(trim_right(src));
	char last_char = str_temp.at(str_temp.length() - 1);
	if (last_char != *separator)
	{
		str_temp += separator;
	}

	std::string::size_type index = str_temp.find_first_of(separator, 0);
	size_t start = 0;
	while (index != str_temp.npos)
	{
		std::string temp = str_temp.substr(start, index - start);
		start = index + 1;
		index = str_temp.find_first_of(separator, start);
		if (temp.size() != 0)
		{
			dest.push_back(temp);
		}
	}
	return true;
}