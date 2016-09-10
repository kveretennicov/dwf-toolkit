
#include "whiptk/w2d_class_factory.h"

class WhipClassFactoryWrapper
{
private:
	WT_W2D_Class_Factory *m_pWhipClassFactory;

public:
	WhipClassFactoryWrapper();
	~WhipClassFactoryWrapper();

	WT_Class_Factory *const operator -> ();
};

