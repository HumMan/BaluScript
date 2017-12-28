#pragma once

#include <vector>
#include <assert.h>

#include <common.h>

#include "../SemanticTreeApi.h"

class TSClass;
class TSMethod;


class TExpressionResult
{
	//���� ������������ �������� - �� ��� ��� � �����������
	TSClass* result_value_type;
	bool is_ref;

	TSClass* type;//��������� �� ��� (��� ������� � ����������� ������ � ��������)

	std::vector<SemanticApi::ISMethod*> methods;//��������� �� ������������ ������ - ��� ��������� �� ������� ��������� (���� ��������� �������� �������)
	void Init();
public:
	TExpressionResult();
	TExpressionResult(std::vector<SemanticApi::ISMethod*> use_methods, bool use_need_push_this);
	TExpressionResult(TSClass* use_class, bool use_is_ref);
	TExpressionResult(TSClass* use_type);
	bool IsRef()const;
	void SetIsRef(bool use_is_ref);
	bool IsMethods()const;
	bool IsType()const;
	TSClass* GetType()const;
	std::vector<SemanticApi::ISMethod*>& GetMethods();
	TSClass* GetClass()const;
	bool IsVoid()const;
};

