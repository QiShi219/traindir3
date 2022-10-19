#include "GTFS.h"
#include "CSV.h"
#include "Traindir3.h"

extern  Traindir *traindir;

void    Panic(const void *msg)
{
        traindir->Panic();
}

GTFS_Agency::GTFS_Agency()
{
}

GTFS_Agency::~GTFS_Agency()
{
}


GTFS_Stop::GTFS_Stop()
{
}

GTFS_Stop::~GTFS_Stop()
{
}


GTFS_Route::GTFS_Route()
{
}


GTFS_Route::~GTFS_Route()
{
}



GTFS_Trip::GTFS_Trip()
{
}
GTFS_Trip::~GTFS_Trip()
{
}


GTFS_StopTime::GTFS_StopTime()
{
}

GTFS_StopTime::~GTFS_StopTime()
{
}


GTFS_Calendar::GTFS_Calendar()
{
}

GTFS_Calendar::~GTFS_Calendar()
{
}

int GTFS_Calendar::GetMask()
{
        int days = 0;
        int m = 1;
        int i;
        for(i = 0; i < 7; ++i) {
            if(_days[i])
                days |= m;
            m <<= 1;
        }
        return days;
}

GTFS_CalendarDate::GTFS_CalendarDate()
{
}

GTFS_CalendarDate::~GTFS_CalendarDate()
{
}


GTFS_FareAttribute::GTFS_FareAttribute()
{
}

GTFS_FareAttribute::~GTFS_FareAttribute()
{
}


GTFS_FareRule::GTFS_FareRule()
{
}

GTFS_FareRule::~GTFS_FareRule()
{
}


GTFS_Shape::GTFS_Shape()
{
}

GTFS_Shape::~GTFS_Shape()
{
}


GTFS_Frequency::GTFS_Frequency()
{
}

GTFS_Frequency::~GTFS_Frequency()
{
}


GTFS_Transfer::GTFS_Transfer()
{
}

GTFS_Transfer::~GTFS_Transfer()
{
}


GTFS_FeedInfo::GTFS_FeedInfo()
{
}

GTFS_FeedInfo::~GTFS_FeedInfo()
{
}



GTFS::GTFS()
{
}

GTFS::~GTFS()
{
}


static  CSVFile *exists(const Char *rootDir, const Char *fileName)
{
        Char    path[512];
        CSVFile *csv;

        wxSnprintf(path, sizeof(path)/sizeof(path[0]), wxT("%s/%s.txt"), rootDir, fileName);
        csv = new CSVFile(path);
        if(csv->Load())
            return csv;
        delete csv;
        return 0;
}


bool    GTFS::Load(const Char *rootDir)
{
        CSVFile *csv;
        if((csv = exists(rootDir, wxT("agency")))) {
            if(csv->ReadColumns()) {
                CSVColumn *agencyId       = csv->FindColumn(wxT("agency_id"));
                CSVColumn *agencyName     = csv->FindColumn(wxT("agency_name"));
                CSVColumn *agencyUrl      = csv->FindColumn(wxT("agency_url"));
                CSVColumn *agencyTimeZone = csv->FindColumn(wxT("agency_timezone"));
                CSVColumn *agencyLang     = csv->FindColumn(wxT("agency_lang"));
                CSVColumn *agencyPhone    = csv->FindColumn(wxT("agency_phone"));
                CSVColumn *agencyFareUrl  = csv->FindColumn(wxT("agency_fare_url"));
                while(csv->ReadLine()) {
                    GTFS_Agency *a = new GTFS_Agency();
                    csv->GetColumn(a->_agencyId, agencyId);
                    csv->GetColumn(a->_agencyName, agencyName);
                    csv->GetColumn(a->_agencyUrl, agencyUrl);
                    csv->GetColumn(a->_agencyTimeZone, agencyTimeZone);
                    csv->GetColumn(a->_agencyLang, agencyLang);
                    csv->GetColumn(a->_agencyPhone, agencyPhone);
                    csv->GetColumn(a->_agencyFareUrl, agencyFareUrl);
                    _agencies.Add(a);
                }
            }
            delete csv;
        }

        if((csv = exists(rootDir, wxT("stops")))) {
            if(csv->ReadColumns()) {
                CSVColumn *stopId = csv->FindColumn(wxT("stop_id"));
                CSVColumn *stopCode = csv->FindColumn(wxT("stop_code"));
                CSVColumn *stopName = csv->FindColumn(wxT("stop_name"));
                CSVColumn *stopDesc = csv->FindColumn(wxT("stop_desc"));
                CSVColumn *stopLat = csv->FindColumn(wxT("stop_lat"));
                CSVColumn *stopLon = csv->FindColumn(wxT("stop_lon"));
                CSVColumn *zoneId = csv->FindColumn(wxT("zone_id"));
                CSVColumn *stopUrl = csv->FindColumn(wxT("stop_url"));
                CSVColumn *locationType = csv->FindColumn(wxT("location_type"));
                CSVColumn *parentStation = csv->FindColumn(wxT("parent_station"));
                CSVColumn *stopTimeZone = csv->FindColumn(wxT("stop_timezone"));
                CSVColumn *wheelchairBoarding = csv->FindColumn(wxT("wheelchair_boarding"));
                while(csv->ReadLine()) {
                    GTFS_Stop *s = new GTFS_Stop();
                    csv->GetColumn(s->_stopId, stopId);
                    csv->GetColumn(s->_stopCode, stopCode);
                    csv->GetColumn(s->_stopName, stopName);
                    csv->GetColumn(s->_stopDesc, stopDesc);
                    csv->GetColumn(s->_stopLat, stopLat);
                    csv->GetColumn(s->_stopLon, stopLon);
                    csv->GetColumn(s->_zoneId, zoneId);
                    csv->GetColumn(s->_stopUrl, stopUrl);
                    csv->GetColumn(s->_locationType, locationType);
                    csv->GetColumn(s->_parentStation, parentStation);
                    csv->GetColumn(s->_stopTimeZone, stopTimeZone);
                    csv->GetColumn(s->_wheelchairBoarding, wheelchairBoarding);
                    _stops.Add(s);
                }
            }
            delete csv;
        } else
            return false;

        if((csv = exists(rootDir, wxT("routes")))) {
            if(csv->ReadColumns()) {
                CSVColumn *routeId = csv->FindColumn(wxT("route_id"));
                CSVColumn *agencyId = csv->FindColumn(wxT("agency_id"));
                CSVColumn *routeShortName = csv->FindColumn(wxT("route_short_name"));
                CSVColumn *routeLongName = csv->FindColumn(wxT("route_long_name"));
                CSVColumn *routeDesc = csv->FindColumn(wxT("route_desc"));
                CSVColumn *routeType = csv->FindColumn(wxT("route_type"));
                CSVColumn *routeUrl = csv->FindColumn(wxT("route_url"));
                CSVColumn *routeColor = csv->FindColumn(wxT("route_color"));
                CSVColumn *routeTextColor = csv->FindColumn(wxT("route_text_color"));
                while(csv->ReadLine()) {
                    GTFS_Route *r = new GTFS_Route();
                    csv->GetColumn(r->_routeId, routeId);
                    csv->GetColumn(r->_agencyId, agencyId);
                    csv->GetColumn(r->_routeShortName, routeShortName);
                    csv->GetColumn(r->_routeLongName, routeLongName);
                    csv->GetColumn(r->_routeDesc, routeDesc);
                    csv->GetColumn(r->_routeType, routeType);
                    csv->GetColumn(r->_routeUrl, routeUrl);
                    csv->GetColumnHex(r->_routeColor, routeColor);
                    csv->GetColumnHex(r->_routeTextColor, routeTextColor);
                    _routes.Add(r);
                }
            }
            delete csv;
        }


        if((csv = exists(rootDir, wxT("trips")))) {
            if(csv->ReadColumns()) {
                CSVColumn *routeId = csv->FindColumn(wxT("route_id"));
                CSVColumn *serviceId = csv->FindColumn(wxT("service_id"));
                CSVColumn *tripId = csv->FindColumn(wxT("trip_id"));
                CSVColumn *tripHeadSign = csv->FindColumn(wxT("trip_head_sign"));
                CSVColumn *tripShortName = csv->FindColumn(wxT("trip_short_name"));
                CSVColumn *directionId = csv->FindColumn(wxT("direction_id"));
                CSVColumn *blockId = csv->FindColumn(wxT("block_id"));
                CSVColumn *shapeId = csv->FindColumn(wxT("shape_id"));
                while(csv->ReadLine()) {
                    GTFS_Trip *t = new GTFS_Trip();

                    csv->GetColumn(t->_routeId, routeId);
                    csv->GetColumn(t->_serviceId, serviceId);
                    csv->GetColumn(t->_tripId, tripId);
                    csv->GetColumn(t->_tripHeadsign, tripHeadSign);
                    csv->GetColumn(t->_tripShortName, tripShortName);
                    csv->GetColumn(t->_directionId, directionId);
                    csv->GetColumn(t->_blockId, blockId);
                    csv->GetColumn(t->_shapeId, shapeId);
                    _trips.Add(t);
                }
            }
            delete csv;
        }


        if((csv = exists(rootDir, wxT("stop_times")))) {
            if(csv->ReadColumns()) {
                CSVColumn *tripId = csv->FindColumn(wxT("trip_id"));
                CSVColumn *arrivalTime = csv->FindColumn(wxT("arrival_time"));
                CSVColumn *departureTime = csv->FindColumn(wxT("departure_time"));
                CSVColumn *stopId = csv->FindColumn(wxT("stop_id"));
                CSVColumn *stopSequence = csv->FindColumn(wxT("stop_sequence"));
                CSVColumn *stopHeadsign = csv->FindColumn(wxT("stop_headsign"));
                CSVColumn *pickupType = csv->FindColumn(wxT("pickup_type"));
                CSVColumn *dropoffType = csv->FindColumn(wxT("dropoff_type"));
                CSVColumn *shapeDistTraveled = csv->FindColumn(wxT("shape_dist_traveled"));
                while(csv->ReadLine()) {
                    GTFS_StopTime *t = new GTFS_StopTime();

                    csv->GetColumn(t->_tripId, tripId);
                    csv->GetColumn(t->_arrivalTime, arrivalTime);
                    csv->GetColumn(t->_departureTime, departureTime);
                    csv->GetColumn(t->_stopId, stopId);
                    csv->GetColumn(t->_stopSequence, stopSequence);
                    csv->GetColumn(t->_stopHeadsign, stopHeadsign);
                    csv->GetColumn(t->_pickupType, pickupType);
                    csv->GetColumn(t->_dropoffType, dropoffType);
                    csv->GetColumn(t->_shapeDistTraveled, shapeDistTraveled);
                }
            }
            delete csv;
        }


        if((csv = exists(rootDir, wxT("calendar")))) {
            if(csv->ReadColumns()) {
                CSVColumn *serviceId = csv->FindColumn(wxT("service_id"));
                CSVColumn *mon = csv->FindColumn(wxT("monday"));
                CSVColumn *tue = csv->FindColumn(wxT("tuesday"));
                CSVColumn *wed = csv->FindColumn(wxT("wednesday"));
                CSVColumn *thu = csv->FindColumn(wxT("thursday"));
                CSVColumn *fri = csv->FindColumn(wxT("friday"));
                CSVColumn *sat = csv->FindColumn(wxT("saturday"));
                CSVColumn *sun = csv->FindColumn(wxT("sunday"));
                CSVColumn *startDate = csv->FindColumn(wxT("start_date"));
                CSVColumn *endDate = csv->FindColumn(wxT("end_date"));
                while(csv->ReadLine()) {
                    GTFS_Calendar *c = new GTFS_Calendar();

                    csv->GetColumn(c->_serviceId, serviceId);
                    csv->GetColumn(c->_days[0], mon); 
                    csv->GetColumn(c->_days[1], tue); 
                    csv->GetColumn(c->_days[2], wed); 
                    csv->GetColumn(c->_days[3], thu); 
                    csv->GetColumn(c->_days[4], fri); 
                    csv->GetColumn(c->_days[5], sat); 
                    csv->GetColumn(c->_days[6], sun); 
                    csv->GetColumn(c->_startDate, startDate);
                    csv->GetColumn(c->_endDate, endDate);
                    _calendar.Add(c);
                }
            }
            delete csv;
        }


        if((csv = exists(rootDir, wxT("calendar_dates")))) {
            if(csv->ReadColumns()) {
                CSVColumn *serviceId = csv->FindColumn(wxT("service_id"));
                CSVColumn *date = csv->FindColumn(wxT("date"));
                CSVColumn *exceptionType = csv->FindColumn(wxT("exception_type"));
                while(csv->ReadLine()) {
                    GTFS_CalendarDate *c = new GTFS_CalendarDate();

                    csv->GetColumn(c->_serviceId, serviceId);
                    csv->GetColumn(c->_date, date);
                    csv->GetColumn(c->_exceptionType, exceptionType);
                    _calendarDates.Add(c);
                }
            }
            delete csv;
        }

        if((csv = exists(rootDir, wxT("fare_attributes")))) {
            if(csv->ReadColumns()) {
                CSVColumn *fareId = csv->FindColumn(wxT("fare_id"));
                CSVColumn *price = csv->FindColumn(wxT("price"));
                CSVColumn *currencyType = csv->FindColumn(wxT("currency_type"));
                CSVColumn *paymentMethod = csv->FindColumn(wxT("payment_method"));
                CSVColumn *transfers = csv->FindColumn(wxT("transfers"));
                CSVColumn *transferDuration = csv->FindColumn(wxT("transfer_duration"));
                while(csv->ReadLine()) {
                    GTFS_FareAttribute *f = new GTFS_FareAttribute();

                    csv->GetColumn(f->_fareId, fareId);
                    csv->GetColumn(f->_price, price);
                    csv->GetColumn(f->_currencyType, currencyType);
                    csv->GetColumn(f->_paymentMethod, paymentMethod);
                    csv->GetColumn(f->_transfers, transfers);
                    csv->GetColumn(f->_transferDuration, transferDuration);
                    _fareAttributes.Add(f);
                }
            }
            delete csv;
        }


        if((csv = exists(rootDir, wxT("fare_rules")))) {
            if(csv->ReadColumns()) {
                CSVColumn *fareId = csv->FindColumn(wxT("fare_id"));
                CSVColumn *routeId = csv->FindColumn(wxT("route_id"));
                CSVColumn *originId = csv->FindColumn(wxT("origin_id"));
                CSVColumn *destinationId = csv->FindColumn(wxT("destination_id"));
                CSVColumn *containsId = csv->FindColumn(wxT("contains_id"));
                while(csv->ReadLine()) {
                    GTFS_FareRule *f = new GTFS_FareRule();

                    csv->GetColumn(f->_fareId, fareId);
                    csv->GetColumn(f->_routeId, routeId);
                    csv->GetColumn(f->_originId, originId);
                    csv->GetColumn(f->_destinationId, destinationId);
                    csv->GetColumn(f->_containsId, containsId);
                    _fareRules.Add(f);
                }
            }
            delete csv;
        }

        if((csv = exists(rootDir, wxT("shapes")))) {
            if(csv->ReadColumns()) {
                CSVColumn *shapeId = csv->FindColumn(wxT("shape_id"));
                CSVColumn *shapePtLat = csv->FindColumn(wxT("shape_pt_lat"));
                CSVColumn *shapePtLon = csv->FindColumn(wxT("shape_pt_lon"));
                CSVColumn *shapePtSequence = csv->FindColumn(wxT("shape_pt_sequence"));
                CSVColumn *shapeDistTraveled = csv->FindColumn(wxT("shape_dist_traveled"));
                while(csv->ReadLine()) {
                    GTFS_Shape *s = new GTFS_Shape();

                    csv->GetColumn(s->_shapeId, shapeId);
                    csv->GetColumn(s->_shapePtLat, shapePtLat);
                    csv->GetColumn(s->_shapePtLon, shapePtLon);
                    csv->GetColumn(s->_shapePtSequence, shapePtSequence);
                    csv->GetColumn(s->_shapeDistTraveled, shapeDistTraveled);
                    _shapes.Add(s);
                }
            }
            delete csv;
        }


        if((csv = exists(rootDir, wxT("frequencies")))) {
            if(csv->ReadColumns()) {
                CSVColumn *tripId = csv->FindColumn(wxT("trip_id"));
                CSVColumn *startTime = csv->FindColumn(wxT("start_time"));
                CSVColumn *endTime = csv->FindColumn(wxT("end_time"));
                CSVColumn *headwaySecs = csv->FindColumn(wxT("headways_secs"));
                CSVColumn *exactTimes = csv->FindColumn(wxT("exact_times"));
                while(csv->ReadLine()) {
                    GTFS_Frequency *f = new GTFS_Frequency();

                    csv->GetColumn(f->_tripId, tripId);
                    csv->GetColumn(f->_startTime, startTime);
                    csv->GetColumn(f->_endTime, endTime);
                    csv->GetColumn(f->_headwaySecs, headwaySecs);
                    csv->GetColumn(f->_exactTimes, exactTimes);
                    _frequencies.Add(f);
                }
            }
            delete csv;
        }


        if((csv = exists(rootDir, wxT("transfers")))) {
            if(csv->ReadColumns()) {
                CSVColumn *fromStopId = csv->FindColumn(wxT("from_stop_id"));
                CSVColumn *toStopId = csv->FindColumn(wxT("to_stop_id"));
                CSVColumn *transferType = csv->FindColumn(wxT("transfer_type"));
                CSVColumn *minTransferTime = csv->FindColumn(wxT("min_transfer_time"));
                while(csv->ReadLine()) {
                    GTFS_Transfer *t = new GTFS_Transfer();

                    csv->GetColumn(t->_fromStopId, fromStopId);
                    csv->GetColumn(t->_toStopId, toStopId);
                    csv->GetColumn(t->_transferType, transferType);
                    csv->GetColumn(t->_minTransferTime, minTransferTime);
                    _transfers.Add(t);
                }
            }
            delete csv;
        }

        if((csv = exists(rootDir, wxT("feed_info")))) {
            if(csv->ReadColumns()) {
                CSVColumn *feedPublisherName = csv->FindColumn(wxT("feed_publisher_name"));
                CSVColumn *feedPublisherUrl = csv->FindColumn(wxT("feed_publisher_url"));
                CSVColumn *feedLang = csv->FindColumn(wxT("feed_lang"));
                CSVColumn *feedStartDate = csv->FindColumn(wxT("feed_start_date"));
                CSVColumn *feedEndDate = csv->FindColumn(wxT("feed_end_date"));
                CSVColumn *feedVersion = csv->FindColumn(wxT("feed_version"));
                if(csv->ReadLine()) {
                    csv->GetColumn(_feedInfo._feedPublisherName, feedPublisherName);
                    csv->GetColumn(_feedInfo._feedPublisherUrl, feedPublisherUrl);
                    csv->GetColumn(_feedInfo._feedLang, feedLang);
                    csv->GetColumn(_feedInfo._feedStartDate, feedStartDate);
                    csv->GetColumn(_feedInfo._feedEndDate, feedEndDate);
                    csv->GetColumn(_feedInfo._feedVersion, feedVersion);
                }
            }
            delete csv;
        }

        return true;
}


GTFS_Calendar *GTFS::FindCalendarByService(const Char *serviceId)
{
        for(int c = 0; c < _calendar.Length(); ++c) {
            GTFS_Calendar *calEntry = _calendar.At(c);
            if(calEntry->_serviceId.CompareTo(serviceId) == 0) {
                return calEntry;
            }
        }
        return 0;
}


GTFS_Route *GTFS::FindRouteById(const Char *routeId)
{
        int r;
        GTFS_Route *route;

        for(r = 0; r < _routes.Length(); ++r) {
            route = _routes.At(r);
            if(route->_routeId.CompareTo(routeId) == 0)
                return route;
        }
        return 0;
}


void    GTFS::SetOurRoutes(const Char *r)
{
        _our_routes = r;
}

bool    GTFS::IgnoreRoute(const Char *routeId)
{
        if(_our_routes.IsNull() || _our_routes.IsEmpty())
            return false;       // no route list specified, allow all
        Char    buff[256];
        wxStrncpy(buff, _our_routes.c_str(), sizeof(buff)/sizeof(buff[0]));
        Char    *p, *begin;
        for(p = begin = buff; *p; ) {
            if(*p == ',') {
                *p++ = 0;
                if(!wxStrcmp(begin, routeId))
                    return false;
                begin = p;
            } else
                ++p;
        }
        if(begin != p) {
            if(!wxStrcmp(begin, routeId))
                return false;
        }
        return true;        // not found in our list, so ignore it
}
