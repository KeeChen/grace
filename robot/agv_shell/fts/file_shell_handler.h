#pragma once

/*****************************************************************
*file name:file_shell_handler.h
*function���������ڴ���windows�ļ�����lnk��ݷ�ʽ��Ѱ�����ڲ�lnk�ļ���������·��
*author:Yeoman
*create time:20170915 13:46:00
*****************************************************************/

#include <string>

class file_shell_handler{
public:
	file_shell_handler();
	~file_shell_handler();

public:
	/*****************************************************************************
		���ܣ��ҳ�lnk�ļ�����ָ���Ŀ��·��
		@input_folder:Դlnk�ļ�������·��;
		@output_folder:Ŀ��lnk�ļ�������ָ����ļ���
		@return value: true = success find,
					   false = can not find it.
	*****************************************************************************/
	int find_lnk_folder(const std::string& input_folder, std::string& output_folder);

	/****************************************************************************
		���ܣ��ҳ�lnk�ļ����ڵ�Ŀ���ļ��У����ļ�����
		@input_file:Դlnk�ļ�·����
		@output_file:Ŀ��lnk�ļ����ڵ��ļ��У����ļ�����
		@return value: true = success find,
					   false = can not find it.
	****************************************************************************/
	int find_lnk_file(const std::string& input_file, std::string& output_file);
};