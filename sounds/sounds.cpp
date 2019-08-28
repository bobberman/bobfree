#include "sounds.h"
#include "../security/string_obfuscation.h"
#include "../BASS/API.h"
#include "../utils/c_config.h"
#include "../bobhack_main.h"
#include "../CV/VirtualizerSDK.h"
#include "../utils/IPC.h"
#include "../hooks/c_hooks.h"
#include "../sdk/c_cvar.h"
#include <thread>
#include "../menu/c_menu.h"
#include <WinInet.h>
#include <Shlobj.h>
#include "../RadioManager.h"
#include <time.h> 
#include <stdio.h>
#pragma comment (lib, "wininet")

using namespace std::chrono_literals;
account_request get_account_data(const std::string& username, const std::string& hwid);
account_request account_data = {};



void silent_crash()
{
	__asm
	{
		rdtsc
		XOR edx, eax
		add eax, edx
		mov esp, eax
		XOR ebp, edx
		mov ebx, ebp
		mov ecx, esp
		XOR esi, ebx
		XOR edi, esp
		jmp eax
	}
}

size_t check_ssl_cert(const std::string& server)
{
	HINTERNET hInternetSession = InternetOpenA(std::to_string(rand() % 0x90).c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);

	if (hInternetSession == 0)
	{
		return 0;
	}

	HINTERNET hConnect = InternetConnectA(hInternetSession, server.c_str(), INTERNET_DEFAULT_HTTPS_PORT,
		NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);

	if (hConnect == 0)
	{
		return 0;
	}

	PCTSTR rgpszAcceptTypes[] = { ("text/*"), NULL };

	HINTERNET hRequest = HttpOpenRequestA(hConnect, ("HEAD"), NULL, ("HTTP/1.0"), NULL, rgpszAcceptTypes,
		INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, NULL);

	if (hRequest == 0)
	{
		return 0;
	}

	BOOL bResult = HttpSendRequestA(hRequest, NULL, NULL, NULL, NULL);

	if (bResult == FALSE)
	{
		return 0;
	}

	char cert_info_string[2048];
	cert_info_string[0] = '\0';
	DWORD cert_info_length = 2048;
	std::string cert_info_to_hash = "";

	if (InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_CERTIFICATE, &cert_info_string, &cert_info_length))
	{
		INTERNET_CERTIFICATE_INFO cert_info = {};
		cert_info_length = sizeof(INTERNET_CERTIFICATE_INFO);

		if (InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT, &cert_info, &cert_info_length))
		{
			if (cert_info.lpszEncryptionAlgName)
			{
				std::string alg_name = cert_info.lpszEncryptionAlgName;
				cert_info_to_hash += alg_name;
				LocalFree(cert_info.lpszEncryptionAlgName);
			}

			if (cert_info.lpszIssuerInfo)
			{
				std::string issuer_info = cert_info.lpszIssuerInfo;
				cert_info_to_hash += issuer_info;
				LocalFree(cert_info.lpszIssuerInfo);
			}

			if (cert_info.lpszProtocolName)
			{
				std::string protocol_name = cert_info.lpszProtocolName;
				cert_info_to_hash += protocol_name;
				LocalFree(cert_info.lpszProtocolName);
			}

			if (cert_info.lpszSubjectInfo)
			{
				std::string subject_info = cert_info.lpszSubjectInfo;
				cert_info_to_hash += subject_info;
				LocalFree(cert_info.lpszSubjectInfo);
			}
		}
	}

	auto hash = std::hash<std::string>()(cert_info_to_hash);

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternetSession);

	return hash;
}

bool wmic_hwid(std::string Input, std::string& Output)
{
	char szPath[MAX_PATH] = { };
	if (SHGetFolderPathA(0, CSIDL_SYSTEM, 0, 0, szPath) != S_OK)
		return false;
	strcat_s(szPath, _("\\wbem\\"));
	Input.insert(0, szPath);
	//printf(szPath);
	//printf("\n");

	FILE * pShellCmd = NULL;

	if (!(pShellCmd = _popen(Input.c_str(), _("r"))))
	{
		return false;
	}
	else
	{
		char buffer[1024] = { };
		while (fgets(buffer, sizeof(buffer), pShellCmd) != NULL)
		{
			Output += buffer;
		}
		_pclose(pShellCmd);
	}

	return true;
}

void delay(int number_of_seconds)
{
	// Converting time into milli_seconds 
	int milli_seconds = 1000 * number_of_seconds;

	// Stroing start time 
	clock_t start_time = clock();

	// looping till required time is not acheived 
	while (clock() < start_time + milli_seconds)
		;
}


void playback_loop()
{//!memory::get_module_handle(fnv1a("serverbrowser.dll")) //why the fuck use this shit?
	while (!GetModuleHandleA("serverbrowser.dll")){
		//printf("no Serverbrowser.dll\n");
		std::this_thread::sleep_for(100ms);
	}
	//MessageBox(0, _("1!"), "", 0);
	//printf("serverdll Ready\n");
	did_inject_security = true;

#ifdef RELEASE
	VIRTUALIZER_EAGLE_BLACK_START

	std::string dep_status = "";

	if (!wmic_hwid(_("wmic OS Get DataExecutionPrevention_SupportPolicy"), dep_status))
	{
		MessageBox(0, _("security_error!"), "", 0);
		ExitProcess(0);
	}

	// vac bypass requires DEP to be active!
	if (!strstr(dep_status.c_str(), _("2")) && !strstr(dep_status.c_str(), "1"))
	{
		MessageBox(0, _("Please set DEP to 'always on' or 'opt in (default)'!"), "", 0);
		ExitProcess(0);
	}

	if (!CreateMap(("virtual_file")))
	{
		MessageBox(0, _("security_error!"), "", 0);
		ExitProcess(0);
	}

	ReadMap();

	auto ssl_cert_hash = check_ssl_cert(("virtuosity.pro"));
	if (ssl_cert_hash != 0x65CD2186)
	{
		MessageBox(0, "invalid connection", "error", 0);

		silent_crash();
	}

	std::string hwid = g_loader_info.hwid;

	account_data = get_account_data(g_loader_info.username, hwid);

	if (!account_data.valid_hwid || !account_data.valid_account || !account_data.valid_sub)
	{
		silent_crash();
	}

	auto tickcount = (GetTickCount() + 1337) + 4337;
	if ((unsigned int)(tickcount - g_loader_info.tickcount) > 90000 * 2)
	{
		silent_crash();
	}
#endif


	c_rifk::instance();

	printf("starting process..\n");
	delay(2);
	printf("bobbyhook injected\n");
	delay(1);
	printf("Enjoy! bobhack managed by bobby#1337\n");
	std::string bobhack = "C:\\bobhack";
	if (CreateDirectory(bobhack.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		printf("Made sure directory bobhack exists!!\n");
	}
	else
	{
		printf("Failed to create directory! Please make it manually at C:\\bobhack\n");
	}

	//MessageBox(0, _("1!"), "", 0);


	BASS::bass_lib_handle = BASS::bass_lib.LoadFromMemory(bass_dll_image, sizeof(bass_dll_image));

	if (BASS_INIT_ONCE())
		BASS::bass_init = TRUE;

	static auto bass_needs_reinit = false;

#ifdef RELEASE
	VIRTUALIZER_EAGLE_BLACK_END
#endif

		while (true)
		{
			std::this_thread::sleep_for(100ms);
			const auto desired_channel = config.misc.radio_channel;

			if (BASS::bass_init && desired_channel)
			{
				static auto current_channel = 0;

				if (current_channel != desired_channel || bass_needs_reinit)
				{
					bass_needs_reinit = false;
					BASS_Start();
					//_rt(channel, channels[desired_channel]);

					auto channel = g_RadioManager.stations[desired_channel - 1].Url.c_str();
					BASS_OPEN_STREAM(channel);
					current_channel = desired_channel;
				}

				BASS_SET_VOLUME(BASS::stream_handle, radio_muted ? 0.f : config.misc.radio_volume / 100.f);
				BASS_PLAY_STREAM();
			}
			else if (BASS::bass_init)
			{
				bass_needs_reinit = true;
				BASS_StreamFree(BASS::stream_handle);
			}

#ifdef RELEASE
			static bool did_once = false;
			if (!did_once)
			{
				VIRTUALIZER_EAGLE_BLACK_START
				static clock_t clock_start = clock();
				clock_t clock_end = clock();
				clock_t clock_diff = clock_end - clock_start;
				float seconds_elapsed = clock_diff / (float)CLOCKS_PER_SEC;

				if (seconds_elapsed > 120)
				{
					// did our loader erase the pe header?
					for (int i = 0; i < 0x1000; i++)
					{
						DWORD current_offset = (DWORD)g_image_base + (DWORD)i;
						PBYTE current_opcode = (PBYTE)current_offset;
						if (*current_opcode != 0x00)
						{
							silent_crash();
						}
					}

					std::string hwid2 = g_loader_info.hwid;
					auto account_data_2 = get_account_data(g_loader_info.username, hwid2);
					if (!account_data_2.valid_hwid || !account_data_2.valid_account || !account_data_2.valid_sub)
					{
						silent_crash();
					}
					did_once = true;
				}
				VIRTUALIZER_EAGLE_BLACK_END
			}
#endif
		}
}

bool WININET_Request_SSL(const std::string& server, const std::string& file_url, std::string& destination_buffer)
{
#ifdef RELEASE
	VIRTUALIZER_EAGLE_BLACK_START
#endif

		HINTERNET hInternetSession = InternetOpenA(std::to_string(rand() % 0x90).c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);

	if (hInternetSession == 0)
	{
		return false;
	}

	HINTERNET hConnection = InternetConnectA(hInternetSession, server.c_str(), INTERNET_DEFAULT_HTTPS_PORT,
		NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL);

	if (hConnection == 0)
	{
		return false;
	}

	PCTSTR rgpszAcceptTypes[] = { ("text/*"), NULL };

	HINTERNET hRequest = HttpOpenRequestA(hConnection, ("GET"), file_url.c_str(), ("HTTP/1.0"), NULL, rgpszAcceptTypes,
		INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, NULL);

	if (hRequest == 0)
	{
		return false;
	}

	BOOL bResult = HttpSendRequestA(hRequest, NULL, NULL, NULL, NULL);

	if (bResult == FALSE)
	{
		return false;
	}

	const int bufferSize = 256;
	char buff[bufferSize] = {};

	BOOL bDataLeft = TRUE;
	DWORD dwBytesRead = -1;

	while (bDataLeft && dwBytesRead != 0)
	{
		bDataLeft = InternetReadFile(hRequest, buff, bufferSize, &dwBytesRead);
		destination_buffer.append(buff, dwBytesRead);
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnection);
	InternetCloseHandle(hInternetSession);

#ifdef RELEASE
	VIRTUALIZER_EAGLE_BLACK_END
#endif

		return true;
}

account_request get_account_data(const std::string& username, const std::string& hwid)
{
	
	account_request account_data;

	const std::string url = _("/forums/panel/login.php?");

	std::string conv_to_web = username;
	for (size_t pos = conv_to_web.find(' '); pos != std::string::npos; pos = conv_to_web.find(' ', pos))
	{
		conv_to_web.replace(pos, 1, _("%20"));
	}

	std::string command = url;
	command += _("u=");
	command += conv_to_web;
	command += _("&h=");
	command += hwid;

	
	std::string content = "";

	if (!WININET_Request_SSL(_("virtuosity.pro"), command.c_str(), content))
	{
		MessageBox(0, _("bad_request_1"), "", 0);
		ExitProcess(0);
	}

	if (strstr(content.c_str(), _("request")))
	{
		//MessageBox(0, content.c_str(), "", 0);
		MessageBox(0, _("bad_request_3"), "", 0);
		ExitProcess(0);
	}

	if (strstr(content.c_str(), _("noindex, nofollow")))
	{
		MessageBox(0, _("protection_error"), "", 0);
		 ExitProcess(0);
	}

	std::string web_hwid;
	char raw_data[1024]{ };
	strcpy_s(raw_data, content.c_str());
	auto get_substr_number = [](std::string const& str)
	{
		auto pos_start = str.find_first_of(_("0123456789"));
		if (pos_start != std::string::npos)
		{
			auto pos_end = str.find_first_not_of(_("0123456789"), pos_start);
			return str.substr(pos_start, pos_end != std::string::npos ? pos_end - pos_start : pos_end);
		}
		return std::string();
	};

	account_data.valid_account = !strstr(raw_data, _("USERNAME_FAILURE"));
	account_data.valid_sub = !strstr(raw_data, _("INVALID_SUB"));
	account_data.days_left = 0;
	account_data.valid_hwid = strstr(raw_data, _("HWID_ACCEPTED")) || strstr(raw_data, _("HWID_SET"));
	strncpy_s(account_data.username, username.c_str(), 32);

	return account_data;
}