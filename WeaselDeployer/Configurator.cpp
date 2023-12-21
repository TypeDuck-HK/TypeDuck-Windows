#include "stdafx.h"
#include "WeaselDeployer.h"
#include "Configurator.h"
#include "SwitcherSettingsDialog.h"
#include "UIStyleSettings.h"
#include "UIStyleSettingsDialog.h"
#include "DictManagementDialog.h"
#include "TypeDuckSettingsDialog.h"
#include <WeaselCommon.h>
#include <WeaselIPC.h>
#include <WeaselUtility.h>
#include <WeaselVersion.h>
#pragma warning(disable: 4005)
#include <rime_api.h>
#include <rime_levers_api.h>
#pragma warning(default: 4005)

Configurator::Configurator()
{
}

void Configurator::Initialize()
{
	RIME_STRUCT(RimeTraits, weasel_traits);
	weasel_traits.shared_data_dir = weasel_shared_data_dir();
	weasel_traits.user_data_dir = weasel_user_data_dir();
	const int len = 20;
	char utf8_str[len];
	memset(utf8_str, 0, sizeof(utf8_str));
	WideCharToMultiByte(CP_UTF8, 0, WEASEL_IME_NAME, -1, utf8_str, len - 1, NULL, NULL);
	weasel_traits.distribution_name = utf8_str;
	weasel_traits.distribution_code_name = WEASEL_CODE_NAME;
	weasel_traits.distribution_version = WEASEL_VERSION;
	weasel_traits.app_name = "rime.TypeDuck";
	RimeSetup(&weasel_traits);
	
	LOG(INFO) << "WeaselDeployer reporting.";
	RimeDeployerInitialize(NULL);
}

static bool configure_switcher(RimeLeversApi* api, RimeSwitcherSettings* switchcer_settings, bool* reconfigured)
{
	RimeCustomSettings* settings = (RimeCustomSettings*)switchcer_settings;
    if (!api->load_settings(settings))
        return false;
	SwitcherSettingsDialog dialog(switchcer_settings);
	if (dialog.DoModal() == IDOK) {
		if (api->save_settings(settings))
			*reconfigured = true;
		return true;
	}
	return false;
}

static bool configure_ui(RimeLeversApi* api, UIStyleSettings* ui_style_settings, bool* reconfigured) {
	RimeCustomSettings* settings = ui_style_settings->settings();
    api->load_settings(settings);
	UIStyleSettingsDialog dialog(ui_style_settings);
	if (dialog.DoModal() == IDOK) {
		if (api->save_settings(settings))
			*reconfigured = true;
		return true;
	}
	return false;
}

static bool configure_typeduck(RimeLeversApi* api, TypeDuckSettings* typeduck_settings, bool* reconfigured) {
	RimeCustomSettings* settings = typeduck_settings->settings();
	RimeCustomSettings* custom_settings = typeduck_settings->custom_settings();
	if (!api->load_settings(settings) || !api->load_settings(custom_settings))
		return false;
	{ // This has to be scoped. See https://www.codeproject.com/Articles/4028/WTL-for-MFC-Programmers-Part-IV-Dialogs-and-Contro#:~:text=The%20block%20around,apps%20did%20crash.%29
		TypeDuckSettingsDialog dialog(typeduck_settings);
		dialog.DoModal();
	}
	*reconfigured = api->save_settings(settings);
	*reconfigured = api->save_settings(custom_settings);
	return true;
}

int Configurator::Run(bool installing)
{
	RimeModule* levers = rime_get_api()->find_module("levers");
	if (!levers) return 1;
	RimeLeversApi* api = (RimeLeversApi*)levers->get_api();
	if (!api) return 1;

	bool reconfigured = false;
	TypeDuckSettings typeduck_settings(api);
	if (!installing || api->is_first_run(typeduck_settings.settings()) || api->is_first_run(typeduck_settings.custom_settings()))
		configure_typeduck(api, &typeduck_settings, &reconfigured);
	
	if (installing || reconfigured) {
		return UpdateWorkspace(reconfigured);
	}
	return 0;
}

int Configurator::UpdateWorkspace(bool report_errors) {
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"TypeDuckDeployerMutex");
	if (!hMutex)
	{
		LOG(ERROR) << "Error creating TypeDuckDeployerMutex.";
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LOG(WARNING) << "another deployer process is running; aborting operation.";
		CloseHandle(hMutex);
		if (report_errors)
		{
			MessageBox(NULL, L"Another deployment task is in progress. TypeDuck needs to be restarted for changes to take effect.", L"TypeDuck Deployment Failed", MB_OK | MB_ICONINFORMATION);
		}
		return 1;
	}

	weasel::Client client;
	if (client.Connect())
	{
		LOG(INFO) << "Turning WeaselServer into maintenance mode.";
		client.StartMaintenance();
	}

	{
		RimeApi* rime = rime_get_api();
		// initialize default config, preset schemas
		rime->deploy();
		// initialize weasel config
		rime->deploy_config_file("weasel.yaml", "config_version");
	}

	CloseHandle(hMutex);  // should be closed before resuming service.

	if (client.Connect())
	{
		LOG(INFO) << "Resuming service.";
		client.EndMaintenance();
	}
	return 0;
}

int Configurator::DictManagement() {
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"TypeDuckDeployerMutex");
	if (!hMutex)
	{
		LOG(ERROR) << "Error creating TypeDuckDeployerMutex.";
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LOG(WARNING) << "another deployer process is running; aborting operation.";
		CloseHandle(hMutex);
		MessageBox(NULL, L"Another deployment task is in progress. Please try again later.", L"TypeDuck Deployment Failed", MB_OK | MB_ICONINFORMATION);
		return 1;
	}

	weasel::Client client;
	if (client.Connect())
	{
		LOG(INFO) << "Turning WeaselServer into maintenance mode.";
		client.StartMaintenance();
	}

	{
		RimeApi* rime = rime_get_api();
		if (RIME_API_AVAILABLE(rime, run_task))
		{
			rime->run_task("installation_update");  // setup user data sync dir
		}
		DictManagementDialog dlg;
		dlg.DoModal();
	}

	CloseHandle(hMutex);  // should be closed before resuming service.

	if (client.Connect())
	{
		LOG(INFO) << "Resuming service.";
		client.EndMaintenance();
	}
	return 0;
}

int Configurator::SyncUserData() {
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"TypeDuckDeployerMutex");
	if (!hMutex)
	{
		LOG(ERROR) << "Error creating TypeDuckDeployerMutex.";
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LOG(WARNING) << "another deployer process is running; aborting operation.";
		CloseHandle(hMutex);
		MessageBox(NULL, L"Another deployment task is in progress. Please try again later.", L"TypeDuck Deployment Failed", MB_OK | MB_ICONINFORMATION);
		return 1;
	}

	weasel::Client client;
	if (client.Connect())
	{
		LOG(INFO) << "Turning WeaselServer into maintenance mode.";
		client.StartMaintenance();
	}

	{
		RimeApi* rime = rime_get_api();
		if (!rime->sync_user_data())
		{
			LOG(ERROR) << "Error synching user data.";
			CloseHandle(hMutex);
			return 1;
		}
		rime->join_maintenance_thread();
	}

	CloseHandle(hMutex);  // should be closed before resuming service.

	if (client.Connect())
	{
		LOG(INFO) << "Resuming service.";
		client.EndMaintenance();
	}
	return 0;
}
