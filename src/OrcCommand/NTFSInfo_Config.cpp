//
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// Copyright © 2011-2019 ANSSI. All Rights Reserved.
//
// Author(s): Jean Gautier (ANSSI)
//

#include "stdafx.h"

#include "NTFSInfo.h"
#include "ParameterCheck.h"
#include "SystemDetails.h"
#include "Temporary.h"
#include "ConfigFile_NTFSInfo.h"
#include "FileInfoCommon.h"

#include <vector>
#include <algorithm>

using namespace std;

using namespace Orc;
using namespace Orc::Command::NTFSInfo;

ConfigItem::InitFunction Main::GetXmlConfigBuilder()
{
    return Orc::Config::NTFSInfo::root;
}

HRESULT Main::GetColumnsAndFiltersFromConfig(const ConfigItem& configItem)
{
    using namespace string_view_literals;

    HRESULT hr = E_FAIL;

    const ConfigItem& configitem = configItem[NTFSINFO_COLUMNS];

    std::for_each(
        begin(configitem[DEFAULT].NodeList), end(configitem[DEFAULT].NodeList), [this](const ConfigItem& item) {
            if (item)
                config.DefaultIntentions = static_cast<Intentions>(
                    config.DefaultIntentions
                    | FileInfo::GetIntentions(
                        item.c_str(), NtfsFileInfo::g_NtfsAliasNames, NtfsFileInfo::g_NtfsColumnNames));
        });

    std::for_each(begin(configitem[ADD].NodeList), end(configitem[ADD].NodeList), [this](const ConfigItem& item) {
        HRESULT hr1 = E_FAIL;
        Filter filter;
        filter.bInclude = true;

        if (FAILED(
                hr1 = FileInfoCommon::GetFilterFromConfig(
                    item, filter, NtfsFileInfo::g_NtfsAliasNames, NtfsFileInfo::g_NtfsColumnNames)))
            return;
        else
        {
            if (hr1 == S_OK)
            {
                config.Filters.push_back(filter);
            }
            else if (hr1 == S_FALSE)
            {
                config.ColumnIntentions = static_cast<Intentions>(config.ColumnIntentions | filter.intent);
            }
        }
    });

    std::for_each(begin(configitem[OMIT].NodeList), end(configitem[OMIT].NodeList), [this](const ConfigItem& item) {
        HRESULT hr1 = E_FAIL;
        Filter filter;
        filter.bInclude = false;

        hr1 = FileInfoCommon::GetFilterFromConfig(
            item, filter, NtfsFileInfo::g_NtfsAliasNames, NtfsFileInfo::g_NtfsColumnNames);

        if (FAILED(hr1))
            return;
        else
        {
            if (hr1 == S_OK)
            {
                config.Filters.push_back(filter);
            }
            else if (hr1 == S_FALSE)
            {
                config.ColumnIntentions =
                    static_cast<Intentions>(config.ColumnIntentions & static_cast<Intentions>(~filter.intent));
            }
        }
    });

    return S_OK;
}

bool Main::GetKnownLocationFromConfig(const ConfigItem& config)
{
    if (config[NTFSINFO_KNOWNLOCATIONS])
        return true;
    return false;
}

std::wstring Main::GetWalkerFromConfig(const ConfigItem& config)
{
    if (config[NTFSINFO_WALKER])
    {
        using namespace std::string_view_literals;
        const auto USN = L"USN"sv;
        const auto MFT = L"MFT"sv;

        if (!equalCaseInsensitive((const std::wstring&)config[NTFSINFO_WALKER], USN, USN.size())
            && !equalCaseInsensitive((const std::wstring&)config[NTFSINFO_WALKER], MFT, MFT.size()))
        {
            return L""s;
        }
        return (std::wstring)config[NTFSINFO_WALKER];
    }
    return L""s;
}

boost::logic::tribool Main::GetResurrectFromConfig(const ConfigItem& config)
{
    if (config[NTFSINFO_RESURRECT])
    {
        using namespace std::string_view_literals;
        const auto NO = L"no"sv;

        if (equalCaseInsensitive((const std::wstring&)config[NTFSINFO_RESURRECT], NO, NO.size()))
            return false;
        else
            return true;
    }
    return boost::logic::indeterminate;
}

boost::logic::tribool Main::GetPopulateSystemObjectsFromConfig(const ConfigItem& config)
{
    if (config[NTFSINFO_POP_SYS_OBJ])
    {
        using namespace std::string_view_literals;
        const auto NO = L"no"sv;
        if (equalCaseInsensitive((const std::wstring&)config[NTFSINFO_POP_SYS_OBJ], NO, NO.size()))
            return false;
        else
            return true;
    }
    return boost::logic::indeterminate;
}

HRESULT Main::GetConfigurationFromConfig(const ConfigItem& configitem)
{
    HRESULT hr = E_FAIL;

    if (FAILED(hr = config.outFileInfo.Configure(configitem[NTFSINFO_FILEINFO])))
    {
        Log::Error("Invalid output specified [{}]", SystemError(hr));
        return hr;
    }

    if (FAILED(hr = config.outAttrInfo.Configure(configitem[NTFSINFO_ATTRINFO])))
    {
        Log::Error("Invalid attrinfo output specified [{}]", SystemError(hr));
        return hr;
    }

    if (FAILED(hr = config.outI30Info.Configure(configitem[NTFSINFO_I30INFO])))
    {
        Log::Error("Invalid attrinfo output specified [{}]", SystemError(hr));
        return hr;
    }

    if (FAILED(hr = config.outTimeLine.Configure(configitem[NTFSINFO_TIMELINE])))
    {
        Log::Error("Invalid timeline output file specified [{}]", SystemError(hr));
        return hr;
    }

    if (FAILED(hr = config.outSecDescrInfo.Configure(configitem[NTFSINFO_SECDESCR])))
    {
        Log::Error(L"Invalid secdescr output file specified [{}]", SystemError(hr));
        return hr;
    }

    auto walker = GetWalkerFromConfig(configitem);
    if (!walker.empty())
    {
        config.strWalker = walker;
    }

    if (configitem[NTFSINFO_COMPUTER])
    {
        WCHAR szComputerName[MAX_PATH];
        DWORD dwComputerLen = MAX_PATH;

        if (auto actualLen =
                ExpandEnvironmentStringsW(configitem[NTFSINFO_COMPUTER].c_str(), szComputerName, dwComputerLen);
            actualLen > 0)
        {
            m_utilitiesConfig.strComputerName.assign(szComputerName, actualLen - 1);
        }
    }

    config.bResurrectRecords = GetResurrectFromConfig(configitem);
    config.bGetKnownLocations = GetKnownLocationFromConfig(configitem);
    config.bPopSystemObjects = GetPopulateSystemObjectsFromConfig(configitem);

    if (boost::logic::indeterminate(config.bPopSystemObjects))
        config.bPopSystemObjects = false;
    config.locs.SetPopulateSystemObjects((bool)config.bPopSystemObjects);

    const ConfigItem& locationsConfig = configitem[NTFSINFO_LOCATIONS];

    if (boost::logic::indeterminate(config.bAddShadows))
    {
        for (auto& item : locationsConfig.NodeList)
        {
            if (item.SubItems[CONFIG_VOLUME_SHADOWS])
            {
                config.bAddShadows = true;
            }
        }
    }

    if (FAILED(hr = config.locs.AddLocationsFromConfigItem(locationsConfig)))
    {
        Log::Error(L"Failed to get locations definition from config [{}]", SystemError(hr));
        return hr;
    }

    if (FAILED(hr = GetColumnsAndFiltersFromConfig(configitem)))
    {
        Log::Error(L"Failed to get column definition from config [{}]", SystemError(hr));
        return hr;
    }

    return S_OK;
}

HRESULT Main::GetSchemaFromConfig(const ConfigItem& schemaitem)
{
    config.outFileInfo.Schema = TableOutput::GetColumnsFromConfig(
        config.outFileInfo.TableKey.empty() ? L"fileinfo" : config.outFileInfo.TableKey.c_str(), schemaitem);
    config.volumesStatsOutput.Schema = TableOutput::GetColumnsFromConfig(
        config.volumesStatsOutput.TableKey.empty() ? L"volstats" : config.volumesStatsOutput.TableKey.c_str(),
        schemaitem);
    config.outAttrInfo.Schema = TableOutput::GetColumnsFromConfig(
        config.outAttrInfo.TableKey.empty() ? L"attrinfo" : config.outAttrInfo.TableKey.c_str(), schemaitem);
    config.outI30Info.Schema = TableOutput::GetColumnsFromConfig(
        config.outI30Info.TableKey.empty() ? L"i30info" : config.outI30Info.TableKey.c_str(), schemaitem);
    config.outTimeLine.Schema = TableOutput::GetColumnsFromConfig(
        config.outTimeLine.TableKey.empty() ? L"timeline" : config.outTimeLine.TableKey.c_str(), schemaitem);
    config.outSecDescrInfo.Schema = TableOutput::GetColumnsFromConfig(
        config.outSecDescrInfo.TableKey.empty() ? L"secdescr" : config.outSecDescrInfo.TableKey.c_str(), schemaitem);
    return S_OK;
}

HRESULT Main::GetConfigurationFromArgcArgv(int argc, LPCWSTR argv[])
{
    HRESULT hr = E_FAIL;

    try
    {

        for (int i = 0; i < argc; i++)
        {
            switch (argv[i][0])
            {
                case L'/':
                case L'-':
                    if (ParameterOption(argv[i] + 1, L"Walker", config.strWalker))
                    {
                        if (config.strWalker.compare(L"USN") && config.strWalker.compare(L"MFT"))
                        {
                            Log::Error("Option /Walker should be like: /Walker=USN or /Walker=MFT");
                            return E_INVALIDARG;
                        }
                    }
                    else if (ParameterOption(argv[i] + 1, L"Computer", m_utilitiesConfig.strComputerName))
                        ;
                    else if (OutputOption(argv[i] + 1, L"FileInfo", config.outFileInfo))
                        ;
                    else if (OutputOption(argv[i] + 1, L"Out", config.outFileInfo))
                        ;
                    else if (OutputOption(argv[i] + 1, L"TimeLine", config.outTimeLine))
                        ;
                    else if (OutputOption(argv[i] + 1, L"AttrInfo", config.outAttrInfo))
                        ;
                    else if (OutputOption(argv[i] + 1, L"I30Info", config.outI30Info))
                        ;
                    else if (OutputOption(argv[i] + 1, L"SecDescr", config.outSecDescrInfo))
                        ;
                    else if (BooleanOption(argv[i] + 1, L"KnownLocations", config.bGetKnownLocations))
                        ;
                    else if (BooleanOption(argv[i] + 1, L"Shadows", config.bAddShadows))
                        ;
                    else if (ParameterOption(argv[i] + 1, L"ResurrectRecords", config.bResurrectRecords))
                        ;
                    else if (ParameterOption(argv[i] + 1, L"PopSysObj", config.bPopSystemObjects))
                        ;
                    else if (EncodingOption(argv[i] + 1, config.outFileInfo.OutputEncoding))
                    {
                        config.outI30Info.OutputEncoding = config.outAttrInfo.OutputEncoding =
                            config.outTimeLine.OutputEncoding = config.outSecDescrInfo.OutputEncoding =
                                config.outFileInfo.OutputEncoding;
                    }
                    else if (AltitudeOption(argv[i] + 1, L"Altitude", config.locs.GetAltitude()))
                        ;
                    else if (ProcessPriorityOption(argv[i] + 1))
                        ;
                    else if (UsageOption(argv[i] + 1))
                        ;
                    else if (IgnoreCommonOptions(argv[i] + 1))
                        ;
                    else
                    {
                        // Here we have the selected default intentions tag

                        LPCWSTR pCur = argv[i] + 1;

                        while (pCur)
                        {
                            LPWSTR pNext = wcschr((LPWSTR)pCur, L',');
                            if (pNext)
                            {
                                *pNext = L'\0';
                                config.DefaultIntentions = static_cast<Intentions>(
                                    config.DefaultIntentions
                                    | NtfsFileInfo::GetIntentions(
                                        pCur, NtfsFileInfo::g_NtfsAliasNames, NtfsFileInfo::g_NtfsColumnNames));
                                pCur = pNext + 1;
                            }
                            else
                            {
                                config.DefaultIntentions = static_cast<Intentions>(
                                    config.DefaultIntentions
                                    | NtfsFileInfo::GetIntentions(
                                        pCur, NtfsFileInfo::g_NtfsAliasNames, NtfsFileInfo::g_NtfsColumnNames));
                                pCur = NULL;
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    catch (...)
    {
        Log::Error("NTFSInfo failed during argument parsing, exiting");
        return E_ABORT;
    }
    // argc/argv parameters only
    if (boost::logic::indeterminate(config.bAddShadows))
        config.bAddShadows = false;
    if (boost::logic::indeterminate(config.bPopSystemObjects))
        config.bPopSystemObjects = false;
    config.locs.SetPopulateSystemObjects((bool)config.bPopSystemObjects);

    if (FAILED(hr = config.locs.AddLocationsFromArgcArgv(argc, argv)))
        return hr;

    if (FAILED(
            hr = FileInfoCommon::GetFiltersFromArgcArgv(
                argc, argv, config.Filters, NtfsFileInfo::g_NtfsAliasNames, NtfsFileInfo::g_NtfsColumnNames)))
        return hr;

    return S_OK;
}

HRESULT Main::CheckConfiguration()
{
    HRESULT hr = E_FAIL;

    if (!config.strWalker.compare(L"USN"))
    {
        Log::Trace("USN requirement: 'EXACT' altitude enforced");
        config.locs.GetAltitude() = LocationSet::Altitude::Exact;
    }

    if (m_utilitiesConfig.strComputerName.empty())
    {
        SystemDetails::GetOrcComputerName(m_utilitiesConfig.strComputerName);
    }

    if (config.bGetKnownLocations)
    {
        hr = config.locs.AddKnownLocations();
        if (FAILED(hr))
            return hr;
    }

    if (boost::logic::indeterminate(config.bAddShadows))
    {
        config.bAddShadows = false;
    }

    config.locs.Consolidate((bool)config.bAddShadows, FSVBR::FSType::NTFS);

    if (config.locs.IsEmpty() != S_OK)
    {
        Log::Critical(
            L"No NTFS volumes configured for parsing. Use \"*\" to parse all mounted volumes or list the volumes you "
            L"want parsed");
        return E_INVALIDARG;
    }

    if (config.outFileInfo.Type == OutputSpec::Kind::None && config.outAttrInfo.Type == OutputSpec::Kind::None
        && config.outTimeLine.Type == OutputSpec::Kind::None)
    {
        config.outFileInfo.Type = OutputSpec::Kind::TableFile;
        config.outFileInfo.Path = L"NTFSInfo.csv";
        config.outFileInfo.OutputEncoding = OutputSpec::Encoding::UTF8;
    }

    if (config.outFileInfo.Type == OutputSpec::Kind::Directory || config.outFileInfo.Type == OutputSpec::Kind::Archive)
    {
        config.volumesStatsOutput.Path = config.outFileInfo.Path;
        config.volumesStatsOutput.Type = config.outFileInfo.Type;
        config.volumesStatsOutput.OutputEncoding = config.outFileInfo.OutputEncoding;
    }

    if (config.DefaultIntentions == Intentions::FILEINFO_NONE)
    {
        config.DefaultIntentions =
            NtfsFileInfo::GetIntentions(L"Default", NtfsFileInfo::g_NtfsAliasNames, NtfsFileInfo::g_NtfsColumnNames);
    }

    if (boost::logic::indeterminate(config.bResurrectRecords))
    {
        config.bResurrectRecords = false;
    }

    // Default Parser is MFT;
    if (config.strWalker.empty())
        config.strWalker = L"MFT";

    config.ColumnIntentions = static_cast<Intentions>(
        config.DefaultIntentions
        | (config.Filters.empty() ? Intentions::FILEINFO_NONE : NtfsFileInfo::GetFilterIntentions(config.Filters)));

    if (config.outFileInfo.Type == OutputSpec::Kind::Directory)
    {
        if (FAILED(hr = ::VerifyDirectoryExists(config.outFileInfo.Path.c_str())))
        {
            Log::Error(
                L"Specified file information output directory '{}' is not a directory [{}]",
                config.outFileInfo.Path,
                SystemError(hr));
            return hr;
        }
    }
    if (config.outAttrInfo.Type == OutputSpec::Kind::Directory)
    {
        if (FAILED(hr = ::VerifyDirectoryExists(config.outAttrInfo.Path.c_str())))
        {
            Log::Error(
                L"Specified attribute information output directory '{}' is not a directory [{}]",
                config.outAttrInfo.Path,
                SystemError(hr));
            return hr;
        }
    }
    if (config.outTimeLine.Type == OutputSpec::Kind::Directory)
    {
        if (FAILED(hr = ::VerifyDirectoryExists(config.outTimeLine.Path.c_str())))
        {
            Log::Error(
                L"Specified timeline information output directory '{}' is not a directory [{}]",
                config.outTimeLine.Path,
                SystemError(hr));
            return hr;
        }
    }
    if (config.outSecDescrInfo.Type == OutputSpec::Kind::Directory)
    {
        if (FAILED(hr = ::VerifyDirectoryExists(config.outSecDescrInfo.Path.c_str())))
        {
            Log::Error(
                L"Specified secdescr information output directory '{}' is not a directory [{}]",
                config.outSecDescrInfo.Path,
                SystemError(hr));
            return hr;
        }
    }

    if (config.strWalker.empty())
    {
        Log::Error("A parser must be selected");
        return E_INVALIDARG;
    }

    return S_OK;
}
