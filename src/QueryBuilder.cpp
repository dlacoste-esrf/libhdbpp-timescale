/* Copyright (C) : 2014-2019
   European Synchrotron Radiation Facility
   BP 220, Grenoble 38043, FRANCE

   This file is part of libhdb++timescale.

   libhdb++timescale is free software: you can redistribute it and/or modify
   it under the terms of the Lesser GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   libhdb++timescale is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the Lesser
   GNU General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with libhdb++timescale.  If not, see <http://www.gnu.org/licenses/>. */

#include "QueryBuilder.hpp"

#include <map>
#include <vector>

using namespace std;

namespace hdbpp_internal
{
namespace pqxx_conn
{
    namespace query_utils
    {
        // these specializations just return the correct postgres cast for the insert queries,
        // this is important for the custom types, since the library libpqxx and postgres will
        // not know how to store them.
        template<>
        auto postgresCast<double>(bool is_array) -> std::string
        {
            return is_array ? "float8[]" : "float8";
        }

        template<>
        auto postgresCast<float>(bool is_array) -> std::string
        {
            return is_array ? "float4[]" : "float4";
        }

        template<>
        auto postgresCast<string>(bool is_array) -> std::string
        {
            return is_array ? "text[]" : "text";
        }

        template<>
        auto postgresCast<bool>(bool is_array) -> std::string
        {
            return is_array ? "bool[]" : "bool";
        }

        template<>
        auto postgresCast<int32_t>(bool is_array) -> std::string
        {
            return is_array ? "int4[]" : "int4";
        }

        template<>
        auto postgresCast<uint32_t>(bool is_array) -> std::string
        {
            return is_array ? "ulong[]" : "ulong";
        }

        template<>
        auto postgresCast<int64_t>(bool is_array) -> std::string
        {
            return is_array ? "int8[]" : "int8";
        }

        template<>
        auto postgresCast<uint64_t>(bool is_array) -> std::string
        {
            return is_array ? "ulong64[]" : "ulong64";
        }

        template<>
        auto postgresCast<int16_t>(bool is_array) -> std::string
        {
            return is_array ? "int2[]" : "int2";
        }

        template<>
        auto postgresCast<uint16_t>(bool is_array) -> std::string
        {
            return is_array ? "ushort[]" : "ushort";
        }

        template<>
        auto postgresCast<uint8_t>(bool is_array) -> std::string
        {
            return is_array ? "uchar[]" : "uchar";
        }

        template<>
        auto postgresCast<vector<uint8_t>>(bool is_array) -> std::string
        {
            return is_array ? "bytea[]" : "bytea";
        }

        template<>
        auto postgresCast<Tango::DevState>(bool is_array) -> std::string
        {
            return is_array ? "int4[]" : "int4";
        }
    } // namespace query_utils

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeDataEventName(const AttributeTraits &traits) -> const string &
    {
        // generic check and emplace for new items
        return handleCache(_data_event_query_names, traits, StoreDataEvent);
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeDataEventErrorName(const AttributeTraits &traits) -> const string &
    {
        // generic check and emplace for new items
        return handleCache(_data_event_error_query_names, traits, StoreDataEventError);
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeAttributeStatement() -> const string &
    {
        // clang-format off
        static string query =
            "INSERT INTO " + schema::ConfTableName + " (" +
                schema::ConfColName + "," +
                schema::ConfColTypeId + "," +
                schema::ConfColFormatTypeId + "," +
                schema::ConfColWriteTypeId + "," +
                schema::ConfColTableName + "," +
                schema::ConfColCsName + "," + 
                schema::ConfColDomain + "," +
                schema::ConfColFamily + "," +
                schema::ConfColMember + "," +
                schema::ConfColLastName + "," + 
                schema::ConfColTtl + "," + 
                schema::ConfColHide + ") (" +
                "SELECT " + 
                    "$1," + 
                    schema::ConfTypeColTypeId + "," + 
                    schema::ConfFormatColFormatId + "," + 
                    schema::ConfWriteColWriteId + 
                    ",$2,$3,$4,$5,$6,$7,$8,$9 " +
                "FROM " + 
                    schema::ConfTypeTableName + ", " +
                    schema::ConfFormatTableName + ", " +
                    schema::ConfWriteTableName + " " +
                "WHERE " + schema::ConfTypeTableName + "." + schema::ConfTypeColTypeNum + " = $10 " + 
                "AND " + schema::ConfFormatTableName + "." + schema::ConfFormatColFormatNum + " = $11 " + 
                "AND " + schema::ConfWriteTableName + "." + schema::ConfWriteColWriteNum + " = $12) " +
                "RETURNING " + schema::ConfColId;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeHistoryStringStatement() -> const string &
    {
        // clang-format off
        static string query = 
            "INSERT INTO " + schema::HistoryEventTableName + " (" +
                schema::HistoryEventColEvent + 
                ") VALUES ($1) RETURNING " + schema::HistoryEventColEventId;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeHistoryEventStatement() -> const string &
    {
        // clang-format off
        static string query =
            "INSERT INTO " + schema::HistoryTableName + " (" + 
                schema::HistoryColId + "," +
                schema::HistoryColEventId + "," +
                schema::HistoryColTime + ") " +
                "SELECT " +
                    "$1," + schema::HistoryEventColEventId + ",CURRENT_TIMESTAMP(6)" +
                " FROM " + schema::HistoryEventTableName +
                " WHERE " + schema::HistoryEventColEvent + " = $2";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeParameterEventStatement() -> const string &
    {
        // clang-format off
        static string query =
            "INSERT INTO " +
            schema::ParamTableName + " (" +
            schema::ParamColId + "," +
            schema::ParamColEvTime + "," +
            schema::ParamColLabel + "," +
            schema::ParamColEnumLabels + "," +
            schema::ParamColUnit + "," +
            schema::ParamColStandardUnit + "," +
            schema::ParamColDisplayUnit + "," +
            schema::ParamColFormat + "," +
            schema::ParamColArchiveRelChange + "," +
            schema::ParamColArchiveAbsChange + "," +
            schema::ParamColArchivePeriod + "," +
            schema::ParamColDescription + ") " +
            
	    "VALUES ($1, TO_TIMESTAMP($2), $3, $4, $5, $6, $7, $8, $9, $10, $11, $12)";
        // clang-format on

        return query;
    }  

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeParameterEventString(const std::string &full_attr_name,
                    const std::string &event_time,
                    const std::string &label,
                    const std::vector<std::string> &enum_labels,
                    const std::string &unit,
                    const std::string &standard_unit,
                    const std::string &display_unit,
                    const std::string &format,
                    const std::string &archive_rel_change,
                    const std::string &archive_abs_change,
                    const std::string &archive_period,
                    const std::string &description) -> const string &
    {
        // clang-format off
        static string query =
            "INSERT INTO " +
            schema::ParamTableName + " (" +
            schema::ParamColId + "," +
            schema::ParamColEvTime + "," +
            schema::ParamColLabel + "," +
            schema::ParamColEnumLabels + "," +
            schema::ParamColUnit + "," +
            schema::ParamColStandardUnit + "," +
            schema::ParamColDisplayUnit + "," +
            schema::ParamColFormat + "," +
            schema::ParamColArchiveRelChange + "," +
            schema::ParamColArchiveAbsChange + "," +
            schema::ParamColArchivePeriod + "," +
            schema::ParamColDescription + ") " +
            "VALUES ('" + full_attr_name + "'";
        
        query = query + ",TO_TIMESTAMP(" + event_time + ")";

        query = query + ",'" + label + "'";
        auto iter = enum_labels.begin();
        string result = "ARRAY[";
                            
        result = result + "$$" + pqxx::to_string((*iter)) + "$$";
                                              
        for (++iter; iter != enum_labels.end(); ++iter)
        {
            result += ",";
            result += "$$" + pqxx::to_string((*iter)) + "$$";
        }                                                        
        result += "]";
        query = query + "," + result + "::text[]";
        query = query + ",'" + unit + "'";
        query = query + ",'" + standard_unit + "'";
        query = query + ",'" + display_unit + "'";
        query = query + ",'" + format + "'";
        query = query + ",'" + archive_rel_change + "'";
        query = query + ",'" + archive_abs_change + "'";
        query = query + ",'" + archive_period + "'";
        query = query + ",'" + description + "')";
            
        return query;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeDataEventErrorStatement(const AttributeTraits &traits) -> const string &
    {
        // search the cache for a previous entry
        auto result = _data_event_error_queries.find(traits);

        if (result == _data_event_error_queries.end())
        {
            auto param_number = 0;

            auto query = "INSERT INTO " + QueryBuilder::tableName(traits) + " (" + schema::DatColId + "," +
                schema::DatColDataTime;

            // split to ensure increments are in the correct order
            query = query + "," + schema::DatColQuality + "," + schema::DatColErrorDescId + ") VALUES ($" +
                to_string(++param_number);

            query = query + ",TO_TIMESTAMP($" + to_string(++param_number) + ")";

            query = query + "," + "$" + to_string(++param_number);
            query = query + "," + "$" + to_string(++param_number) + ")";

            // cache the query string against the traits
            _data_event_error_queries.emplace(traits, query);

            spdlog::debug("Built new data event error query and cached it against traits: {}", traits);
            spdlog::debug("New data event error query is: {}", query);

            // now return it (must dereference the map again to get the static version)
            return _data_event_error_queries[traits];
        }

        // return the previously cached example
        return result->second;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeDataEventErrorString(const string &id,
        const string &event_time,
        const string &quality,
        const string &err_id,
        const AttributeTraits &traits) -> std::string
    {
        // clang-format off
        auto query = "INSERT INTO " + 
            QueryBuilder::tableName(traits) + " (" +
            schema::DatColId + "," +
            schema::DatColDataTime + "," + 
            schema::DatColQuality + "," + 
            schema::DatColErrorDescId + ") " + 
            "VALUES (" + 
                id + "," +
                "TO_TIMESTAMP(" + event_time + ")," +
                quality + "," +
                err_id + ")";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeErrorStatement() -> const string &
    {
        // clang-format off
        static string query = 
            "INSERT INTO " + schema::ErrTableName + " (" +
                schema::ErrColErrorDesc + ") VALUES ($1) RETURNING " + schema::ErrColId;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::storeTtlStatement() -> const string &
    {
        // clang-format off
        static string query = 
            "UPDATE " + schema::ConfTableName + " SET " +
                schema::ConfColTtl + "=$1::int WHERE " + schema::ConfColId + "=$2";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::fetchAllValuesStatement(
        const string &column_name, const string &table_name, const string &reference) -> const string
    {
        return "SELECT " + column_name + ", " + reference + " " + "FROM " + table_name;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::fetchValueStatement(
        const string &column_name, const string &table_name, const string &reference) -> const string
    {
        return "SELECT " + column_name + " " + "FROM " + table_name + " WHERE " + reference + "=$1";
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::fetchLastHistoryEventStatement() -> const string &
    {
        // clang-format off
        static string query = 
            "SELECT " + schema::HistoryEventColEvent +
                " FROM " + schema::HistoryTableName +
                " JOIN " + schema::HistoryEventTableName +
                " ON " + schema::HistoryEventTableName + "." + 
                    schema::HistoryEventColEventId + "=" + schema::HistoryTableName + "." + schema::HistoryColEventId +
                " WHERE " + schema::HistoryColId + " =$1" +
                " ORDER BY " + schema::HistoryColTime + " DESC LIMIT 1";
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::fetchAttributeTraitsStatement() -> const string &
    {
        // clang-format off
        static string query = 
            "SELECT " + 
                schema::ConfTypeColTypeNum + "," +
                schema::ConfFormatColFormatNum + "," +
                schema::ConfWriteColWriteNum + " " +
            "FROM " +
	            schema::ConfTypeTableName + " t," +
                schema::ConfFormatTableName + " f," + 
                schema::ConfWriteTableName + " w, " + 
                "(SELECT " + 
                    schema::ConfColTypeId + "," + 
                    schema::ConfColFormatTypeId + "," + 
                    schema::ConfColWriteTypeId + " " + 
                "FROM " + schema::ConfTableName + " WHERE " + schema::ConfColName + "=$1) AS tmp " + 
            "WHERE " +
	            "t." + schema::ConfColTypeId + "=tmp." + schema::ConfColTypeId + " " +
            "AND " +  
            	"f." + schema::ConfColFormatTypeId + "=tmp." + schema::ConfColFormatTypeId + " " +
            "AND " + 
	            "w." + schema::ConfColWriteTypeId  + "=tmp." + schema::ConfColWriteTypeId;
        // clang-format on

        return query;
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::tableName(const AttributeTraits &traits) -> string
    {
        return schema::SchemaTablePrefix +
            [&traits]() {
                switch (traits.formatType())
                {
                    case Tango::SCALAR: return schema::TypeScalar;
                    case Tango::SPECTRUM: return schema::TypeArray;
                    case Tango::IMAGE: return schema::TypeImage;
                }

                return string("Unknown");
            }() +
            "_" + [&traits]() {
                switch (traits.type())
                {
                    case Tango::DEV_DOUBLE: return schema::TypeDevDouble;
                    case Tango::DEV_FLOAT: return schema::TypeDevFloat;
                    case Tango::DEV_STRING: return schema::TypeDevString;
                    case Tango::DEV_LONG: return schema::TypeDevLong;
                    case Tango::DEV_ULONG: return schema::TypeDevUlong;
                    case Tango::DEV_LONG64: return schema::TypeDevLong64;
                    case Tango::DEV_ULONG64: return schema::TypeDevUlong64;
                    case Tango::DEV_SHORT: return schema::TypeDevShort;
                    case Tango::DEV_USHORT: return schema::TypeDevUshort;
                    case Tango::DEV_BOOLEAN: return schema::TypeDevBoolean;
                    case Tango::DEV_UCHAR: return schema::TypeDevUchar;
                    case Tango::DEV_STATE: return schema::TypeDevState;
                    case Tango::DEV_ENCODED: return schema::TypeDevEncoded;
                    case Tango::DEV_ENUM: return schema::TypeDevEnum;
                }

                return string("Unknown");
            }();
    }

    //=============================================================================
    //=============================================================================
    auto QueryBuilder::handleCache(
        map<AttributeTraits, string> &cache, const AttributeTraits &traits, const string &stub) -> const string &
    {
        auto result = cache.find(traits);

        if (result == cache.end())
        {
            // clang-format off
            string new_name = stub + 
                "_Write_" + to_string(traits.writeType()) +
                "_Format_" + to_string(traits.formatType()) + 
                "_Type_" + to_string(traits.type());
            // clang-format on

            // add to the cache for future hits
            cache.emplace(traits, new_name);

            spdlog::debug("New query name: {} cached against traits:", new_name, traits);
            return cache[traits];
        }

        return result->second;
    }

    //=============================================================================
    //=============================================================================
    void QueryBuilder::print(std::ostream &os) const noexcept
    {
        os << "QueryBuilder(cached "
           << "data_event: name/query " << _data_event_query_names.size() << "/" << _data_event_queries.size() << ", "
           << "data_event_error: name/query " << _data_event_error_query_names.size() << "/"
           << _data_event_error_queries.size() << ")";
    }
} // namespace pqxx_conn
} // namespace hdbpp_internal
