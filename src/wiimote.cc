#include <v8.h>
#include <node.h>

#include <bluetooth/bluetooth.h>
#include "cwiid.h"

#include <stdlib.h>

#include "../include/wiimote.h"

using namespace v8;
using namespace node;

#define ARRAY_SIZE(a)                               \
  ((sizeof(a) / sizeof(*(a))) /                     \
  static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

int cwiid_set_err(cwiid_err_t *err);

void WiiMote_cwiid_err(struct wiimote *wiimote, const char *str, va_list ap) {
	(void)wiimote;

	// TODO move this into a error object, so we can return it in the error objects
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
}


void UV_NOP(uv_work_t* req) { /* No operation */ }


/**
 * Constructor: WiiMote
 */
WiiMote::WiiMote() {
	DEBUG_OPT("WiiMote()");
	wiimote = NULL;
};
/**
 * Deconstructor: WiiMote
 */
WiiMote::~WiiMote() {
	DEBUG_OPT("~WiiMote()");
	Disconnect();
};

#define NODE_DEFINE_CONSTANT_NAME(target, name, constant)                 \
  (target)->Set(v8::String::NewSymbol(name),                              \
                v8::Integer::New(constant),                               \
                static_cast<v8::PropertyAttribute>(v8::ReadOnly|v8::DontDelete))

void WiiMote::Initialize (Handle<v8::Object> target) {
  HandleScope scope;

  DEBUG("WiiMote::Initialize()");

  cwiid_set_err(&WiiMote_cwiid_err);

  Local<FunctionTemplate> t = FunctionTemplate::New(WiiMote::New);

  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("WiiMote"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "connect", Connect);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "disconnect", Disconnect);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "rumble", Rumble);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "led", Led);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "ir", IrReporting);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "acc", AccReporting);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "ext", ExtReporting);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "button", ButtonReporting);

  NODE_DEFINE_CONSTANT_NAME(target, "IR_X_MAX", CWIID_IR_X_MAX);
  NODE_DEFINE_CONSTANT_NAME(target, "IR_Y_MAX", CWIID_IR_Y_MAX);
  NODE_DEFINE_CONSTANT_NAME(target, "IR_SRC_COUNT", CWIID_IR_SRC_COUNT);

  NODE_DEFINE_CONSTANT_NAME(target, "BATTERY_MAX", CWIID_BATTERY_MAX);

  NODE_DEFINE_CONSTANT_NAME(target, "BTN_1", CWIID_BTN_1);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_2", CWIID_BTN_2);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_A", CWIID_BTN_A);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_B", CWIID_BTN_B);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_MINUS", CWIID_BTN_MINUS);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_PLUS",  CWIID_BTN_PLUS);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_HOME",  CWIID_BTN_HOME);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_LEFT",  CWIID_BTN_LEFT);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_RIGHT", CWIID_BTN_RIGHT);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_UP",    CWIID_BTN_UP);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_DOWN",  CWIID_BTN_DOWN);

  NODE_DEFINE_CONSTANT_NAME(target, "EXT_NONE",       CWIID_EXT_NONE);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_NUNCHUK",    CWIID_EXT_NUNCHUK);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_CLASSIC",    CWIID_EXT_CLASSIC);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_BALANCE",    CWIID_EXT_BALANCE);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_MOTIONPLUS", CWIID_EXT_MOTIONPLUS);
  //NODE_DEFINE_CONSTANT_NAME(target, "EXT_GUITAR",     CWIID_EXT_GUITAR);
  //NODE_DEFINE_CONSTANT_NAME(target, "EXT_DRUMS",      CWIID_EXT_DRUMS);
  //NODE_DEFINE_CONSTANT_NAME(target, "EXT_TURNTABLES", CWIID_EXT_TURNTABLES);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_UNKNOWN",    CWIID_EXT_UNKNOWN);

  NODE_DEFINE_CONSTANT_NAME(target, "ERROR_NONE",       CWIID_ERROR_NONE);
  NODE_DEFINE_CONSTANT_NAME(target, "ERROR_DISCONNECT", CWIID_ERROR_DISCONNECT);
  NODE_DEFINE_CONSTANT_NAME(target, "ERROR_COMM",       CWIID_ERROR_COMM);

  target->Set(String::NewSymbol("WiiMote"), constructor_template->GetFunction());
}

int WiiMote::Connect(bdaddr_t * mac) {
  DEBUG_OPT("Connecting to %s", batostr(mac));
  bacpy(&this->mac, mac);

  if(!(this->wiimote = cwiid_open(&this->mac, CWIID_FLAG_MESG_IFC))) {
    return -1;
  }

  return 0;
}

int WiiMote::Disconnect() {
  DEBUG_OPT("Disconnect()");
  if (this->wiimote) {

    if (cwiid_get_data(this->wiimote)) {
      cwiid_set_mesg_callback(this->wiimote, NULL);
      cwiid_set_data(this->wiimote, NULL);
    }

    cwiid_close(this->wiimote);
    this->wiimote = NULL;
  }

  return 0;
}

int WiiMote::Rumble(bool on) {
  unsigned char rumble = on ? 1 : 0;

  assert(this->wiimote != NULL);

  if(cwiid_set_rumble(this->wiimote, rumble)) {
    return -1;
  }
  
  return 0;
}

int WiiMote::Led(int index, bool on) {
  int indexes[] = { CWIID_LED1_ON, CWIID_LED2_ON, CWIID_LED3_ON, CWIID_LED4_ON };

  assert(this->wiimote != NULL);

  if(cwiid_get_state(this->wiimote, &this->state)) {
    return -1;
  }

  int led = this->state.led;

  led = on ? led | indexes[index-1] : led & indexes[index-1];

  if(cwiid_set_led(this->wiimote, led)) {
    return -1;
  }

  return 0;
}

/**
 * Turns on or off the particular modes passed
 */
int WiiMote::Reporting(int mode, bool on) {
  assert(this->wiimote != NULL);

  if(cwiid_get_state(this->wiimote, &this->state)) {
    return -1;
  }

  int newmode = this->state.rpt_mode;

  newmode = on ? (newmode | mode) : (newmode & ~mode);

  if(cwiid_set_rpt_mode(this->wiimote, newmode)) {
    return -1;
  }

  return 0;
}

void WiiMote::HandleAccMessage(struct timespec *ts, cwiid_acc_mesg * msg) {
  HandleScope scope;

  Local<Object> pos = Object::New();   // Create array of x,y,z
  pos->Set(String::NewSymbol("x"), Integer::New(msg->acc[CWIID_X]) );
  pos->Set(String::NewSymbol("y"), Integer::New(msg->acc[CWIID_Y]) );
  pos->Set(String::NewSymbol("z"), Integer::New(msg->acc[CWIID_Z]) );

  Local<Value> argv[2] = { String::New("acc"), pos };
  MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleButtonMessage(struct timespec *ts, cwiid_btn_mesg * msg) {
  HandleScope scope;

  Local<Integer> btn = Integer::New(msg->buttons);

  Local<Value> argv[2] = { String::New("button"), btn };
  MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleErrorMessage(struct timespec *ts, cwiid_error_mesg * msg) {
  HandleScope scope;

  Local<Integer> err = Integer::New(msg->error);

  Local<Value> argv[2] = { String::New("error"), err };
  MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleNunchukMessage(struct timespec *ts, cwiid_nunchuk_mesg * msg) {
  // TODO - event "nunchuk"
}

void WiiMote::HandleIRMessage(struct timespec *ts, cwiid_ir_mesg * msg) {
  HandleScope scope;
  Local<Array> poss = Array::New(CWIID_IR_SRC_COUNT);

  // Check IR data sources
  for(int i=0; i < CWIID_IR_SRC_COUNT; i++) {
    if (!msg->src[i].valid)
      break; // Once we find one invalid then we stop

    // Create array of x,y
    Local<Object> pos = Object::New();
    pos->Set(String::NewSymbol("x"), Integer::New( msg->src[i].pos[CWIID_X] ));
    pos->Set(String::NewSymbol("y"), Integer::New( msg->src[i].pos[CWIID_Y] ));
    pos->Set(String::NewSymbol("size"), Integer::New( msg->src[i].size ));

    poss->Set(Integer::New(i), pos);
  }

  Local<Value> argv[2] = { String::New("ir"), poss };
  MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleStatusMessage(struct timespec *ts, cwiid_status_mesg * msg) {
  HandleScope scope;

  Local<Object> obj = Object::New();

  obj->Set(String::NewSymbol("battery"),    Integer::New(msg->battery));
  obj->Set(String::NewSymbol("extensions"), Integer::New(msg->ext_type));

  Local<Value> argv[2] = { String::New("status"), obj };
  MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleMessagesAfter(uv_work_t *req, int status) {
  message_request* r = static_cast<message_request* >(req->data);
  WiiMote * self = r->wiimote;
  delete req;

  for (int i = 0; i < r->len; i++) {
    switch(r->mesgs[i].type) {
      case CWIID_MESG_STATUS:
        self->HandleStatusMessage(&r->timestamp, (cwiid_status_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_BTN:
        self->HandleButtonMessage(&r->timestamp, (cwiid_btn_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_ACC:
        self->HandleAccMessage(&r->timestamp, (cwiid_acc_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_IR:
        self->HandleIRMessage(&r->timestamp, (cwiid_ir_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_NUNCHUK:
        self->HandleNunchukMessage(&r->timestamp, (cwiid_nunchuk_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_ERROR:
        self->HandleErrorMessage(&r->timestamp, (cwiid_error_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_UNKNOWN:
      case CWIID_MESG_CLASSIC:
      case CWIID_MESG_BALANCE:
      case CWIID_MESG_MOTIONPLUS:
      default:
        break;
    }
  }

  free(r);
}

// Called by libcwiid when an event from the wiimote arrives
void WiiMote::HandleMessages(cwiid_wiimote_t *wiimote, int len, union cwiid_mesg mesgs[], struct timespec *timestamp) {
  WiiMote *self = const_cast<WiiMote*>(static_cast<const WiiMote*>(cwiid_get_data(wiimote)));

  // There is a race condition where this might happen
  if (self == NULL)
    return;

  // Make a copy of the message
  struct message_request * req = (struct message_request *)malloc( sizeof(*req) + sizeof(req->mesgs) * (len - 1) );
  req->wiimote = self;
  req->timestamp = *timestamp;
  req->len = len;
  memcpy(req->mesgs, mesgs, len * sizeof(union cwiid_mesg));

  // We need to pass this over to the nodejs thread, so it can create V8 objects
  uv_work_t* uv = new uv_work_t;
  uv->data = req;
  int r = uv_queue_work(uv_default_loop(), uv, UV_NOP, WiiMote::HandleMessagesAfter);
  if (r != 0) {
    DEBUG_OPT("err: %d while queuing wiimote message", r);
    free(req);
    delete uv;
  }
}

Handle<Value> WiiMote::New(const Arguments& args) {
  HandleScope scope;

  assert(args.IsConstructCall());

  WiiMote* wiimote = new WiiMote();
  wiimote->Wrap(args.This());

  wiimote->self = Persistent<Object>::New(args.This());

  return scope.Close(args.This());
}

Handle<Value> WiiMote::Connect(const Arguments& args) {
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());
  Local<Function> callback;

  HandleScope scope;

  if(args.Length() == 0 || !args[0]->IsString()) {
    return ThrowException(Exception::Error(String::New("MAC address is required and must be a String.")));
  }

  if(args.Length() == 1 || !args[1]->IsFunction()) {
    return ThrowException(Exception::Error(String::New("Callback is required and must be a Function.")));
  }

  callback = Local<Function>::Cast(args[1]);

  connect_request* ar = new connect_request();
  ar->wiimote = wiimote;

  String::Utf8Value mac(args[0]);
  str2ba(*mac, &ar->mac); // TODO Validate the mac and throw an exception if invalid

  ar->callback = Persistent<Function>::New(callback);

  wiimote->Ref();

  uv_work_t* req = new uv_work_t;
  req->data = ar;
  int r = uv_queue_work(uv_default_loop(), req, UV_Connect, UV_AfterConnect);
  if (r != 0) {

    ar->callback.Dispose();
    delete ar;
    delete req;

    wiimote->Unref();

    return ThrowException(Exception::Error(String::New("Internal error: Failed to queue connect work")));
  }

  return Undefined();
}

void WiiMote::UV_Connect(uv_work_t* req) {
  connect_request* ar = static_cast<connect_request* >(req->data);

  assert(ar->wiimote != NULL);

  ar->err = ar->wiimote->Connect(&ar->mac);
}

void WiiMote::UV_AfterConnect(uv_work_t* req, int status) {
  HandleScope scope;

  connect_request* ar = static_cast<connect_request* >(req->data);
  delete req;

  WiiMote * wiimote = ar->wiimote;
  Local<Value> argv[1] = { Integer::New(ar->err) };

  if (ar->err == 0) {
    // Setup the callback to receive events
    cwiid_set_data(wiimote->wiimote, wiimote);
    cwiid_set_mesg_callback(wiimote->wiimote, WiiMote::HandleMessages);
  }

  wiimote->Unref();

  TryCatch try_catch;

  ar->callback->Call(Context::GetCurrent()->Global(), 1, argv);

  if(try_catch.HasCaught())
    FatalException(try_catch);

  ar->callback.Dispose();

  delete ar;
}

Handle<Value> WiiMote::Disconnect(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());
  return Integer::New(wiimote->Disconnect());
}


Handle<Value> WiiMote::Rumble(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();

  return Integer::New(wiimote->Rumble(on));
}

Handle<Value> WiiMote::Led(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsNumber()) {
    return ThrowException(Exception::Error(String::New("Index is required and must be a Number.")));
  }

  if(args.Length() == 1 || !args[1]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  int index = args[0]->ToInteger()->Value();
  bool on = args[1]->ToBoolean()->Value();

  return Integer::New(wiimote->Led(index, on));
}


Handle<Value> WiiMote::IrReporting(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();
  return Integer::New(wiimote->Reporting(CWIID_RPT_IR, on));
}

Handle<Value> WiiMote::AccReporting(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();
  return Integer::New(wiimote->Reporting(CWIID_RPT_ACC, on));
}

Handle<Value> WiiMote::ExtReporting(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();
  return Integer::New(wiimote->Reporting(CWIID_RPT_EXT, on));
}

Handle<Value> WiiMote::ButtonReporting(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();

  return Integer::New(wiimote->Reporting(CWIID_RPT_BTN, on));
}

Persistent<FunctionTemplate> WiiMote::constructor_template;
