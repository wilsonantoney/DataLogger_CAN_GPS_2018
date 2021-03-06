#ifndef SINGLE_GPS_FRAME_DATA_H
#define SINGLE_GPS_FRAME_DATA_H

#include "AGHUtils/ReadingClass.h"
#include "AGHUtils/FixedPoint.h"

using namespace std;

class SingleGPSFrameData : ReadableFromBin
{
public:
    enum class EnGPSFixType : int {
        Fix_NoFix = 1,
        Fix_2DFix = 2,
        Fix_3DFix = 3
    };

private:
    unsigned int msTime;
    tm           gpsDateTime;
    FixedPoint	 longitude;
    FixedPoint	 latitude;
    int 		 nSatellites;
    FixedPoint	 altitude; //< in meters
    FixedPoint	 speed; //< in km/h
    FixedPoint	 trackAngle; //< in degrees
    EnGPSFixType fixType;//< GPGSA
    FixedPoint	 horizontalPrecision;//< GPGSA
    FixedPoint	 verticalPrecision; //< GPGSA

    static const int GPS_FIXED_POINT_FRACTIONAL_BITS = 12;
public:
    SingleGPSFrameData(unsigned int msTime, ReadingClass& reader);

    unsigned int getMsTime() const;
    tm           getGpsDateTime() const;
    FixedPoint	 getLongitude() const;
    FixedPoint	 getLatitude() const;
    int 		 getNSatellites() const;
    FixedPoint	 getAltitude() const; //< in meters
    FixedPoint	 getSpeed() const; //< in km/h
    FixedPoint	 getTrackAngle() const; //< in km/h
    EnGPSFixType getFixType() const; //< GPGSA
    FixedPoint	 getHorizontalPrecision() const; //< GPGSA
    FixedPoint	 getVerticalPrecision() const; //< GPGSA

    virtual void readFromBin(ReadingClass& reader) override;
};

#endif // SINGLE_GPS_FRAME_DATA_H
