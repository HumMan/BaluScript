#pragma once

#include "lexer.h"

#include "Semantic/SMethod.h"
#include "Semantic/SClassField.h"
#include "Semantic/SParameter.h"

void ValidateAccess(TTokenPos* field_pos, TSClass* source, TSClassField* target);
void ValidateAccess(TTokenPos* field_pos, TSClass* source, TSMethod* target);

bool IsEqualClasses(TExpressionResult actual_parameter, TFormalParameter formal_parameter, int& need_conv);

TSMethod* FindMethod(TTokenPos* source, std::vector<TSMethod*> &methods_to_call, const std::vector<TExpressionResult> &formal_params);

void InitializeStaticClassFields(std::vector<TSClassField*> static_fields, std::vector<TStaticValue> &static_objects);
void InitializeStaticVariables(std::vector<TSLocalVar*> static_variables, std::vector<TStaticValue> &static_objects);
