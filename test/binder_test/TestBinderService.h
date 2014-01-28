#ifndef ANDROID_TESTBINDERSERVICE_H_
#define ANDROID_TESTBINDERSERVICE_H_

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/KeyedVector.h>
#include "ITestBinderService.h"

namespace android {

class TestBinderService: public BnTestBinderService {
public:
	static void instantiate();
	int add(int a,int b);

private:
    TestBinderService();
    virtual ~TestBinderService();
};

}

#endif /* ANDROID_TESTBINDERSERVICE_H_ */
