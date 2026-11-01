#pragma once

#include <Windows.h>
#include "Controller.h"
#include "UnrealString.h"
#include "OnlineReplStructs.h"
#include "log.h"

#include <fstream>

class AGameModeBase;

class AGameSession : public AActor
{
public:
	static inline void (*KickPlayerOriginal)(AGameSession*, AController*);

	static void KickPlayerHook(AGameSession*, AController*) { return; }

	static inline void (*PreLoginOriginal)(AGameModeBase*, const FString&, const FString&, const FUniqueNetIdRepl&, FString&) = nullptr;
	static inline __int64 PreLoginImplAddr = 0;
	static inline int PreLoginIdx = -1;

	// Accept listen-server logins that carry no UniqueId (loopback/host client:
	// "PreLogin failure: Client failed to send unique id"). The resulting
	// rejection aborts our match and tears down the server, so treat them
	// like splitscreen guests instead of rejecting.
	static void PreLoginHook(AGameModeBase* GM, const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
	{
		{ std::ofstream T("PreLoginHook.txt", std::ios::app); T << "HIT PreLoginHook\n"; }

		// Empty ReplicationBytes = no id sent. Fail-safe: unknown layout
		// (offset <= 0) passes through to the original.
		static int ReplBytesOffset = -2;

		if (ReplBytesOffset == -2)
			ReplBytesOffset = FindOffsetStruct("/Script/Engine.UniqueNetIdRepl", "ReplicationBytes", false);

		{ std::ofstream T("PreLoginHook.txt", std::ios::app); T << "ReplBytesOffset=" << ReplBytesOffset << "\n"; }

		if (ReplBytesOffset > 0)
		{
			auto& Bytes = *(const TArray<uint8>*)(__int64(&UniqueId) + ReplBytesOffset);

			{ std::ofstream T("PreLoginHook.txt", std::ios::app); T << "Bytes.Num=" << Bytes.Num() << " Data=" << std::format("0x{:x}", __int64(Bytes.Data)) << "\n"; }

			if (Bytes.Num() <= 0 || Bytes.Data == nullptr)
			{
				{ std::ofstream FireMarker("PreLoginHook.txt", std::ios::app); FireMarker << "ACCEPTED empty-id login\n"; }
				return; // leave ErrorMessage empty = accept
			}
		}

		if (PreLoginOriginal)
			PreLoginOriginal(GM, Options, Address, UniqueId, ErrorMessage);
	}
};