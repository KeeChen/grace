#include "PathSearch.h"
#include "point_algorithm.hpp"

#define NAV_TYPE_FLOYD 0       //���������㷨
#define NAV_TYPE_DIJSKTRA 1    //�Ͻ�˹�����㷨

#define DISTANCE_INFINITY 9999999

class NavSearch
{
private:
	NavSearch();
	~NavSearch();
public:
	static NavSearch* Instance()
	{
		static NavSearch search_instance;
		return &search_instance;
	}
public:
	void get_user_id();
	bool InitNavigationData(const LAYOUTDATA& layoutData, int type, const char* filePath,
		int(*FinishLevelCB)(const unsigned int iPercent, const void *pUsr), void* pUsr);
	//�������������յ����������·�������е������յ��WOP������Ϻ��ѳ����Ž�
	double SearchNavPath(const position_t& src_pos, const position_t& dest_pos, 
		std::vector<UPL>& path_list, PathSearch_UPL& dest_upl_best);
	//���غ������������������յ��upl��·���˺��������յ��wop�Ѿ�ȷ�������
	double SearchNavPath(const position_t& src_pos, const PathSearch_UPL& dest_upl, std::vector<UPL>& path_list);
	void set_docks_data(DOCKDATA& dockData);
	void set_start_edge(int edge_id);
private:
	void TransformData(const LAYOUTDATA& layoutData, PathSearch_Wop*& wopArray, 
		PathSearch_LINK*& linkArray, PathSearch_EDGEINFO*& edgeArray);
	void TransfromEdge(const LAYOUTDATA& layoutData, PathSearch_EDGEINFO*& edgeArray);
	void TransfromPoint(const POSPOINT& posPoint, PathSearch_POSPOINT& pos);
	void Point2UplList(const position_t& pos_t, std::vector<PathSearch_UPL>& upl_list);
private:
	int edge_id_= 0;
	int user_id = 0;
	DOCKDATA dock_data_;
};