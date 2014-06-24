
class TNotOptimizedProgram
{
private:
	//TList<TOp> instructions;
	TAllocator<TListItem<TOp>, 1023> instr_alloc;
	int curr_label;

	TVector<char*> string_consts;
#ifndef NDEBUG
	TVector<TOp*> ops;
#endif
	TVector<TMethod*> methods_table;
	TVector<TClass*> array_element_classes;
public:

	TOpArray static_vars_init;
	TOpArray static_vars_destroy;

	int static_vars_size;

	TVectorList<TMethod> internal_methods;

	TNotOptimizedProgram();
	~TNotOptimizedProgram();
	void ListItems();
	void CreateOptimizedProgram(TProgram& optimized, TTime& time);
	int AddStringConst(TNameId string);
	int GetUniqueLabel();
	void Push(TOp use_op, TOpArray &ops_array);
	void PushFront(TOp use_op, TOpArray &ops_array);
	int AddMethodToTable(TMethod* use_method);
	int CreateArrayElementClassId(TClass* use_class);
	int FindMethod(TMethod* use_method);
};