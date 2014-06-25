#include "../Syntax/Expression.h"

#include "../syntaxAnalyzer.h"
#include "../Syntax/Class.h"
#include "../Syntax/Method.h"
#include "../Syntax/Statements.h"

TFormalParam TExpression::TBinOp::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	std::vector<TFormalParam> param;
	std::vector<TMethod*> bin_operators;

	param.resize(2);
	param[0]=left->Build(program,parent);
	param[1]=right->Build(program,parent);

	int left_conv_count=-1,right_conv_count=-1;

	TMethod *bin_operator=NULL,*bin_operator2=NULL;

	if(param[0].IsVoid())
		Error("К левому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");
	if(param[1].IsVoid())
		Error("К правому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");

	std::vector<TMethod*> operators;

	param[0].GetClass()->GetOperators(operators,op);
	bin_operator=FindMethod(this,operators,param,left_conv_count);
	if(op>=TOperator::Assign&&op<=TOperator::OrA && !param[0].IsRef())
		Error("Для присваиваниия требуется ссылка, а не значение!");

	if(!(op>=TOperator::Assign&&op<=TOperator::OrA))
	{
		operators.resize(0);
		param[1].GetClass()->GetOperators(operators,op);
		bin_operator2=FindMethod(this,operators,param,right_conv_count);
	}

	TFormalParam result;
	if(bin_operator!=NULL&&(bin_operator2==NULL||left_conv_count<right_conv_count))
	{
		ValidateAccess(this,parent->owner,bin_operator);
		result=bin_operator->BuildCall(program,param);
	}
	else if(bin_operator2!=NULL)
	{
		ValidateAccess(this,parent->owner,bin_operator2);
		result=bin_operator2->BuildCall(program,param);
	}
	else Error("Бинарного оператора для данных типов не существует!");
	return param[0].GetOps()+param[1].GetOps()+result;
}

TFormalParam TExpression::TUnaryOp::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	std::vector<TFormalParam> param;

	param.resize(1);
	param[0]=left->Build(program,parent);

	int conv_count;
	TMethod *unary_operator=NULL;

	if(param[0].GetClass()==NULL)
		Error("К данному операнду нельзя применить унарный оператор (нужен тип отличающийся от void)!");

	std::vector<TMethod*> operators;
	param[0].GetClass()->GetOperators(operators,op);

	unary_operator=FindMethod(this,operators,param,conv_count);

	if(unary_operator!=NULL)
	{
		ValidateAccess(this,parent->owner,unary_operator);
		TFormalParam result=unary_operator->BuildCall(program,param);
		return param[0].GetOps()+result;
	}
	else Error("Унарного оператора для данного типа не существует!");
	return TVoid();
}

TFormalParam TExpression::TId::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	TVariable* var=parent->GetVar(name);
	if(var!=NULL)
	{
		if(var->GetType()==TVariableType::ClassField)
		{
			ValidateAccess(this,parent->owner,(TClassField*)var);
			if(!((TClassField*)var)->IsStatic()&&parent->method->IsStatic())
				Error("К нестатическому полю класса нельзя обращаться из статического метода!");
		}
		TOpArray temp;
		if(var->NeedPushThis())
			program.Push(TOp(TOpcode::PUSH_THIS),temp);
		return temp+var->PushRefToStack(program);
	}
	std::vector<TMethod*> methods;
	if(parent->method->IsStatic())
		parent->owner->GetMethods(methods,name,true);
	else
		parent->owner->GetMethods(methods,name);
	if(methods.size()!=0)
	{
		return TFormalParam(methods,true);
	}else
	{
		if(parent->method->IsStatic())
		{
			methods.clear();
			if(parent->owner->GetMethods(methods,name,false))
			{
				Error("К нестатическому методу класса нельзя обращаться из статического метода!");
			}
		}
	}
	TClass* type=parent->owner->GetClass(name);
	if(type==NULL)
	{
		Error("Неизвестный идентификатор!");
	}
	return TFormalParam(type);
}

TFormalParam TExpression::TThis::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	if(parent->method->IsStatic())
		Error("Ключевое слово 'this' можно использовать только в нестатических методах!");
	TOpArray temp;
	program.Push(TOp(TOpcode::PUSH_THIS),temp);
	return TFormalParam(parent->owner,true,temp);
}

TFormalParam TExpression::TGetMemberOp::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	TFormalParam left_result=left->Build(program,parent);
	if(left_result.IsMethods())
		Error("Оператор доступа к члену класса нельзя применить к методу!");
	if(left_result.IsType())
	{
		if(left_result.GetType()->IsEnum())
		{
			int id=left_result.GetType()->GetEnumId(name);
			//TODO ввести спец функции min max count
			if(id==-1)
				Error("Перечислимого типа с таким именем не существует!");
			else
			{
				program.Push(TOp(TOpcode::PUSH,id),left_result.GetOps());
				return TFormalParam(left_result.GetType(),false,left_result.GetOps());
			}
		}else
		{
			TClassField* static_member=left_result.GetType()->GetField(name,true,true);
			if(static_member!=NULL)
				return static_member->PushRefToStack(program);
			std::vector<TMethod*> methods;
			if(left_result.GetType()->GetMethods(methods,name,true))
				return TFormalParam(methods,false);
			TClass* nested=left_result.GetType()->GetNested(name);
			if(nested!=NULL)
				return TFormalParam(nested);
			Error("Статического поля или метода с таким именем не существует!");
		}
	}
	TClassField* member=left_result.GetClass()!=NULL
		?left_result.GetClass()->GetField(name,true)
		:NULL;
	if(member!=NULL)
	{
		if(member->IsStatic())
			Error("Оператор доступа к члену класса нельзя применить к объекту для доступа к статическому члену, \nвместо объекта следует использовать имя класса!");
		ValidateAccess(this,parent->owner,member);
		if(left_result.IsRef())
		{
			TFormalParam result=member->PushRefToStack(program);
			if(member->IsReadOnly())
			{
				TMethod* copy_constr=result.GetClass()->GetCopyConstr();
				program.Push(TOp(TOpcode::RVALUE,result.GetClass()->GetSize(),program.AddMethodToTable(copy_constr))
					,result.GetOps());
				result.SetIsRef(false);
			}
			return left_result.GetOps()+result;
		}
		else 
		{
			program.Push(TOp(TOpcode::PUSH,left_result.GetClass()->GetSize()),left_result.GetOps());
			program.Push(TOp(TOpcode::PUSH,member->GetOffset()),left_result.GetOps());
			program.Push(TOp(TOpcode::PUSH,member->GetClass()->GetSize()),left_result.GetOps());
			program.Push(TOp(TOpcode::GET_MEMBER,
				program.AddMethodToTable(member->GetClass()->GetCopyConstr()),
				program.AddMethodToTable(member->GetClass()->GetDestructor())),
				left_result.GetOps());
			return TFormalParam(member->GetClass(),false,left_result.GetOps());
		}
		//Error("Оператор доступа к члену объекта нельзя использовать для временного объекта!");//TODO
	}else
	{
		if(!left_result.IsRef())
			Error("Вызов метода для временного объекта недопустим!");
		std::vector<TMethod*> methods;
		if(!left_result.GetClass()->GetMethods(methods,name,false))
			Error("Члена класса с таким именем не существует!");
		TFormalParam result(methods,false);
		result.GetOps()=left_result.GetOps();
		return result;
	}
	assert(false);
}

TFormalParam TExpression::TCallParamsOp::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	TFormalParam left_result=left->Build(program,parent);
	std::vector<TFormalParam> params_result;

	for(int i=0;i<=param.GetHigh();i++)
		params_result.push_back(param[i]->Build(program,parent));
	int conv_need=0;
	if(left_result.IsMethods())
	{
		//вызов метода
		if(is_bracket)assert(false);//при вызове метода используются круглые скобки
		TMethod* method=FindMethod(this,left_result.GetMethods(),params_result,conv_need);
		if(method!=NULL)
		{
			ValidateAccess(this,parent->owner,method);
			TOpArray params_ops;
			TFormalParam call_ops=method->BuildCall(program,params_result);
			for(int i=0;i<=param.GetHigh();i++)
				params_ops+=params_result[i].GetOps();
			if(!method->IsStatic()&&left_result.NeedPushThis())
				program.Push(TOp(TOpcode::PUSH_THIS),params_ops);
			return left_result.GetOps()+params_ops+call_ops;
		}
		else Error("Метода с такими параметрами не существует");
	}
	else if(left_result.IsType())
	{
		//if(left_result.GetType()->GetType()==TYPE_ENUM)
		//	Error("Для перечислений нельзя использовать оператор вызова параметров!");
		int conv_need=-1;
		std::vector<TMethod*> constructors;
		TClass* constr_class=left_result.GetType();
		constr_class->GetConstructors(constructors);
		TMethod* constructor=FindMethod(this,constructors,params_result,conv_need);
		TOpArray before_params;
		if(constructor==NULL)
			program.PushFront(TOp(TOpcode::PUSH_COUNT,constr_class->GetSize()),before_params);
		if(constructor!=NULL)
		{
			ValidateAccess(this,parent->owner,constructor);
			//if(constructor->GetType()==MT_USER||constructor->GetType()==MT_EXTERNAL)
			//{
			program.Push(TOp(TOpcode::PUSH_STACK_HIGH_REF,constr_class->GetSize()),before_params);
			//}
			TOpArray call_ops=constructor->BuildCall(program,params_result).GetOps();
			TOpArray params_ops;
			for(int i=0;i<=param.GetHigh();i++)
				params_ops+=params_result[i].GetOps();
			return before_params+params_ops+TFormalParam(constr_class,false,call_ops);
		}else if(params_result.size()>0)
			Error("Конструктора с такими параметрами не существует!");
	}
	else
	{
		params_result.insert(params_result.begin(),left_result);
		std::vector<TMethod*> operators;
		left_result.GetClass()->GetOperators(operators,is_bracket?(TOperator::GetArrayElement):(TOperator::ParamsCall));
		TMethod* method=FindMethod(this,operators,params_result,conv_need);
		if(method!=NULL)
		{
			ValidateAccess(this,parent->owner,method);
			TOpArray params_ops;
			TFormalParam call_ops=method->BuildCall(program,params_result);
			for(int i=0;i<params_result.size();i++)
				params_ops+=params_result[i].GetOps();
			return params_ops+call_ops;
		}
		else Error("Оператора с такими параметрами не существует!");
	}
	assert(false);return TVoid();
}