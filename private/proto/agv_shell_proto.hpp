#if !defined AGV_SHELL_PROTO_HPP
#define AGV_SHELL_PROTO_HPP

#include "proto_definion.h"
#include "serialize.hpp"
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

		struct agv_shell_proto_head : public nsp::proto::proto_interface {
			nsp::proto::proto_crt_t<uint8_t> op = 0x0C;
			nsp::proto::proto_crt_t<uint8_t> fn = 'C';
			nsp::proto::proto_crt_t<uint8_t> ln = 'K';
			nsp::proto::proto_crt_t<uint8_t> type;

			agv_shell_proto_head() {}
			agv_shell_proto_head(int ty) {
				type = ty;
			}
			~agv_shell_proto_head() {}

			virtual const int length() const override  {
				return op.length() + fn.length() + ln.length() + type.length();
			}

			static const int type_length()
			{
				return
					decltype(op)::type_length() +
					decltype(fn)::type_length() +
					decltype(ln)::type_length() +
					decltype(type)::type_length();
			}

			virtual unsigned char *serialize(unsigned char *bytes) const override  {
				unsigned char *pos = op.serialize(bytes);
				pos = fn.serialize(pos);
				pos = ln.serialize(pos);
				pos = type.serialize(pos);
				return pos;
			}

			virtual const unsigned char *build(const unsigned char *bytes, int &cb) override  {
				const unsigned char *pos = op.build(bytes, cb);
				pos = fn.build(pos, cb);
				pos = ln.build(pos, cb);
				pos = type.build(pos, cb);
				return pos;
			}
		};
	}
}


#endif

