//
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Copyright © 2011-2019 ANSSI. All Rights Reserved.
//
// Author(s): Jean Gautier (ANSSI)
//
#pragma once

#include "OrcLib.h"

#include "OrcException.h"

#include <optional>
#include <variant>
#include <filesystem>

namespace Orc {

template <typename _ResultT, typename _ErrorT = HRESULT> class Result
{
public:
    Result() = default;
    Result(const _ResultT& result)
        : m_result(result)
    {
    }
    Result(_ResultT&& result) noexcept
        : m_result(std::move(result))
    {
    }
    Result(const _ErrorT& err)
        : m_result(err)
    {
    }
    Result(_ErrorT&& err) noexcept
        : m_result(std::move(err))
    {
    }

    bool is_ok() const {
        return std::holds_alternative<_T>(m_result);
    }
    bool is_err() const {
        return !std::holds_alternative<_T>(m_result);
    }

    _ResultT unwrap()
    {
        std::visit([](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, _ResultT>)
                    return std::move(arg);
                else if constexpr (std::is_same_v<T, HRESULT>)
                    throw Orc::Exception(ExceptionSeverity::Continue, arg, L"unwrap called with error result");
                else if constexpr (std::is_same_v<T, _ErrorT>)
                    throw Orc::Exception(ExceptionSeverity::Continue, E_FAIL, L"unwrap called with error result");
                else
                    throw Orc::Exception(ExceptionSeverity::Continue, E_FAIL, L"unwrap called with invalid result state");
            },
            m_result);
    }
    _ResultT unwrap_or(_ResultT or_value)
    {
        std::visit(
            [](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, _T>)
                    return std::move(arg);
                else
                    return std::move(or_value);
            },
            m_result);
    }
    _ResultT unwrap_or_default()
    {
        std::visit(
            [](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, _T>)
                    return std::move(arg);
                else
                    return _ResultT();
            },
            m_result);
    }

private:

    std::variant<std::monostate, _ResultT, _ErrorT> m_result;

};

    using StringResult = Result<std::wstring>;
    using PathResult = Result<std::filesystem::path>;

}

#pragma managed(push, off)
