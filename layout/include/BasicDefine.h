#ifndef BASIC_DEFINE_ZHH_20161011
#define BASIC_DEFINE_ZHH_20161011

#include <vector>
#include <memory>

#define FALSE 0
#define TRUE 1

#define BIG_SIZE 1
#define SHAPE_R (10 * BIG_SIZE)
#define PI 3.14159
#define OPT_PARAM_NUM 20
#define OPT_CTRL_NUM 10

enum ItemType
{
	ITEM_NULL = -1,
	ITEM_NODE,
	ITEM_EDGE,
	ITEM_VEHICLE,
	ITEM_TARGET,
	ITEM_ANCHOR
};

enum ActionType
{
	ACTION_SELECT,
	ACTION_HAND_DRAG,
	ACTION_ADD_NODE,
	ACTION_ADD_EDGE,
	ACTION_POINT_LINE,
	ACTION_ADD_VEHICLE,
	ACTION_MOVE_ITEM,
	ACTION_ENTER_NAV,
	ACTION_FORMAT,
	ACTION_ADD_ANCHOR,
	ACTION_LINK_CURVE
};

enum Direction
{
	FORWARD,
	BACKWARD,
	BOTH
};
enum AngleType //����·����ά�ֽǶȵķ�ʽ�����ǣ�
{
	CONST_ANGLE,            //����ʸ���ֺ㶨�Ƕ�
	GLOBAL_CONST_ANGLE,     //����ȫ�ֺ㶨��
	VARIABLE_ANGLE,         //����ʸ���ֱ��
	GLOBAL_VAR_ANGLE        //����ȫ�ֱ��
};

typedef struct NodeInfo
{
	int node_id = 0;
	double pos_x = 0.0;
	double pos_y = 0.0;
	int spin = FALSE;//�Ƿ�������ת��0��ʾ������1��ʾ����
	int tshaped = FALSE;//�Ƿ���·�ڣ�0���ǣ�1��

	NodeInfo& operator = (const NodeInfo& other)
	{
		if (this == &other)
		{
			return *this;
		}
		node_id = other.node_id;
		pos_x = other.pos_x;
		pos_y = other.pos_y;
		spin = other.spin;
		tshaped = other.tshaped;

		return *this;
	}
}NODEINFO;

typedef struct EdgeSpeed
{
	double percent = 1.0;//�����߶εİٷֱ�
	double speed = 0.4;//����·��λ�õ��ٶ�

	EdgeSpeed& operator = (const EdgeSpeed& other)
	{
		if (this != &other)
		{
			percent = other.percent;
			speed = other.speed;
		}
		return *this;
	}
}EDGESPEED;

typedef struct Point
{
	double x = 0.0;
	double y = 0.0;
	double distance_to_start = 0.0;//�˵㵽��ʼ��ľ��룬���ֶ�ֻ�ڱߵ���ɢ����ʹ��

	Point& operator = (const Point& other)
	{
		if (this != &other)
		{
			x = other.x;
			y = other.y;
			distance_to_start = other.distance_to_start;
		}
		return *this;
	}
}POSPOINT;

typedef struct Wop
{
	int id = -1;
	Direction direction = FORWARD;
	AngleType angle_type = CONST_ANGLE;
	double angle = 0.0;

	Wop& operator = (const Wop& other)
	{
		if (this != &other)
		{
			id = other.id;
			direction = other.direction;
			angle_type = other.angle_type;
			angle = other.angle;
		}
		return *this;
	}
}WOP;
typedef struct EdgeInfo
{
	int id = -1;
	int start_node_id = 0;
	int end_node_id = 0;
	int direction = Direction::FORWARD;//�ߵķ���0����1����2˫��
	int backward = FALSE;//�Ƿ�������
	POSPOINT ctrl_point1;
	POSPOINT ctrl_point2;
	POSPOINT termstpos;
	POSPOINT termendpos;
	double terminal_start_percent = 0.0;
	double terminal_end_percent = 0.0;
	int priority = 0;

	//����Ϊ�����ӵ�����
	int type = 0;  //����Ϊ3�ױ��������������㷨
	double length = 0.0; //�ߵĳ���
	std::vector<EDGESPEED> speed;  //·�߷ֶ��ٶȣ����10��
	std::vector<POSPOINT> disperse_point;//�ߵ���ɢ�㣬Ĭ��200����
	std::vector<int> wop_list; //������Ӧ������WOP��id
	EdgeInfo()
	{
		EDGESPEED edgeSpeed;
		speed.push_back(edgeSpeed);//����·���һ��Ĭ���ٶ�
	}

	EdgeInfo& operator = (const EdgeInfo& other)
	{
		if (this != &other)
		{
			start_node_id = other.start_node_id;
			end_node_id = other.end_node_id;
			direction = other.direction;
			backward = other.backward;
			ctrl_point1 = other.ctrl_point1;
			ctrl_point2 = other.ctrl_point2;
			termstpos = other.termstpos;
			termendpos = other.termendpos;
			terminal_start_percent = other.terminal_start_percent;
			terminal_end_percent = other.terminal_end_percent;
			priority = other.priority;

			id = other.id;
			type = other.type;
			length = other.length;
			speed = other.speed;
			disperse_point = other.disperse_point;
			wop_list = other.wop_list;
		}
		return *this;
	}
}EDGEINFO;

typedef struct LinkEdge
{
	int edge_id = 0;
	int wop_id = 0;

	LinkEdge& operator = (const LinkEdge& other)
	{
		if (this != &other)
		{
			edge_id = other.edge_id;
			wop_id = other.wop_id;
		}
		return *this;
	}
}LINKEDGE;

typedef struct Link
{
	int id;
	LINKEDGE link_from;
	LINKEDGE link_to;
	double convert_cost = 0.0; //wopת���Ĵ��ۣ���Ҫ��������������ڽڵ�ԭ����ת�Ļ�����ת�ĽǶȵ�ʱ����ۣ������ÿ30����1�ױ�ʾ

	Link& operator = (const Link& other)
	{
		if (this != &other)
		{
			link_from = other.link_from;
			link_to = other.link_to;
			convert_cost = other.convert_cost;
		}
		return *this;
	}

}LINK;

typedef struct PosXya
{
	double pos_x = 0.0;
	double pos_y = 0.0;
	double angle = 0.0;
	PosXya& operator = (const PosXya& other)
	{
		if (this == &other)
		{
			return *this;
		}
		pos_x = other.pos_x;
		pos_y = other.pos_y;
		angle = other.angle;
		return *this;
	}
}POSXYA;
typedef struct VehicleInfo
{
	int vehicle_id = -1;
	std::string vehicle_name = "";
	std::string vehicle_ip = "127.0.0.1";
	int vehicle_port = 409;
	double pos_x = 0.0;
	double pos_y = 0.0;
	double angle = 90.0;
	double speed = 0.4;//��ǰ����
	double speed_min = 0.1;//��С����
	double speed_max = 0.8;//�����
	double wspeed = 0.2;//���ٶ�
	int status = 0; //0: ����״̬��1������ִ���У�2��������ͣ�У�3��������
	int edge_id = 0; //�����ڱߵ�ID
	VehicleInfo& operator = (const VehicleInfo& other)
	{
		if (this == &other)
		{
			return *this;
		}
		vehicle_id = other.vehicle_id;
		angle = other.angle;
		speed = other.speed;
		speed_min = other.speed_min;
		speed_max = other.speed_max;
		wspeed = other.wspeed;
		status = other.status;
		pos_x = other.pos_x;
		pos_y = other.pos_y;
		vehicle_name = other.vehicle_name;
		vehicle_ip = other.vehicle_ip;
		vehicle_port = other.vehicle_port;
		edge_id = other.edge_id;
		return *this;
	}
}VEHICLEINFO;

typedef struct TargetInfo
{
	int id = 0;
	int edge_id = 0;
	int wop_id = 0;
	double pos_x = 0.0;
	double pos_y = 0.0;
	double angle = 0.0;
	TargetInfo& operator = (const TargetInfo& other)
	{
		if (this != &other)
		{
			id = other.id;
			edge_id = other.edge_id;
			wop_id = other.wop_id;
			pos_x = other.pos_x;
			pos_y = other.pos_y;
			angle = other.angle;
		}
		return *this;
	}
}TARGETINFO;

typedef struct OptInfo
{
	int id = 0;
	int type = 0;
	uint64_t params[OPT_PARAM_NUM];
	OptInfo()
	{
		for (int i = 0; i < OPT_PARAM_NUM; ++i)
		{
			params[i] = 0;
		}
	}
	OptInfo& operator = (const OptInfo& other)
	{
		if (this != &other)
		{
			id = other.id;
			type = other.type;
			for (int i = 0; i < OPT_PARAM_NUM; ++i)
			{
				params[i] = other.params[i];
			}
		}
		return *this;
	}
	void clear_param()
	{
		for (int i = 0; i < OPT_PARAM_NUM; ++i)
		{
			params[i] = 0;
		}
	}
}OPTINFO;

typedef struct OptCtrlInfo
{
	int code = 0;
	int mask = 0;
	uint64_t params[OPT_CTRL_NUM];
	OptCtrlInfo()
	{
		for (int i = 0; i < OPT_CTRL_NUM; ++i)
		{
			params[i] = 0;
		}
	}
	OptCtrlInfo& operator = (const OptCtrlInfo& other)
	{
		if (this != &other)
		{
			code = other.code;
			mask = other.mask;
			for (int i = 0; i < OPT_CTRL_NUM; ++i)
			{
				params[i] = other.params[i];
			}
		}
		return *this;
	}
}OPTCTRLINFO;

typedef struct AnchorInfo
{
	int id = 0;
	int edge_id = 0;
	int wop_id = 0;
	double pos_x = 0.0;
	double pos_y = 0.0;
	double angle = 0.0;
	double percent = 0.0;
	double aoa = 0.0; //upl�Ĺ���
	std::vector<OPTINFO> opts;
	AnchorInfo& operator = (const AnchorInfo& other)
	{
		if (this != &other)
		{
			id = other.id;
			edge_id = other.edge_id;
			wop_id = other.wop_id;
			pos_x = other.pos_x;
			pos_y = other.pos_y;
			angle = other.angle;
			percent = other.percent;
			aoa = other.aoa;
			opts = other.opts;
		}
		return *this;
	}
}ANCHORINFO;

typedef struct LayoutData
{
	int type = 0;//·�����ͣ�0����������
	int search_type = 0;
	std::vector<NODEINFO> nodeList;
	std::vector<EDGEINFO> edgeList;
	std::vector<WOP> wopList;
	std::vector<LINK> linkList;

	LayoutData& operator = (const LayoutData& other)
	{
		if (this != &other)
		{
			type = other.type;
			search_type = other.search_type;
			nodeList = other.nodeList;
			edgeList = other.edgeList;
			wopList = other.wopList;
			linkList = other.linkList;
		}
		return *this;
	}
}LAYOUTDATA;

typedef struct ConfigData
{
	int search_type = 1; //·�������㷨����,0:floyd��1:dijkstra
	std::vector<VEHICLEINFO> vehicleList; //��ȡ�����ļ��еĳ���
	ConfigData& operator = (const ConfigData& other)
	{
		if (this != &other)
		{
			search_type = other.search_type;
			vehicleList = other.vehicleList;
		}
		return *this;
	}
}CONFIGDATA;

typedef struct DockData
{
	std::vector<ANCHORINFO> anchorList;
	DockData& operator = (const DockData& other)
	{
		if (this != &other)
		{
			anchorList = other.anchorList;
		}
		return *this;
	}
}DOCKDATA;

#endif