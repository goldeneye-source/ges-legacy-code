///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : ges_navgenerator
//   File        : polygon.cpp
//   Description :
//      [TODO: Write the purpose of polygon.cpp.]
//
//   Created On: 9/19/2009 9:36:21 AM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////


#include "polygon.h"
#include "segment.h"

CPolygon::CPolygon()
{
	m_iHint = NO_HINT;
}

CPolygon::CPolygon(std::vector<Vector> pointList)
{
	for (size_t x=0; x<pointList.size(); x++)
	{
		m_vPointList.push_back(pointList[x]);
	}

	m_iHint = NO_HINT;
}

CPolygon::CPolygon(const CPolygon& poly)
{
	for (size_t x=0; x<poly.getCount(); x++)
	{
		m_vPointList.push_back(poly.getPoint(x));
	}

	m_iHint = NO_HINT;
}

void CPolygon::addPoint(Vector point)
{
	m_vPointList.push_back(point);
}

Vector CPolygon::getNormal()
{
	if (m_vPointList.size() < 3)
		return Vector();

	Vector res = ((m_vPointList[1]-m_vPointList[0]).cross(m_vPointList[2] - m_vPointList[0]));
	res.normalize();
	return res;
}


bool CPolygon::isWall()
{
	Vector n = getNormal();
	Vector a = n;

	a.setY(0);
	double ang = a.angle(n);
	return (ang > 45 || ang < 45);
}


int CPolygon::getHint()
{
	return m_iHint;
}

void CPolygon::setHint(int hint)
{
	m_iHint = hint;
}


void CPolygon::removeDupPoints()
{
	std::vector<bool> removeList;

	removeList.resize(m_vPointList.size());

	for (size_t x=0; x<removeList.size(); x++)
	{
		removeList[x] = false;
	}

	for (size_t x=0; x<m_vPointList.size(); x++)
	{
		if (removeList[x])
			continue;

		for (size_t y=x+1; y<m_vPointList.size(); y++)
		{
			if (removeList[y])
				continue;

			if (m_vPointList[x] == m_vPointList[y])
				removeList[y] = true;
		}
	}

	for (size_t x=0; x<removeList.size(); x++)
	{
		size_t index = (removeList.size()-1) - x;
		if (removeList[index]== true)
		{
			m_vPointList.erase(m_vPointList.begin()+index);
		}
	}
}

bool CPolygon::isValid()
{	
	return true;
}


void CPolygon::orderPoints()
{
	std::vector<CSegment> segList;
	std::vector<CSegment> nonIntersecting;

	for (size_t x=0; x< m_vPointList.size(); x++)
	{
		for (size_t y=x+1; y< m_vPointList.size(); y++)
		{
			segList.push_back(CSegment(m_vPointList[x], m_vPointList[y]));
		}
	}

	for (size_t x=0; x< segList.size(); x++)
	{
		bool ni = true;

		for (size_t y=0; y< segList.size(); y++)
		{
			Vector point;
			if (segList[x].findIntersection(segList[y], point))
			{
				//ignore point intersections
				if (!(point == segList[x].getPointOne()) && !(point == segList[x].getPointTwo()))
				{
					ni = false;
					break;
				}
			}
		}

		if (ni)
			nonIntersecting.push_back(segList[x]);
	}
	
	std::vector<Vector> points;
	size_t c=0;
	size_t pos=0;

	while ( c < nonIntersecting.size() )
	{
		points.push_back(nonIntersecting[pos].getPointOne());

		for (size_t y=0; y< nonIntersecting.size(); y++)
		{
			if (y==pos)
				continue;

			bool o = nonIntersecting[y].getPointOne() == nonIntersecting[pos].getPointTwo();
			bool t = nonIntersecting[y].getPointTwo() == nonIntersecting[pos].getPointTwo();

			bool tDone = false;

			for (size_t z=0; z<points.size(); z++)
			{
				if (points[z] == nonIntersecting[y].getPointTwo())
				{
					tDone = true;
					break;
				}
			}

			if (o || (t && !tDone))
			{
				if (t)
					nonIntersecting[y].swapPoints();

				pos = y;
				break;
			}
		}
		c++;
	}


	m_vPointList.clear();

	for (size_t z=0; z<points.size(); z++)
	{
		m_vPointList.push_back(points[z]);
	}
	
}


void CPolygon::invertNormal()
{
	std::vector<Vector> points;

	for (size_t z=m_vPointList.size(); z>0; z--)
	{
		points.push_back(m_vPointList[z-1]);
	}

	m_vPointList.clear();

	for (size_t z=0; z<points.size(); z++)
	{
		m_vPointList.push_back(points[z]);
	}
}

//if any line from point to point cross they intersect
bool CPolygon::intersects(const CPolygon& p) const
{
	//check to see if any lines intersect
	for (size_t x=0; x< getCount()-1; x++)
	{
		CSegment a(getPoint(x), getPoint(x+1));

		for (size_t y=0; y< p.getCount()-1; y++)
		{
			CSegment b(p.getPoint(y), p.getPoint(y+1));

			Vector res;
			if (a.findIntersection(b, res))
				return true;
		}
	}


	//check to see if one is inside the other
	



	return false;
}