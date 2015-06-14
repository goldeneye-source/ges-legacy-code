///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : ges_navgenerator
//   File        : navGenerator.cpp
//   Description :
//      [TODO: Write the purpose of navGenerator.cpp.]
//
//   Created On: 9/19/2009 10:41:06 AM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#define uint64 unsigned long long
#define uint32 unsigned int
#define uint8 unsigned char
#define HANDLE void* 

#include "navGenerator.h"
#include "util_fs.h"

#include <exception>
#include <string>
#include <map>

#include "Recast\Include\Recast.h"

using namespace UTIL::FS;

typedef struct
{
	std::string name;
	std::vector<std::string> tokens;
} TokenData;

const char* SpawnNames [] =
{
"info_player_deathmatch",
"info_player_mi6",
"info_player_janus",
"info_player_start",
NULL
};

bool IsSpawnEntity(const char* name)
{
	size_t x=0;
	while (SpawnNames[x] != NULL)
	{
		if (strcmp(name, SpawnNames[x]) == 0)
			return true;
		x++;
	}

	return false;
}

void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " ")
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
		std::string sub = str.substr(lastPos, pos - lastPos);

		// Found a token, add it to the vector.
		if (sub.length() != 0)
			tokens.push_back(sub);

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

void Group(std::vector<std::string>& tokens, std::vector<TokenData*>& groups, std::vector<std::string>& delimiters)
{
	TokenData *curData = NULL;
	
	for (size_t t=0; t<tokens.size(); t++)
	{
		bool tokenHeader = false;

		for (size_t d=0; d<delimiters.size(); d++)
		{
			if (tokens[t] == delimiters[d])
			{
				if (curData)
					groups.push_back(curData);

				curData = new TokenData;
				curData->name = tokens[t];


				tokenHeader = true;
				break;
			}
		}

		if (tokenHeader)
			continue;

		if (!curData)
		{
			curData = new TokenData;
			curData->name = "__default__";
		}

		if (tokens[t] != " ")
			curData->tokens.push_back(tokens[t]);
	}

	if (curData)
		groups.push_back(curData);
}

NavGenerator::NavGenerator()
{
}

void NavGenerator::parseFile(const char* name)
{
	std::vector<std::string> tokens;
	std::vector<TokenData*> groups;
	
	char* buff = NULL;
	uint32 size = 0;

	try
	{
		UTIL_readWholeFile(name, &buff);
	}
	catch (std::exception &e)
	{
		printf("Failed to parse file! %s\n", e.what());
		exit(-1);
	}

	Tokenize(std::string(buff), tokens, "{}\"\n\r\t");

	delete [] buff;

	size_t x=0;

	std::vector<std::string> delimiters;

	delimiters.push_back("versioninfo");
	delimiters.push_back("viewsettings");
	delimiters.push_back("world");
	delimiters.push_back("entity");
	delimiters.push_back("cameras");
	delimiters.push_back("cordon");

	Group(tokens, groups, delimiters);

	for (size_t x=0; x<groups.size(); x++)
	{
		if (groups[x]->name == "versioninfo")
		{
			parseVersionInfo(groups[x]->tokens);
		}
		else if (groups[x]->name == "viewsettings")
		{
			parseViewSettings(groups[x]->tokens);
		}
		else if (groups[x]->name == "world")
		{
			parseWorld(groups[x]->tokens);
		}
		else if (groups[x]->name == "entity")
		{
			parseEntity(groups[x]->tokens);
		}
		else if (groups[x]->name == "cameras")
		{
			parseCameras(groups[x]->tokens);
		}
		else if (groups[x]->name == "cordon")
		{
			parseCordon(groups[x]->tokens);
		}
	}

	Vector res =  m_vTR - m_vBL;
	printf( "World size: %.2f by %.2f by %.2f\n", res.getX(), res.getY(), res.getZ());
}


void NavGenerator::parseVersionInfo(std::vector<std::string> &tokens)
{

}

void NavGenerator::parseViewSettings(std::vector<std::string> &tokens)
{

}

void NavGenerator::parseWorld(std::vector<std::string> &tokens)
{
	std::vector<TokenData*> groups;
	std::vector<std::string> delimiters;
	delimiters.push_back("solid");

	Group(tokens, groups, delimiters);

	for (size_t x=0; x<groups.size(); x++)
	{
		if (groups[x]->name == "solid")
		{
			//printf("Found Solid!\n");

			std::vector<TokenData*> solidGroups;
			std::vector<std::string> solidDelimiters;
			solidDelimiters.push_back("side");

			Group(groups[x]->tokens, solidGroups, solidDelimiters);

			std::vector< CPlane > sideList;

			for (size_t x=0; x<solidGroups.size(); x++)
			{
				if (solidGroups[x]->name == "side")
				{
					std::string plane;

					for (size_t y=0; x<solidGroups[x]->tokens.size(); y++)
					{
						if (solidGroups[x]->tokens[y] == "plane")
						{
							plane = solidGroups[x]->tokens[y+1];
							break;
						}
					}

					std::vector<std::string> planePos;
					Tokenize(plane, planePos, "() ");

					if (planePos.size() == 9)
					{
						Vector a[3];

						for (size_t y=0; y<3; y++)
						{
							int z = y*3;
							double i = atof(planePos[z+0].c_str());
							double j = atof(planePos[z+1].c_str());
							double k = atof(planePos[z+2].c_str());

							a[y] = Vector(i,j,k);

						}

						sideList.push_back(CPlane(a[0], a[1], a[2]));
					}
				}
			}

			processSolid(sideList);
		}
	}

}

void NavGenerator::parseEntity(std::vector<std::string> &tokens)
{
	std::string classname;
	std::string pos;

	for (size_t x=0; x<tokens.size(); x++)
	{
		if (tokens[x] == "classname")
			classname = tokens[x+1];
		else if (tokens[x] == "origin")
			pos = tokens[x+1];
	}

	if ( IsSpawnEntity(classname.c_str()) )
	{
		std::vector<std::string> posList;
		Tokenize(pos, posList, " ");

		if (posList.size() == 3)
		{
			printf("Found spawn spot!\n");

			double x = atof(posList[0].c_str());
			double y = atof(posList[1].c_str());
			double z = atof(posList[2].c_str());

			m_vSpawnPoints.push_back(Vector(x,y,z));
		}
	}
}

void NavGenerator::parseCameras(std::vector<std::string> &tokens)
{

}

void NavGenerator::parseCordon(std::vector<std::string> &tokens)
{

}


void NavGenerator::processSolid(std::vector< CPlane >& sideList)
{
	for (size_t x=0; x<sideList.size(); x++)
	{
		CPolygon poly;

		Vector norm = sideList[x].getNormal();
		std::vector< CPlane > planeList;

		//remove prallel sides from current side
		for (size_t y=0; y<sideList.size(); y++)
		{
			if (x==y)
				continue;

			Vector norm2 = sideList[y].getNormal();
			double ang = norm.angle(norm2);

			if (ang > 0.001 && ang < 179.999)
				planeList.push_back(sideList[y]);
		}


		std::vector<CSegment> setList;
		
		//find intersection lines of every side to current side
		for (size_t y=0; y<planeList.size(); y++)
		{
			CSegment seg;
			if (sideList[x].findIntersection(planeList[y], seg))
			{
				setList.push_back(seg);
			}
		}

		//find instersection points of every line
		for (size_t y=0; y<setList.size(); y++)
		{
			for (size_t z=0; z<setList.size(); z++)
			{
				if (y==z)
					continue;

				Vector point;
				if (setList[y].findIntersection(setList[z], point))
				{
					poly.addPoint(point);
				}
			}
		}

		//remove dup points
		poly.removeDupPoints();
		poly.orderPoints();


		Vector n1 = poly.getNormal();
		if (n1 != norm)
			poly.invertNormal();

		if (poly.isValid())
			m_vFaceList.push_back(poly);


		for (size_t x=0; x<poly.getCount(); x++)
		{
			Vector a = poly.getPoint(x);

			if (a.getX() < m_vBL.getX() )
				m_vBL.setX(a.getX());

			if (a.getX() > m_vTR.getX() )
				m_vTR.setX(a.getX());


			if (a.getY() < m_vBL.getY() )
				m_vBL.setY(a.getY());

			if (a.getY() > m_vTR.getY() )
				m_vTR.setY(a.getY());

			if (a.getZ() < m_vBL.getZ() )
				m_vBL.setZ(a.getZ());

			if (a.getZ() > m_vTR.getZ() )
				m_vTR.setZ(a.getZ());
		}
	}
}




int findPoint(std::vector<Vector> &pointList, const Vector &point)
{
	for (size_t x=0; x<pointList.size(); x++)
	{
		if (pointList[x] == point)
			return x+1;
	}

	return -1;
}

int index = 1;

int findPoint(std::map< double, std::map< double, std::map<double, int> > > &pointList, const Vector &point)
{
	double a = point.getX();
	double b = point.getY();
	double c = point.getZ();

	int pos = pointList[a][b][c];

	if (pos == 0)
	{
		pointList[a][b][c] = index;
		index++;
		return -1;
	}
	
	return pos;
}



void NavGenerator::saveFile(const char* name)
{
	index = 1;

	FILE* fh = fopen(name, "w");

	std::map< double, std::map< double, std::map<double, int> > > pointList;

	for (size_t x=0; x<m_vFaceList.size(); x++)
	{
		if (m_vFaceList[x].getCount() < 3)
			continue;

		for (size_t y=0; y<m_vFaceList[x].getCount(); y++)
		{
			Vector v = m_vFaceList[x].getPoint(y);

			if ( findPoint(pointList, v) == -1)
			{
				fprintf(fh, "v %f %f %f\n", v.getX(), v.getZ(), v.getY());
			}
		}
	}

	for (size_t x=0; x<m_vFaceList.size(); x++)
	{
		if (m_vFaceList[x].getCount() < 3)
			continue;

		fprintf(fh, "f");

		for (size_t y=0; y<m_vFaceList[x].getCount(); y++)
		{
			Vector v = Vector(m_vFaceList[x].getPoint(y));
			fprintf(fh, " %d", findPoint(pointList, v));
		}

		fprintf(fh, "\n");
	}

	fclose(fh);
}


typedef struct
{
	int a;
	int b;
	int c;
} threeVert;


void saveMesh(float* verts, int nverts, int* tris, int ntris, const char* file)
{
	FILE* fh = fopen(file, "w");

	for (int x=0; x<nverts; x++)
	{
		const float* v = &verts[x*3];
		fprintf(fh, "v %f %f %f\n", v[0]/100.0, v[1]/100.0, v[2]/100.0);
	}

	for (int x=0; x<ntris; x++)
	{
		fprintf(fh, "f %d %d %d\n", tris[x*3+0]+1, tris[x*3+1]+1, tris[x*3+2]+1);
	}

	fclose(fh);
}

bool NavGenerator::genNavFile(const char* name)
{
	index = 1;

	float* m_verts;
	int* m_tris;


	printf("1: Initialize build config...\n");

	std::map< double, std::map< double, std::map<double, int> > > pointList;
	std::vector<Vector> vectList;

	size_t count = 0;

	for (size_t x=0; x<m_vFaceList.size(); x++)
	{
		if (m_vFaceList[x].getCount() < 3)
			continue;

		for (size_t y=0; y<m_vFaceList[x].getCount(); y++)
		{
			Vector v = m_vFaceList[x].getPoint(y);
			if (findPoint(pointList, v) == -1)
				count++;
		}
	}

	vectList.resize(count);

	for (size_t x=0; x<m_vFaceList.size(); x++)
	{
		if (m_vFaceList[x].getCount() < 3)
			continue;

		for (size_t y=0; y<m_vFaceList[x].getCount(); y++)
		{
			Vector v = m_vFaceList[x].getPoint(y);

			int index = findPoint(pointList, v)-1;
			vectList[index] = v;
		}
	}

	m_verts = new float[3*vectList.size()];

	for (size_t x=0; x<vectList.size(); x++)
	{
		m_verts[x*3 + 0] = (float)vectList[x].getX();
		m_verts[x*3 + 1] = (float)vectList[x].getZ();
		m_verts[x*3 + 2] = (float)vectList[x].getY();
	}

	
	std::vector<threeVert> triList;


	for (size_t x=0; x<m_vFaceList.size(); x++)
	{
		if (m_vFaceList[x].getCount() < 3)
			continue;

		threeVert tv;
		tv.a = findPoint(pointList, m_vFaceList[x].getPoint(0))-1;
		tv.b = findPoint(pointList, m_vFaceList[x].getPoint(1))-1;
		tv.c = findPoint(pointList, m_vFaceList[x].getPoint(2))-1;

		if (tv.b == tv.c || tv.a == tv.b || tv.a == tv.c)
			continue;

		triList.push_back(tv);

		for (size_t y=3; y<m_vFaceList[x].getCount(); y++)
		{
			tv.b = tv.c;
			tv.c = findPoint(pointList, m_vFaceList[x].getPoint(y))-1;

			if (tv.c != tv.b)
				triList.push_back(tv);
		}
	}

	m_tris = new int[3*triList.size()];

	for (size_t x=0; x<triList.size(); x++)
	{
		m_tris[x*3 + 0] = triList[x].a;
		m_tris[x*3 + 1] = triList[x].b;
		m_tris[x*3 + 2] = triList[x].c;
	}

	int m_nverts = vectList.size();
	int m_ntris = triList.size();


	saveMesh(m_verts, m_nverts, m_tris, m_ntris, "test_mesh.obj");

	if (!m_verts || ! m_tris)
	{
		printf("buildNavigation: Input mesh is not specified.");
		return false;
	}
	

	
	//
	// Step 1. Initialize build config.
	//
	
	//// The units of the parameters are specified in parenthesis as follows:
	//// (vx) voxels, (wu) world units
	//struct rcConfig
	//{
	//	int width, height;				// Dimensions of the rasterized heighfield (vx)
	//	int tileSize;					// Width and Height of a tile (vx)
	//	int borderSize;					// Non-navigable Border around the heightfield (vx)
	//	float cs, ch;					// Grid cell size and height (wu)
	//	float bmin[3], bmax[3];			// Grid bounds (wu)
	//	float walkableSlopeAngle;		// Maximum walkble slope angle in degrees.
	//	int walkableHeight;				// Minimum height where the agent can still walk (vx)
	//	int walkableClimb;				// Maximum height between grid cells the agent can climb (vx)
	//	int walkableRadius;				// Radius of the agent in cells (vx)
	//	int maxEdgeLen;					// Maximum contour edge length (vx)
	//	float maxSimplificationError;	// Maximum distance error from contour to cells (vx)
	//	int minRegionSize;				// Minimum regions size. Smaller regions will be deleted (vx)
	//	int mergeRegionSize;			// Minimum regions size. Smaller regions will be merged (vx)
	//	int maxVertsPerPoly;			// Max number of vertices per polygon
	//	float detailSampleDist;			// Detail mesh sample spacing.
	//	float detailSampleMaxError;		// Detail mesh simplification max sample error.
	//};


	rcConfig m_cfg;

	// Init build configuration from GUI
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = 2.0f;
	m_cfg.ch = 2.0f;
	m_cfg.walkableSlopeAngle = 45;
	//m_cfg.walkableHeight = 32;
	//m_cfg.walkableClimb = 16;
	//m_cfg.walkableRadius = 16;

	m_cfg.walkableHeight = (int)ceilf(72.0f / m_cfg.ch);
	m_cfg.walkableClimb = (int)ceilf(16.0f / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(16.0f / m_cfg.cs);

	//m_cfg.maxEdgeLen = 2048;
	//m_cfg.maxSimplificationError = 2;
	//m_cfg.minRegionSize = 32;
	//m_cfg.mergeRegionSize = 32;
	//m_cfg.maxVertsPerPoly = 4;
	//m_cfg.detailSampleDist = 6;
	//m_cfg.detailSampleMaxError = 1;

	m_cfg.maxEdgeLen = (int)(2048 / m_cfg.cs);
	m_cfg.maxSimplificationError = 1.3f;
	m_cfg.minRegionSize = (int)rcSqr(2);
	m_cfg.mergeRegionSize = (int)rcSqr(32);
	m_cfg.maxVertsPerPoly = (int)6;
	m_cfg.detailSampleDist = m_cfg.cs * 6;
	m_cfg.detailSampleMaxError = m_cfg.ch * 1;


	
	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcCalcBounds(m_verts, m_nverts, m_cfg.bmin, m_cfg.bmax);
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	//
	//{
	//	rcGetLog()->log(RC_LOG_PROGRESS, "Building navigation:");
	//	rcGetLog()->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
	//	rcGetLog()->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", m_nverts/1000.0f, m_ntris/1000.0f);
	//}
	
	//
	// Step 2. Rasterize input polygon soup.
	//
	printf("2: Rasterize input polygon soup...\n");
	// Allocate voxel heighfield where we rasterize our input data to.
	rcHeightfield m_solid;
	if (!rcCreateHeightfield(m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		printf("buildNavigation: Could not create solid heightfield.");
		return false;
	}
	
	// Allocate array that can hold triangle flags.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	unsigned char* m_triflags = new unsigned char[m_ntris];
	if (!m_triflags)
	{
		printf("buildNavigation: Out of memory 'triangleFlags' (%d).", m_ntris);
		return false;
	}
	
	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the flags for each of the meshes and rasterize them.
	memset(m_triflags, 0, m_ntris*sizeof(unsigned char));
	rcMarkWalkableTriangles(m_cfg.walkableSlopeAngle, m_verts, m_nverts, m_tris, m_ntris, m_triflags);
	rcRasterizeTriangles(m_verts, m_nverts, m_tris, m_triflags, m_ntris, m_solid);

	delete [] m_triflags;
	m_triflags = 0;
	
	//
	// Step 3. Filter walkables surfaces.
	//
	printf("3: Filter walkables surfaces...\n");
	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLedgeSpans(m_cfg.walkableHeight, m_cfg.walkableClimb, m_solid);
	rcFilterWalkableLowHeightSpans(m_cfg.walkableHeight, m_solid);

	//
	// Step 4. Partition walkable surface to simple regions.
	//
	printf("4: Partition walkable surface to simple regions...\n");
	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	rcCompactHeightfield m_chf;

	if (!rcBuildCompactHeightfield(m_cfg.walkableHeight, m_cfg.walkableClimb, RC_WALKABLE, m_solid, m_chf))
	{
		printf("buildNavigation: Could not build compact data.");
		return false;
	}
	
	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(m_chf))
	{
		printf("buildNavigation: Could not build distance field.");
		return false;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(m_chf, m_cfg.walkableRadius, m_cfg.borderSize, m_cfg.minRegionSize, m_cfg.mergeRegionSize))
	{
		printf("buildNavigation: Could not build regions.");
	}
	
	//
	// Step 5. Trace and simplify region contours.
	//
	printf("5: Trace and simplify region contours...\n");
	// Create contours.
	rcContourSet m_cset;
	if (!rcBuildContours(m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, m_cset))
	{
		printf("buildNavigation: Could not create contours.");
		return false;
	}
	
	//
	// Step 6. Build polygons mesh from contours.
	//
	printf("6: Build polygons mesh from contours...\n");
	// Build polygon navmesh from the contours.
	rcPolyMesh m_pmesh;
	if (!rcBuildPolyMesh(m_cset, m_cfg.maxVertsPerPoly, m_pmesh))
	{
		printf("buildNavigation: Could not triangulate contours.");
		return false;
	}
	
	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//
	printf("7: Creating detail mesh...\n");

	rcPolyMeshDetail m_dmesh;
	if (!rcBuildPolyMeshDetail(m_pmesh, m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, m_dmesh))
	{
		printf("buildNavigation: Could not build detail mesh.");
	}


	printf("Saving file...\n");

	FILE* fh = fopen(name, "w");




	for (int i = 0; i < m_dmesh.nmeshes; ++i)
	{
		const unsigned short* m = &m_dmesh.meshes[i*4];
		const unsigned short bverts = m[0];
		const unsigned short btris = m[2];
		const unsigned short ntris = m[3];
		const float* verts = &m_dmesh.verts[bverts*3];
		const unsigned char* tris = &m_dmesh.tris[btris*4];

		fprintf(fh, "----\n");

		for (int j = 0; j < ntris; ++j)
		{
			const float* vert1 = &verts[tris[j*4+0]*3];
			const float* vert2 = &verts[tris[j*4+1]*3];
			const float* vert3 = &verts[tris[j*4+2]*3];

			fprintf(fh, "f %f %f %f\n", vert1[0], vert1[1], vert1[2]);
			fprintf(fh, "f %f %f %f\n", vert2[0], vert2[1], vert2[2]);
			fprintf(fh, "f %f %f %f\n", vert3[0], vert3[1], vert3[2]);
		}
	}


	/*for (int z=0; z<m_dmesh.nmeshes; z++)
	{
		short*meshData = m_dmesh.meshes[z*sizeof(short)*4];

		short vbase = meshData[0];
		short vcount = meshData[1];
		short tbase = meshData[2];
		short tcount = meshData[3];

		const unsigned char* t = m_dmesh.tris[(tbase+z)*3]; and
		
		for (int x=0; x<vcount; x++)
		{
			const float* v = m_dmesh.verts[(vbase+t[x])*3];
			fprintf(fh, "v %f %f %f\n", v[0], v[1], v[2]);
		}

		for (int x=0; x<tcount; x++)
		{
			fprintf(fh, "f %d %d %d\n", m_dmesh.tris[x*3+0]+1, m_dmesh.tris[x*3+1]+1, m_dmesh.tris[x*3+2]+1);
		}

	}*/

	fclose(fh);



	printf("Total Meshes: %d\n", m_dmesh.nmeshes);
	printf("Total Tris: %d\n", m_dmesh.ntris);
	printf("Total Verts: %d\n", m_dmesh.nverts);


	// At this point the navigation mesh data is ready, you can access it from m_pmesh.
	// See rcDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.
	
	return true;
}








