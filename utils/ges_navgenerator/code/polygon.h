///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : ges_navgenerator
//   File        : polygon.h
//   Description :
//      [TODO: Write the purpose of polygon.h.]
//
//   Created On: 9/19/2009 9:33:56 AM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef MC_POLYGON_H
#define MC_POLYGON_H
#ifdef _WIN32
#pragma once
#endif

#include "vector.h"
#include <vector>

class CPolygon
{
public:
	CPolygon();
	CPolygon(std::vector<Vector> pointList);
	CPolygon(const CPolygon& poly);

	void addPoint(Vector point);

	Vector getNormal();
	bool isWall();
	bool isValid();

	int getHint();
	void setHint(int hint);

	void removeDupPoints();
	void orderPoints();
	void invertNormal();

	size_t getCount() const
	{
		return m_vPointList.size();
	}

	const Vector& getPoint(size_t index) const
	{
		return m_vPointList[index];
	}

	enum
	{
		NO_HINT = 0,
		DUCK,
		USE,
	};

	bool intersects(const CPolygon& p) const;

private:
	int m_iHint;

	std::vector<Vector> m_vPointList;
};

#endif //MC_POLYGON_H
