#pragma once

#include "Controller.h"
#include "UnrealString.h"
#include "OnlineReplStructs.h"
#include "log.h"

#include <fstream>
#include <intrin.h>

class AGameModeBase;

class AGameSession : public AActor
{
public:
	static inline void (*KickPlayerOriginal)(AGameSession*, AController*);

	static void KickPlayerHook(AGameSession*, AController*) { return; }

	static inline void (*PreLoginOriginal)(AGameModeBase*, const FString&, const FString&, const FUniqueNetIdRepl&, FString&) = nullptr;
	static inline __int64 PreLoginImplAddr = 0;

	// Accept local listen-server logins even when the client sends no UniqueId
	// (cracked builds + Cobalt: "PreLogin failure: Client failed to send unique id").
	// DIAGNOSTIC VERSION: logs raw args + caller, then passes through to the original.
	static void PreLoginHook(AGameModeBase* GM, const FString& A2, const FString& A3, const FUniqueNetIdRepl& A4, FString& A5, void* A6 = nullptr, void* A7 = nullptr)
	{
		std::ofstream marker("PreLoginHook.txt", std::ios::app);
		marker << "hit gm=" << std::format("0x{:x}", __int64(GM))
			<< " ret=" << std::format("0x{:x}", __int64(_ReturnAddress()))
			<< " a2=" << std::format("0x{:x}", __int64(&A2))
			<< " a3=" << std::format("0x{:x}", __int64(&A3))
			<< " a4=" << std::format("0x{:x}", __int64(&A4))
			<< " a5=" << std::format("0x{:x}", __int64(&A5))
			<< " a5data=" << std::format("0x{:x}", __int64(A5.Data.Data))
			<< std::endl;

		if (PreLoginOriginal)
			PreLoginOriginal(GM, A2, A3, A4, A5);
	}
};