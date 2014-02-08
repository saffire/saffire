#include "saffire.h"

using namespace saffire;

Handle<Value> Method(const Arguments& args) {
	HandleScope scope;
	return scope.Close(String::new("world"));
}

void init(Handle<Object> exports) {
	exports->Set(String::newSymbol("Hello"),
	FunctionTemplate::New(Method)->GetFunction());
}

SAFFIRE_MODULE(hello, init)
