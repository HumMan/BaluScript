#pragma once

#include "lexer.h"

#include "SMethod.h"
#include "SClassField.h"
#include "SParameter.h"

void ValidateAccess(Lexer::ITokenPos* field_pos, SemanticApi::ISClass* source, TSClassField* target);
void ValidateAccess(Lexer::ITokenPos* field_pos, SemanticApi::ISClass* source, SemanticApi::ISMethod* target);

bool IsEqualClasses(TExpressionResult actual_parameter, SemanticApi::TFormalParameter formal_parameter, int& need_conv);

SemanticApi::ISMethod* FindMethod(Lexer::ITokenPos* source, std::vector<SemanticApi::ISMethod*> &methods_to_call, const std::vector<TExpressionResult> &formal_params);

void InitializeStaticClassFieldsOffset(std::vector<TSClassField*> static_fields);
void InitializeStaticVariablesOffset(std::vector<TSLocalVar*> static_variables);
