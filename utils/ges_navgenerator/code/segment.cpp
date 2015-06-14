///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : ges_navgenerator
//   File        : segment.cpp
//   Description :
//      [TODO: Write the purpose of segment.cpp.]
//
//   Created On: 9/19/2009 12:30:47 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#include "segment.h"

CSegment::CSegment()
{
}

CSegment::CSegment(Vector a, Vector b)
{
	m_vA = a;
	m_vB = b;
}

Vector CSegment::getPointOne() const
{
	return m_vA;
}

Vector CSegment::getPointTwo() const
{
	return m_vB;
}

bool CSegment::intersectsLine(Vector a)
{
	Vector p = m_vA - a;
	Vector q = m_vB - a;

	return (abs(p.angle(q)) < 0.0001 );
}

void CSegment::setPointOne(Vector p)
{
	m_vA = p;
}

void CSegment::setPointTwo(Vector p)
{
	m_vB = p;
}

bool CSegment::findIntersection(const CSegment& seg, Vector &point)
{
	Vector x1 = m_vA;
	Vector x2 = m_vB;
	Vector x3 = seg.getPointOne();
	Vector x4 = seg.getPointTwo();

	//if the lines are parallel they never intersect

	Vector q = x1 - x2;
	Vector r = x3 - x4;

	if (q.angle(r) < 0.001)
		return false;




	//http://mathworld.wolfram.com/Line-LineIntersection.html


	Vector a = x2 - x1;
	Vector b = x4 - x3;
	Vector c = x3 - x1;

	double s = (c.cross(b)).dot(a.cross(b)) / ((a.cross(b)).len() * (a.cross(b)).len());

	Vector sres = x1 + (x2-x1)*s;

	double ta = ((sres.getX() - x3.getX()) / (x4.getX()-x3.getX()));
	double tb = ((sres.getY() - x3.getY()) / (x4.getY()-x3.getY()));
	double tc = ((sres.getZ() - x3.getZ()) / (x4.getZ()-x3.getZ()));

	// if min distance between them is greater than zero they never intersect

	double avg = (ta+tb+tc)/3.0;
	double diff = abs(avg - ta);

	if ( diff > 0.001 )
		return false;


	point = sres;
	return true;
}

