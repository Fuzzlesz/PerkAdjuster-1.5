#include <spdlog/sinks/basic_file_sink.h>

#include "settings.h"
#include "perkManipulator.h"
#include "papyrus.h"
#include "serde.h"

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");

    auto pluginName = Version::NAME;
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));

    spdlog::set_default_logger(std::move(loggerPtr));
#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
#endif

    //Pattern
    spdlog::set_pattern("%v");
}

void MessageHandler(SKSE::MessagingInterface::Message* a_message) {
    switch (a_message->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        Settings::Read();
        PerkManipulation::Manipulator::Install();
        break;
    default:
        break;
    }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = Version::MAJOR, Version::MINOR, Version::PATCH;
    a_info->name = "ContainerDistributionFramework";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
        _loggerError("WRONG VERSION OF THE GAME");
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver

		< SKSE::RUNTIME_1_5_39
	) {
        _loggerError("WRONG VERSION OF THE GAME");
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
    SetupLog();
    _loggerInfo("Starting up {}.", Version::NAME);
    _loggerInfo("Plugin Version: {}.", Version::VERSION);
    _loggerInfo("-------------------------------------------------------------------------------------");
    SKSE::Init(a_skse);

    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(MessageHandler);

    const auto serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(Serialization::ID);
    serialization->SetSaveCallback(&Serialization::SaveCallback);
    serialization->SetLoadCallback(&Serialization::LoadCallback);
    serialization->SetRevertCallback(&Serialization::RevertCallback);

    SKSE::GetPapyrusInterface()->Register(Papyrus::RegisterFunctions);
    return true;
}