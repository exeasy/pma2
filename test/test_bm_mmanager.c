#include <utils/common.h>
#include <utils/utils.h>
#include <mmanager/module_manager.h>
#include <stdio.h>

struct test_module{
	int module_id;
	int module_fd;
	char *info;
};

int main(int argc, const char *argv[])
{
	module_manager_init();
	struct test_module module1,module2,module3;
	char *msg;
	int len;
	module1.module_id = 1;
	module1.module_fd = 2;
	module1.info = "This is Test Module1";
	module2.module_id = 2;
	module2.module_fd = 3;
	module2.info = "This is Test Module2";
	module3.module_id = 3;
	module3.module_fd = 4;
	module3.info = "This is Test Module3";
	insert_register_table(module1.module_id, module1.module_fd, module1.info);

	GetModuleInfo(&msg,&len); 
	printf("%s\n",msg);
	insert_register_table(module2.module_id, module2.module_fd, module2.info);

	GetModuleInfo(&msg,&len); 
	printf("%s\n",msg);
	insert_register_table(module3.module_id, module3.module_fd, module3.info);
	insert_register_table(module3.module_id, module3.module_fd+2, module3.info);

	GetModuleInfo(&msg,&len); 
	printf("%s\n",msg);
	return 0;
}
