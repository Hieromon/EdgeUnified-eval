/**
 *	EdgeUnified implementations.
 *	@file	EdgeUnified.cpp
 *	@author	hieromon@gmail.com
 *	@version	0.9.1
 *	@date	2022-08-24
 *	@copyright	MIT license.
 */

#include <algorithm>
#include "EdgeUnified.h"

/**
 * Specifies automatic restoration of EdgeData for EdgeDriver. Attaching
 * EdgeDriver to EdgeUnified will automatically restore EdgeData.
 * @param  onOff  Take either True or False, with True specifying automatic
 * restoration.
 */
void EdgeDriverBase::autoRestore(const bool onOff) {
  if (onOff) {
    _persistance |= ED_PERSISTENT_AUTORESTORE;
  }
  else {
    _persistance &= 0xf ^ (uint8_t)ED_PERSISTENT_AUTORESTORE;
  }
}

/**
 * Specifies automatic saving EdgeData for EdgeDriver. If autoSave is enabled,
 * the EdgeData will be autosaved when the EdgeDriver is detached from
 * EdgeUnified or the end function is executed.
 * @param  onOff  Take either True or False, with True specifying automatic
 * saving.
 */
void EdgeDriverBase::autoSave(const bool onOff) {
  if (onOff)
    _persistance |= ED_PERSISTENT_AUTOSAVE;
  else
    _persistance &= 0xf ^ (uint8_t)ED_PERSISTENT_AUTOSAVE;
}

/**
 * Call the end callback to terminate EdgeDriver. If auto-save is enabled
 * EdgeData is saved with the EdgeDriver<T>::save function.
 * Once the end function is executed, the EdgeDriver is deactivated and
 * excluded from the event loop of the process function unless the start is
 * called again.
 */
void EdgeDriverBase::end(void) {
  if (_cbEnd)
    _cbEnd();

  if (isAutoSave())
    save();
  _enable = false;
}

/**
 * Call the on-error callback to abort EdgeDriver processing. Once the error
 * callback is called, the EdgeDriver's process callback is disabled until the
 * start function is executed.
 * @param  error  Error code. The value should be uniquely defined by the user
 * sketch according to the EdgeDriver properties. EdgeUnified is not involved
 * in the value and semantics of the error code.
 */
void EdgeDriverBase::error(const int error) {
  if (_cbError)
    _cbError(error);
  _enable = false;
}

/**
 * Calls the process callback function when the EdgeDriver is in the enable
 * state. Also, if that EdgeDriver is periodic, it measures the period.
 * If the period has not reached the interval, the call to process callback
 * is abandoned.
 */
void EdgeDriverBase::process(void) {
  if (_enable && _cbProcess && _elapse())
    _cbProcess();
}

/**
 * Call the start callback to start EdgeDriver. If auto-restore is enabled
 * EdgeData is restored with the EdgeDriver<T>::restore function.
 * @param  interval Specifies the period interval at which the EdgeDriver::
 * process is allowed to run. If a negative value is specified, the current
 * interval is not changed.
 */
void EdgeDriverBase::start(const long interval) {
  _enable = true;

  if (isAutoRestore())
    restore();

  if (interval >= 0)
    setEdgeInterval(interval);

  if (_cbStart)
    _cbStart();
}

/**
 * Restores EdgeData from the file system.
 * @param  fs       Specifies a reference to the file system. The default is
 * the file system recognized by AutoConnect.
 * @param  fileName Specify the name of the file containing the EdgeData to be
 * restored. If this value is nullptr, the restore function will adopt the
 * type name of the EdgeData as the file name. Also, if EdgeDriver owns the
 * deserializer, the file extension is given as `.json`, otherwise `.dat`.
 * @return The size of the restored EdgeData. If it is zero, the restore failed.
 */
size_t EdgeDriverBase::restore(AUTOCONNECT_APPLIED_FILECLASS& fs, const char* fileName) {
  String  fn = String(fileName);
  size_t  size = 0;

  if (!fileName)
    fn = '/' + getTypeName() + (_deserializer ?
      String(F(ED_EDGEDATA_OFFERED_FILEEXTENSION)) : String(F(ED_EDGEDATA_IMMEDIATE_FILEEXTENSION)));
  else if (fn[0] != '/')
    fn = '/' + fn;

  File  inFile = fs.open(fn.c_str(), "r");
  ED_DBG("Restore EdgeData %s ", fn.c_str());

  if (inFile) {
    if (_deserializer) {
      ArduinoJsonBuffer doc(_jsonBufferSize);

#if ARDUINOJSON_VERSION_MAJOR<=5
      JsonObject& json = doc.parseObject(in)File;
      bool  prc = json.success();
      size = doc.size();
#else
      DeserializationError  err = ArduinoJson::deserializeJson(doc, inFile);
      JsonObject json = doc.as<JsonObject>();
      bool  prc = !err;
      size = doc.memoryUsage();
#endif
      if (prc)
        _deserializer(json);
      else
        ED_DBG_DUMB("deserialize:%s, ", err.c_str());
    }
    else
      size = _dataReader(inFile);

    inFile.close();
    ED_DBG_DUMB("%d bytes\n", size);
  }
  else
    ED_DBG_DUMB("open failed\n");

  return size;
}

/**
 * Save EdgeData to the file system.
 * @param  fs       Specifies a reference to the file system. The default is
 * the file system recognized by AutoConnect.
 * @param  fileName Specify the name of the file containing the EdgeData to be
 * saved. If this value is nullptr, the save function will adopt the type
 * name of the EdgeData as the file name. Also, if EdgeDriver owns the
 * serializer, the file extension is given as `.json`, otherwise `.dat`.
 * @return The size of the saving EdgeData. If it is zero, the save failed.
 */
size_t EdgeDriverBase::save(AUTOCONNECT_APPLIED_FILECLASS& fs, const char* fileName) {
  String  fn = String(fileName);
  size_t  size = 0;

  if (!fileName)
    fn = '/' + getTypeName() + (_serializer ?
      String(F(ED_EDGEDATA_OFFERED_FILEEXTENSION)) : String(F(ED_EDGEDATA_IMMEDIATE_FILEEXTENSION)));
  else if (fn[0] != '/')
    fn = '/' + fn;

  File  outFile = fs.open(fn.c_str(), "w");
  ED_DBG("Save EdgeData %s ", fn.c_str());

  if (outFile) {
    if (_serializer) {
      ArduinoJsonBuffer doc(_jsonBufferSize);
      ArduinoJsonObject json = ARDUINOJSON_CREATEOBJECT(doc);
      _serializer(json);
      size = ArduinoJson::serializeJson(json, outFile);
    }
    else {
      size = _dataWritter(outFile);
    }
    outFile.close();
    ED_DBG_DUMB("%d bytes\n", size);
  }
  else
    ED_DBG_DUMB("open failed\n");

  return size;
}

void EdgeDriverBase::serializer(EdgeDataSerializerT serializer, EdgeDataSerializerT deserializer, const size_t bufferSize) {
  _serializer = serializer;
  _deserializer = deserializer;
  _jsonBufferSize = bufferSize;
}

/**
 * Constrains the execution of the relevant EdgeDriver by cycle.
 * EdgeDriverBase::setEdgeInterval function allows the EdgeDriver::process
 * has a periodicity of the driver execution. This _elapse function measures
 * the period when the EdgeDriver has periodicity.
 * EdgeDriver's periodic process function calls do not utilize timer
 * interrupts. It is a simple event handling loop that is asynchronous to the
 * event handling driven by the associated EdgeDriver. Therefore, if an
 * EdgeDriver inadvertently waits or forms a loop with a while; delay, it will
 * affect the event handling of other EdgeDrivers. In particular, WebServer
 * and AutoConnect will not be able to respond to TCP requests.
 * @return true   The end of the period was reached.
 * @return false  The end of the period has not been reached.
 */
bool EdgeDriverBase::_elapse(void) {
  if (millis() - _tm > _interval) {
    _tm = millis();
    return true;
  }
  return false;
}

/**
 * Embeds the user type of EdgeData as a String into EdgeDriver instance.
 * It will be the type name interpreted by the compiler processor derived
 * from __PRETTY_FUNCTION__.
 */
void EdgeDriverBase::_embedType(const String& pf) {
  int dlm = pf.indexOf(F(ED_GETTYPE_DELIMITER));
  if (dlm > 0)
    _edgeDataType = pf.substring(dlm + sizeof(ED_GETTYPE_DELIMITER), pf.lastIndexOf(ED_GETTYPE_TERMINATOR));
}

/**
 * Attach EdgeDriver to EdgeUnified. The attached EdgeDriver is integrated
 * into the event loop formed by the EdgeUnified, and EdgeDriver::process
 * is called as an extension of the EdgeUnified::process function call.
 * The EdgeData is also restored from the file system by the attach function
 * when that EdgeDriver is in the EdgeDriver::autoRestore enabled state.
 * @param  driver   EdgeDriver instance to be integrated into EdgeUnified.
 * @param  interval Specifies the period interval at which the EdgeDriver::
 * process is allowed to run. If a negative value is specified, the current
 * interval is not changed.
 */
void EdgeUnified::attach(EdgeDriverBase& driver, const long interval) {
  ED_DBG("Attaching driver...");
  _drivers.push_back(driver);
  ED_DBG_DUMB("%s\n", driver.getTypeName().c_str());
  driver.start(interval);
}

/**
 * Consolidate multiple EdgeDrivers into EdgeUnified at once.
 * @param  drivers  Array of EdgeDriver instances to be integrated
 * into EdgeUnified.
 */
void EdgeUnified::attach(std::vector<std::reference_wrapper<EdgeDriverBase>> drivers) {
  for (EdgeDriverBase& driver : drivers) {
    attach(driver);
  }
}

/**
 * Detach a EdgeDriver from EdgeUnified. Also it calls EdgeDriver's end
 * callback upon detachment.
 * @param  driver EdgeDriver instance to be detached from EdgeUnified.
 */
void EdgeUnified::detach(const EdgeDriverBase& driver) {
  _drivers.erase(std::remove_if(_drivers.begin(), _drivers.end(), [&](const EdgeDriverBase& _driver) {
    return std::addressof(driver) == std::addressof(_driver);
  }), _drivers.end());
}

/**
 * Pair the JSON description of the AutoConnectAux custom web page with the
 * request handler and bind it to EdgeUnified. If EdgeUnified does not own
 * the AutoConnect instance at the time of binding (i.e. AutoConnect::begin
 * has not run), EdgeUnified will delay loading the AutoConnectAux JSON.
 * @param  json       Pointer to JSON description of the AutoConnectAux.
 * @param  auxHandler AutoConnectAux request handler.
 */
void EdgeUnified::join(PGM_P json, AuxHandlerFunctionT auxHandler) {
  EdgeAux aux(json, auxHandler);
  join({ aux });
}

/**
 * Pair the JSON description of the AutoConnectAux custom web page with the
 * request handler and bind it to EdgeUnified. If EdgeUnified does not own
 * the AutoConnect instance at the time of binding (i.e. AutoConnect::begin
 * has not run), EdgeUnified will delay loading the AutoConnectAux JSON.
 * @param  json       __FlashStringHelper pointer through PROGMEM area. When
 * the PROGMEM attribute is given to the JSON description and placed in the
 * flash area, the JSON description is passed through the FPSTR macro.
 * @param  auxHandler AutoConnectAux request handler.
 */
void EdgeUnified::join(const __FlashStringHelper* json, AuxHandlerFunctionT auxHandler) {
  EdgeAux aux(json, auxHandler);
  join({ aux });
}

/**
 * Combines multiple JSON description of the AutoConnectAux custom web page
 * and the request handler pairs into EdgeUnified at once.
 * If page specifier has the `FILE:` identifier, the join try to load JSON
 * from the file.
 * @param  pages  Array of JSON and the request handler pairs.
 */
void EdgeUnified::join(const std::vector<EdgeAux>& pages) {
  for (const EdgeAux& page : pages) {
    if (!page.json && !page.json_p) {
      ED_DBG("AutoConnectAux JSON descriptor missing\n");
      continue;
    }

    // Determines the input source of the JSON description.
    // If `PGM_P json` has a File: identifier as prefix, then a JSON description
    // file is loaded from the stream originating from its opened.
    File  jsonFile;
    if (page.json) {
      const char* jsonIn = page.json;
      const char* jsonProtocol = ED_AUXJSONPROTOCOL_FILE;
      int diff = 0;
      while (!diff && *jsonProtocol)
        diff = tolower((int)*jsonIn++) - (int)*jsonProtocol++;
      if (!diff) {
        jsonFile = AUTOCONNECT_APPLIED_FILESYSTEM.open(jsonIn, "r");
        if (!jsonFile.available()) {
          ED_DBG("join %s open failed or empty\n", page.json);
          continue;
        }
      }
    }

    AutoConnectAux* aux = new AutoConnectAux;
    if (aux) {
      // Loading AutoConnectAux JSON description
      bool  ldcc = false;
      if (jsonFile)
        ldcc = aux->load(jsonFile);
      else if (page.json)
        ldcc = aux->load(page.json);
      else if (page.json_p)
        ldcc = aux->load(page.json_p);
      if (ldcc) {
        if (page.auxHandler) {
          aux->on(page.auxHandler);
          if (_portal) {
            AutoConnectAux* hasLoaded = _portal->aux(aux->uri());
            if (hasLoaded) {
              _portal->detach(hasLoaded->uri());
              delete hasLoaded;
            }
            _portal->join(*aux);
          }
          else {
            _auxQueue.push_back(aux);
            ED_DBG("%s has entered _auxQueue.\n", aux->uri());
          }
        }
      }
      else
        // JSON deserialize error, ignore AutoCOnnectAux
        delete aux;
    }
    else {
      ED_DBG("New AutoConnectAux allocation failed\n");
    }

    // Delete instances in `file:` of `PGM_P json` file.
    if (jsonFile)
      jsonFile.close();
  }
}

/**
 * Calls the on-error callback of all EdgeDrivers bound to EdgeUnified to abort
 * processing. Once the error callback is called, the EdgeDriver's process
 * callback is disabled until the EdgeDriver::start function is executed.
 * @param  error  Error code. The value should be uniquely defined by the user
 * sketch according to the EdgeDriver properties. EdgeUnified is not involved
 * in the value and semantics of the error code.
 */
void EdgeUnified::abort(const int error) {
  for (EdgeDriverBase& driver : _drivers)
    driver.error(error);
}

/**
 * Calls the end callback of all EdgeDrivers bound to EdgeUnified to end
 * processing.
 */
void EdgeUnified::end(void) {
  for (EdgeDriverBase& driver : _drivers)
    driver.end();
}

/**
 * Loads the JSON custom web page descriptions which have not yet loaded into
 * AutoConnect among those bound to EdgeUnified.
 * @param  portal A reference to the AutoConnect instance.
 */
void EdgeUnified::portal(AutoConnect& portal) {
  if (!_portal)
    _portal = &portal;
  
  if (_auxQueue.size()) {
    for (AutoConnectAux* aux : _auxQueue)
      _portal->join(*aux);
    _auxQueue.clear();
  }
}

/**
 * Consecutively calls the process function of the EdgeDrivers bound to the
 * EdgeUnifined to execute an event loop.
 * EdgeUnified::process function with the portal argument allows EdgeUnified
 * to dynamically load and bind AutoConnectAux. Sketches can release and join
 * AutoConnectAuxes during EdgeUnified's event loop using the process function
 * with a portal argument.
 * @param  portal A reference to the AutoConnect instance.
 */
void EdgeUnified::process(AutoConnect& portal) {
  // Bind AutoConnectAuxes waiting to join the portal to AutoConnect
  EdgeUnified::portal(portal);
  process();
}

/**
 * Consecutively calls the process function of the EdgeDrivers bound to the
 * EdgeUnifined to execute an event loop.
 */
void EdgeUnified::process(void) {
  // Loop for EdgeDrivers
  for (EdgeDriverBase& driver : _drivers)
    driver.process();
}

/**
 * Releases AutoConnectAux with the specified uri from EdgeUnified.
 * @param  uri    Specify the uri of AutoConnectAux to be released from EdgeUnified.
 * @return true   Released AutoConnectAux with specified uri from EdgeUnified.
 * @return false  AutoConnectAux with specified uri is not joined.
 */
bool EdgeUnified::release(const String& uri) {
  if (!_portal) {
    ED_DBG("Releasing %s, AutoConnect not bound\n", uri.c_str());
    return false;
  }
  return _portal->detach(uri);
}

/**
 * Restore EdgeDates using EdgeData::restore function of all EdgeDrivers bound
 * to EdgeUnified. If the autoMount argument is set to true, the file system
 * will automatically begin and end as needed.
 * @param  autoMount  If True, the restore function tries to begin the
 * specified file system. It also terminates the file system at the exit of
 * the function.
 */
void EdgeUnified::restore(AUTOCONNECT_APPLIED_FILECLASS& fs, const bool autoMount) {

  if (autoMount) {
    bool  mounted = AutoConnectFS::_isMounted(&fs);
    if (!mounted) {
      if (!fs.begin(AUTOCONNECT_FS_INITIALIZATION)) {
        ED_DBG("%s mount failed\n", AUTOCONNECT_STRING_DEPLOY(AUTOCONNECT_APPLIED_FILESYSTEM));
        return;
      }
    }
  }

  for (EdgeDriverBase& driver : _drivers)
    driver.restore(fs, nullptr);

  if (autoMount)
    fs.end();
}

/**
 * Save EdgeDates using EdgeData::save function of all EdgeDrivers bound to
 * EdgeUnified. If the autoMount argument is set to true, the file system will
 * automatically begin and end as needed.
 * @param  autoMount  If True, the save function tries to begin the specified
 * file system. It also terminates the file system at the exit of the function.
 */
void EdgeUnified::save(AUTOCONNECT_APPLIED_FILECLASS& fs, const bool autoMount) {
  if (autoMount) { 
    bool  mounted = AutoConnectFS::_isMounted(&fs);
    if (!mounted) {
      if (!fs.begin(AUTOCONNECT_FS_INITIALIZATION)) {
        ED_DBG("%s mount failed\n", AUTOCONNECT_STRING_DEPLOY(AUTOCONNECT_APPLIED_FILESYSTEM));
        return;
      }
    }
  }

  for (EdgeDriverBase& driver : _drivers)
    driver.save(fs, nullptr);

  if (autoMount)
    fs.end();
}

// Export an EdgeUnified instance as an Edge to the global.
#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EDGE)
EdgeUnified Edge;
#endif
