/*
* This file is part of Teseo Android HAL
*
* Copyright (c) 2016-2017, STMicroelectronics - All Rights Reserved
* Author(s): Baudouin Feildel <baudouin.feildel@st.com> for STMicroelectronics.
*
* License terms: Apache 2.0.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/
/**
 * @brief Android Location Service proxy
 * @file LocServiceProxy.h
 * @author Baudouin Feildel <baudouin.feildel@st.com>
 * @copyright 2016, STMicroelectronics, All rights reserved.
 */

#ifndef TESEO_HAL_LOCATION_PROXY_H
#define TESEO_HAL_LOCATION_PROXY_H

#include <cstdint>
#include <string>
#include <map>
#include <hardware/gps.h>

#include <teseo/utils/Signal.h>
#include <teseo/model/NmeaMessage.h>
#include <teseo/model/Location.h>
#include <teseo/model/SatInfo.h>
#include <teseo/geofencing/model.h>

namespace stm {

/**
 * The LocServiceProxy namespace contains functions, structures and signals used to communicate
 * between the Android Location Services and the HAL.
 * 
 * @details    The HAL interfaces are
 */
namespace LocServiceProxy {

/**
 * @brief      HAL public interfaces
 */
struct Interfaces {
	GpsInterface                  gps;
	GpsXtraInterface              xtra;
	GpsDebugInterface             debug;
	AGpsInterface                 agps;
	SuplCertificateInterface      suplCertificate;
	GpsNiInterface                ni;
	AGpsRilInterface              ril;
	GpsGeofencingInterface        geofencing;
	GpsMeasurementInterface       measurement;
	GpsNavigationMessageInterface navigationMessage;
	GnssConfigurationInterface    configuration;
};

/**
 * @brief      Register GpsCallbacks
 *
 * @param[in]  cb    The GpsCallbacks to register
 */
void RegisterCallbacks(const GpsCallbacks * cb);

/**
 * @brief      Open function called by the Android platform to create the "HAL device"
 *
 * @param[in]  module  The GPS module instance
 * @param      name    The GPS module name
 * @param      dev     The pointer to device where to store the allocated device
 *
 * @return     0 on success, 1 on failure
 */
int openDevice(const struct hw_module_t * module, char const * name, struct hw_device_t ** dev);

/**
 * @brief      Close function called by the Android platform to close the "HAL device"
 *
 * @param      dev   The device to close
 *
 * @return     0 on succes, 1 on failure
 */
int closeDevice(struct hw_device_t * dev);

/**
 * @brief      Function called by the Android platform to get the HAL Gps Interface
 *
 * @param      device  The HAL device
 *
 * @return     The gps interface.
 */
const GpsInterface * getGpsInterface(struct gps_device_t * device);

/**
 * Namespace containing interfaces and callbacks from GpsInterface and GpsCallbacks
 */
namespace gps {

	/**
	 * @brief GPS Signals received from Android Location services
	 */
	struct Signals {
		Signal<int, GpsCallbacks *> init = Signal<int, GpsCallbacks *>("gps::signals::init");

		Signal<int> start = Signal<int>("gps::signals::start");

		Signal<int> stop = Signal<int>("gps::signals::stop");

		Signal<void> cleanup = Signal<void>("gps::signals::cleanup");

		Signal<int, GpsUtcTime, int64_t, int> injectTime =
			Signal<int, GpsUtcTime, int64_t, int>("gps::signals::injectTime");

		Signal<int, double, double, float> injectLocation =
			Signal<int, double, double, float>("gps::signals::injectLocation");

		Signal<void, GpsAidingData> deleteAidingData =
			Signal<void, GpsAidingData>("gps::signals::deleteAidingData");

		Signal<int, GpsPositionMode, GpsPositionRecurrence, uint32_t, uint32_t, uint32_t> setPositionMode =
			Signal<int, GpsPositionMode, GpsPositionRecurrence, uint32_t, uint32_t, uint32_t>("gps::signals::setPositionMode");
	};

	/**
	 * @brief      Gets the GPS signal list
	 *
	 * @return     The GPS signal list
	 */
	Signals & getSignals();

	void sendNmea(GpsUtcTime timestamp, const NmeaMessage & nmea);

	void sendLocationUpdate(const Location & loc);

	void sendSatelliteListUpdate(const std::map<SatIdentifier, SatInfo> & satellites);

	void sendCapabilities(uint32_t capabilities);

	void sendStatusUpdate(GpsStatusValue status);

	void sendSystemInfo(uint16_t yearOfHardware);

	void acquireWakelock();

	void releaseWakelock();

	void requestUtcTime();

} // namespace gps

namespace geofencing {

	// Import only needed names to avoid too much writing
	using stm::geofencing::model::GeofenceDefinition;
	using stm::geofencing::model::GeofenceId;
	using stm::geofencing::model::Transition;
	using stm::geofencing::model::TransitionFlags;
	using stm::geofencing::model::SystemStatus;
	using stm::geofencing::model::OperationStatus;

	struct Signals {
		Signal<void, GpsGeofenceCallbacks *> init = Signal<void, GpsGeofenceCallbacks *>("geofencing::signals::init");

		Signal<void, GeofenceDefinition> addGeofenceArea = Signal<void, GeofenceDefinition>("geofencing::signals::addGeofenceArea");
		
		Signal<void, GeofenceId> pauseGeofence = Signal<void, GeofenceId>("geofencing::signals::pauseGeofence");
		
		Signal<void, GeofenceId, TransitionFlags> resumeGeofence = Signal<void, GeofenceId, TransitionFlags>("geofencing::signals::resumeGeofence");
		
		Signal<void, GeofenceId> removeGeofenceArea = Signal<void, GeofenceId>("geofencing::signals::removeGeofenceArea");
	};

	Signals & getSignals();

	void sendGeofenceTransition(GeofenceId geofence_id,  const Location & loc, Transition transition, GpsUtcTime timestamp);

	void sendGeofenceStatus(SystemStatus status, const Location & last_location);

	void answerGeofenceAddRequest(GeofenceId geofence_id, OperationStatus status);

	void answerGeofenceRemoveRequest(GeofenceId geofence_id, OperationStatus status);
	
	void answerGeofencePauseRequest(GeofenceId geofence_id, OperationStatus status);
	
	void answerGeofenceResumeRequest(GeofenceId geofence_id, OperationStatus status);
	
} // namespace geofencing

namespace debug {

	struct Signals {
		Signal<std::string> getInternalState = Signal<std::string>("debug::signals::getInternalState");
	};

	Signals & getSignals();

	/**
	 * @brief      Dump HAL internal state into buffer
	 *
	 * @param      buffer      The buffer
	 * @param[in]  bufferSize  The buffer size
	 *
	 * @return     Number of byte dumped
	 * 
	 * @todo Currently dummy data is dumped
	 */
	size_t getInternalState(char * buffer, size_t bufferSize);
}

} // namespace LocServiceProxy
} // namespace stm

#endif // TESEO_HAL_LOCATION_PROXY_H