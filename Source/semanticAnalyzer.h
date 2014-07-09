#pragma once

#include "lexer.h"

#include "Semantic\SMethod.h"
#include "Semantic\SClassField.h"
#include "Semantic\FormalParam.h"

void ValidateAccess(TTokenPos* field_pos, TSClass* source, TSClassField* target);
void ValidateAccess(TTokenPos* field_pos, TSClass* source, TSMethod* target);

bool IsEqualClasses(TFormalParam formal_par, TSClass* param_class, bool param_ref, int& need_conv);

//возвращать не количество необходимых преобразований, а их массив TFormalParamWithConversions
//TSMethod* FindMethod(TTokenPos* source, std::vector<TSMethod*> &methods_to_call, const std::vector<TFormalParam> &formal_params) //int& conv_needed);
TSMethod* FindMethod(TTokenPos* source, std::vector<TSMethod*> &methods_to_call, const std::vector<TFormalParam> &formal_params,int& conv_needed);