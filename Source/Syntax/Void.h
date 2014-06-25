#pragma once

#include "FormalParam.h"

class TVoid:public TFormalParam
{
public:
	TVoid(){}
	TVoid(TOpArray use_ops):TFormalParam(NULL,false,use_ops){}
};