TOpArray TStatement::BuildLocalsAndParamsDestructor(
		TNotOptimizedProgram &program, int &deallocate_size) {
	return parent->BuildLocalsAndParamsDestructor(program, deallocate_size);
}
