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
