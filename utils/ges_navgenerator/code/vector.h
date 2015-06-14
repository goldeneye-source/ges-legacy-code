///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : ges_navgenerator
//   File        : vector.h
//   Description :
//      [TODO: Write the purpose of vector.h.]
//
//   Created On: 9/19/2009 9:44:05 AM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef MC_VECTOR_H
#define MC_VECTOR_H
#ifdef _WIN32
#pragma once
#endif

#include <math.h>
#define PI 3.14159265

class Vector
{
public:
	Vector()
	{
		m_dX = 0;
		m_dY = 0;
		m_dZ = 0;
	}

	Vector(double x, double y, double z)
	{
		m_dX = x;
		m_dY = y;
		m_dZ = z;
	}

	void setX(double x)
	{
		m_dX = x;
	}

	void setY(double y)
	{
		m_dY = y;
	}

	void setZ(double z)
	{
		m_dZ = z;
	}

	double getX() const
	{
		return m_dX;
	}

	double getY() const
	{
		return m_dY;
	}

	double getZ() const
	{
		return m_dZ;
	}

	bool operator==(const Vector& v) const
	{
		double a = abs(getX() - v.getX());
		double b = abs(getY() - v.getY());
		double c = abs(getZ() - v.getZ());

		return (a < 0.0001 && b < 0.0001 && c < 0.0001);
	}

	bool operator!=(const Vector& v) const
	{
		return !(operator==(v));
	}

	Vector operator+(double v) const
	{
		return Vector(getX()+v, getY()+v, getZ()+v);
	}

	Vector operator-(double v) const
	{
		return Vector(getX()-v, getY()-v, getZ()-v);
	}

	Vector operator*(double v) const
	{
		return Vector(getX()*v, getY()*v, getZ()*v);
	}

	Vector operator/(double v) const
	{
		return Vector(getX()/v, getY()/v, getZ()/v);
	}


	Vector operator+(const Vector& v) const
	{
		return Vector(getX()+v.getX(), getY()+v.getY(), getZ()+v.getZ());
	}

	Vector operator-(const Vector& v) const
	{
		return Vector(getX()-v.getX(), getY()-v.getY(), getZ()-v.getZ());
	}

	Vector cross(const Vector& v) const
	{
		double x = (getY() * v.getZ()) - (getZ() * v.getY());
		double y = (getZ() * v.getX()) - (getX() * v.getZ());
		double z = (getX() * v.getY()) - (getY() * v.getX());

		return Vector(x, y, z);
	}

	double len() const
	{
		return sqrt(m_dX*m_dX+m_dY*m_dY+m_dZ*m_dZ);
	}

	double dot(const Vector& v) const
	{
		return getX()*v.getX() + getY()*v.getY() + getZ()*v.getZ();
	}

	double angle(const Vector& v)
	{
		double angR = dot(v) / (len() * v.len());
		double ang = acos(angR)* 180.0 / PI;

		while (ang < -180.0)
			ang += 180.0;

		while (ang > 180.0)
			ang -= 180.0;

		return ang;
	}

	void normalize()
	{
		double l = len();
		m_dX /= l;
		m_dY /= l;
		m_dZ /= l;
	}

private:
	double m_dX;
	double m_dY;
	double m_dZ;
};

#endif //MC_VECTOR_H
