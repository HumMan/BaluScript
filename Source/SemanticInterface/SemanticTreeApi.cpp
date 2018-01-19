#include "SemanticTreeApi.h"

#include "Internal/SClass.h"
#include "Internal/SMethod.h"


namespace SemanticApi
{
	void CreateInternalClasses(Lexer::ILexer* lexer, SyntaxApi::IClass* base_class, SemanticApi::ISClass* _sbase_class)
	{
		lexer->ParseSource("class dword {}");
		SyntaxApi::IClass* t_syntax = SyntaxApi::AnalyzeNestedClass(lexer, base_class);

		auto sbase_class = dynamic_cast<TSClass*>(_sbase_class);
		TSClass* t = new TSClass(sbase_class, t_syntax);
		t->SetSize(1);
		t->SetSignatureLinked();
		t->SetBodyLinked();
		sbase_class->AddClass(t);
	}

	ISClass* SAnalyzeExternalClass(Lexer::ILexer* lexer, SyntaxApi::IClass* base_class, SemanticApi::ISClass* _sbase_class, TExternalClassDecl decl)
	{
		auto sbase_class = dynamic_cast<TSClass*>(_sbase_class);
		lexer->ParseSource(decl.source.c_str());

		auto cl = SyntaxApi::AnalyzeNestedClass(lexer, base_class);

		TSClass* scl = new TSClass(sbase_class, cl);
		sbase_class->AddClass(scl);
		scl->Build();

		scl->SetSize(decl.size);
		scl->SetAutoMethodsInitialized();

		//TODO проверка что заданы все external_func

		if (decl.def_constr != nullptr)
		{
			assert(scl->GetDefConstr() != nullptr);
			dynamic_cast<TSMethod*>(scl->GetDefConstr())->SetAsExternal(decl.def_constr);
		}

		std::vector<ISMethod*> m;

		if (decl.copy_constr != nullptr)
		{
			m.clear();
			scl->GetCopyConstructors(m);
			dynamic_cast<TSMethod*>(m[0])->SetAsExternal(decl.copy_constr);
		}

		if (decl.destr != nullptr)
		{
			dynamic_cast<TSMethod*>(scl->GetDestructor())->SetAsExternal(decl.destr);
		}

		for (int i = 0; i<(int)Lexer::TOperator::End; i++)
		{
			auto curr_op = decl.operators[i];
			if (curr_op != nullptr)
			{
				m.clear();
				scl->GetOperators(m, (Lexer::TOperator)i);
				dynamic_cast<TSMethod*>(m[0])->SetAsExternal(curr_op);
			}
		}

		for (auto method : decl.methods)
		{
			m.clear();
			scl->GetMethods(m, lexer->GetIdFromName(method.first.c_str()),SemanticApi::Filter::NotSet, false, false);
			dynamic_cast<TSMethod*>(m[0])->SetAsExternal(method.second);
		}

		return scl;
	}

	ISClass * SAnalyze(Lexer::ILexer* lexer, SyntaxApi::IClass* base_class, std::vector<TExternalClassDecl> external_classes, TGlobalBuildContext build_context)
	{
		auto sem_base_class = new TSClass(nullptr, base_class);
		sem_base_class->Build();

		CreateInternalClasses(lexer, base_class, sem_base_class);

		for (auto v : external_classes)
			SAnalyzeExternalClass(lexer, base_class, sem_base_class, v);

		sem_base_class->LinkSignature(build_context);
		sem_base_class->InitAutoMethods();
		sem_base_class->LinkBody(build_context);

		std::vector<TSClass*> owners;
		sem_base_class->CalculateSizes(owners);
		sem_base_class->CalculateMethodsSizes();

		return sem_base_class;
	}	

	ISMethod * SAnalyzeMethodSignature(Lexer::ILexer* lexer, SyntaxApi::IMethod* method_decl_syntax, ISClass* sem_base_class)
	{
		TSMethod* method_decl = new TSMethod(
			dynamic_cast<TSClass*>(sem_base_class->GetNestedByFullName(method_decl_syntax->GetOwner()->GetFullClassName(), 1)),
			method_decl_syntax);
		method_decl->Build();
		method_decl->LinkSignatureForMethodFind();
		return method_decl;
	}
	void SDestroyMethodSignature(ISMethod * method)
	{
		delete (dynamic_cast<TSMethod*>(method));
	}
	
	void SDestroy(ISClass* p)
	{
		delete (dynamic_cast<TSClass*>(p));
	}
}
