///////////// Copyright � 2010, Goldeneye: Source. All rights reserved. /////////////
// 
// File: ge_vieweffects.h
// Description:
//      Adds client view effects for GE (namely breathing effect)
//
// Created On: 12 Jan 2010
// Created By: Jonathan White <killermonkey> 
/////////////////////////////////////////////////////////////////////////////
#ifndef GE_VIEWEFFECTS_H
#define GE_VIEWEFFECTS_H
#ifdef _WIN32
#pragma once
#endif

#include "view_effects.h"

#define NUM_ATTRACTORS 3

struct attractor_t
{
	Vector attr;
	float spawntime;
	float stepsize;
};

class CGEViewEffects : public CViewEffects
{
public:
	CGEViewEffects();

	virtual void	LevelInit( void );
	virtual void	ApplyBreath( void );
	virtual void	BreathMouseMove( float *x, float *y );

	virtual void	ResetBreath( void );

protected:
	void	GenerateRandomAttractor( attractor_t &attr );

private:
	int			m_iNextAttractor;
	float		m_flAttractorTimeout;
	attractor_t	m_vAttractors[NUM_ATTRACTORS];
	Vector		m_vOldBO;
	Vector		m_vBO;

	float		m_flPixelScale;
};

inline CGEViewEffects *GEViewEffects( void ) {
	return (CGEViewEffects*)vieweffects;
}

#endif