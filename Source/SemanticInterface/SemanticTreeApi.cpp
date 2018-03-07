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

	ISClass* SAnalyzeExternalClass(Lexer::ILexer* lexer, SyntaxApi::IClass* base_class, SemanticApi::ISClass* _sbase_class, 
		TExternalClassDecl decl, TGlobalBuildContext build_context, 
		std::vector<SemanticApi::TExternalSMethod> external_classes_bindings, int& curr_bind)
	{
		auto sbase_class = dynamic_cast<TSClass*>(_sbase_class);
		lexer->ParseSource(decl.source.c_str());

		auto cl = SyntaxApi::AnalyzeNestedClass(lexer, base_class);

		TSClass* scl = new TSClass(sbase_class, cl);
		sbase_class->AddClass(scl);
		scl->Build();

		scl->LinkSignature(build_context);
		//scl->CalculateMethodsSizes();

		scl->SetSize(decl.size);
		scl->SetAutoMethodsInitialized();

		//TODO проверка что заданы все external_func
		scl->SetExternal(external_classes_bindings, curr_bind);

		return scl;
	}

	ISClass * SAnalyze(Lexer::ILexer* lexer, SyntaxApi::IClass* base_class, std::vector<TExternalClassDecl> external_classes,
		std::vector<SemanticApi::TExternalSMethod> external_classes_bindings, TGlobalBuildContext build_context)
	{
		auto sem_base_class = new TSClass(nullptr, base_class);
		sem_base_class->Build();

		CreateInternalClasses(lexer, base_class, sem_base_class);

		int curr_bind=0;

		//TODO внешние классы должны объ€вл€тьс€ в пор€дке испоьлзовани€, т.к. линкуютс€ последовательно
		for (auto v : external_classes)
			SAnalyzeExternalClass(lexer, base_class, sem_base_class, v, build_context, external_classes_bindings, curr_bind);

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
