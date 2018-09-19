#include <node.h>

#include "wiimote.h"

namespace wii {

using v8::Local;
using v8::Object;

void InitAll(v8::Local<v8::Object> exports) {
	WiiMote::Initialize(exports);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, InitAll);

}
