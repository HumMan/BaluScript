#include "../Syntax/Parameter.h"

#include "../Syntax/Method.h"
#include "../Syntax/Statements.h"

int TParameter::GetSize(){
	return is_ref?1:type.GetClass()->GetSize();
}
