class TVoid:public TFormalParam
{
public:
	TVoid(){}
	TVoid(TOpArray use_ops):TFormalParam(NULL,false,use_ops){}
};