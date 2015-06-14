///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : ges_navgenerator
//   File        : navGenerator.h
//   Description :
//      [TODO: Write the purpose of navGenerator.h.]
//
//   Created On: 9/19/2009 10:39:32 AM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef MC_NAVGENERATOR_H
#define MC_NAVGENERATOR_H
#ifdef _WIN32
#pragma once
#endif

#include "polygon.h"
#include "vector.h"
#include "plane.h"
#include "segment.h"
#include <vector>

typedef std::vector<CPolygon*> PolyPVector;

class NavGenerator
{
public:
	NavGenerator();

	void parseFile(const char* name);
	void saveFile(const char* name);

	bool genNavFile(const char* name);

protected:
	void parseVersionInfo(std::vector<std::string> &tokens);
	void parseViewSettings(std::vector<std::string> &tokens);
	void parseWorld(std::vector<std::string> &tokens);
	void parseEntity(std::vector<std::string> &tokens);
	void parseCameras(std::vector<std::string> &tokens);
	void parseCordon(std::vector<std::string> &tokens);

	void processSolid(std::vector< CPlane >& sideList);


private:
	Vector m_vBL;
	Vector m_vTR;

	std::vector<Vector> m_vSpawnPoints;
	std::vector<CPolygon> m_vFaceList;
};


#endif //MC_NAVGENERATOR_H
