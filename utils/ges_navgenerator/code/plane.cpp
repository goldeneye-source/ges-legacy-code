///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : ges_navgenerator
//   File        : plane.cpp
//   Description :
//      [TODO: Write the purpose of plane.cpp.]
//
//   Created On: 9/19/2009 12:12:34 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////


#include "plane.h"
#include <stdio.h>

CPlane::CPlane(Vector x, Vector y, Vector z)
{
	m_vX = x;
	m_vY = y;
	m_vZ = z;
}


Vector CPlane::getX() const
{
	return m_vX;
}


Vector CPlane::getY() const
{
	return m_vY;
}

Vector CPlane::getZ() const
{
	return m_vZ;
}

Vector CPlane::getNormal() const
{
	Vector res = ((m_vY-m_vX).cross(m_vZ - m_vX));
	res.normalize();
	return res;
}

Vector solveMatrix(const Vector& a, const Vector& b, int el, double k1, double k2);

bool CPlane::findIntersection(const CPlane& p, CSegment &seg)
{
	Vector norm = getNormal();
	Vector pNorm = p.getNormal();
	Vector cross = norm.cross(pNorm);

	//planes dont intersect if parallel
	if (cross.len() < 0.001)
		return false;

	double k1 = norm.dot(getX());
	double k2 = pNorm.dot(p.getX());

	int element = 0;

	double a[3];

	a[0] = cross.getX();
	a[1] = cross.getY();
	a[2] = cross.getZ();

	for (int x=0; x<3; x++)
	{
		if (abs(a[x]) > abs(a[element]))
			element = x;
	}


	Vector res = solveMatrix(norm, pNorm, element, k1, k2);

	seg.setPointOne( res + cross * 0 );
	seg.setPointTwo( res + cross * 1 );

	return true;
}

//http://softsurfer.com/Archive/algorithm_0104/algorithm_0104B.htm

Vector solveMatrix(const Vector& a, const Vector& b, int el, double k1, double k2)
{
	double d;
	double e;
	double f;
	double g;
	double h = k1;
	double i = k2;

	
	if (el == 0)		//leave off x
	{
		d = a.getY();
		e = a.getZ();

		f = b.getY();
		g = b.getZ();
	}
	else if (el == 1)	//leave off y 
	{
		d = a.getX();
		e = a.getZ();

		f = b.getX();
		g = b.getZ();
	}
	else				//leave off z
	{
		d = a.getX();
		e = a.getY();

		f = b.getX();
		g = b.getY();
	}

	double r1 = (e*k2-g*k1)/(d*g-f*e);
	double r2 = (f*k1-d*k2)/(d*g-f*e);

	Vector res;

	if (el == 0)		//leave off x
	{
		res = Vector(0, r1, r2);
	}
	else if (el == 1)	//leave off y 
	{
		res = Vector(r1, 0, r2);
	}
	else				//leave off z
	{
		res = Vector(r1, r2, 0);
	}
	
	return res;
}