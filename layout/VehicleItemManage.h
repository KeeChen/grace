#pragma once
#include "VehicleItem.h"
#include <mutex>
#include "rapidxml.hpp"

class MapScene;

class VehicleItemManage
{
public:
	VehicleItemManage(MapScene* pMapScene);
	~VehicleItemManage();
public:
	VehicleItem* AddVehicleItem(const VEHICLEINFO& vehicleInfo,bool manual);
	//manual:�������ֶ������ӣ����ǻ�ȡ�������ݺ��Զ���ӣ�Ĭ���ֶ�
	VehicleItem* AddVehicleItem(const QPointF& pos, const EDGEINFO& edgeInfo,bool manual = true);
	VehicleItem* FindVehicle(int iVehicleId);
	void UpdateVehiclePos(int iVehicleId, const POSXYA& posXya);
	bool CollisionDetect(int iVehicleId);
	int GetMaxVehicleId();
	bool GetVehicleInfo(int iVehicleId, VEHICLEINFO& vehicleInfo);
	void ResetVehicleItem(VehicleItem* pVehicleItem, const QPointF& pos, const EDGEINFO& edgeInfo);
	void SetVehiclePos(int iVehicleId, const POSXYA& pos);
	VehicleItem* FindVehicleItem(int iVehicleId);
	void SetVehicleInfo(const VEHICLEINFO& vehicleInfo);
private:
	MapScene* m_pMapScene;

	std::recursive_mutex m_mutexVehicleItem;
	std::vector<VehicleItem*> m_vecVehicleItem;
};