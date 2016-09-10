
#include "StdAfx.h"
#include "WhipClassFactoryWrapper.h"

WhipClassFactoryWrapper::WhipClassFactoryWrapper()
{
	// Create a new instance of the Whip Class Factory
	m_pWhipClassFactory = new WT_W2D_Class_Factory();
}

WhipClassFactoryWrapper::~WhipClassFactoryWrapper()
{
	// Delete the Whip Class Factory object
	delete m_pWhipClassFactory;
}

WT_Class_Factory *const WhipClassFactoryWrapper::operator -> ()
{
	return m_pWhipClassFactory;
}
