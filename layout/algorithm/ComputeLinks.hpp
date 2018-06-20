#pragma once
#include "DataManage.h"
#include "PosandTrack.h"
#include "AlgorithmDefine.hpp"

class Links
{
private:
	Links(){}
	~Links(){}
public:
	static Links* instance()
	{
		static Links link_instance;
		return &link_instance;
	}
private:
	std::vector<NODEINFO> _node_list;
public:
	void SetNodeList(const std::vector<NODEINFO> node_list)
	{
		_node_list = node_list;
	}
	void ComputeLinks(const std::vector<EDGEINFO>& vecEdgeInfo, bool bSpin, std::vector<LINK>& vecLink)
	{
		for (auto &edgeCur : vecEdgeInfo)
		{
			for (auto &edgeLink : vecEdgeInfo)
			{
				//��ǰ�ߵ��յ����ڽӱߵ����,���ߵ�ǰ�ߵ��յ�Ҳ���ڽӱߵ��յ�
				if (edgeCur.end_node_id == edgeLink.start_node_id ||
					(edgeCur.end_node_id == edgeLink.end_node_id /*&& edgeCur.start_node_id != edgeLink.start_node_id*/))
				{
					//�������������ת����ô�ж�������֮��Ľڵ��Ƿ����ת,����ֱ�����嶼��������ת
					bool nodeSpin = bSpin;
					if (nodeSpin){
						nodeSpin = GetNodeSpin(edgeCur.end_node_id);
					}
					ComputeLinksForward(edgeCur, edgeLink, nodeSpin, vecLink);
				}
				//��ǰ�ߵ�������ڽӱߵ��յ�,���ߵ�ǰ�ߵ����Ҳ���ڽӱߵ����
				if (edgeCur.start_node_id == edgeLink.end_node_id ||
					(edgeCur.start_node_id == edgeLink.start_node_id/* && edgeCur.end_node_id != edgeLink.end_node_id*/))
				{
					//�������������ת����ô�ж�������֮��Ľڵ��Ƿ����ת,����ֱ�����嶼��������ת
					bool nodeSpin = bSpin;
					if (nodeSpin){
						nodeSpin = GetNodeSpin(edgeCur.start_node_id);
					}
					ComputeLinksBackward(edgeCur, edgeLink, nodeSpin, vecLink);
				}
			}
		}
	}
private:
	bool GetNodeSpin(int nodeId)
	{
		int spin = 1;
		for (auto nodeInfo : _node_list)
		{
			if (nodeInfo.node_id == nodeId)
			{
				spin = nodeInfo.spin;
				break;
			}
		}
		return spin;
	}
	double PointAngle(const EDGEINFO& edgeInfo, double t)
	{
		var__edge_t edge_t;
		PT_C::ED(edgeInfo,edge_t);
		double angle;
		GetAnglebyT(t, &edge_t, &angle);
		angle = angle * 180 / 3.14159;//�˺������ص��ǻ��ȵ�λ����Ҫ����ɽǶ�
		if (angle < 0.0)
		{
			angle += 360;
		}
		return angle;
	}

	double cost(const EDGEINFO& edgeCur, const EDGEINFO& edgeLink, const WOP& wopCur, const WOP& wopLink)
	{
		double edgeCurAngle = PointAngle(edgeCur, 1.0);
		if (wopCur.direction == BACKWARD)
		{
			edgeCurAngle = PointAngle(edgeCur, 0.0);
		}
		double edgeLinkAngle = PointAngle(edgeLink, 0.0);
		if (wopLink.direction == BACKWARD)
		{
			edgeLinkAngle = PointAngle(edgeLink, 1.0);
		}
		double costAngle = 0.0;
		if (wopCur.angle_type == AngleType::CONST_ANGLE && wopLink.angle_type == AngleType::CONST_ANGLE)
		{
			costAngle = (edgeLinkAngle + wopLink.angle) - (edgeCurAngle + wopCur.angle);
		}
		if (wopCur.angle_type == AngleType::GLOBAL_CONST_ANGLE && wopLink.angle_type == AngleType::CONST_ANGLE)
		{
			costAngle = (edgeLinkAngle + wopLink.angle) - wopCur.angle;
		}
		if (wopCur.angle_type == AngleType::CONST_ANGLE && wopLink.angle_type == AngleType::GLOBAL_CONST_ANGLE)
		{
			costAngle = wopLink.angle - (edgeCurAngle + wopCur.angle);
		}
		if (wopCur.angle_type == GLOBAL_CONST_ANGLE && wopLink.angle_type == GLOBAL_CONST_ANGLE)
		{
			costAngle = wopLink.angle - wopCur.angle;
		}
		costAngle = fabs(costAngle);//ȡ�ǶȲ����ֵ
		while (costAngle > 180.0)  //���Ƕ�ת��Ϊ0-180���ڵ�ֵ
		{
			costAngle = fabs(360.0 - costAngle);
		}
		double cost = costAngle / 30.0;//ÿ30�Ȼ����1�ף�m��
		return (cost * 100.0);
	}

	void AddLink(const EDGEINFO& edgeCur, const EDGEINFO& edgeLink, const WOP& wopCur, const WOP& wopLink, bool bSpin, std::vector<LINK>& vecLink)
	{
		LINK link;
		link.id = static_cast<int>(vecLink.size()) + 1;
		link.link_from.edge_id = edgeCur.id;
		link.link_from.wop_id = wopCur.id;
		link.link_to.edge_id = edgeLink.id;
		link.link_to.wop_id = wopLink.id;
		if (bSpin)
		{
			link.convert_cost = cost(edgeCur, edgeLink, wopCur, wopLink);
		}
		vecLink.push_back(link);
	}

	void AddLinkWithDirection(const EDGEINFO& edgeCur, const EDGEINFO& edgeLink, const WOP& wopCur, const WOP& wopLink, bool bSpin,
		std::vector<LINK>& vecLink)
	{
		if (bSpin) //��������Խ���ԭ����ת���������߿���link
		{
			AddLink(edgeCur, edgeLink, wopCur, wopLink, true, vecLink);
		}
		else
		{
			//�������wop�ĽǶ�������Σ�ֻҪת���Ƕ�С��30�ȣ�������link
			double edgeCurAngle = PointAngle(edgeCur, 1.0);
			if (wopCur.direction == BACKWARD)
			{
				edgeCurAngle = PointAngle(edgeCur, 0.0);
			}
			double edgeLinkAngle = PointAngle(edgeLink, 0.0);
			if (wopLink.direction == BACKWARD)
			{
				edgeLinkAngle = PointAngle(edgeLink, 1.0);
			}
			double costAngle = 0.0;
			costAngle = fabs((edgeLinkAngle + wopLink.angle) - (edgeCurAngle + wopCur.angle));
			while (costAngle > 360.0)  //���Ƕ�ת��Ϊ0-360���ڵ�ֵ
			{
				costAngle = fabs(360.0 - costAngle);
			}
			if (costAngle < 30.0 ||
				costAngle > 330.0)//�Ƕ�ת��С��30��
			{
				AddLink(edgeCur, edgeLink, wopCur, wopLink, false, vecLink);
			}
		}
	}

	void ComputeLinksForward(const EDGEINFO& edgeCur, const EDGEINFO& edgeLink, bool bSpin, std::vector<LINK>& vecLink)
	{
		for (auto &wopCurId : edgeCur.wop_list)
		{
			WOP wopCur;
			DATAMNG->GetWopById(wopCurId, wopCur);
			for (auto &wopLinkId : edgeLink.wop_list)
			{
				WOP wopLink;
				DATAMNG->GetWopById(wopLinkId, wopLink);
				//�����ǰ���������ߣ��ڽӱ�Ҳ�������ߣ���ô��ǰ�ߵ��յ�������ڽӱߵ���㣬������link
				//�����ǰ���������ߣ��ڽӱ��Ƿ����ߣ���ô��ǰ�ߵ��յ�������ڽӱߵ��յ㣬������link
				if (wopCur.direction == FORWARD &&  wopLink.direction == FORWARD && edgeCur.end_node_id == edgeLink.start_node_id ||
					wopCur.direction == FORWARD && wopLink.direction == BACKWARD && edgeCur.end_node_id == edgeLink.end_node_id)
				{
					AddLinkWithDirection(edgeCur, edgeLink, wopCur, wopLink, bSpin, vecLink);
				}
			}
		}
	}
	void ComputeLinksBackward(const EDGEINFO& edgeCur, const EDGEINFO& edgeLink, bool bSpin, std::vector<LINK>& vecLink)
	{
		for (auto &wopCurId : edgeCur.wop_list)
		{
			WOP wopCur;
			DATAMNG->GetWopById(wopCurId, wopCur);
			for (auto &wopLinkId : edgeLink.wop_list)
			{
				WOP wopLink;
				DATAMNG->GetWopById(wopLinkId, wopLink);
				//�����ǰ���Ƿ����ߣ��ڽӱ��������ߣ���ô��ǰ�ߵ����������ڽӱߵ���㣬������link
				//�����ǰ���Ƿ����ߣ��ڽӱ�Ҳ�Ƿ����ߣ���ô��ǰ�ߵ����������ڽӱߵ��յ㣬������link
				if (wopCur.direction == BACKWARD &&  wopLink.direction == FORWARD && edgeCur.start_node_id == edgeLink.start_node_id ||
					wopCur.direction == BACKWARD && wopLink.direction == BACKWARD && edgeCur.start_node_id == edgeLink.end_node_id)
				{
					AddLinkWithDirection(edgeCur, edgeLink, wopCur, wopLink, bSpin, vecLink);
				}
			}
		}
	}

};