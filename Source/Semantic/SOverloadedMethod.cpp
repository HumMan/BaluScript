#include "SOverloadedMethod.h"

#include "SMethod.h"
#include "../Syntax/Method.h"
#include "../Syntax/OverloadedMethod.h"
#include "SStatements.h"
#include "../Syntax/Statements.h"

TSOverloadedMethod::TSOverloadedMethod(TSClass* use_owner, SyntaxApi::IOverloadedMethod* use_syntax)
	:linked_signature(false), linked_body(false), TSyntaxNode(use_syntax), owner(use_owner)
{
}
TSMethod* TSOverloadedMethod::GetMethod(size_t i)const
{
	return methods[i].get();
}
size_t TSOverloadedMethod::GetMethodsCount()const
{
	return methods.size();
}
TSMethod* TSOverloadedMethod::FindParams(const std::vector<TSParameter*>& params)const
{
	for (const std::unique_ptr<TSMethod>& method : methods)
		if (method->HasParams(params))
			return method.get();
	return nullptr;
}

TSMethod* TSOverloadedMethod::FindConversion(const std::vector<TSParameter*>& params, TSClass* ret_class)const
{
	for (const std::unique_ptr<TSMethod>& i : methods)
		if (i->HasParams(params) && i->GetRetClass() == ret_class)
			return i.get();
	return nullptr;
}

Lexer::TNameId TSOverloadedMethod::GetName()const
{
	return GetSyntax()->GetName();
}
void TSOverloadedMethod::CalculateParametersOffsets()
{
	for (const std::unique_ptr<TSMethod>& i : methods)
		i->CalculateParametersOffsets();
}

void TSOverloadedMethod::CopyExternalMethodBindingsFrom(TSOverloadedMethod* source)
{
	for (size_t i = 0; i < source->methods.size(); i++)
		methods[i]->CopyExternalMethodBindingsFrom(source->methods[i].get());
}

bool TSOverloadedMethod::ParamsExists(TSMethod* use_method)const
{
	return FindParams(use_method->GetParameters()) != nullptr;
}

TSMethod* TSOverloadedMethod::FindParams(TSMethod* use_method)const
{
	return FindParams(use_method->GetParameters());
}

void TSOverloadedMethod::GetMethods(std::vector<TSMethod*> &result)const
{
	for (const std::unique_ptr<TSMethod>& i : methods)
		result.push_back(i.get());
}

void TSOverloadedMethod::CheckForErrors(bool is_conversion)
{
	for (const std::unique_ptr<TSMethod>& i : methods)
	{
		i->CheckForErrors();
		if (is_conversion)
		{
			if (FindConversion(i->GetParameters(), i->GetRetClass()) != &(*i))
				i->GetSyntax()->Error("Метод с такими параметрами уже существует!");
		}
		else
		{
			if (FindParams(i->GetParameters()) != &(*i))
				i->GetSyntax()->Error("Метод с такими параметрами уже существует!");
		}
	}
}

void TSOverloadedMethod::LinkSignature(TGlobalBuildContext build_context)
{
	if (linked_signature)
		return;
	for (const std::unique_ptr<TSMethod>& i : methods)
		i->LinkSignature(build_context);
	linked_signature = true;
}

void TSOverloadedMethod::LinkBody(TGlobalBuildContext build_context)
{
	if (linked_body)
		return;
	for (const std::unique_ptr<TSMethod>& i : methods)
		i->LinkBody(build_context);
	linked_body = true;
}

void TSOverloadedMethod::Build()
{
	std::vector<SyntaxApi::IMethod*> syntax_methods;
	GetSyntax()->GetMethods(syntax_methods);
	for (auto method : syntax_methods)
	{
		methods.emplace_back(new TSMethod(owner, method));
		methods.back()->Build();
	}
}