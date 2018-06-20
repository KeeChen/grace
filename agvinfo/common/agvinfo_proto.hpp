#if !defined AGVINFO_PROTO_H
#define AGVINFO_PROTO_H

#include "proto_definion.h"

// Э�����Ͷ���
enum AgvInfoProtoType {
	kAgvInfoProto_heart_beat = 0,	// ��������� 
	kAgvInfoProto_get_agvinfo,		// �ͻ��˻�ȡagvinfo���� 
	kAgvInfoProto_set_agvinfo,		// �ͻ��˸���agvinfo���� 
	kAgvInfoProto_get_agvdetail,	// �ͻ��˻�ȡagvdetail���� 
	kAgvInfoProto_set_agvdetail,	// �ͻ��˸���agvdetail���� 
	kAgvInfoProto_update_notify,	// ��������ݸ����ˣ�֪ͨ�ͻ���
	kAgvInfoProto_heart_beat_ack	//��������
};

// �������Ͷ���
enum AgvInfoProtoError {
	kAgvInfoProto_OK = 0,	// ���� 
	kAgvInfoProto_FAILED	// �쳣 
	// �Ժ������Ҫ����
};

// agvinfo��ȡ���� 
enum AgvInfoLoadMenthod {
	kAgvInfoProto_load_local = 0,	// load agv info from local xml file�� @status field in agv_info must be AS_IGNORE
	kAgvInfoProto_load_xml,			// load agv info from xml file configure which storage on agvinfo-server
	kAgvInfoProto_load_realtime,	// load agv info from agvinfo-server, include xml file configure and realtime report
};

// agv_attribute����
struct proto_agvattribute : public nsp::proto::proto_interface {
	virtual const int length() const override {
		return name.length() + describe.length();
	}

	virtual unsigned char *serialize(unsigned char *bytes) const override {
		unsigned char *pos = bytes;
		pos = name.serialize(pos);
		pos = describe.serialize(pos);
		return pos;
	}

	virtual const unsigned char *build(const unsigned char *bytes, int &cb) override {
		const unsigned char *pos = bytes;
		pos = name.build(pos, cb);
		pos = describe.build(pos, cb);
		return pos;
	}
	nsp::proto::proto_string_t<char>  name;		// ������ 
	nsp::proto::proto_string_t<char>  describe;	// �������� 
};

// agv_detail����
struct proto_agvdetail : public nsp::proto::proto_interface {
	virtual const int length() const override {
		return vhid.length() + attrs.length();
	}

	virtual unsigned char *serialize(unsigned char *bytes) const override {
		unsigned char *pos = bytes;
		pos = vhid.serialize(pos);
		pos = attrs.serialize(pos);
		return pos;
	}

	virtual const unsigned char *build(const unsigned char *bytes, int &cb) override {
		const unsigned char *pos = bytes;
		pos = vhid.build(pos, cb);
		pos = attrs.build(pos, cb);
		return pos;
	}
	nsp::proto::proto_crt_t<uint32_t>  vhid;				// ����ID 
	nsp::proto::proto_vector_t<proto_agvattribute> attrs;	// �������� 
};

// �ͻ��˻�ȡ/����agvdetail����Э�鶨��
struct proto_req_agvdetail : public nsp::proto::proto_interface {
	virtual const int length() const override {
		return head.length() + load_menthod.length() + detail.length();
	}

	virtual unsigned char *serialize(unsigned char *bytes) const override {
		unsigned char *pos = bytes;
		pos = head.serialize(pos);
		pos = load_menthod.serialize(pos);
		pos = detail.serialize(pos);
		return pos;
	}

	virtual const unsigned char *build(const unsigned char *bytes, int &cb) override {
		const unsigned char *pos = bytes;
		pos = head.build(pos, cb);
		pos = load_menthod.build(pos, cb);
		pos = detail.build(pos, cb);
		return pos;
	}

	nsp::proto::proto_head head; // Э��ͷ 
	nsp::proto::proto_crt_t<uint8_t> load_menthod; // �������ݷ���:����/�����/ʵʱ
	proto_agvdetail detail; // agvdetail
};

// �ͻ��˻�ȡagvdetail
struct proto_get_agvdetail : public proto_req_agvdetail {
	proto_get_agvdetail() {
		head.id_ = 0;
		head.type_ = kAgvInfoProto_get_agvdetail;
		head.err_ = kAgvInfoProto_OK;
	}
};

// �ͻ��˸���agvdetail
struct proto_set_agvdetail : public proto_req_agvdetail {
	proto_set_agvdetail() {
		head.id_ = 0;
		head.type_ = kAgvInfoProto_set_agvdetail;
		head.err_ = kAgvInfoProto_OK;
	}
};

// ȫ��agv���ݶ��嶨��
struct proto_agvinfo : public nsp::proto::proto_interface {
	virtual const int length() const override {
		return vhid.length() + vhtype.length() + inet.length() + mtport.length()
			+ shport.length() + ftsport.length() + hwaddr.length() + status.length() + attrs.length();
	}

	virtual unsigned char *serialize(unsigned char *bytes) const override {
		unsigned char *pos = bytes;
		pos = vhid.serialize(pos);
		pos = vhtype.serialize(pos);
		pos = inet.serialize(pos);
		pos = mtport.serialize(pos);
		pos = shport.serialize(pos);
		pos = ftsport.serialize(pos);
		pos = hwaddr.serialize(pos);
		pos = status.serialize(pos);
		pos = attrs.serialize(pos);
		return pos;
	}

	virtual const unsigned char *build(const unsigned char *bytes, int &cb) override {
		const unsigned char *pos = bytes;
		pos = vhid.build(pos, cb);
		pos = vhtype.build(pos, cb);
		pos = inet.build(pos, cb);
		pos = mtport.build(pos, cb);
		pos = shport.build(pos, cb);
		pos = ftsport.build(pos, cb);
		pos = hwaddr.build(pos, cb);
		pos = status.build(pos, cb);
		pos = attrs.build(pos, cb);
		return pos;
	}

	nsp::proto::proto_crt_t<uint32_t> vhid;		// vehicle's id 
	nsp::proto::proto_crt_t<uint32_t> vhtype;	// type of this vehicle
	nsp::proto::proto_string_t<char>  inet;		// real/configure ipv4 address of this vehicle, like '192.168.1.101'
	nsp::proto::proto_crt_t<uint16_t> mtport;	// port of motion template --fixed(4409)
	nsp::proto::proto_crt_t<uint16_t> shport;	// port of agv shell --default(4410)
	nsp::proto::proto_crt_t<uint16_t> ftsport;	// port of agv shell fts --default(4411)
	nsp::proto::proto_string_t<char>  hwaddr;	// mac address of this vehicle, like '00:16:3e:06:6c:6c'
	nsp::proto::proto_crt_t<uint16_t> status;	// status of this vehicle
	nsp::proto::proto_vector_t<proto_agvattribute> attrs;	// �������� 
};

// ȫ��agv���ݶ��嶨��
struct proto_req_agvinfo : public nsp::proto::proto_interface {
	virtual const int length() const override {
		return head.length() + load_menthod.length() + info.length();
	}

	virtual unsigned char *serialize(unsigned char *bytes) const override {
		unsigned char *pos = bytes;
		pos = head.serialize(pos);
		pos = load_menthod.serialize(pos);
		pos = info.serialize(pos);
		return pos;
	}

	virtual const unsigned char *build(const unsigned char *bytes, int &cb) override {
		const unsigned char *pos = bytes;
		pos = head.build(pos, cb);
		pos = load_menthod.build(pos, cb);
		pos = info.build(pos, cb);
		return pos;
	}

	nsp::proto::proto_head head; // Э��ͷ 
	nsp::proto::proto_crt_t<uint8_t> load_menthod; // �������ݷ���:����/�����/ʵʱ
	nsp::proto::proto_vector_t<proto_agvinfo> info; // agvinfo
};

// �ͻ��˻�ȡȫ��agv��Ϣ 
struct proto_get_agvinfo : public proto_req_agvinfo {
	proto_get_agvinfo() {
		head.id_ = 0;
		head.type_ = kAgvInfoProto_get_agvinfo;
		head.err_ = kAgvInfoProto_OK;
	}
};

// �ͻ�������ȫ��agv��Ϣ 
struct proto_set_agvinfo : public proto_req_agvinfo {
	proto_set_agvinfo() {
		head.id_ = 0;
		head.type_ = kAgvInfoProto_set_agvinfo;
		head.err_ = kAgvInfoProto_OK;
	}
};

namespace nsp
{
	namespace proto
	{
		struct proto_heart_beat :public nsp::proto::proto_interface
		{
			proto_heart_beat(){ head_.err_ = kAgvInfoProto_OK; head_.type_ = kAgvInfoProto_heart_beat; };
			~proto_heart_beat(){};

			nsp::proto::proto_head head_;

			const int length() const {
				return head_.length();
			};

			virtual unsigned char *serialize(unsigned char *bytes) const{
				if (!bytes){
					return nullptr;
				}

				return head_.serialize(bytes);
			}

			virtual const unsigned char *build(const unsigned char *bytes, int &cb){
				if (!bytes || cb < 0){
					return nullptr;
				}

				return head_.build(bytes, cb);
			}
		};
	}
}


#endif //AGVINFO_PROTO_H
