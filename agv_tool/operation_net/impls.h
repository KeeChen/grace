#pragma once

#include <stdint.h>

#ifdef _USRDLL
#define METH dllexport
#else
#define METH dllimport
#endif


#ifdef WIN32
#define EXP(type) extern "C" _declspec (METH) type __stdcall
#else
#define EXP(type) extern "C"
#endif

//�ļ�״̬
enum file_status{
	network_error = -4,				//�������
	file_not_exist = -3,			//�ļ�������
	file_read_err = -2,				//��ʧ��
	file_write_err = -1,			//дʧ��
	transmitting = 0,				//������
	complete = 1,					//���
	wait = 2						//���ϴ�
};

enum transfer_block_size {
	block_4kb = 1,					//���С
	block_8kb,
	block_16kb,
	block_32kb,
	block_64kb
};

//�ϴ��ļ�ʱ����������Ȼص��Ľṹ��
struct task_group_info{
	uint64_t	task_id_;
	char		target_ep_[24];
	uint32_t	task_status_;
	uint32_t	task_step_;
};

//�ϴ��ļ�ʱ�����������ļ�������ϸ��Ϣ
struct task_file_detail_info {
	char file_name_[512];
	char file_modify_time_[32];
	uint64_t file_size_;
	uint32_t file_status_;
	uint32_t file_step_;
};

enum FIRMWARE_SOFTWARE_TYPE
{
	BOOT_SOFTWARE = 0,
	MAIN_SOFTWARE,
	USER_SOFTWARE,
	CAMERA_SOFTWARE,
	COMPARE_SOFTWARE,		//�����ʾ��ȡ�ո��·����ļ�
	CAN_CUSTOM_SOFTWARE		//canЭ����������
};

enum VCU_MESSAGE
{
	VCU_Type = 0,
	VCU_Version,
	VCU_CPU
};

//�����ͨ�ŵ�type����ö��
enum Interface_Package{
	Proto_Agv_Process_List,				//��ȡagv_shell���̱�
	Proto_Agv_Process_Status,			//��ȡagv_shell����״̬
	Proto_A_Core_Info,					//����A����Ϣ��ϵͳʱ�䣬VCUʹ��״̬��
	Proto_UpdateNTPServer,				//����NTP�����ַ
	Proto_RunScript,					//ִ�нű�
	Proto_Cmd_List,						//��ȡ���̹�����ϸ��Ϣ
	Proto_UpdateCmd,					//����agv_shell��������������
	Proto_VCUEnable,					//ʹ��VCU���ư�
	Proto_GetDHCPList,					//��ȡDHCP�б�
	Proto_NotifyDHCP,					//DHCP����֪ͨ
	Proto_ModifyMutex,					//�޸��ļ���״̬
	Proto_Update_Progress,				//���ȸ���
	Proto_BackupDemarcate				//�޸ı궨�ļ�ʱ���ݸ��ļ�
};

//agv_shell��������ȡM����Ϣ״̬
enum FRIMWARE_STATUS 
{
	kNormal = 0,			//����
	kBusy,					//������
	kRestarting,			//��������
	kTranfmiting,			//�����·��ļ�
	kTranfmited,			//�·��ļ��ɹ�

	kFailReadVCUInfo = -100, //��ȡVCU��Ϣʧ��
	kFailRestart,			//����ʧ��
	kFailUpdate,			//����ʧ��
	kFailDownload,			//�����ļ�ʧ��
	kFailCompare,			//�ļ��ȶ�ʧ��
	kRequestTimeout,		//����ʱ
	kFailStartForward,		//canЭ������ת��ʧ��
	kFailCanInitRom			//canЭ���³�ʼ��ROMʧ��
};

enum ProcessStatus
{
	kStop=0,
	kRunning=1
};

enum file_type
{
	folder = 0,				//Ϊ�ļ���
	file =1					//Ϊ�ļ�
};

//��ö������ҵ�������fts������������
enum Fts_Function_Type
{
	TY_PULL_FILE,			//���ļ�
	TY_PUSH_FILE,			//���ļ�
	TY_KEY_TO_UPGRADE,		//һ������
	TY_M_CORE_FILE,			//����M���ļ�
	TY_KEY_BACKUP			//һ������
};

//��ʼ������
//��һ���ص������в���Ϊrobot_id
//�ڶ����ص������в���һ��robot_id,��������״̬(0:������-1�Ѿ�����)����������������
EXP(int)  init_net(void(__stdcall *callback_link_closed)(int32_t), 
	void(__stdcall *callback_link_connect)(int32_t, int32_t, const char* host_name),
	const char* ip_port);

//��������
EXP(int)  connect_host(uint32_t robot_id, const char* str_ep);

//�Ͽ�����
EXP(int)  disconnect(uint32_t robot_id);

//��ȡĿ��˵�ǰָ����Ŀ¼�����е��ļ���
EXP(int)  get_listdir(const char* ip_port, const char* data_stream, void** output_stream, int& output_cb);

//��ȡ��ǰһ��Ŀ¼�ṹ
EXP(int) get_current_catalog(const char* ft_endpoint, const char* input_catalog, char** data_ptr, int* cb);

//ע�������ļ����Ȼص�����
EXP(int)  regist_pull_callback(void(__stdcall *callback_func)(const int status, const int steps));

//ע���ϴ��ļ����Ȼص�����
EXP(int)  regist_push_callback(void(__stdcall *callback_func)(const char* chr, const int cb));

//��ȡ�ļ�push�ϴ���ϸ�б�
EXP(int) get_push_files_detail(const uint64_t task_id, void** data_ptr, int& cb);

//�ͷ��ڴ�
EXP(void) free_detail(void* ptr);

//�ϴ��ļ���Զ���ļ�����
EXP(int)  push_files_to_remote(const char* ip_port, const char* push_files, const int push_cb );

//��ȡ�ļ����ļ���crcУ����
EXP(int)  get_file_crc(const char* input_file, unsigned int* crc_value);

//ɾ��Զ���ļ�
EXP(int) delete_remote_files(const char* ip_port, const char* str_files, const int cb);

//����pushʧ�ܵ��ļ�����Ҫ���·���
EXP(int) repeat_push_files(const uint64_t, const char* str_files, const int cb);

//�ָ�ָ�������˳�������
EXP(void) restore_factory_setting(const char* ip_port, void(__stdcall * callback_step)(int32_t step, int32_t status, const char* ip));

//��ָ��Զ����ȡĳ�����ض��ļ�
EXP(int) pull_files_from_remote(const char* ip_port, const char* str_files, const int cb, const char* save_path);

//���������ļ���ָ���ļ�������
EXP(void) update_folder_config();

//���÷��Ϳ��С
EXP(void) set_block_size(const transfer_block_size block_size);

//������λ�����̲���,�������̣��������̣��������̣��������ػ�
EXP(int) post_cmd_agv_process(const char* robot_id_all, const char* process_id_all, const int cmd);

//ȡ��pull����
EXP(int) cancel_pull_opreate();

//�����ȡM�˹̼���������ͺţ��汾��cpu��Ϣ �����������ȡcanЭ�������������źţ��汾��Ϣ
EXP(int) get_m_camera_info_request(const char* robot_id_all,const FIRMWARE_SOFTWARE_TYPE frimware_type, const char* target_endpoint, 
	const int node_id, const int can_bus);

//ע���ȡM�˹̼������Ϣ�Ļص�����
EXP(void) regist_m_core_info_callback(void(__stdcall * function)(const char* ip, const VCU_MESSAGE vcu_type, const char* msg));

//����M���ļ�������ļ�
EXP(int) push_m_camera_file(const char* ip_port, const FIRMWARE_SOFTWARE_TYPE type, const char* file_path, const char* target_endpoint);

//ע�� ����/���� M���ļ�������ļ�ʱ���ļ��Ĵ���״̬���������
EXP(void) regist_m_core_transfer_callback(void(__stdcall* function)(const char* ip, const FRIMWARE_STATUS frimware_type, const int step));

//��������M��
EXP(int) restart_m_core(const char* robot_id_all,const FIRMWARE_SOFTWARE_TYPE frimware_type, const char* target_endpoint);

//�����ȡM���ļ�,�����ڵ�ǰĿ¼�µ�m_core�ļ�����
EXP(int) pull_m_camera_file(const char* robot_id_all, const FIRMWARE_SOFTWARE_TYPE frimware_type, const int frimware_length,
	const char* local_path, const char* target_endpoint);

//����can ���������̼��ļ�
EXP(int) push_can_bin_file(const char* ip_port, const uint32_t node_id, const uint32_t serial_type, const char* file_path, const char* target_endpoint);

//ע����̻ص�
EXP(void) regist_process_status_callback(void(__stdcall* function)(const char* endpoint, ProcessStatus status));

//����AGVShell,���ʵ��һ�����¹���
EXP(int) update_agv_shell(const char* robot_id_list, const char* shell_path);

//ע���������ݰ��ص�
EXP(void) regist_netowrk_callback(void(__stdcall *callback_func)(const char* robot_ip, const int type, const int error, const char* str, const int cb));

//��ȡ����״̬ӳ���
EXP(void) get_agv_process_table();

//�첽���������ֽ���
EXP(int) post_async_package(const int robot_id, const int type, const void* str, const int cb);

//ͬ�����������ֽ���
EXP(int) post_sync_package(const int robot_id, const int type, const void* str, const int cb, void **_ack_msg, int *_ack_len);

//�Ƿ��DHCP����
EXP(int) is_open_dhcp(bool status);

//�첽���������ֽ���,��ʹ��id
EXP(int) post_sync_without_id(const int type, const void* str, const int cb, void **_ack_msg, int *_ack_len);

//һ�����ݳ�����Ҫ���ݵ������ļ�
EXP(int) post_backup_file(const void* str, const int cb);
