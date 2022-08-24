/**
 *	Declaration of EdgeUnified classes.
 *	@file	EdgeUnified.h
 *	@author	hieromon@gmail.com
 *	@version	0.9.1
 *	@date	2022-08-24
 *	@copyright	MIT license.
 */

#ifndef _EDGEUNIFIED_H_
#define _EDGEUNIFIED_H_

#include <deque>
#include <functional>
#include <vector>
#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
namespace EdgeUnifiedNS { using WebServer = ESP8266WebServer; };
#elif defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
namespace EdgeUnifiedNS { using WebServer = WebServer; };
#endif
#include <ArduinoJson.h>
#include <AutoConnect.h>

// Uncomment the following ED_DEBUG to enable debug output.
//#define ED_DEBUG

// Debug output destination can be defined externally with AC_DEBUG_PORT
#ifndef ED_DEBUG_PORT
#define ED_DEBUG_PORT Serial
#endif // !ED_DEBUG_PORT

#ifdef ED_DEBUG
#define ED_DBG_DUMB(fmt, ...) do {ED_DEBUG_PORT.printf_P((PGM_P)PSTR(fmt), ## __VA_ARGS__ );} while (0)
#define ED_DBG(fmt, ...) do {ED_DEBUG_PORT.printf_P((PGM_P)PSTR("[ED] " fmt), ## __VA_ARGS__ );} while (0)
#else
#define ED_DBG(...) do {(void)0;} while(0)
#define ED_DBG_DUMB(...) do {(void)0;} while(0)
#endif // !ED_DEBUG

// ED_SERIALIZE_BUFFER_SIZE is the allocation size for the area of the
// DynamicJsonDocument for ArduinoJson used to achieve serialization
// and deserialization of EdgeData in JSON format.
// If EdgeData is constructed from many data items, ArduinoJson parsing
// will fail due to insufficient buffer. In such cases, increase the
// value of ED_SERIALIZE_BUFFER_SIZE.
#ifndef ED_SERIALIZE_BUFFER_SIZE
#define ED_SERIALIZE_BUFFER_SIZE              256
#endif // !ED_SERIALIZE_BUFFER_SIZE

// File extension that EdgeData outputs directly without a serializer provided.
#ifndef ED_EDGEDATA_IMMEDIATE_FILEEXTENSION
#define ED_EDGEDATA_IMMEDIATE_FILEEXTENSION   ".dat"
#endif // !ED_EDGEDATA_IMMEDIATE_FILEEXTENSION

// File extension that EdgeData outputs via the offered serializer.
#ifndef ED_EDGEDATA_OFFERED_FILEEXTENSION
#define ED_EDGEDATA_OFFERED_FILEEXTENSION     ".json"
#endif // !ED_EDGEDATA_OFFERED_FILEEXTENSION

// Delimiter for extracting the type name from __PRETTY_FUNCTION___.
#ifndef ED_GETTYPE_DELIMITER
#define ED_GETTYPE_DELIMITER                  "[with T ="
#endif // !ED_GETTYPE_DELIMITER

// A symbol of terminator of __PRETTY_FUNCTION___ for extracting the type name.
#ifndef ED_GETTYPE_TERMINATOR
#define ED_GETTYPE_TERMINATOR                 ';'
#endif // !ED_GETTYPE_TERMINATOR

//
#ifndef ED_AUXJSONPROTOCOL_FILE
#define ED_AUXJSONPROTOCOL_FILE               "file:"
#endif // !ED_AUXJSONPROTOCOL_FILE

/**
 * EdgeAux: Combines a JSON description of a custom web page interpreted
 * by AutoConnect with its request handler. EdgeUnifined::join function
 * will join AutoConnect custom web pages to the Edge event loop via
 * EdgeAux.
 * @param  json     AutoConnectAux JSON description. The JSON description
 * can be either PGM_P or a pointer to the __FlashStringHelper class;
 * when specifying a JSON description with PROGMEM attribute, it must be
 * cast to a pointer to __FlashStringHelper using FPSTR macro.
 * by the FPSTR macro.
 * @param  handler  Request handler for a custom web page of the specified JSON description.
 */
class EdgeAux {
 public:
  EdgeAux() {}
  EdgeAux(PGM_P json, AuxHandlerFunctionT handler) : json(json), auxHandler(handler) {}
  EdgeAux(const __FlashStringHelper* json, AuxHandlerFunctionT handler) : json_p(json), auxHandler(handler) {}
  EdgeAux(const EdgeAux& rhs) : json(rhs.json), json_p(rhs.json_p), auxHandler(rhs.auxHandler) {}
  ~EdgeAux() {}

  PGM_P   json = nullptr;                               /**< JSON for on memory */
  const __FlashStringHelper*  json_p = nullptr;         /**< jSON for on flash */
  AuxHandlerFunctionT auxHandler;                       /**< AutoConnectAux request handler */
};

// Forward references
class EdgeUnified;

/**
 * EdgeDriverBase: Base class of EdgeDriver and provides the basic
 * capabilities of EdgeDriver.
 */
class EdgeDriverBase {
 public:
  // An identifier that specifies automatic saving and restoration of EdgeData.
  typedef enum PERSISTANCE {
    ED_PERSISTENT_AUTORESTORE = 0x01,
    ED_PERSISTENT_AUTOSAVE    = 0x10,
  } PERSISTANCE_t;

  // EdgeDriver handler functions; EdgeUnified calls each handler at
  // each stage of the event loop.
  typedef std::function<void(void)>   EdgeDriverHandlerT;
  typedef std::function<void(int)>    EdgeDriverErrorHandlerT;
  typedef std::function<void(ArduinoJson::JsonObject&)> EdgeDataSerializerT;

  EdgeDriverBase() : _enable(true), _interval(0), _tm(0), _persistance(0x00), _jsonBufferSize(0) {}
  EdgeDriverBase(const EdgeDriverBase& rhs) :
    _enable(rhs._enable),
    _interval(rhs._interval), _tm(rhs._tm),
    _persistance(rhs._persistance),
    _jsonBufferSize(rhs._jsonBufferSize),
    _cbStart(rhs._cbStart), _cbProcess(rhs._cbProcess), _cbEnd(rhs._cbEnd), _cbError(rhs._cbError),
    _serializer(rhs._serializer), _deserializer(rhs._deserializer),
    _edgeDataType(rhs._edgeDataType) {}

  // Only an interface for embedding EdgeData types into a class instance.
  virtual const String& getTypeName(void) = 0;

  // EdgeDriver process controls
  void  enable(const bool onOff) { _enable = onOff; }
  void  end(void);
  void  error(const int error);
  void  process(void);
  void  start(const long interval = -1);
  
  // Controls the periodicity of the active state of EdgeDriver::process
  void  clearEdgeInterval(void) { setEdgeInterval(0); }
  unsigned long getEdgeInterval(void) const { return _interval; }
  void  setEdgeInterval(const unsigned long interval) { _interval = interval; _tm = millis(); }
  
  // Serialization and deserialization of EdgeData
  void  autoRestore(const bool onOff);
  void  autoSave(const bool onOff);
  bool  isAutoRestore(void) { return _persistance & ED_PERSISTENT_AUTORESTORE; }
  bool  isAutoSave(void) { return _persistance & ED_PERSISTENT_AUTOSAVE; }
  size_t  restore(AUTOCONNECT_APPLIED_FILECLASS& fs = AUTOCONNECT_APPLIED_FILESYSTEM, const char* fileName = nullptr);
  size_t  save(AUTOCONNECT_APPLIED_FILECLASS& fs = AUTOCONNECT_APPLIED_FILESYSTEM, const char* fileName = nullptr);
  void  serializer(EdgeDataSerializerT serializer, EdgeDataSerializerT deserializer, const size_t bufferSize = ED_SERIALIZE_BUFFER_SIZE);

 protected:
  virtual ~EdgeDriverBase() { end(); }
  bool  _elapse(void);
  void  _embedType(const String& pf);
  const String& _getType(void) const { return _edgeDataType; }

  bool    _enable;                                      /**< The enable status of the EdgeDriver process call */
  unsigned long _interval;                              /**< Period during which EdgeDriver::process is enabled */
  unsigned long _tm;                                    /**< Time remaining until next cycle for EdgeDriver::process call */
  uint8_t _persistance;                                 /**< Composite value of PERSISTANCE_t indicating automatic save and restore */
  size_t  _jsonBufferSize;                              /**< Json dynamic buffer allocation size */

  EdgeDriverHandlerT  _cbStart   = nullptr;             /**< On-start callback */
  EdgeDriverHandlerT  _cbProcess = nullptr;             /**< On-process callback */
  EdgeDriverHandlerT  _cbEnd     = nullptr;             /**< On-end callback */
  EdgeDriverErrorHandlerT _cbError  = nullptr;          /**< On-error callback */ 

  EdgeDataSerializerT _serializer   = nullptr;          /**< Serializer */
  EdgeDataSerializerT _deserializer = nullptr;          /**< Deserializer */

 private:
  virtual size_t  _dataReader(File& file) = 0;          /**< Default serializer interface */
  virtual size_t  _dataWritter(File& file) = 0;         /**< Default deserializer interface */

  String  _edgeDataType;                                /**< Declared EdgeData type */
};

/**
 * EdgeDriver class template; Declares the actual EdgeDriver class with
 * accompanying EdgeData type.
 * @param  start    Registers a start callback that is called when the
 * EdgeDriver is attached to EdgeUnified.
 * @param  process  Registers a process callback to be called by the
 * EdgeUnified::process function.
 * @param  end      Registers a process callback to be called by the
 * EdgeUnified::end function.
 */
template<typename T>
class EdgeDriver : public EdgeDriverBase {
 public:
  using EdgeDriverBase::EdgeDriverBase;
  EdgeDriver() { EdgeDriverBase::_embedType(String(__PRETTY_FUNCTION__)); }
  EdgeDriver(EdgeDriverHandlerT start, EdgeDriverHandlerT process, EdgeDriverHandlerT end) {
    EdgeDriverBase::_embedType(String(__PRETTY_FUNCTION__));
    bind(start, process, end);
  }
  ~EdgeDriver() {}

  // Coupling point with EdgeUnified
  void bind(EdgeDriverHandlerT start, EdgeDriverHandlerT process, EdgeDriverHandlerT end) {
    _cbStart = std::bind(start);
    _cbProcess = std::bind(process);
    _cbEnd = std::bind(end);
  }

  // EdgeDriver process controls
  void onError(EdgeDriverErrorHandlerT error) { _cbError = std::bind(error, std::placeholders::_1); }

  // Returns embedded EdgeData type.
  const String& getTypeName(void) override { return _getType(); }

  // EdgeData instance is more flexible if dynamically allocated; should verify
  // replacement with std::unique_ptr. In that case, a reference operator
  // overloading implementation for instance references via unique_ptr needs to
  // be added for convenience of access from sketches.
  T data;

 private:
  size_t  _dataReader(File& file) override { return file.read(reinterpret_cast<uint8_t*>(&data), sizeof(T)); }
  size_t  _dataWritter(File& file) override { return file.write(reinterpret_cast<const uint8_t*>(&data), sizeof(T)); }
};

/**
 * EdgeUnified: Integration of the interface logic governing device IO with
 * AutoConnect custom web pages. It hides from the user sketch the parameter
 * persistence and AutoConnectAux loading required for peripheral IO processing,
 * and integrates multiple event loops that were separated per device into one.
 */
class EdgeUnified {
 public:
  EdgeUnified() {}
  ~EdgeUnified() {}

  // Deprecated functions
  //  void begin(void); // Role ambiguous, may not be necessary?
  //  void portal(AutoConnect& autoConnect);  // AutoConnect handling is more flexible if left to sketches.

  // Consideration of necessity
  //  class EdgeConfig;
  //  void config(EdgeConfig& config);

  // Release candidates functions
  void  abort(const int error);
  void  attach(EdgeDriverBase& driver, const long interval = -1);
  void  attach(std::vector<std::reference_wrapper<EdgeDriverBase>> drivers);
  void  detach(const EdgeDriverBase& driver);
  void  end(void);
  void  join(PGM_P json, AuxHandlerFunctionT auxHandler = nullptr);
  void  join(const __FlashStringHelper* json, AuxHandlerFunctionT auxHandler = nullptr);
  void  join(const std::vector<EdgeAux>& pages);
  void  portal(AutoConnect& portal);
  void  process(AutoConnect& portal);
  void  process(void);
  bool  release(const String& uri);
  void  restore(AUTOCONNECT_APPLIED_FILECLASS& fs = AUTOCONNECT_APPLIED_FILESYSTEM, const bool autoMount = false);
  void  save(AUTOCONNECT_APPLIED_FILECLASS& fs = AUTOCONNECT_APPLIED_FILESYSTEM, const bool autoMount = false);
  EdgeUnifiedNS::WebServer& server(void) { return _portal->host(); }

 protected:
  std::vector<std::reference_wrapper<EdgeDriverBase>> _drivers;
  std::deque<AutoConnectAux*> _auxQueue;

  AutoConnect*  _portal = nullptr;
};

// Export an EdgeUnified instance as an Edge to the global.
#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EDGE)
extern EdgeUnified  Edge;
#endif

#endif // !_EDGEUNIFIED_H_
