#ifndef CALCULATE_20161228
#define CALCULATE_20161228

#include "PathSearch.h"
#include <map>
#include <math.h>
#include <memory>
#include <string.h>
#define PI 3.14159

/* ��λ���ٶ������� */
typedef struct PS__POSITION
{
	double x_;
	double y_;
	union
	{
		double theta_;
		double angle_;
		double w_;
		double distance_;
	};
}PS_position_t, PS_velocity_t;

typedef struct PS__var__vector
{
	void *data_;
	int   count_;
}PS_var__vector_t;

struct PS_avltree_node_t
{
	struct PS_avltree_node_t *lchild, *rchild; /* �ֱ�ָ���������� */
	int height;                     /* ���ĸ߶� */
};

typedef struct
{
	struct PS_avltree_node_t		index_;
	int							id_;
	int							direction_;
	int							angle_type_;
	double						angle_;				// ����
}PS_var__way_of_pass_t;

typedef struct
{
	struct PS_avltree_node_t		index_;
	int							id_;
	int							start_node_id_;
	int							end_node_id_;
	PS_position_t					control_point_1_;
	PS_position_t					control_point_2_;
	PS_position_t					start_point_;
	PS_position_t					end_point_;
	double						length_;			// �ߵĳ��ȵ�λ m
	PS_var__vector_t				speed_limit_;		// var__speed_limit_t[] �ֶ��������
	PS_var__vector_t				disperse_points_;	// position_t [] ��ɢ�㶨�壬 ʵʩ���������ɺ� �����ļ��� �·�����׼ƽ̨
	PS_var__vector_t				wops_;				// int [] wop ����
}PS_var__edge_t;

class DataCalculate
{
public:
	static double map2world( double loc )
	{
		return loc / 100;
	};
	static double world2map( double world )
	{
		return world * 100;
	}
	static double radian2angle( double radian )
	{
		return radian * 180 / 3.14159;
	}
	static double Getdistsqure( const PS_position_t *p1, const PS_position_t *p2 )
	{ 
		return ( p2->x_ - p1->x_ )*( p2->x_ - p1->x_ ) + ( p2->y_ - p1->y_ )*( p2->y_ - p1->y_ ); 
	}
	static double GetdistbyPos( const PS_position_t *p1, const PS_position_t *p2 )
	{ 
		return sqrt( Getdistsqure( p1, p2 ) ); 
	}
	static int GetdistbyT( double t, const PS_var__edge_t * curgraph, double *output )
	{
		double dist_result;
		PS_position_t* disperse_points;
		int is, ie;

		if( !output || !curgraph ) {
			return -1;
		}

		disperse_points = ( PS_position_t* )curgraph->disperse_points_.data_;
		if( !disperse_points || curgraph->disperse_points_.count_ != 201 ) {
			return -1;
		}

		if( t <= 0 ) {
			*output = 0;
			return 0;
		} else if( t >= 1 ) {
			*output = disperse_points[200].distance_;
			return 0;
		}

		is = ( int )( t * 200.0 );
		if( is >= 200 ) {
			*output = disperse_points[200].distance_;
			return 0;
		}

		ie = is + 1;
		dist_result = ( ( ie*0.005 - t )*disperse_points[is].distance_ + ( t - is*0.005 )*disperse_points[ie].distance_ )*200.0;
		*output = dist_result;
		return 0;
	}
	static int GetposbyT( double t, const PS_var__edge_t * curgraph, PS_position_t *pos )
	{
		double x[9], dif_x, dif_y;

		if( !curgraph || !pos ) return -1;

		x[0] = curgraph->start_point_.x_;     x[1] = curgraph->start_point_.y_;
		x[2] = curgraph->control_point_1_.x_; x[3] = curgraph->control_point_1_.y_;
		x[4] = curgraph->control_point_2_.x_; x[5] = curgraph->control_point_2_.y_;
		x[6] = curgraph->end_point_.x_;       x[7] = curgraph->end_point_.y_;
		x[8] = t;
		pos->x_ = x[0] * pow( ( 1 - x[8] ), 3 ) + x[2] * 3 * pow( ( 1 - x[8] ), 2 )*x[8] + x[4] * 3 * ( 1 - x[8] )*pow( x[8], 2 ) + x[6] * pow( x[8], 3 ); // x
		pos->y_ = x[1] * pow( ( 1 - x[8] ), 3 ) + x[3] * 3 * pow( ( 1 - x[8] ), 2 )*x[8] + x[5] * 3 * ( 1 - x[8] )*pow( x[8], 2 ) + x[7] * pow( x[8], 3 ); // y
		dif_x = 3 * x[6] * pow( x[8], 2 ) - 3 * x[4] * pow( x[8], 2 ) - 3 * x[0] * pow( ( x[8] - 1 ), 2 ) + 3 * x[2] * pow( ( x[8] - 1 ), 2 ) - 6 * x[4] * x[8] * ( x[8] - 1 ) + 3 * x[2] * x[8] * ( 2 * x[8] - 2 );
		dif_y = 3 * x[7] * pow( x[8], 2 ) - 3 * x[5] * pow( x[8], 2 ) - 3 * x[1] * pow( ( x[8] - 1 ), 2 ) + 3 * x[3] * pow( ( x[8] - 1 ), 2 ) - 6 * x[5] * x[8] * ( x[8] - 1 ) + 3 * x[3] * x[8] * ( 2 * x[8] - 2 );
		pos->angle_ = atan2( dif_y, dif_x );
		return 0;
	}
	static int GetpercentbyT( double t, const PS_var__edge_t * curgraph, double* per )
	{
		int retval;
		double dist;
		PS_position_t* dis_points;
		double dist_whole;
		double percent;

		if( !curgraph ) return -1;

		retval = GetdistbyT( t, curgraph, &dist );
		if( retval < 0 ) {
			return -1;
		}

		dis_points = ( PS_position_t* )curgraph->disperse_points_.data_;
		if( !dis_points || curgraph->disperse_points_.count_ != 201 ) return -1;

		dist_whole = dis_points[curgraph->disperse_points_.count_ - 1].distance_;
		percent = ( 100.0*dist / dist_whole );

		if( percent < 0 ) {
			percent = 0;
		} else if( percent>100 ) {
			percent = 100;
		}

		*per = percent;
		return 0;
	}
	static PS_position_t PT( const PathSearch_POSPOINT& point )
	{
		PS_position_t pos;
		pos.x_ = point.x;
		pos.y_ = point.y;

		return pos;
	}
	static PS_var__edge_t ED( const PathSearch_EDGEINFO& edgeInfo )
	{
		PS_var__edge_t edge_t;
		edge_t.id_ = edgeInfo.id;
		edge_t.start_node_id_ = edgeInfo.start_node_id;
		edge_t.end_node_id_ = edgeInfo.end_node_id;
		edge_t.control_point_1_ = PT( edgeInfo.ctrl_point1 );
		edge_t.control_point_2_ = PT( edgeInfo.ctrl_point2 );
		edge_t.start_point_ = PT( edgeInfo.termstpos );
		edge_t.end_point_ = PT( edgeInfo.termendpos );
		edge_t.length_ = edgeInfo.length;

		edge_t.disperse_points_.count_ = edgeInfo.disperse_point.size( );
		if( edge_t.disperse_points_.count_ > 0 ) {
			edge_t.disperse_points_.data_ = new PS_position_t[edge_t.disperse_points_.count_];
			memcpy( edge_t.disperse_points_.data_, &edgeInfo.disperse_point[0], sizeof( PS_position_t )* edge_t.disperse_points_.count_ );
		}
		else{
			edge_t.disperse_points_.data_ = NULL;
		}

		return edge_t;
	}
	static PS_var__edge_t EDforUPL(const PathSearch_EDGEINFO& edgeInfo)
	{
		PS_var__edge_t edge_t;
		edge_t.id_ = edgeInfo.id;
		edge_t.start_node_id_ = edgeInfo.start_node_id;
		edge_t.end_node_id_ = edgeInfo.end_node_id;
		edge_t.control_point_1_ = PT(edgeInfo.ctrl_point1);
		edge_t.control_point_2_ = PT(edgeInfo.ctrl_point2);
		edge_t.start_point_ = PT(edgeInfo.termstpos);
		edge_t.end_point_ = PT(edgeInfo.termendpos);
		edge_t.length_ = edgeInfo.length;

		return edge_t;
	}
	static int GetTClosestOnLineBycurg( PS_position_t current_pos, const PS_var__edge_t * curgraph, double *output )
	{
		double t_result;
		PS_position_t* disperse_points;
		int index_mindist_sq;
		double thisdist_sq;
		double min_dist_sq;
		double ts, te, tm;//ts:�����е���㣬te�������е���ֹ�� tm���������е�
		double space_t = 0.005;
		int i;
		double eplon = 0.0001;//����ֿ��Ķ��߾����С�ڸ�ֵ����ֹͣ����
		double dts, dte, dtm;
		PS_position_t pos;

		if( !curgraph || !output ) {
			return -1;
		}

		disperse_points = ( PS_position_t* )curgraph->disperse_points_.data_; //�ӵ�ͼ�л�ȡ��ɢ��
		if( !disperse_points || curgraph->disperse_points_.count_ != 201 ) {
			return -1;
		}

		//��201�����ҵ�����ĵ�,��sq�ı�ʾ����ƽ��,��201����ı������ٲ��˵�
		index_mindist_sq = 0;
		thisdist_sq = 0;
		min_dist_sq = Getdistsqure( &disperse_points[index_mindist_sq], &current_pos );
		for( i = 1; i < 201; i++ ) {
			thisdist_sq = Getdistsqure( &disperse_points[i], &current_pos );
			if( thisdist_sq < min_dist_sq ) {
				min_dist_sq = thisdist_sq;
				index_mindist_sq = i;
			}
		}
		//�ö��ַ��ҵ���������ĵ�		
		if( index_mindist_sq == 0 ) {
			ts = 0;
			te = ( index_mindist_sq + 1 ) * space_t;
		} else if( index_mindist_sq == 199 ) {
			ts = ( index_mindist_sq - 1 )*space_t;
			te = index_mindist_sq*space_t;
		} else {
			ts = ( index_mindist_sq - 1 )*space_t;
			te = ( index_mindist_sq + 1 )*space_t;
		}

		for( int i = 0;; i++ ) {
			if( GetposbyT( ts, curgraph, &pos ) >= 0 ) {
				dts = GetdistbyPos( &current_pos, &pos );
			}
			if( GetposbyT( te, curgraph, &pos ) >= 0 ) {
				dte = GetdistbyPos( &current_pos, &pos );
			}
			if( fabs( dts - dte ) < eplon || i > 10 ) {
				t_result = ( ts + te ) / 2.0;
				break;
			} else {
				tm = ( ts + te ) / 2.0;
				if( GetposbyT( tm, curgraph, &pos ) >= 0 ) {
					dtm = GetdistbyPos( &current_pos, &pos );
				}
				if( fabs( dts - dtm ) < fabs( dte - dtm ) ) {
					te = tm;
				} else {
					ts = tm;
				}
			}
		}

		*output = t_result;
		return 0;
	}

	static int point_mapping_upl( const PS_position_t& pos,
		const std::vector<PathSearch_Wop>& vctWops,
		const std::map<unsigned int, PathSearch_EDGEINFO>& mapEdgeInfo, PathSearch_UPL& upl )
	{
		double min_weight = 9999999;

		for( auto edgeInfo : mapEdgeInfo )//������
		{
			double edge_weight = 9999999;
			PS_var__edge_t e = ED( edgeInfo.second );
			double t;
			GetTClosestOnLineBycurg( pos, &e, &t );
			PS_position_t p_on_line;
			GetposbyT( t, &e, &p_on_line );
			p_on_line.angle_ *= ( 180 / 3.14159 );//������ת��Ϊ�Ƕ�
			double per;
			GetpercentbyT( t, &e, &per );
			double dis_weight = sqrt( pow( p_on_line.x_ - pos.x_, 2 ) + pow( p_on_line.y_ - pos.y_, 2 ) );
			double angle_weight = 9999999;
			int suitable_wop_id = -1;
			for( auto wop_id : edgeInfo.second.wop_list ) {

				if( true ) {

				}
				PS_var__way_of_pass_t wop = { 0 };
		//		search_wop( wop_id, wop );
				
				double angle_err = 0.0;
				if( wop.angle_type_ == PathSearch_CONST_ANGLE ) {
					double global_angle = p_on_line.angle_ + wop.angle_;//�������ת��Ϊȫ�ֽ�
					global_angle = ( global_angle >= 0.0 ) ? ( ( global_angle >= 360.0 ) ? ( global_angle - 360.0 ) : global_angle ) : ( global_angle + 360.0 );//���Ƕ�ȫ��ת��Ϊ0-360��ֵ
					angle_err = fabs( pos.angle_ - global_angle );
					angle_err = ( angle_err > 180.0 ) ? ( 360 - angle_err ) : angle_err;//���ǶȲ�ת��������180������
				} else if( wop.angle_type_ == PathSearch_GLOBAL_CONST_ANGLE ) {
					double global_angle = ( wop.angle_ >= 0.0 ) ? wop.angle_ : ( wop.angle_ + 360.0 );
					angle_err = fabs( pos.angle_ - global_angle );
					angle_err = ( angle_err > 180.0 ) ? ( 360 - angle_err ) : angle_err;
				}
				if( angle_err < angle_weight ) {
					angle_weight = angle_err;
					suitable_wop_id = wop.id_;
				}
			}

			edge_weight = dis_weight + ( angle_weight / 180.0 * PI ) * 0.4;//���Ƕ�ת���ɻ��ȣ�Ȼ�����Ȩ�أ�1����Ϊ0.4�ף�
			if( edge_weight < min_weight ) 
			{
				min_weight = edge_weight;
				upl.edge_id = edgeInfo.second.id;
				upl.fPercent = ( float )per / 100.0;//��Ҫ�����0-1�İٷֱ�
				upl.wop_id = suitable_wop_id;
				upl.fAoa = ( float )angle_weight;
			}
			if (e.disperse_points_.data_){
				delete[] (PS_position_t*)e.disperse_points_.data_;
				e.disperse_points_.data_ = NULL;
			}

		}

		return 0;
	}

	static double angle_to_normal( double angle )
	{
		double result = angle;
		while( result >= 360.0 ) {
			result = result - 360.0;
		}
		while( result < 0.0 ) {
			result = result + 360.0;
		}
		return result;
	}
	static double angle_to_normal180( double angle )
	{
		double result = angle_to_normal( angle );
		while( result > 180.0 ) {
			result = 360.0 - result;
		}
		return result;
	}

	static int point_mapping_upl( const PS_position_t& pos,
		                          std::map<unsigned int, PathSearch_Wop> mapWopType,
		                          std::map<unsigned int, PathSearch_EDGEINFO> mapEdgeInfo,
								  PathSearch_UPL& upl )
	{

		double min_weight = 9999999;

		for( auto edgeInfo : mapEdgeInfo )//������
		{
			double edge_weight = 9999999;
			PS_var__edge_t e = ED( edgeInfo.second );
			double t;
			GetTClosestOnLineBycurg( pos, &e, &t );
			PS_position_t p_on_line;
			GetposbyT( t, &e, &p_on_line );
			p_on_line.angle_ *= ( 180 / 3.14159 );//������ת��Ϊ�Ƕ�
			double per;
			GetpercentbyT( t, &e, &per );
			double dis_weight = sqrt( pow( p_on_line.x_ - pos.x_, 2 ) + pow( p_on_line.y_ - pos.y_, 2 ) );
			double angle_weight = 9999999;
			int suitable_wop_id = -1;
			for( auto wop_id : edgeInfo.second.wop_list ) {
				PathSearch_Wop wop;
				wop = mapWopType[wop_id];
				double angle_err = 0.0;
				if( wop.angle_type == PathSearch_CONST_ANGLE ) {
					double global_angle = p_on_line.angle_ + wop.angle;//�������ת��Ϊȫ�ֽ�
					global_angle = ( global_angle >= 0.0 ) ? ( ( global_angle >= 360.0 ) ? ( global_angle - 360.0 ) : global_angle ) : ( global_angle + 360.0 );//���Ƕ�ȫ��ת��Ϊ0-360��ֵ
					angle_err = fabs( pos.angle_ - global_angle );
					angle_err = ( angle_err > 180.0 ) ? ( 360 - angle_err ) : angle_err;//���ǶȲ�ת��������180������
				} else if( wop.angle_type == PathSearch_GLOBAL_CONST_ANGLE ) {
					double global_angle = ( wop.angle >= 0.0 ) ? wop.angle : ( wop.angle + 360.0 );
					angle_err = fabs( pos.angle_ - global_angle );
					angle_err = ( angle_err > 180.0 ) ? ( 360 - angle_err ) : angle_err;
				}
				if( angle_err < angle_weight ) {
					angle_weight = angle_err;
					suitable_wop_id = wop.wop_id;
				}
			}

			edge_weight = dis_weight + ( angle_weight / 180.0 * PI ) * 0.4;//���Ƕ�ת���ɻ��ȣ�Ȼ�����Ȩ�أ�1����Ϊ0.4�ף�
			if( edge_weight < min_weight ) {
				min_weight = edge_weight;
				upl.edge_id = edgeInfo.second.id;
				upl.fPercent = per / 100.0;//��Ҫ�����0-1�İٷֱ�
				upl.wop_id = suitable_wop_id;
				upl.fAoa = angle_to_normal( pos.angle_ - p_on_line.angle_ );//upl�Ĺ��Ǿ��ǵ�ĽǶȼ�ȥӳ��������ǣ�ת��Ϊ-180-180
			}
			if (e.disperse_points_.data_){
				delete[] (PS_position_t*)e.disperse_points_.data_;
				e.disperse_points_.data_ = NULL;
			}

		}

		return 0;
	}

	//����ӳ�䵽���ϵĵ�
	static void point_on_edge( int edge_id, const PS_position_t& pos_cur, std::map<unsigned int, PathSearch_EDGEINFO>& mapEdgeInfo, PS_position_t& point_on_edge )
	{
		PathSearch_EDGEINFO  edge_cur;
		edge_cur = mapEdgeInfo[edge_id];
		
		PS_var__edge_t edge_t = ED( edge_cur );
		double t;
		GetTClosestOnLineBycurg( pos_cur, &edge_t, &t );
		GetposbyT( t, &edge_t, &point_on_edge );

		if (edge_t.disperse_points_.data_){
			delete[](PS_position_t*)edge_t.disperse_points_.data_;
			edge_t.disperse_points_.data_ = NULL;
		}

	}


	static void Point2UplList( const PS_position_t& pos_t,
		                       std::map<unsigned int, PathSearch_Wop> mapWopType,
		                       std::map<unsigned int, PathSearch_EDGEINFO> mapEdgeInfo, 
							   std::vector<PathSearch_UPL>& upl_list )
	{
		PathSearch_UPL map_upl;
		point_mapping_upl( pos_t, mapWopType, mapEdgeInfo, map_upl );//�˺�����ȡ��upl�е�wop�Ǿ����ǶȻ���ĵ�ǰ����wop
		PathSearch_EDGEINFO edgeInfo;
		edgeInfo = mapEdgeInfo[map_upl.edge_id];//������Ҫ�������ڵ�����wopȫ���ó���������·���Ӷ��ѳ����·��
		for( auto wopId : edgeInfo.wop_list ) {
			PathSearch_Wop woptype;
			woptype = mapWopType[wopId];
			PS_position_t p_on_edge;
			point_on_edge( map_upl.edge_id, pos_t, mapEdgeInfo, p_on_edge );
			double wop_global_angle = ( woptype.angle >= 0.0 ) ? woptype.angle : ( woptype.angle + 360.0 ); //wop��ȫ�ֽǶ�
			if( woptype.angle_type == PathSearch_CONST_ANGLE ) {
				wop_global_angle = radian2angle( p_on_edge.angle_ ) + woptype.angle;
				wop_global_angle = angle_to_normal( wop_global_angle );//���Ƕ�ȫ��ת��Ϊ0-360��ֵ
			}
			double angle_err = fabs( wop_global_angle - pos_t.angle_ );
			angle_err = angle_to_normal180( angle_err ); //ת��0-180���ڵĽǶȲ�
			if( angle_err <= 40.0 )  //ֻ�нǶȲ�С��40�ȵ�wop���ǿ�ѡ��WOP
			{
				PathSearch_UPL upl;
				upl.edge_id = map_upl.edge_id;
				upl.wop_id = wopId;
				upl.fPercent = map_upl.fPercent;
				upl.fAoa = map_upl.fAoa;

				upl_list.push_back( upl );
			}
		}
	}

	// upl���������upl�Ͽ��õĴ�wop�б�
	static void Upl2UplList( const PathSearch_UPL upl,
		                     std::map<unsigned int, PathSearch_Wop>& mapWopType,
							 std::map<unsigned int, PathSearch_EdgeInfo>& mapEdgeInfo,
		                     std::vector<PathSearch_UPL>& upl_list )
	{
	   // ���Ƚ�UPLת�ɵ�
		PS_var__edge_t e = DataCalculate::ED(mapEdgeInfo[upl.edge_id]);
		PS_position_t p_on_line;
		DataCalculate::GetposbyT( upl.fPercent, &e, &p_on_line );

		double car_angle_ = ( p_on_line.angle_ + upl.fAoa ) * 180 / PI;//���ĽǶ�
		//p_on_line.angle_ = (p_on_line.angle_*180) / PI + upl.fAoa ;

		p_on_line.angle_ = angle_to_normal(p_on_line.angle_ * 180 / PI);
		

		PathSearch_EDGEINFO edgeInfo;
		edgeInfo = mapEdgeInfo[upl.edge_id];//������Ҫ�������ڵ�����wopȫ���ó���������·���Ӷ��ѳ����·��
		for( auto wopId : edgeInfo.wop_list ) {
			PathSearch_Wop woptype;
			woptype = mapWopType[wopId];
			//PS_position_t p_on_edge;
			//point_on_edge( upl.edge_id, p_on_line, mapEdgeInfo, p_on_edge );
			double wop_global_angle = ( woptype.angle >= 0.0 ) ? woptype.angle : ( woptype.angle + 360.0 ); //wop��ȫ�ֽǶ�
			if( woptype.angle_type == PathSearch_CONST_ANGLE ) {
				//wop_global_angle = radian2angle(p_on_line.angle_) + woptype.angle;
				wop_global_angle = p_on_line.angle_ + woptype.angle;
				wop_global_angle = angle_to_normal( wop_global_angle );//���Ƕ�ȫ��ת��Ϊ0-360��ֵ
			}
			double angle_err = fabs(wop_global_angle - car_angle_);//wop�ĽǶȺͳ��ĽǶȲ�������Ƿ�����
			angle_err = angle_to_normal180( angle_err ); //ת��0-180���ڵĽǶȲ�
			if( angle_err <= 90.0 )  //ֻ�нǶȲ�С��40�ȵ�wop���ǿ�ѡ��WOP//��Ϊ��Ǽ�����·
			{
				PathSearch_UPL validupl;
				validupl.edge_id = upl.edge_id;
				validupl.wop_id = wopId;
				validupl.fPercent = upl.fPercent;
				validupl.fAoa = upl.fAoa;

				upl_list.push_back( validupl );
			}
		}
		if (e.disperse_points_.data_){
			delete[](PS_position_t*)e.disperse_points_.data_;
			e.disperse_points_.data_ = NULL;
		}
	}
};

#endif