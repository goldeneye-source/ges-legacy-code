///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : ges_navgenerator
//   File        : segment.h
//   Description :
//      [TODO: Write the purpose of segment.h.]
//
//   Created On: 9/19/2009 12:30:44 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef MC_SEGMENT_H
#define MC_SEGMENT_H
#ifdef _WIN32
#pragma once
#endif

#include "vector.h"

class CSegment
{
public:
	CSegment();
	CSegment(Vector a, Vector b);

	void setPointOne(Vector p);
	void setPointTwo(Vector p);

	Vector getPointOne() const;
	Vector getPointTwo() const;

	bool intersectsLine(Vector a);

	bool findIntersection(const CSegment& seg, Vector &point);

	void swapPoints()
	{
		Vector temp = m_vB;
		m_vB = m_vA;
		m_vA = temp;
	}

private:
	Vector m_vA;
	Vector m_vB;
};

#endif //MC_SEGMENT_H
