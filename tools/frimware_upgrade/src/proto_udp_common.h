#ifndef PROTO_UDP_COMMON_H_20170717
#define PROTO_UDP_COMMON_H_20170717

#include "proto_head_definition.h"

namespace nsp{
	namespace proto{
		namespace udp{
			//���ֽڶ���
		#pragma pack(push, 1)
			typedef struct __common_data : public nsp::proto::proto_interface{
				nsp::proto::udp::udp_head head_;
				short int data_len_ = 0;//˫�ֽڴ洢���ݳ���
				std::string data_;//���ݿ�

				__common_data(unsigned char op, unsigned short int sequence_id) :head_(sequence_id, 0x00, 0x06, op, 0x00, 0){}
				__common_data(unsigned char op, unsigned char sub_op, unsigned short int sequence_id) :head_(sequence_id, 0x00, 0x06, op, sub_op, 0){}
				__common_data(){}

				const int length()const
				{
					return head_.length() + sizeof(data_len_) + data_len_*sizeof(unsigned char);
				}

				unsigned char *serialize(unsigned char*bytes)const
				{
					if (!bytes)return nullptr;
					bytes = head_.serialize(bytes);
					memcpy_s(bytes, sizeof(data_len_), &data_len_, sizeof(data_len_));
					bytes += sizeof(data_len_);
					for (size_t i = 0; i < data_.size(); i++){
						memcpy_s(bytes, sizeof(unsigned char), &data_.at(i), sizeof(unsigned char));
						bytes += sizeof(unsigned char);
					}
					return bytes;
				}

				const unsigned char *build(const unsigned char *bytes, int &cb)
				{
					if (!bytes || cb < length())return nullptr;
					memcpy_s(&data_len_, sizeof(data_len_), bytes, sizeof(data_len_));
					bytes += sizeof(data_len_);
					cb -= sizeof(data_len_);
					data_.assign((char*)bytes,cb);
					return bytes;
				}

				void set_sub_operate(uint8_t value){
					head_.sub_operate_ = value;
				}

			}common_data;
		#pragma pack(pop)


			class unpackage{
			private:
				const unsigned char* buffer_ = nullptr;
				const unsigned char* pos_ = nullptr;
				nsp::proto::udp::udp_ack_head pkt_head_;
				int cb_;

				int build_head(){
					if (cb_ < pkt_head_.length()){
						return -1;
					}
					pos_ = pkt_head_.build(pos_, cb_);
					if (!pos_){
						return -1;
					}
					return 0;
				}

			public:
				unpackage(const std::string& pkt){
					buffer_ = (unsigned char*)pkt.c_str();
					pos_ = buffer_;
					cb_ = pkt.length();
					build_head();
				}
				~unpackage(){};

				template<class T>
				std::shared_ptr<T> shared_for() {
					try {
						std::shared_ptr<T> s_ptr = std::make_shared<T>();
						if (s_ptr->build(pos_, cb_) < 0) {
							return std::shared_ptr<T>(nullptr);
						}
						return s_ptr;
					}
					catch (std::bad_alloc &) {
						return std::shared_ptr<T>(nullptr);
					}
				}

				uint8_t ack_type(){
					return pkt_head_.ack_;
				}

				uint16_t pkt_id(){
					return pkt_head_.sequence_id_;
				}

				uint8_t main_operate(){
					return pkt_head_.operate_;
				}

				uint8_t sub_operate(){
					return pkt_head_.sub_operate_;
				}
			};


#pragma pack(push, 1)
			typedef struct __recv_data{
				short int data_len_ = 0;//˫�ֽڴ洢���ݳ���
				std::string data_;//���ݿ�

				const int length()const
				{
					return  sizeof(data_len_) + data_len_*sizeof(data_);
				}

				unsigned char *serialize(unsigned char*bytes)const
				{
					if (!bytes)return nullptr;
					memcpy_s(bytes, sizeof(data_len_), &data_len_, sizeof(data_len_));
					bytes += sizeof(data_len_);
					for (size_t i = 0; i < data_len_; i++){
						if (data_.at(i)){
							memcpy_s(bytes, sizeof(unsigned char), &data_.at(i), 1);
							bytes += sizeof(unsigned char);
						}
					}
					return bytes;
				}

				const unsigned char *build(const unsigned char *bytes, int &cb)
				{
					if (!bytes || cb < length())return nullptr;
					memcpy_s(&data_len_, sizeof(data_len_), bytes, sizeof(data_len_));
					bytes += sizeof(data_len_);
					cb -= sizeof(data_len_);
					data_.assign((char*)bytes, cb);
					bytes += sizeof(char)*data_.size();
					cb -= data_.size();
					return bytes;
				}

			}recv_data;
#pragma pack(pop)

		}
	}
}

#endif