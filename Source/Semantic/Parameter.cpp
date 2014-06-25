#include "../Syntax/Parameter.h"

int TParameter::GetSize(){
	return is_ref?1:type.GetClass()->GetSize();
}
