#pragma once

#include <stdint.h>
#include <exception>
#include <int56.hpp>

/* Change me */
typedef /* ssr::uint56_t */ uint64_t register_type;

namespace ssr::ads_b {
    /* ADS-B Transponder Registers
     *
     * As Assigned in A.2.1 of Doc.9871
     *
     */
    class Registers {
    private:
        register_type storage[0xFF];

    public:
        constexpr auto Value(uint8_t reg)                  -> register_type&         { return storage[reg]; }
        constexpr auto Value(uint8_t reg)            const -> const register_type&   { return storage[reg]; }

        template<class TMsg>
        constexpr auto SetData(uint8_t reg, const TMsg& m) {
            static_assert(sizeof(TMsg) == 7);
            storage[reg] = (uint64_t)m;
        }
        template<class TMsg>
        constexpr auto GetData(uint8_t reg) const -> const TMsg& {
            static_assert(sizeof(TMsg) == 7);
            return (const TMsg)storage[reg];
        }
        
        /* 0x00: Not valid */
        /* 0x01: Reserved */

        /* Linked Comm-B Segment 2 */
        static constexpr auto LinkedCommB_2 = 0x02;
        /* Linked Comm-B Segment 3 */
        static constexpr auto LinkedCommB_3 = 0x03;
        /* Linked Comm-B Segment 4 */
        static constexpr auto LinkedCommB_4 = 0x04;
        /* Extended Squitter: Airborne Position */
        static constexpr auto ES_AirbornePosition = 0x05;
        /* Extended Squitter: Surface Position */
        static constexpr auto ES_SurfacePosition = 0x06;
        /* Extended Squitter: Status */
        static constexpr auto ES_Status = 0x07;
        /* Extended Squitter: Identification and Category */
        static constexpr auto ES_IdentAndCategory = 0x08;
        /* Extended Squitter: Airborne Velocity */
        static constexpr auto ES_AirborneVelocity = 0x09;
        /* Extended Squitter: Event-Driven Information
         * Not used for ACAS or GICB
         */
        static constexpr auto ES_EventDrivenInfo = 0x0A;
        /* Air-Air Information 1 (Aircraft State) */
        static constexpr auto AirToAir_State = 0x0B;
        /* Air-Air Information 2 (Aircraft Intent) */
        static constexpr auto AirToAir_Intent = 0x0C;

        /* 0x0D-0x0E: A2A Reserved */
        /* 0x0F: ACAS Reserved */

        /* Data-Link Capability Report */
        static constexpr auto DataLinkCapability = 0x10;

        /* 0x11-0x16: Reserved for extension of Data-Link Capability */

        /* Common Usage: GICB Capability Report */
        static constexpr auto GICBCapability = 0x17;

        /* Mode-S Capability
         *
         * Each bit represents a register being enabled
         * [0x18] = 0x01 - 0x38
         * [0x19] = 0x39 - 0x70
         * [0x1a] = 0x71 - 0xA8
         * [0x1b] = 0xA9 - 0xE0
         * [0x1c] = 0xE1 - 0xFF (First 25bits are not used)
         * 
         * ModeS Specific Protocol, each register is split into 28 up/down channel bits
         * [0x1d] = MSP Channels 1-28
         * [0x1e] = MSP Channels 29-56
         * [0x1f] = MSP Channels 57-63 (the least significant bits of both uplink and downlink channel fields shall not be used.)
         */
        static constexpr auto ModeSCapability = 0x18;

        /* Aircraft Identification */
        static constexpr auto Identification = 0x20;
        /* Aircraft and Airline Registration Markings */
        static constexpr auto Registration = 0x21;
        /* Aircraft Antenna Positions */
        static constexpr auto AntennaPositions = 0x22;

        /* 0x23: Reserved for antenna positiion */
        /* 0x24: Reserved for aircraft parameters */

        /* Aircraft Type */
        static constexpr auto Type = 0x25;

        /* 0x26-0x2F: Reserved */

        /* ACAS active Resolution Advisory */
        static constexpr auto ACAS_ActiveResolutionAdvisory = 0x30;

        /* 0x31-0x3F: Reserved */

        /* Selected Vertical Intention */
        static constexpr auto VerticalIntention = 0x40;
        /* Next Waypoint Identifier */
        static constexpr auto NextWaypointIdentifier = 0x41;
        /* Next Waypoint Position */
        static constexpr auto NextWaypointPosition = 0x42;
        /* Next Waypoint Information */
        static constexpr auto NextWaypointInformation = 0x43;
        /* Meteorological Routine Air Report */
        static constexpr auto Meteorological_RoutineAirReport = 0x44;
        /* Meteorological Hazard Report */
        static constexpr auto Meteorological_HazardReport = 0x45;

        /* 0x46: Reserved for FMS Mode-1 */
        /* 0x47: Reserved for FMS Mode-2 */

        /* VHF Channel Report */
        static constexpr auto VHFChannel = 0x48;

        /* 0x49-0x4F: Reserved */

        /* Track And Turn Report */
        static constexpr auto TrackAndTurn = 0x50;
        /* Position Report Coarse */
        static constexpr auto PositionCoarse = 0x51;
        /* Position Report Fine */
        static constexpr auto PositionFine = 0x52;
        /* Air-Referenced State Vector */
        static constexpr auto AirRefStateVector = 0x53;
        /* Waypoint 1 */
        static constexpr auto Waypoint_1 = 0x54;
        /* Waypoint 2 */
        static constexpr auto Waypoint_2 = 0x55;
        /* Waypoint 3 */
        static constexpr auto Waypoint_3 = 0x56;

        /* 0x57-0x5E: Reserved */

        /* Quasi-Static Parameter Monitoring */
        static constexpr auto QuasiStaticPM = 0x5F;
        /* Heading and Speed report */
        static constexpr auto HeadingAndSpeed= 0x60;
        /* Extended Squitter: Emergency/Priority Status */
        static constexpr auto ES_EmergencyStatus = 0x61;
        
        /* 0x62: Reserved for target state and status information */
        /* 0x63: Reserved for extended squitter */
        /* 0x64: Reserved for extended squitter */
        
        /* Extended Squitter: Aircraft Operational Status */
        static constexpr auto ES_OperationalStatus = 0x65;
        
        /* 0x66-0x6F: Reserved for extended squitter */
        /* 0x70-0x75: Reserved for future aircraft downlink parameters */
        /* 0x76-0xE0: Reserved */
        /* 0xE1-0xE2: Reserved for Mode-S BITE */

        /* Transponder type/part number */
        static constexpr auto Transponder_PartNumber = 0xE3;
        /* Transponder software revision number */
        static constexpr auto Transponder_Revision = 0xE4;
        /* ACAS unit part number */
        static constexpr auto ACAS_PartNumber = 0xE5;
        /* ACAS unit software revision number */
        static constexpr auto ACAS_Revision = 0xE6;
        /* Transponder Status and Diagnostics */
        static constexpr auto Transponder_Status = 0xE7;
        
        /* 0xE8: Reserved for future diagnostics */
        /* 0xE9: Reserved for future diagnostics */

        /* Vender Specific Status and Diagnostics */
        static constexpr auto Vendor_Status = 0xEA;
        
        /* 0xEB: Reserved for future vendor specfic diagnostics */
        /* 0xEC: Reserved for future vendor specfic diagnostics */
        /* 0xED-0xF0: Reserved */
        
        /* Military Applications Register 1 */
        static constexpr auto MilitaryApplications_1 = 0xF1;
        /* Military Applications Register 2 */
        static constexpr auto MilitaryApplications_2 = 0xF2;
        
        /* 0xF3-0xFF: Reserved */

        /* Creates a Comm-B Data Selector */
        static constexpr const uint8_t BDS(uint8_t bds_1, uint8_t bds_2) {
            return (bds_1 << 4) || (bds_2 && 0x0F);
        }
    };
}