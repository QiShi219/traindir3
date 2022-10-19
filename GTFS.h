#ifndef _GTFS_H
#define _GTFS_H

#include "wx/filename.h"
#include "TDFile.h"
#include "trsim.h"
#include "Array.h"

class GTFS_Agency {
public:
        GTFS_Agency();
        ~GTFS_Agency();

        TString  _agencyId;
        TString  _agencyName;
        TString  _agencyUrl;
        TString  _agencyTimeZone;
        TString  _agencyLang;
        TString  _agencyPhone;
        TString  _agencyFareUrl;
};

class GTFS_Stop {
public:
        GTFS_Stop();
        ~GTFS_Stop();

        TString  _stopId;
        TString  _stopCode;
        TString  _stopName;
        TString  _stopDesc;
        double  _stopLat, _stopLon;
        TString  _zoneId;
        TString  _stopUrl;
        int     _locationType;      // 0=platform, 1=station with multiple platforms
        TString  _parentStation;
        TString  _stopTimeZone;
        int     _wheelchairBoarding;
};

class GTFS_Route {
public:
        GTFS_Route();
        ~GTFS_Route();

        TString  _routeId;
        TString  _agencyId;
        TString  _routeShortName;
        TString  _routeLongName;
        TString  _routeDesc;
        int     _routeType;
        TString  _routeUrl;
        int     _routeColor;
        int     _routeTextColor;
};

class GTFS_Trip {
public:
        GTFS_Trip();
        ~GTFS_Trip();

        TString  _routeId;
        TString  _serviceId;
        TString  _tripId;
        TString  _tripHeadsign;
        TString  _tripShortName;
        int     _directionId;
        TString  _blockId;
        TString  _shapeId;
};

class GTFS_StopTime {
public:
        GTFS_StopTime();
        ~GTFS_StopTime();

        TString  _tripId;
        TString  _arrivalTime;
        TString  _departureTime;
        TString  _stopId;
        int     _stopSequence;
        TString  _stopHeadsign;
        int     _pickupType;
        int     _dropoffType;
        double  _shapeDistTraveled;
};

class GTFS_Calendar {
public:
        GTFS_Calendar();
        ~GTFS_Calendar();

        int GetMask();

        TString  _serviceId;
        int      _days[7];       // 0=Monday, 7=Sunday
        TString  _startDate, _endDate;
};

class GTFS_CalendarDate {
public:
        GTFS_CalendarDate();
        ~GTFS_CalendarDate();
        TString  _serviceId;
        TString  _date;
        int     _exceptionType; // 1=added for date, 2=removed for date
};

class GTFS_FareAttribute {
public:
        GTFS_FareAttribute();
        ~GTFS_FareAttribute();
        TString  _fareId;
        double  _price;
        TString  _currencyType;
        int     _paymentMethod; // 0=on board, 1=before boarding
        int     _transfers;     // 0=no transfers, 1=tranfer once, 2=transfer twice, -1=unlimited transfers
        int     _transferDuration;
};

class GTFS_FareRule {
public:
        GTFS_FareRule();
        ~GTFS_FareRule();

        TString  _fareId;
        TString  _routeId;
        TString  _originId;
        TString  _destinationId;
        TString  _containsId;
};

class GTFS_Shape {
public:
        GTFS_Shape();
        ~GTFS_Shape();

        TString  _shapeId;
        double  _shapePtLat, _shapePtLon;
        int     _shapePtSequence;
        double  _shapeDistTraveled;
};

class GTFS_Frequency {
public:
        GTFS_Frequency();
        ~GTFS_Frequency();
        TString  _tripId;
        TString  _startTime;
        TString  _endTime;
        int     _headwaySecs;
        int     _exactTimes;
};

class GTFS_Transfer {
public:
        GTFS_Transfer();
        ~GTFS_Transfer();

        TString  _fromStopId;
        TString  _toStopId;
        int     _transferType;
        int     _minTransferTime;
};

class GTFS_FeedInfo {
public:
        GTFS_FeedInfo();
        ~GTFS_FeedInfo();

        TString  _feedPublisherName;
        TString  _feedPublisherUrl;
        TString  _feedLang;
        TString  _feedStartDate, _feedEndDate;
        TString  _feedVersion;
};

class GTFS {
public:
        GTFS();
        ~GTFS();

        bool    Load(const Char *rootDir);

        GTFS_Calendar *FindCalendarByService(const Char *serviceId);
        GTFS_Route *FindRouteById(const Char *routeId);
        void    SetOurRoutes(const Char *r);
        bool    IgnoreRoute(const Char *routeId);

        ManagedArray<GTFS_Agency *>        _agencies;
        ManagedArray<GTFS_Stop *>          _stops;
        ManagedArray<GTFS_Route *>         _routes;
        ManagedArray<GTFS_Trip *>          _trips;
        ManagedArray<GTFS_StopTime *>      _stopTimes;
        ManagedArray<GTFS_Calendar *>      _calendar;
        ManagedArray<GTFS_CalendarDate *>  _calendarDates;
        ManagedArray<GTFS_FareAttribute *> _fareAttributes;
        ManagedArray<GTFS_FareRule *>      _fareRules;
        ManagedArray<GTFS_Shape *>         _shapes;
        ManagedArray<GTFS_Frequency *>     _frequencies;
        ManagedArray<GTFS_Transfer *>      _transfers;
        GTFS_FeedInfo                      _feedInfo;

        wxString    _our_routes;       // ignore routes not in this list
};

extern  GTFS    *gtfs;

#endif // _GTFS_H
