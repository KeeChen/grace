#if !defined AGV_SHELL_PROTO_HPP
#define AGV_SHELL_PROTO_HPP

#include "proto_definion.h"
#include <memory>

namespace nsp {
	namespace proto {

		enum errorno_t :int32_t {
			kSuccessful = 0,

			kUnsuccessful = -100,
			kFailCreate,                    // �����������ʧ��
			kFailConnected,                 // ���ӷ�����ʧ��
			kInvalidParameter,				// ��������
			kInsufficientResource,			// ��Դ����
			kFatalSyscall,					// ϵͳ����
			kLogicError,					// �߼�����
			kSecurityFatal,					// ��ȫУ�����
			kRequestTimeout,				// ����ʱ
			kRuntimeError,					// C/C++����ʱ����
			kObjectError,                   // �������
			kItemNoFound,					// û���ҵ�����
			kNoConnected,					// û��TCP����
			kInvalidSzie,					// ��Ч�ĳ���
			kBuildError,                    // ���ʧ��
			kEndpointExist,					//endpoint�����Ѿ�����
			kFailDisconnect,				//�������
			kItemExist						//�����Ѿ�����
		};
	}
}


#endif

