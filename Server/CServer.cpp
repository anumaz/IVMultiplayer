//========== IV:Multiplayer - https://github.com/IVMultiplayer/IVMultiplayer ==========
//
// File: CServer.cpp
// Project: Client.Core
// Author: xForce <xf0rc3.11@gmail.com>
// License: See LICENSE in root directory
//
//==============================================================================

#include "CServer.h"
#include <Common.h>
#include <CSettings.h>
#include <SharedUtility.h>
#include <CLogFile.h>

CServer::CServer()
{
	m_pNetServer = new CNetworkServer();
}

CServer::~CServer()
{
	SAFE_DELETE(m_pNetServer);
}


bool CServer::Startup(string& outStrError)
{
	// Open the settings file
	if(!CSettings::Open(SharedUtility::GetAbsolutePath("settings.xml"), false, false))
	{
		CLogFile::Print("Failed to open settings.xml, server will shut down in 3 seconds..");
		Sleep(3000);
		return false;
	}

	CLogFile::Print("");
	CLogFile::Print("====================================================================");


	CLogFile::Print(" " VERSION_IDENTIFIER " " OS_STRING " Server");
	CLogFile::Print(" Copyright (C) 2009-2013 IV:MP Team");
	CLogFile::Printf(" Port: %d", CVAR_GET_INTEGER("port"));
	CLogFile::Printf(" HTTP Port: %d", CVAR_GET_INTEGER("httpport"));
	CLogFile::Printf(" Query Port: %d", (CVAR_GET_INTEGER("queryport")));

	if(CVAR_GET_STRING("hostaddress").IsNotEmpty())
		CLogFile::Printf(" Host Address: %s", CVAR_GET_STRING("hostaddress").Get());

	if(CVAR_GET_STRING("httpserver").IsNotEmpty())
		CLogFile::Printf(" HTTP Server: %s", CVAR_GET_STRING("httpserver").Get());

	CLogFile::Printf(" Max Players: %d", CVAR_GET_INTEGER("maxplayers"));

	CLogFile::Print("====================================================================");
	

	if(!m_pNetServer->EnsureStarted(CVAR_GET_INTEGER("port"), CVAR_GET_INTEGER("maxplayers"), CVAR_GET_STRING("hostaddress")))
	{
		outStrError = "Failed to start network port.";
		return false;
	}
	
	// Load modules
	auto modules = CVAR_GET_LIST("module");
	if(modules.size() > 0)
	{
		CLogFile::Print("");
		CLogFile::Print("========================== Loading Modules =========================");

		for(auto strModule : modules)
		{
			CLogFile::Printf("Loading module %s.", strModule.Get());
			if(!strModule.ToLower().EndsWith(".so") && !strModule.ToLower().EndsWith(".dll"))
			{
				// If no extension specified in module name string, load SO for linux and DLL for Win
#ifdef _WIN32
				strModule.Append(".dll");
#else
				strModule.Append(".so");
#endif		
			}
			//CModule * pModule = g_pModuleManager->LoadModule(strModule);

			//if(!pModule)
				CLogFile::Printf("Warning: Failed to load module %s.", strModule.Get());
		}

		CLogFile::Print("");
	}



	// Loading resources
	CLogFile::Print("");
	CLogFile::Print("========================= Loading Resources ========================");
	
	auto scripts = CVAR_GET_LIST("script");

	int iResourcesLoaded = 0;
	int iFailedResources = 0;

	for(auto strScript : scripts)
	{
		CString strPath(SharedUtility::GetAbsolutePath("scripts/%s", strScript.Get()));

		//if(!g_pScriptingManager->Load(strScript, strPath))
		{
			CLogFile::Printf("Warning: Failed to load script %s.", strScript.Get());
			iFailedResources++;
		}
		/*else
			iResourcesLoaded++;*/
	}

	auto clientresources = CVAR_GET_LIST("clientresource");
	for(auto strClientResource : clientresources)
	{	
		//if(!g_pClientResourceFileManager->Start(strClientResource))
		{
			CLogFile::Printf("Warning: Failed to load client resource %s.", strClientResource.Get());
			iFailedResources++;
		}
		/*else
			iResourcesLoaded++;*/
	}

	CLogFile::Printf("Successfully loaded %d resources (%d failed).", iResourcesLoaded, iFailedResources);

	CLogFile::Print("");
	CLogFile::Print("====================================================================");
	CLogFile::Print("");
	return true;
}

void CServer::Process()
{
	m_pNetServer->Process();
}

void CServer::Shutdown()
{
	m_pNetServer->EnsureStopped(10);
}