#pragma once

#include <stdint.h>

namespace ssr::ads_b::es
{
    /*
     * Purpose: To provide accurate airborne position information.
     */
    struct __attribute__((__packed__)) AirbornePosition
    {
        uint8_t format_type : 5;
        uint8_t surveillance_status : 2;
        uint8_t sa_flag : 1; //SAF, Single Antenna Flag
        uint16_t altitude : 12;
        uint8_t t_flag : 1; // Is UTC synced
        uint8_t f_flag : 1; // CPR Format Odd/Event
        uint32_t latitude : 17;
        uint32_t longitude : 17;
    };
    static_assert(sizeof(AirbornePosition) == 7);

    /*
     * Purpose: To provide accurate surface position information.
     */
    struct __attribute__((__packed__)) SurfacePosition
    {
        uint8_t format_type : 4;
        uint8_t : 1;
        uint8_t movement : 7;
        uint8_t status : 1; //0=invalid, 1=valid
        uint8_t ground_track : 7;
        uint8_t t_flag : 1;
        uint8_t f_flag : 1;
        uint32_t latitude : 17;
        uint32_t longitude : 17;
    };
    static_assert(sizeof(SurfacePosition) == 7);

    /*
     * Purpose: To provide information on the capability and status
     * of the extended squitter rate of the transponder.
     */
    struct __attribute__((__packed__)) Status
    {
        uint8_t transmission_rate : 2;
        uint8_t altitude_type : 1;
        uint64_t : 53; //reserved
    };
    static_assert(sizeof(Status) == 7);

    /*
     * Purpose: To provide aircraft identification and category.
     */
    struct __attribute__((__packed__)) IdentAndCategory
    {
        uint8_t format_type : 5;
        uint8_t category : 3;
        char char_1 : 6;
        char char_2 : 6;
        char char_3 : 6;
        char char_4 : 6;
        char char_5 : 6;
        char char_6 : 6;
        char char_7 : 6;
        char char_8 : 6;
    };
    static_assert(sizeof(IdentAndCategory) == 7);

    /*
     * Purpose: To provide additional state information for both
     * normal and supersonic flight.
     * 
     * This covers Subtype 1/2 in BDS register 0,9
     */
    struct __attribute__((__packed__)) GroundSpeed
    {
        uint8_t format_type : 5;
        uint8_t format_subtype : 3; //TODO: Define (Page 58)
        uint8_t change_flag : 1;    //Intent Change Flag
        uint8_t ifr_flag : 1;       //TODO: Define (Page 58)
        uint8_t accuracy : 3;       //TODO: Define (Page 58)
        uint8_t ew_direction : 1;   //0=East, 1=West
        uint16_t ew_velocity : 10;  //East-West Velocity
        uint8_t ns_direction : 1;   //0=North, 1=South
        uint16_t ns_velocity : 10;
        uint8_t vr_source : 1;    // Vertical Rate Source [0=GNSS, 1=Baro]
        uint8_t vr_direction : 1; // Vertical Rate Direction [0=Up, 1=Down]
        uint16_t vertical_rate : 9;
        uint8_t : 2;                 //reserved for turn indicator
        uint8_t gnss_sign : 1;       //0=Above Baro, 1=Below Baro
        uint8_t gnss_difference : 7; //Difference in altitude from baro
    };
    static_assert(sizeof(GroundSpeed) == 7);

    /*
     * Purpose: To provide additional state information for both
     * normal and supersonic flight.
     * 
     * This covers Subtype 3/4 in BDS register 0,9
     */
    struct __attribute__((__packed__)) AirSpeed
    {
        uint8_t format_type : 5;
        uint8_t format_subtype : 3; //TODO: Define (Page 59)
        uint8_t change_flag : 1;    //Intent Change Flag
        uint8_t ifr_flag : 1;       //TODO: Define (Page 59)
        uint8_t accuracy : 3;       //TODO: Define (Page 59)
        uint8_t status : 1;         //0=Magnetic heading not available, 1=Available
        uint16_t heading : 10;
        uint8_t type : 1; //0=IAS, 1=TAS
        uint16_t airspeed : 10;
        uint8_t vr_source : 1;    // Vertical Rate Source [0=GNSS, 1=Baro]
        uint8_t vr_direction : 1; // Vertical Rate Direction [0=Up, 1=Down]
        uint16_t vertical_rate : 9;
        uint8_t : 2;                 //reserved for turn indicator
        uint8_t gnss_sign : 1;       //0=Above Baro, 1=Below Baro
        uint8_t gnss_difference : 7; //Difference in altitude from baro
    };
    static_assert(sizeof(AirSpeed) == 7);

    /*
     * Purpose: To provide a flexible means to squitter messages
     * other than position, velocity and identification.
     */
    struct __attribute__((__packed__)) EventInfo
    {
        uint64_t : 56;
    };
    static_assert(sizeof(EventInfo) == 7);

    /*
     * Purpose: To report threat aircraft state information in order
     * to improve the ability of ACAS to evaluate the threat and select
     * a resolution manoeuvre.
     */
    struct __attribute__((__packed__)) AirToAirState
    {
        uint8_t true_airspeed_status : 1; //TODO: Find desc in docs
        uint16_t true_airspeed : 11;      //0-2047 knots

        uint8_t heading_source : 1; //0=Magnetic, 1=True Heading
        uint8_t heading_status : 1; //TODO: Find desc in docs
        uint8_t heading_sign : 1;   //TODO: Find desc in docs
        uint16_t heading : 9;

        uint8_t true_track_status : 1; //TODO: Find desc in docs
        uint8_t true_track_sign : 1;   //TODO: Find desc in docs
        uint16_t true_track : 14;

        uint8_t ground_speed_status : 1; //TODO: Find desc in docs
        uint16_t ground_speed : 14;
        uint8_t : 1; //Reserved
    };
    static_assert(sizeof(AirToAirState) == 7);

    /*
     * Purpose: To report threat aircraft state information in order to
     * improve the ability of ACAS to evaluate the threat and select
     * a resolution maneuver.
     */
    struct __attribute__((__packed__)) AirToAirIntent
    {
        uint8_t altitude_status : 1; //TODO: Find desc in docs
        uint16_t altitude : 12;      //0-65,520 ft

        uint8_t next_course_status : 1; //TODO: Find desc in docs
        uint8_t next_course_sign : 1;   //TODO: Find desc in docs
        uint16_t next_course : 9;       // +/- 180 deg

        uint8_t time_to_next_waypoint_status : 1; //TODO: Find desc in docs
        uint16_t time_to_next_waypoint : 9;       //0-256 seconds

        uint8_t vertical_velocity_status : 1; //TODO: Find desc in docs
        uint8_t vertical_velocity_sign : 1;   //TODO: Find desc in docs
        int8_t vertical_velocity : 8;         //Starting from 64 ft/min (*32 ft/min)

        uint8_t roll_angle_status : 1; //TODO: Find desc in docs
        uint8_t roll_angle_sign : 1;   //TODO: Find desc in docs
        int8_t roll_angle : 7;  //Starting from 64 ft/min (*32 ft/min)

        uint8_t : 3; //Reserved
    };
    static_assert(sizeof(AirToAirIntent) == 7);

    /*
     * Purpose: To report the data link capability of the Mode S
     * transponder/data link installation.
     */
    struct __attribute__((__packed__)) DataLinkCapability
    {
        uint8_t bds_code : 8;
        uint8_t continuation : 1;
        uint8_t : 5;           //reserved
        uint8_t occ_flag : 1;  //Overlay Command Capability [0=no, 1=yes]
        uint8_t acas_flag : 1; //1=ACAS is active, 0=disabled/standby

        /* ModeS Version numbers
         * 0 = No ModeS
         * 1 = ICAO Doc.9688 (1996)
         * 2 = ICAO Doc.9688 (1998)
         * 3 = ICAO Annex 10, Vol III, Amdt 77
         * 4 = ICAO Doc.9871, Edition 1
         * 5 = ICAO Doc.9871, Edition 2
         * 6-127 = Reserved
         */
        uint8_t modes_version : 7;

        /* The enhanced protocol indicator shall denote a Level 5 transponder
         * when set to 1, and a Level 2 to 4 transponder when set to 0.
         */
        uint8_t enhanced_flag : 1;

        /* When this is set to 1, it shall indicate that at least one Mode S
         * specific service (other than GICB services related to 
         * registers 02, 03, 04, 10, 17-1C, 20 and 30) is supported and the
         * particular capability reports shall be checked.
         */
        uint8_t modes_flag : 1;

        /* Uplink ELM average throughput capability shall be coded as follows:
         * 0 = No UELM Capability
         * 1 = 16 UELM segments in 1 second
         * 2 = 16 UELM segments in 500 ms
         * 3 = 16 UELM segments in 250 ms
         * 4 = 16 UELM segments in 125 ms
         * 5 = 16 UELM segments in 60 ms
         * 6 = 16 UELM segments in 30 ms
         * 7 = Reserved
         */
        uint8_t elm_uplink_throughput : 3;

        /* Downlink ELM throughput capability shall be coded as follows:
         * 0 = No DELM Capability
         * 1 = One 4 segment DELM every second
         * 2 = One 8 segment DELM every second
         * 3 = One 16 segment DELM every second
         * 4 = One 16 segment DELM every 500 ms
         * 5 = One 16 segment DELM every 250 ms
         * 6 = One 16 segment DELM every 125 ms
         * 7-15 = Reserved
         */
        uint8_t elm_downlink_throughput : 4;

        /* Indicates the availability of Aircraft Identification data. It shall be set by the
         * transponder if the data comes to the transponder through a separate interface and
         * not through the ADLP.
         */
        uint8_t ident_capability : 1;

        /* The squitter capability subfield (SCS) shall be set to 1 if both registers
         * [0x05] and [0x06] have been updated within the last ten, plus or minus
         * one, seconds. Otherwise, it shall be set to 0.
         */
        uint8_t scs_flag : 1;

        /* The surveillance identifier code (SIC) bit shall be interpreted as follows:
         * 0 = no surveillance identifier code capability
         * 1 = surveillance identifier code capability
         */
        uint8_t sic_flag : 1;

        /* This shall be toggled each time the common usage GICB capability
         * report (register [0x17]) changes. To avoid the generation of too many
         * broadcast capability report changes, register [0x17] shall be sampled at
         * approximately one minute intervals to check for changes.
         */
        uint8_t gicb_flag : 1;

        /* Bit 0 shall be set to ONE (1) to indicate the capability of hybrid surveillance,
         * and set to ZERO (0) to indicate that there is no hybrid surveillance capability.
         * 
         * Bit 1 shall be set to ONE (1) to indicate that the ACAS is generating both TAs
         * and RAs, and set to ZERO (0) to indicate the generation of TAs only.
         * 
         * Bit 3 shall be set to ONE (1) to indicate the capability of TCAS and set to ZERO
         * (0) to indicate that there is no TCAS capability.
         * 
         * Bit 4 TCAS-B ??
         */
        uint8_t acas_capability : 4;

        /* 
         * Starting from the MSB, each subsequent bit position shall represent
         * the DTE subaddress in the range from 0 to 15
         * 
         * The current status of the on-board DTE shall be periodically reported
         * to the GDLP by on-board sources. Since a change in this field results
         * in a broadcast of the capability report, status inputs shall be sampled
         * at approximately one minute intervals.
         */
        uint16_t dte_flags : 16;
    };
    static_assert(sizeof(DataLinkCapability) == 7);

    /*
     * Purpose: To indicate common usage GICB services currently
     * supported.
     * 
     * Each bit position shall indicate that the associated register is
     * available in the aircraft installation when set to 1.
     */
    struct __attribute__((__packed__)) GICBCommonReport
    {
        uint8_t es_airborne_position : 1;
        uint8_t es_surface_position : 1;
        uint8_t es_status : 1;
        uint8_t es_ident_and_category : 1;
        uint8_t es_airborne_velocity : 1;
        uint8_t es_event_driven_info : 1;
        uint8_t identification : 1;
        uint8_t registration : 1;

        uint8_t vertical_intention : 1;
        uint8_t next_waypoint_identifier : 1;
        uint8_t next_waypoint_position : 1;
        uint8_t next_waypoint_information : 1;
        uint8_t meterological_routine_report : 1;
        uint8_t meterological_hazard_report : 1;
        uint8_t vhf_channel_report : 1;
        uint8_t track_and_turn_report : 1;

        uint8_t position_coarse : 1;
        uint8_t position_fine : 1;
        uint8_t air_referenced_state_vector : 1;
        uint8_t waypoint_1 : 1;
        uint8_t waypoint_2 : 1;
        uint8_t waypoint_3 : 1;
        uint8_t quasi_static_parameter_monitoring : 1;
        uint8_t heading_and_speed_report : 1;

        uint8_t : 2; //Reserved for aircraft capability
        uint8_t : 2; //Reserved for ModeS BITE (Built In Test Equipment)
        uint32_t military_applications : 28;
    };
    static_assert(sizeof(GICBCommonReport) == 7);

    /*
     * Purpose: To report aircraft identification to the ground.
     */
    struct __attribute__((__packed__)) Identification
    {
        uint8_t : 8;
        char char_1 : 6;
        char char_2 : 6;
        char char_3 : 6;
        char char_4 : 6;
        char char_5 : 6;
        char char_6 : 6;
        char char_7 : 6;
        char char_8 : 6;
    };
    static_assert(sizeof(Identification) == 7);

} // namespace ssr::ads_b::es
