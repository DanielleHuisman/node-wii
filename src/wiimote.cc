#include <node.h>

#include <bluetooth/bluetooth.h>
#include "cwiid.h"

#include <stdlib.h>

#include "wiimote.h"

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

namespace wii {

using namespace v8;

Persistent<Function> WiiMote::constructor;

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

#define NODE_DEFINE_CONSTANT_NAME(target, name, constant)                     \
  do {                                                                        \
    v8::Isolate* isolate = target->GetIsolate();                              \
    v8::Local<v8::Context> context = isolate->GetCurrentContext();            \
    v8::Local<v8::String> constant_name =                                     \
        v8::String::NewFromUtf8(isolate, name);                               \
    v8::Local<v8::Number> constant_value =                                    \
        v8::Number::New(isolate, static_cast<double>(constant));              \
    v8::PropertyAttribute constant_attributes =                               \
        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);    \
    (target)->DefineOwnProperty(context,                                      \
                                constant_name,                                \
                                constant_value,                               \
                                constant_attributes).FromJust();              \
  }                                                                           \
  while (0)

void WiiMote::Initialize (Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  DEBUG("WiiMote::Initialize()");

  cwiid_set_err(&WiiMote_cwiid_err);

  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "WiiMote"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(tpl, "connect", Connect);
  NODE_SET_PROTOTYPE_METHOD(tpl, "disconnect", Disconnect);
  NODE_SET_PROTOTYPE_METHOD(tpl, "requestStatus", RequestStatus);
  NODE_SET_PROTOTYPE_METHOD(tpl, "rumble", Rumble);
  NODE_SET_PROTOTYPE_METHOD(tpl, "led", Led);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ir", IrReporting);
  NODE_SET_PROTOTYPE_METHOD(tpl, "acc", AccReporting);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ext", ExtReporting);
  NODE_SET_PROTOTYPE_METHOD(tpl, "button", ButtonReporting);

  NODE_DEFINE_CONSTANT_NAME(exports, "IR_X_MAX", CWIID_IR_X_MAX);
  NODE_DEFINE_CONSTANT_NAME(exports, "IR_Y_MAX", CWIID_IR_Y_MAX);
  NODE_DEFINE_CONSTANT_NAME(exports, "IR_SRC_COUNT", CWIID_IR_SRC_COUNT);

  NODE_DEFINE_CONSTANT_NAME(exports, "ACC_MAX", CWIID_ACC_MAX);
  NODE_DEFINE_CONSTANT_NAME(exports, "BATTERY_MAX", CWIID_BATTERY_MAX);

  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_L_STICK_MAX", CWIID_CLASSIC_L_STICK_MAX);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_R_STICK_MAX", CWIID_CLASSIC_R_STICK_MAX);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_LR_MAX", CWIID_CLASSIC_LR_MAX);

  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_1", CWIID_BTN_1);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_2", CWIID_BTN_2);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_A", CWIID_BTN_A);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_B", CWIID_BTN_B);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_MINUS", CWIID_BTN_MINUS);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_PLUS",  CWIID_BTN_PLUS);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_HOME",  CWIID_BTN_HOME);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_LEFT",  CWIID_BTN_LEFT);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_RIGHT", CWIID_BTN_RIGHT);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_UP",    CWIID_BTN_UP);
  NODE_DEFINE_CONSTANT_NAME(exports, "BTN_DOWN",  CWIID_BTN_DOWN);

  NODE_DEFINE_CONSTANT_NAME(exports, "NUNCHUK_BTN_Z", CWIID_NUNCHUK_BTN_Z);
  NODE_DEFINE_CONSTANT_NAME(exports, "NUNCHUK_BTN_C", CWIID_NUNCHUK_BTN_C);

  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_UP", CWIID_CLASSIC_BTN_UP);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_LEFT", CWIID_CLASSIC_BTN_LEFT);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_ZR", CWIID_CLASSIC_BTN_ZR);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_X", CWIID_CLASSIC_BTN_X);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_A", CWIID_CLASSIC_BTN_A);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_Y", CWIID_CLASSIC_BTN_Y);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_B", CWIID_CLASSIC_BTN_B);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_ZL", CWIID_CLASSIC_BTN_ZL);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_R", CWIID_CLASSIC_BTN_R);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_PLUS", CWIID_CLASSIC_BTN_PLUS);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_HOME", CWIID_CLASSIC_BTN_HOME);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_MINUS", CWIID_CLASSIC_BTN_MINUS);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_L", CWIID_CLASSIC_BTN_L);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_DOWN", CWIID_CLASSIC_BTN_DOWN);
  NODE_DEFINE_CONSTANT_NAME(exports, "CLASSIC_BTN_RIGHT", CWIID_CLASSIC_BTN_RIGHT);

  NODE_DEFINE_CONSTANT_NAME(exports, "EXT_NONE",       CWIID_EXT_NONE);
  NODE_DEFINE_CONSTANT_NAME(exports, "EXT_NUNCHUK",    CWIID_EXT_NUNCHUK);
  NODE_DEFINE_CONSTANT_NAME(exports, "EXT_CLASSIC",    CWIID_EXT_CLASSIC);
  NODE_DEFINE_CONSTANT_NAME(exports, "EXT_BALANCE",    CWIID_EXT_BALANCE);
  NODE_DEFINE_CONSTANT_NAME(exports, "EXT_MOTIONPLUS", CWIID_EXT_MOTIONPLUS);
  //NODE_DEFINE_CONSTANT_NAME(exports, "EXT_GUITAR",     CWIID_EXT_GUITAR);
  //NODE_DEFINE_CONSTANT_NAME(exports, "EXT_DRUMS",      CWIID_EXT_DRUMS);
  //NODE_DEFINE_CONSTANT_NAME(exports, "EXT_TURNTABLES", CWIID_EXT_TURNTABLES);
  NODE_DEFINE_CONSTANT_NAME(exports, "EXT_UNKNOWN",    CWIID_EXT_UNKNOWN);

  NODE_DEFINE_CONSTANT_NAME(exports, "ERROR_NONE",       CWIID_ERROR_NONE);
  NODE_DEFINE_CONSTANT_NAME(exports, "ERROR_DISCONNECT", CWIID_ERROR_DISCONNECT);
  NODE_DEFINE_CONSTANT_NAME(exports, "ERROR_COMM",       CWIID_ERROR_COMM);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "WiiMote"), tpl->GetFunction());
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

int WiiMote::RequestStatus() {
  assert(this->wiimote != NULL);

  if(cwiid_request_status(this->wiimote)) {
    return -1;
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
  // HandleScope scope;

  // Local<Object> pos = Object::New();   // Create array of x,y,z
  // pos->Set(String::NewSymbol("x"), Integer::New(msg->acc[CWIID_X]) );
  // pos->Set(String::NewSymbol("y"), Integer::New(msg->acc[CWIID_Y]) );
  // pos->Set(String::NewSymbol("z"), Integer::New(msg->acc[CWIID_Z]) );

  // Local<Value> argv[2] = { String::New("acc"), pos };
  // MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleButtonMessage(struct timespec *ts, cwiid_btn_mesg * msg) {
  // HandleScope scope;

  // Local<Integer> btn = Integer::New(msg->buttons);

  // Local<Value> argv[2] = { String::New("button"), btn };
  // MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleErrorMessage(struct timespec *ts, cwiid_error_mesg * msg) {
  // HandleScope scope;

  // Local<Integer> err = Integer::New(msg->error);

  // Local<Value> argv[2] = { String::New("error"), err };
  // MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleNunchukMessage(struct timespec *ts, cwiid_nunchuk_mesg * msg) {
  // HandleScope scope;

  // Local<Object> stick = Object::New();
  // stick->Set(String::NewSymbol("x"), Integer::New(msg->stick[CWIID_X]));
  // stick->Set(String::NewSymbol("y"), Integer::New(msg->stick[CWIID_Y]));

  // Local<Object> pos = Object::New();
  // pos->Set(String::NewSymbol("x"), Integer::New(msg->acc[CWIID_X]));
  // pos->Set(String::NewSymbol("y"), Integer::New(msg->acc[CWIID_Y]));
  // pos->Set(String::NewSymbol("z"), Integer::New(msg->acc[CWIID_Z]));

  // Local<Object> data = Object::New();
  // data->Set(String::NewSymbol("stick"), stick);
  // data->Set(String::NewSymbol("acc"), pos);
  // data->Set(String::NewSymbol("buttons"), Integer::New(msg->buttons));

  // Local<Value> argv[2] = { String::New("nunchuk"), data };
  // MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleClassicMessage(struct timespec *ts, cwiid_classic_mesg * msg) {
  // HandleScope scope;

  // Local<Object> lstick = Object::New();
  // lstick->Set(String::NewSymbol("x"), Integer::New(msg->l_stick[CWIID_X]));
  // lstick->Set(String::NewSymbol("y"), Integer::New(msg->l_stick[CWIID_Y]));

  // Local<Object> rstick = Object::New();
  // rstick->Set(String::NewSymbol("x"), Integer::New(msg->r_stick[CWIID_X]));
  // rstick->Set(String::NewSymbol("y"), Integer::New(msg->r_stick[CWIID_Y]));

  // Local<Object> data = Object::New();
  // data->Set(String::NewSymbol("leftStick"), lstick);
  // data->Set(String::NewSymbol("rightStick"), rstick);
  // data->Set(String::NewSymbol("left"), Integer::New(msg->l));
  // data->Set(String::NewSymbol("right"), Integer::New(msg->r));
  // data->Set(String::NewSymbol("buttons"), Integer::New(msg->buttons));

  // Local<Value> argv[2] = { String::New("classic"), data };
  // MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleBalanceMessage(struct timespec *ts, cwiid_balance_mesg * msg) {
  // HandleScope scope;

  // Local<Object> data = Object::New();
  // data->Set(String::NewSymbol("rightTop"), Integer::New(msg->right_top));
  // data->Set(String::NewSymbol("rightBottom"), Integer::New(msg->right_bottom));
  // data->Set(String::NewSymbol("leftTop"), Integer::New(msg->left_top));
  // data->Set(String::NewSymbol("leftBottom"), Integer::New(msg->left_bottom));

  // Local<Value> argv[2] = { String::New("balance"), data };
  // MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleMotionPlusMessage(struct timespec *ts, cwiid_motionplus_mesg * msg) {
  // HandleScope scope;

  // Local<Object> angle = Object::New();
  // angle->Set(String::NewSymbol("x"), Integer::New(msg->angle_rate[CWIID_X]));
  // angle->Set(String::NewSymbol("y"), Integer::New(msg->angle_rate[CWIID_Y]));
  // angle->Set(String::NewSymbol("z"), Integer::New(msg->angle_rate[CWIID_Z]));

  // // Local<Object> speed = Object::New();
  // // speed->Set(String::NewSymbol("x"), Integer::New(msg->low_speed[CWIID_X]));
  // // speed->Set(String::NewSymbol("y"), Integer::New(msg->low_speed[CWIID_Y]));
  // // speed->Set(String::NewSymbol("z"), Integer::New(msg->low_speed[CWIID_Z]));

  // Local<Object> data = Object::New();
  // data->Set(String::NewSymbol("angleRate"), angle);
  // //data->Set(String::NewSymbol("lowSpeed"), speed);

  // Local<Value> argv[2] = { String::New("motionplus"), data };
  // MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleIRMessage(struct timespec *ts, cwiid_ir_mesg * msg) {
  // HandleScope scope;
  // Local<Array> poss = Array::New(CWIID_IR_SRC_COUNT);

  // // Check IR data sources
  // for(int i=0; i < CWIID_IR_SRC_COUNT; i++) {
  //   if (!msg->src[i].valid)
  //     break; // Once we find one invalid then we stop

  //   // Create array of x,y
  //   Local<Object> pos = Object::New();
  //   pos->Set(String::NewSymbol("x"), Integer::New( msg->src[i].pos[CWIID_X] ));
  //   pos->Set(String::NewSymbol("y"), Integer::New( msg->src[i].pos[CWIID_Y] ));
  //   pos->Set(String::NewSymbol("size"), Integer::New( msg->src[i].size ));

  //   poss->Set(Integer::New(i), pos);
  // }

  // Local<Value> argv[2] = { String::New("ir"), poss };
  // MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
}

void WiiMote::HandleStatusMessage(struct timespec *ts, cwiid_status_mesg * msg) {
  // HandleScope scope;

  // Local<Object> obj = Object::New();

  // obj->Set(String::NewSymbol("battery"),    Integer::New(msg->battery));
  // obj->Set(String::NewSymbol("extensions"), Integer::New(msg->ext_type));

  // Local<Value> argv[2] = { String::New("status"), obj };
  // MakeCallback(self, "emit", ARRAY_SIZE(argv), argv);
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

      case CWIID_MESG_CLASSIC:
        self->HandleClassicMessage(&r->timestamp, (cwiid_classic_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_BALANCE:
        self->HandleBalanceMessage(&r->timestamp, (cwiid_balance_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_MOTIONPLUS:
        self->HandleMotionPlusMessage(&r->timestamp, (cwiid_motionplus_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_UNKNOWN:
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

void WiiMote::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (args.IsConstructCall()) {
    // Invoked as constructor: `new WiiMote(...)`
    WiiMote* wiimote = new WiiMote();
    wiimote->Wrap(args.This());

    args.GetReturnValue().Set(args.This());
  } else {
    // Invoked as plain function `WiMote(...)`, turn into construct call.
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> result = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(result);
  }
}

void WiiMote::Connect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());
  Local<Function> callback;

  if(args.Length() == 0 || !args[0]->IsString()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "MAC address is required and must be a String.")));
    return;
  }

  if(args.Length() == 1 || !args[1]->IsFunction()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Callback is required and must be a Function.")));
    return;
  }

  callback = Local<Function>::Cast(args[1]);

  connect_request* ar = new connect_request();
  ar->wiimote = wiimote;

  String::Utf8Value mac(args[0]);
  str2ba(*mac, &ar->mac); // TODO Validate the mac and throw an exception if invalid

  ar->callback = Local<Function>::New(isolate, constructor);

  wiimote->Ref();

  uv_work_t* req = new uv_work_t;
  req->data = ar;
  int r = uv_queue_work(uv_default_loop(), req, UV_Connect, UV_AfterConnect);
  if (r != 0) {

    // TODO
    // ar->callback.Dispose();
    delete ar;
    delete req;

    wiimote->Unref();

    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Internal error: Failed to queue connect work")));
    return;
  }

  args.GetReturnValue().SetUndefined();
}

void WiiMote::UV_Connect(uv_work_t* req) {
  connect_request* ar = static_cast<connect_request* >(req->data);

  assert(ar->wiimote != NULL);

  ar->err = ar->wiimote->Connect(&ar->mac);
}

void WiiMote::UV_AfterConnect(uv_work_t* req, int status) {
  // HandleScope scope;

  // connect_request* ar = static_cast<connect_request* >(req->data);
  // delete req;

  // WiiMote * wiimote = ar->wiimote;
  // Local<Value> argv[1] = { Integer::New(ar->err) };

  // if (ar->err == 0) {
  //   // Setup the callback to receive events
  //   cwiid_set_data(wiimote->wiimote, wiimote);
  //   cwiid_set_mesg_callback(wiimote->wiimote, WiiMote::HandleMessages);
  // }

  // wiimote->Unref();

  // TryCatch try_catch;

  // ar->callback->Call(Context::GetCurrent()->Global(), 1, argv);

  // if(try_catch.HasCaught())
  //   FatalException(try_catch);

  // ar->callback.Dispose();

  // delete ar;
}

void WiiMote::Disconnect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  args.GetReturnValue().Set(Integer::New(isolate, wiimote->Disconnect()));
}

void WiiMote::RequestStatus(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  args.GetReturnValue().Set(Integer::New(isolate, wiimote->RequestStatus()));
}

void WiiMote::Rumble(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "On state is required and must be a Boolean.")));
    return;
  }

  bool on = args[0]->ToBoolean()->Value();
  args.GetReturnValue().Set(Integer::New(isolate, wiimote->Rumble(on)));
}

void WiiMote::Led(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "Index is required and must be a Number.")));
    return;
  }

  if(args.Length() == 1 || !args[1]->IsBoolean()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "On state is required and must be a Boolean.")));
    return;
  }

  int index = args[0]->ToInteger()->Value();
  bool on = args[1]->ToBoolean()->Value();
  args.GetReturnValue().Set(Integer::New(isolate, wiimote->Led(index, on)));
}

void WiiMote::IrReporting(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "On state is required and must be a Boolean.")));
    return;
  }

  bool on = args[0]->ToBoolean()->Value();
  args.GetReturnValue().Set(Integer::New(isolate, wiimote->Reporting(CWIID_RPT_IR, on)));
}

void WiiMote::AccReporting(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "On state is required and must be a Boolean.")));
    return;
  }

  bool on = args[0]->ToBoolean()->Value();
  args.GetReturnValue().Set(Integer::New(isolate, wiimote->Reporting(CWIID_RPT_ACC, on)));
}

void WiiMote::ExtReporting(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "On state is required and must be a Boolean.")));
    return;
  }

  bool on = args[0]->ToBoolean()->Value();
  args.GetReturnValue().Set(Integer::New(isolate, wiimote->Reporting(CWIID_RPT_EXT, on)));
}

void WiiMote::ButtonReporting(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "On state is required and must be a Boolean.")));
    return;
  }

  bool on = args[0]->ToBoolean()->Value();
  args.GetReturnValue().Set(Integer::New(isolate, wiimote->Reporting(CWIID_RPT_BTN, on)));
}

}
