///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : ges_navgenerator
//   File        : plane.h
//   Description :
//      [TODO: Write the purpose of plane.h.]
//
//   Created On: 9/19/2009 12:12:32 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef MC_PLANE_H
#define MC_PLANE_H
#ifdef _WIN32
#pragma once
#endif


#include "vector.h"
#include "segment.h"

class CPlane
{
public:
	CPlane(Vector x, Vector y, Vector z);

	Vector getX() const;
	Vector getY() const;
	Vector getZ() const;
	Vector getNormal() const;

	bool findIntersection(const CPlane& p, CSegment &seg);

private:
	Vector m_vX;
	Vector m_vY;
	Vector m_vZ;
};

#endif //MC_PLANE_H
