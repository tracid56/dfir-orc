//
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Copyright � 2020 ANSSI. All Rights Reserved.
//
// Author(s): fabienfl (ANSSI)
//

#pragma once

#include <boost/logic/tribool.hpp>

#include "Output/Text/Print.h"

namespace Orc {
namespace Text {

template <typename T>
void Print(Orc::Text::Tree<T>& root, const boost::logic::tribool& value)
{
    std::wstring_view valueString;

    if (value)
    {
        valueString = L"On";
    }
    else if (!value)
    {
        valueString = L"Off";
    }
    else
    {
        valueString = L"Indeterminate";
    }

    Print(root, valueString);
}

}  // namespace Text
}  // namespace Orc
