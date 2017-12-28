#pragma once

#include "lexer.h"

#include "SMethod.h"
#include "SClassField.h"
#include "SParameter.h"

void ValidateAccess(Lexer::ITokenPos* field_pos, TSClass* source, TSClassField* target);
void ValidateAccess(Lexer::ITokenPos* field_pos, TSClass* source, TSMethod* target);

bool IsEqualClasses(TExpressionResult actual_parameter, TFormalParameter formal_parameter, int& need_conv);

TSMethod* FindMethod(Lexer::ITokenPos* source, std::vector<TSMethod*> &methods_to_call, const std::vector<TExpressionResult> &formal_params);

void InitializeStaticClassFields(std::vector<TSClassField*> static_fields, std::vector<TStaticValue> &static_objects);
void InitializeStaticVariables(std::vector<TSLocalVar*> static_variables, std::vector<TStaticValue> &static_objects);
