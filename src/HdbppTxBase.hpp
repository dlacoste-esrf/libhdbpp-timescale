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

#ifndef _HDBPP_TX_BASE_HPP
#define _HDBPP_TX_BASE_HPP

#include "AttributeName.hpp"

#include <iostream>
#include <tango.h>

// why is it OmniORB (via Tango)) and Pqxx define these types in different ways? Perhaps
// its the autotools used to configure them? Either way, we do not use tango, just need its
// types, so undef and allow the Pqxx defines to take precedent
#undef HAVE_UNISTD_H
#undef HAVE_SYS_TYPES_H
#undef HAVE_SYS_TIME_H
#undef HAVE_POLL

namespace hdbpp
{
template<typename Conn>
class HdbppTxBase
{
public:
    HdbppTxBase(Conn &conn) : _conn(conn) {}
    virtual ~HdbppTxBase() {}

    bool result() const noexcept { return _result; };

    /// @brief Print the HdbppTxBase object to the stream
    virtual void print(std::ostream &os) const noexcept { os << "HdbppTxBase(_result: " << _result << ")"; }

protected:
    // access functions for the connection the transaction
    // is templated with
    Conn &connection() { return _conn; }
    const Conn &connection() const { return _conn; }

    void setResult(bool state) noexcept { _result = state; }

    // small helper to generate the attribute name for the db consistently
    // across all the different tx classes
    static std::string attrNameForStorage(AttributeName &attr_name)
    {
        return "tango://" + attr_name.tangoHostWithDomain() + "/" + attr_name.fullAttributeName();
    }

private:
    // instance of the template type, this is the connection to
    // the storage backend, i.e. database, and all requests are routed
    // through it
    Conn &_conn;

    bool _result = false;
};

} // namespace hdbpp
#endif // _HDBPP_TX_BASE_HPP