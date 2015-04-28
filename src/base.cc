#include <node.h>
#include <v8.h>

#include "../include/wiimote.h"

void init(Handle<v8::Object> target) {
	WiiMote::Initialize(target);
}

NODE_MODULE(wii, init);
