#include "AbstractDevice.h"

#define LOG_TAG "teseo_hal_AbstractDevice"
#include <cutils/log.h>
#include <time.h>

#include "../LocServiceProxy.h"

namespace stm {
namespace device {

AbstractDevice::AbstractDevice()
{
	stream = nullptr;
	decoder = nullptr;

	auto & gpsSignals = LocServiceProxy::gps::getSignals();
	gpsSignals.start.connect(SlotFactory::create(*this, &AbstractDevice::start));
	gpsSignals.stop.connect(SlotFactory::create(*this, &AbstractDevice::stop));


	onNmea.connect(SlotFactory::create(LocServiceProxy::gps::sendNmea));
	locationUpdate.connect(SlotFactory::create(LocServiceProxy::gps::sendLocationUpdate));
}

void AbstractDevice::connectStreamToDecoder()
{
	if(stream == nullptr)
	{
		ALOGE("Stream isn't set, won't connect.");
		return;
	}

	if(decoder == nullptr)
	{
		ALOGE("Decoder isn't set, won't connect.");
		return;
	}

	stream->onNewBytes.connect(SlotFactory::create(*decoder, &decoder::IDecoder::onNewBytes));
}

void AbstractDevice::setStream(stream::IStream * s)
{
	if(s == nullptr)
	{
		ALOGW("Setting stream to nullptr");
	}

	stream = s;
}

void AbstractDevice::setDecoder(decoder::IDecoder * d)
{
	if(d == nullptr)
	{
		ALOGW("Setting decoder to nullptr");
	}

	decoder = d;
}

void AbstractDevice::update()
{
	if(location.locationValidity())
		locationUpdate(location);
}

int AbstractDevice::start()
{
	ALOGI("Start navigation");
	LocServiceProxy::gps::acquireWakelock();
	LocServiceProxy::gps::requestUtcTime();
	decoder->start();
	stream->startReading();

	return 0;
}

int AbstractDevice::stop()
{
	ALOGI("Stop navigation");

	stream->stopReading();
	decoder->stop();
	LocServiceProxy::gps::releaseWakelock();

	return 0;
}

void AbstractDevice::setTimestamp(GpsUtcTime t)
{
	timestamp = t;
	location.timestamp(t);
}

void AbstractDevice::invalidateLocation()
{
	location.invalidateLocation();
}

void AbstractDevice::invalidateAltitude()
{
	location.invalidateAltitude();
}

void AbstractDevice::invalidateSpeed()
{
	location.invalidateSpeed();
}

void AbstractDevice::invalidateBearing()
{
	location.invalidateBearing();
}

void AbstractDevice::invalidateAccuracy()
{
	location.invalidateAccuracy();
}

void AbstractDevice::setLocation(double latitude, double longitude)
{
	location.latitude(latitude);
	location.longitude(longitude);
}

void AbstractDevice::setAltitude(double altitude)
{
	location.altitude(altitude);
}

void AbstractDevice::setSpeed(float speed)
{
	location.speed(speed);
}

void AbstractDevice::setBearing(float bearing)
{
	location.bearing(bearing);
}

void AbstractDevice::setAccuracy(float accuracy)
{
	location.accuracy(accuracy);
}

void AbstractDevice::emitNmea(const NmeaMessage & nmea)
{
	onNmea(timestamp, nmea);
}

} // namespace device
} // namespace stm