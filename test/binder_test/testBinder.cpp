#define LOG_TAG "TestBinerService"

#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include "ITestBinderService.h"


#include "TestBinderService.h"

using namespace android;

int main(int argc, char** argv)
 {
	printf("test binder start\n");
	int sum = 0;
	sp<ITestBinderService> mTestBinserService;
	if (mTestBinserService.get() == 0) {
		printf("defaultServiceManager start\n");
		sp<IServiceManager> sm = defaultServiceManager();
		sp<IBinder> binder;
		do {
			printf("getService start\n");
			binder = sm->getService(String16("my.test.binder"));
			if (binder != 0)
				break;
				printf("getService fail\n");
			usleep(5000000); // 0.5 s
		} while (true);
		printf("getService ok\n");
		mTestBinserService = interface_cast<ITestBinderService> (binder);
		//printf(mTestBinserService == 0, "no ITestBinserService!?\n");
	}
	sum = mTestBinserService->add(3, 4);
	printf("sum = %d\n", sum);
	return 0;

}
