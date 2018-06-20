#ifndef BEZIER_TO_POINT_PANG_20161128_H_
#define BEZIER_TO_POINT_PANG_20161128_H_

#include <math.h>
#include "PosandTrack.h"
#include "DataManage.h"
#include "AlgorithmDefine.hpp"

class PT_ALG
{
public:
	static void search_wop(int id, var__way_of_pass_t& wop)
	{
		WOP wopInfo;
		DATAMNG->GetWopById(id, wopInfo);
		wop = PT_C::WP(wopInfo);
	}
	//���Ƕ�ת����0-360��
	static double angle_to_normal(double angle)
	{
		double result = angle;
		while (result >= 360.0) {
			result = result - 360.0;
		}
		while (result < 0.0) {
			result = result + 360.0;
		}
		return result;
	}
	//���Ƕ�ת����0-180��
	static double angle_to_normal180(double angle)
	{
		double result = angle_to_normal(angle);
		while (result > 180.0) {
			result = 360.0 - result;
		}
		return result;
	}
	//���Ƕ�ת����-180-180��
	static double angle_to_plus_minus(double angle)
	{
		double result = angle_to_normal(angle);
		if (result > 180.0)
		{
			result = 180 - result;
		}
		return result;
	}

	static int point_mapping_upl(const position_t& pos,UPL& upl) {


		double min_weight = 9999999;
		std::vector<EDGEINFO> vecEdgeInfo;
		DATAMNG->GetAllEdgeInfo(vecEdgeInfo);
		if (upl.edge_id != 0){
			for (auto& edgeInfo : vecEdgeInfo)//������
			{
				if (upl.edge_id == edgeInfo.id){
					double edge_weight = 9999999;
					var__edge_t e;
					PT_C::ED(edgeInfo, e);
					double t;
					GetTClosestOnLineBycurg(pos, &e, &t);
					position_t p_on_line;
					GetposbyT(t, &e, &p_on_line);
					p_on_line.angle_ *= (180 / 3.14159);//������ת��Ϊ�Ƕ�
					double per;
					GetpercentbyT(t, &e, &per);
					double dis_weight = sqrt(pow(p_on_line.x_ - pos.x_, 2) + pow(p_on_line.y_ - pos.y_, 2));
					double angle_weight = 9999999;
					int suitable_wop_id = -1;
					for (auto& wop_id : edgeInfo.wop_list)
					{
						var__way_of_pass_t wop;
						search_wop(wop_id, wop);
						double angle_err = 0.0;
						if (wop.angle_type_ == CONST_ANGLE)
						{
							double global_angle = p_on_line.angle_ + wop.angle_;//�������ת��Ϊȫ�ֽ�
							global_angle = (global_angle >= 0.0) ? ((global_angle >= 360.0) ? (global_angle - 360.0) : global_angle) : (global_angle + 360.0);//���Ƕ�ȫ��ת��Ϊ0-360��ֵ
							angle_err = fabs(pos.angle_ - global_angle);
							angle_err = (angle_err > 180.0) ? (360 - angle_err) : angle_err;//���ǶȲ�ת��������180������
						}
						else if (wop.angle_type_ == GLOBAL_CONST_ANGLE)
						{
							double global_angle = (wop.angle_ >= 0.0) ? wop.angle_ : (wop.angle_ + 360.0);
							angle_err = fabs(pos.angle_ - global_angle);
							angle_err = (angle_err > 180.0) ? (360 - angle_err) : angle_err;
						}
						if (angle_err < angle_weight)
						{
							angle_weight = angle_err;
							suitable_wop_id = wop.id_;
						}
					}

					edge_weight = dis_weight + (angle_weight / 180.0 * PI) * 0.4;//���Ƕ�ת���ɻ��ȣ�Ȼ�����Ȩ�أ�1����Ϊ0.4�ף�
					if (edge_weight < min_weight)
					{
						min_weight = edge_weight;
						upl.edge_id = edgeInfo.id;
						upl.percent = per / 100.0;//��Ҫ�����0-1�İٷֱ�
						upl.wop_id = suitable_wop_id;
						upl.aoa = angle_to_normal(pos.angle_ - p_on_line.angle_);//upl�Ĺ��Ǿ��ǵ�ĽǶȼ�ȥӳ��������ǣ�ת��Ϊ-180-180
					}
					delete[] e.disperse_points_.data_;
					e.disperse_points_.data_ = NULL;
				}
				}
				
		}
		else{
			for (auto edgeInfo : vecEdgeInfo)//������
			{
				double edge_weight = 9999999;
				var__edge_t e;
				e.disperse_points_.data_ = nullptr;
				PT_C::ED(edgeInfo, e);
				double t;
				GetTClosestOnLineBycurg(pos, &e, &t);
				position_t p_on_line;
				GetposbyT(t, &e, &p_on_line);
				p_on_line.angle_ *= (180 / 3.14159);//������ת��Ϊ�Ƕ�
				double per;
				GetpercentbyT(t, &e, &per);
				double dis_weight = sqrt(pow(p_on_line.x_ - pos.x_, 2) + pow(p_on_line.y_ - pos.y_, 2));
				double angle_weight = 9999999;
				int suitable_wop_id = -1;
				for (auto wop_id : edgeInfo.wop_list)
				{
					var__way_of_pass_t wop;
					search_wop(wop_id, wop);
					double angle_err = 0.0;
					if (wop.angle_type_ == CONST_ANGLE)
					{
						double global_angle = p_on_line.angle_ + wop.angle_;//�������ת��Ϊȫ�ֽ�
						global_angle = (global_angle >= 0.0) ? ((global_angle >= 360.0) ? (global_angle - 360.0) : global_angle) : (global_angle + 360.0);//���Ƕ�ȫ��ת��Ϊ0-360��ֵ
						angle_err = fabs(pos.angle_ - global_angle);
						angle_err = (angle_err > 180.0) ? (360 - angle_err) : angle_err;//���ǶȲ�ת��������180������
					}
					else if (wop.angle_type_ == GLOBAL_CONST_ANGLE)
					{
						double global_angle = (wop.angle_ >= 0.0) ? wop.angle_ : (wop.angle_ + 360.0);
						angle_err = fabs(pos.angle_ - global_angle);
						angle_err = (angle_err > 180.0) ? (360 - angle_err) : angle_err;
					}
					if (angle_err < angle_weight)
					{
						angle_weight = angle_err;
						suitable_wop_id = wop.id_;
					}
				}

				edge_weight = dis_weight + (angle_weight / 180.0 * PI) * 0.4;//���Ƕ�ת���ɻ��ȣ�Ȼ�����Ȩ�أ�1����Ϊ0.4�ף�
				if (edge_weight < min_weight)
				{
					min_weight = edge_weight;
					upl.edge_id = edgeInfo.id;
					upl.percent = per / 100.0;//��Ҫ�����0-1�İٷֱ�
					upl.wop_id = suitable_wop_id;
					upl.aoa = angle_to_normal(pos.angle_ - p_on_line.angle_);//upl�Ĺ��Ǿ��ǵ�ĽǶȼ�ȥӳ��������ǣ�ת��Ϊ-180-180
				}
				//delete[] e.disperse_points_.data_;
				//e.disperse_points_.data_ = NULL;

				if (nullptr != e.disperse_points_.data_)
				{
					delete[] e.disperse_points_.data_;
					e.disperse_points_.data_ = nullptr;
				}
			}
	}
		

		return 0;
	}
	//����ӳ�䵽���ϵĵ�
	static void point_on_edge(int edge_id,const position_t& pos_cur, position_t& point_on_edge)
	{
		EDGEINFO  edge_cur;
		DATAMNG->GetEdgeById(edge_id, edge_cur);
		var__edge_t edge_t;
		PT_C::ED(edge_cur,edge_t);
		double t;
		GetTClosestOnLineBycurg(pos_cur, &edge_t, &t);
		GetposbyT(t, &edge_t, &point_on_edge);
	}

	static double point_angle_on_edge(const EDGEINFO& edgeInfo,double t)
	{
		var__edge_t edge_t; 
		PT_C::ED(edgeInfo,edge_t);
		double angle = 0.0;
		GetAnglebyT(t, &edge_t, &angle);
		return angle;
	}
	static void point_on_edge_upl(int edge_id, const position_t& pos, UPL& upl)
	{
		EDGEINFO  edge_info;
		DATAMNG->GetEdgeById(edge_id, edge_info);
		DATAMNG->DispersePoint(edge_info);//�Ա߼�����ɢ��
		var__edge_t edge_t;
		PT_C::ED(edge_info, edge_t);
		double t;
		GetTClosestOnLineBycurg(pos, &edge_t, &t);
		position_t point_on_edge;
		GetposbyT(t, &edge_t, &point_on_edge);

		upl.edge_id = edge_id;
		upl.percent = t;
		upl.aoa = angle_to_normal(pos.angle_ - point_on_edge.angle_*(180/3.14159));
	}

};

#endif