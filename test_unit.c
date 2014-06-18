extern int callback(int);



int test_data_01 = 10;
int test_data_02[] = {1, 2, 3};

void test_function_01(int a, int out[]) {
	out[0] = a;
	out[1] = callback(a);
// 	out[2] = test_data_01;
// 	out[3] = test_data_02[1];
}

void test_function_02(int a, int out[]) {
	out[0] = a;
	out[1] = callback(a) + 1;
	// out[2] = test_data_01;
	// out[3] = test_data_02[1];
}