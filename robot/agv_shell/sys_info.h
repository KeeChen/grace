#ifndef SYS_INFO_20180103
#define SYS_INFO_20180103

#include <string>

enum NET_ADAPTER_TYPE{
	NCF_Unkown = 0,
	NCF_VIRTUAL = 0x01,				//����Ǹ�����������
	NCF_SOFTWARE_ENUMERATED = 0x02,	//�����һ�����ģ���������
	NCF_PHYSICAL = 0x4,				//�����һ������������
	NCF_HIDDEN  = 0x08,             //�������ʾ�û��ӿ�
	NCF_NO_SERVICE = 0x10,			//���û����صķ���(�豸��������)
	NCF_NOT_USER_REMOVABLE = 0x20,	//���ܱ��û�ɾ��(���磬ͨ�����������豸������)
	NCF_MULTIPORT_INSTANCED_ADAPTER = 0x40//˵������ж���˿�
};

class sys_info
{
public:
	sys_info();
	~sys_info();

public:
	std::string get_mac_addr();

private:
	bool is_virtual_adapter(const char *pAdapterName, NET_ADAPTER_TYPE& ty);
	
};

#endif
