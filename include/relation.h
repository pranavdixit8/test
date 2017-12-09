/**
	@file 		relation.h
	@author 	Hamid Homapour (hamid_homapour@sfu.ca)
	@date		December 3, 2017
	@version 	1.0

	@brief		CMPT-764: Project, Header file for Relation class.
*/

#ifndef RELATION_H
#define RELATION_H

#include <part_base.h>
#include <vector>
#include <math.h>

using std::vector;

class Relation {

	double epsilon = 0.0001;

public:

	enum CoPlanarityType {TOP, BOTTOM, LEFT, RIGHT, BACK, FRONT, NONE};

	BoundingBox fitAxisAlignedCuboid (PartBase *thePart);

	std::pair <Relation::CoPlanarityType,Relation::CoPlanarityType> checkAxisAlignedCoPlanarity (PartBase *part1, PartBase *part2);
	Vector checkAxisAlignedCoCentricity (PartBase *part1, PartBase *part2);


};

BoundingBox Relation::fitAxisAlignedCuboid (PartBase* thePart) {
		// todo: CGAL::Iso_cuboid_3 is the same as BoundingBox, but with two points as the handles, we can implement using this
		return thePart->boundingBox;
}	

vector<std::pair <Relation::CoPlanarityType,Relation::CoPlanarityType>> Relation::checkAxisAlignedCoPlanarity (PartBase *part1, PartBase *part2) {

	BoundingBox bb1 = part1->boundingBox;
	BoundingBox bb2 = part2->boundingBox;

	vector<std::pair <Relation::CoPlanarityType,Relation::CoPlanarityType>> result;

	// ------------------------------------
	// check TOP coplanarity
	if(fabs(bb1.zmax() - bb2.zmax()) < epsilon)
		result.push_back( std::make_pair(Relation::TOP, Relation::TOP) );

	// check BOTTOM coplanarity
		if(fabs(bb1.zmin() - bb2.zmin()) < epsilon)
			result.push_back( std::make_pair(Relation::BOTTOM, Relation::BOTTOM) );

	// check TOP from first and BOTTOM from second coplanarity
	if(fabs(bb1.zmax() - bb2.zmin()) < epsilon)
		result.push_back( std::make_pair(Relation::TOP, Relation::BOTTOM) );


	// check BOTTOM from first and TOP from second coplanarity
	if(fabs(bb1.zmax() - bb2.zmin()) < epsilon)
		result.push_back( std::make_pair(Relation::BOTTOM, Relation::TOP) );

	// ------------------------------------

	// check RIGHT coplanarity
	if(fabs(bb1.xmax() - bb2.xmax()) < epsilon)
		result.push_back( std::make_pair(Relation::RIGHT, Relation::RIGHT) );

	// check LEFT coplanarity
	if(fabs(bb1.xmin() - bb2.xmin()) < epsilon)
		result.push_back( std::make_pair(Relation::LEFT, Relation::LEFT) );

	// check RIGHT from first and LEFT from second coplanarity
	if(fabs(bb1.xmax() - bb2.xmin()) < epsilon)
		result.push_back( std::make_pair(Relation::RIGHT, Relation::LEFT) );

	// check LEFT from first and RIGHT from second coplanarity
	if(fabs(bb1.xmin() - bb2.xmax()) < epsilon)
		result.push_back( std::make_pair(Relation::LEFT, Relation::RIGHT) );

	// ------------------------------------

	// check FRONT coplanarity
	if(fabs(bb1.ymax() - bb2.ymax()) < epsilon)
		result.push_back( std::make_pair(Relation::FRONT, Relation::FRONT) );

	// check BACK coplanarity
	if(fabs(bb1.ymin() - bb2.ymin()) < epsilon)
		result.push_back( std::make_pair(Relation::BACK, Relation::BACK) );

	// check FRONT from first and BACK from second coplanarity
	if(fabs(bb1.ymax() - bb2.ymin()) < epsilon)
		result.push_back( std::make_pair(Relation::FRONT, Relation::BACK) );

	// check BACK from first and FRONT from second coplanarity
	if(fabs(bb1.ymin() - bb2.ymax()) < epsilon)
		result.push_back( std::make_pair(Relation::BACK, Relation::FRONT) );

	//return std::make_pair(Relation::NONE, Relation::NONE);
	return result;
}

Vector Relation::checkAxisAlignedCoCentricity(PartBase *part1, PartBase *part2) {

	double x1_mid = (part1->boundingBox.xmin() + part1->boundingBox.xmax())/2;
	double y1_mid = (part1->boundingBox.ymin() + part1->boundingBox.ymax())/2;
	double z1_mid = (part1->boundingBox.zmin() + part1->boundingBox.zmax())/2;

	double x2_mid = (part2->boundingBox.xmin() + part2->boundingBox.xmax())/2;
	double y2_mid = (part2->boundingBox.ymin() + part2->boundingBox.ymax())/2;
	double z2_mid = (part2->boundingBox.zmin() + part2->boundingBox.zmax())/2;
//
//	Point top_1		(x1_mid, y1_mid, part1->boundingBox.zmax());
//	Point bottom_1	(x1_mid, y1_mid, part1->boundingBox.zmin());
//	Point right_1	(part1->boundingBox.xmax(), y1_mid, z1_mid);
//	Point bottom_1	(part1->boundingBox.xmin(), y1_mid, z1_mid);
//	Point front_1	(x1_mid, part1->boundingBox.ymax(), z1_mid);
//	Point back_1	(x1_mid, part1->boundingBox.ymin(), z1_mid);
//
//
//	Point top_2		(x2_mid, y2_mid, part2->boundingBox.zmax());
//	Point bottom_2	(x2_mid, y2_mid, part2->boundingBox.zmin());
//	Point right_2	(part2->boundingBox.xmax(), y2_mid, z2_mid);
//	Point bottom_2	(part2->boundingBox.xmin(), y2_mid, z2_mid);
//	Point front_2	(x2_mid, part2->boundingBox.ymax(), z2_mid);
//	Point back_2	(x2_mid, part2->boundingBox.ymin(), z2_mid);


	Point_2 xy_1(x1_mid, y1_mid);
	Point_2 xy_2(x2_mid, y2_mid);
	if (CGAL::squared_distance(xy_1,xy_2) < epsilon)
		return Vector(0, 0, 1);

	Point_2 yz_1(y1_mid, z1_mid);
	Point_2 yz_2(y2_mid, z2_mid);
	if (CGAL::squared_distance(yz_1,yz_2) < epsilon)
		return Vector(1, 0, 0);

	Point_2 xz_1(x1_mid, z1_mid);
	Point_2 xz_2(x2_mid, z2_mid);
	if (CGAL::squared_distance(xz_1,xz_2) < epsilon)
		return Vector(0, 1, 0);

	return Vector();
}


#endif
