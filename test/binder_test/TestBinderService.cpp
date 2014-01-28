#define LOG_TAG "TestBinderService"

#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/IInterface.h>

#include "TestBinderService.h"


namespace android {


void TestBinderService::instantiate() {
	printf("Enter TestBinderService::instantiate\n");
	status_t st = defaultServiceManager()->addService(
			String16("my.test.binder"), new TestBinderService());
	printf("ServiceManager addService ret=%d\n", st);
	printf("instantiate> end\n");
}

TestBinderService::TestBinderService() {
	printf(" TestBinderServicet\n");
}

TestBinderService::~TestBinderService() {
	printf("TestBinderService destroyed,never destroy normally\n");
}

int TestBinderService::add(int a,int b) {

	printf("TestBinderService::add a = %d, b = %d\n.", a , b);
	return a+b;
}

}
