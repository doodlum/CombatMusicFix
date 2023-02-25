inline void sendConsoleCommand(std::string a_command)
{
	const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
	const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
	if (script) {
		const auto selectedRef = RE::Console::GetSelectedRef();
		script->SetCommand(a_command);
		script->CompileAndRun(selectedRef.get());
		delete script;
	}
}

class CombatMusicFix {
private:
	static inline const std::string stopCombatMusic = "removemusic MUScombat";
public:
	static void fix() {
		auto asyncFunc = []() {
			std::this_thread::sleep_for(std::chrono::seconds(5));
			sendConsoleCommand(stopCombatMusic);
		};
		std::jthread t(asyncFunc);
		t.detach();
	}

};
class CombatEventHandler : public RE::BSTEventSink<RE::TESCombatEvent>
{
public:
	using EventResult = RE::BSEventNotifyControl;

	virtual EventResult ProcessEvent(const RE::TESCombatEvent* a_event, RE::BSTEventSource<RE::TESCombatEvent>*) {
		if (a_event->actor && a_event->actor->IsPlayerRef()) {
			if (a_event->newState == RE::ACTOR_COMBAT_STATE::kNone) {
				CombatMusicFix::fix();
			}
		}
		return EventResult::kContinue;
	}


	static bool Register()
	{
		

		static CombatEventHandler singleton;
		logger::info("Registering {}...", typeid(singleton).name());
		auto ScriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();
		
		if (!ScriptEventSource) {
			logger::error("Script event source not found");
			return false;
		}

		ScriptEventSource->AddEventSink(&singleton);

		logger::info("..done");

		return true;
	}

};



void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		logger::info("Data Loaded");
		CombatEventHandler::Register();
	case SKSE::MessagingInterface::kPostLoadGame:
		auto pc = RE::PlayerCharacter::GetSingleton();
		if (pc && !pc->IsInCombat()) {
			CombatMusicFix::fix();
		}
		break;
	}
}

void Load()
{
	SKSE::GetMessagingInterface()->RegisterListener("SKSE", MessageHandler);
}
