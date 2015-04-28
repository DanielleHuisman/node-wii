#ifndef CONNECT_H
#define CONNECT_H

#include <node.h>

#include <bluetooth/bluetooth.h>
#include <cwiid.h>

//#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
  #define DEBUG_HEADER fprintf(stderr, "wii [%s:%s() %d]: ", __FILE__, __FUNCTION__, __LINE__);
  #define DEBUG_FOOTER fprintf(stderr, "\n");
  #define DEBUG(STRING) DEBUG_HEADER fprintf(stderr, "%s", STRING); DEBUG_FOOTER
  #define DEBUG_OPT(...) DEBUG_HEADER fprintf(stderr, __VA_ARGS__); DEBUG_FOOTER
  #define DUMP_BYTE_STREAM(STREAM, LENGTH) DEBUG_HEADER for (int i = 0; i < LENGTH; i++) { fprintf(stderr, "0x%02X ", STREAM[i]); } DEBUG_FOOTER
#else
  #define DEBUG(str)
  #define DEBUG_OPT(...)
  #define DUMP_BYTE_STREAM(STREAM, LENGTH)
#endif

using namespace v8;
using namespace node;

/**
 * Class: WiiMote
 *   Wrapper for libcwiid connect.
 */
class WiiMote : public ObjectWrap {
  public:
    /**
     * Variable: ir_event
     *   Used to dispatch infrared event.
     */
    static v8::Persistent<v8::String> ir_event;
    /**
     * Variable: acc_event
     *   Used to dispatch accelerometer event.
     */
    static v8::Persistent<v8::String> acc_event;
    /**
     * Variable: nunchuk_event
     *   Used to dispatch nunchuk extension event.
     */
    static v8::Persistent<v8::String> nunchuk_event;
    /**
     * Variable: classic_event
     *   Used to dispatch nunchuk extension event.
     */
    static v8::Persistent<v8::String> classic_event;
    /**
     * Variable: balance_event
     *   Used to dispatch nunchuk extension event.
     */
    static v8::Persistent<v8::String> balance_event;
    /**
     * Variable: motionplus_event
     *   Used to dispatch nunchuk extension event.
     */
    static v8::Persistent<v8::String> motionplus_event;
    /**
     * Variable: buttondown_event
     *   Used to dispatch buttondown event.
     */
    static v8::Persistent<v8::String> button_event;
    /**
     * Variable: error_event
     *   Used to dispatch error event.
     */
    static v8::Persistent<v8::String> error_event;

    static v8::Persistent<v8::String> status_event;

    /**
     * Variable: constructor_template
     *   Used to create Node.js constructor.
     */
    static v8::Persistent<v8::FunctionTemplate> constructor_template;
    /**
     * Function: Initialize
     *   Used to intialize the EventEmitter from Node.js
     *
     * Parameters:
     *   target - v8::Object the Node.js global module object
     */
    static void Initialize(v8::Handle<v8::Object> target);
    /**
     * Function: Connect
     *   Accepts an address and creates a connection.
     *
     * Returns:
     *   a string explaining the error code.
     */
    int Connect(bdaddr_t * mac);
    int Disconnect();

    int Rumble(bool on);
    int Led(int index, bool on);
    int RequestStatus();
    int Reporting(int mode, bool on);

  protected:
    /**
     * Constructor: WiiMote
     */
    WiiMote();
    /**
     * Deconstructor: WiiMote
     */
    virtual ~WiiMote();

    /**
     * Function: New
     *
     * Parameters:
     *   args v8::Arguments function call
     *
     * Returns:
     *   v8::Object args.This()
     */
    static v8::Handle<v8::Value> New(const v8::Arguments& args);

    /**
     * Function: Connect
     *
     * Parameters:
     *   args v8::Arguments function call
     *
     * Returns:
     *   v8::Object args.This()
     */
    static v8::Handle<v8::Value> Connect(const v8::Arguments& args);
    static void UV_Connect(uv_work_t* req);
    static void UV_AfterConnect(uv_work_t* req, int status);

    static v8::Handle<v8::Value> Disconnect(const v8::Arguments& args);

    // Callback from libcwiid's thread
    static void HandleMessages(cwiid_wiimote_t *, int, union cwiid_mesg [], struct timespec *);

    // Callback from Nodejs's thread which calls one of the Handle*Message methods
    static void HandleMessagesAfter(uv_work_t *req, int status);

    // The following methods parse and emit events
    void HandleAccMessage    (struct timespec *ts, cwiid_acc_mesg * msg);
    void HandleButtonMessage (struct timespec *ts, cwiid_btn_mesg * msg);
    void HandleErrorMessage  (struct timespec *ts, cwiid_error_mesg * msg);
    void HandleNunchukMessage(struct timespec *ts, cwiid_nunchuk_mesg * msg);
    void HandleClassicMessage(struct timespec *ts, cwiid_classic_mesg * msg);
    void HandleBalanceMessage(struct timespec *ts, cwiid_balance_mesg * msg);
    void HandleMotionPlusMessage(struct timespec *ts, cwiid_motionplus_mesg * msg);
    void HandleIRMessage     (struct timespec *ts, cwiid_ir_mesg * msg);
    void HandleStatusMessage (struct timespec *ts, cwiid_status_mesg * msg);

    // The following methods turn things on and off
    static v8::Handle<v8::Value> Rumble(const v8::Arguments& args);
    static v8::Handle<v8::Value> Led(const v8::Arguments& args);
    static v8::Handle<v8::Value> RequestStatus(const v8::Arguments& args);
    static v8::Handle<v8::Value> IrReporting(const v8::Arguments& args);
    static v8::Handle<v8::Value> AccReporting(const v8::Arguments& args);
    static v8::Handle<v8::Value> ExtReporting(const v8::Arguments& args);
    static v8::Handle<v8::Value> ButtonReporting(const v8::Arguments& args);

  private:

    /**
     * The v8 instance for this object
     */
    v8::Handle<v8::Object> self;

    /**
     * Variable: wiimote
     *   Pointer to a wiimote handle
     */
    cwiid_wiimote_t* wiimote;

    /**
     * Variable: state
     *   struct representing a wiimote state
     */
    struct cwiid_state state;

    /**
     * Variable: address
     *   bluetooth address value
     */
    bdaddr_t mac;

    /**
     * Variable: button
     *   button identifier
     */
    int button;

    struct connect_request {
      WiiMote* wiimote;
      bdaddr_t mac;
      int err;
      v8::Persistent<v8::Function> callback;
    };

    /**
     * Passes a WiiMote event from libcwiid's thread to the Nodejs's thread
     */
    struct message_request {
      WiiMote* wiimote;
      struct timespec timestamp;
      int len;
      union cwiid_mesg mesgs[1]; // Variable size array containing len elements
    };

};

#endif
